/*
 * Copyright (c) 2026 Centro de Inovação EDGE
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rp2350_s2ram_ctx.h"
#include "rp2350_powman.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys_clock.h>
#include <zephyr/sys/util.h>

#include <cmsis_core.h>

#include <hardware/powman.h>
#include <hardware/resets.h>
#include <hardware/structs/adc.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/structs/pads_bank0.h>
#include <hardware/structs/sio.h>
#include <hardware/structs/psm.h>
#include <hardware/structs/uart.h>
#include <hardware/structs/watchdog.h>

#define S2RAM_NVIC_WORDS    DIV_ROUND_UP(CONFIG_NUM_IRQS, 32)
#define S2RAM_IO_INTE_WORDS (sizeof(((io_bank0_irq_ctrl_hw_t *)0)->inte) / sizeof(uint32_t))

#define S2RAM_SHCSR_ENA_MASK                                                                       \
	(SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_USGFAULTENA_Msk |       \
	 SCB_SHCSR_SECUREFAULTENA_Msk)

struct s2ram_uart_ctx {
	uart_hw_t *hw;
	uint32_t reset_bit;
	bool saved;
	uint32_t ibrd;
	uint32_t fbrd;
	uint32_t lcr_h;
	uint32_t cr;
	uint32_t ifls;
	uint32_t imsc;
	uint32_t dmacr;
};

/* Disabled instances get a NULL base and are skipped. */
#define S2RAM_UART_HW(label)                                                                       \
	COND_CODE_1(DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(label)),                                  \
		    ((uart_hw_t *)DT_REG_ADDR(DT_NODELABEL(label))), (NULL))

static struct s2ram_uart_ctx uart_ctx[] = {
	{.hw = S2RAM_UART_HW(uart0), .reset_bit = RESETS_RESET_UART0_BITS},
	{.hw = S2RAM_UART_HW(uart1), .reset_bit = RESETS_RESET_UART1_BITS},
};

static struct {
	uint32_t nvic_iser[S2RAM_NVIC_WORDS];
	uint8_t nvic_ipr[CONFIG_NUM_IRQS];
	uint32_t scb_ccr;
	uint32_t scb_shcsr;
#if defined(CONFIG_FPU)
	uint32_t fpu_fpccr;
	uint32_t fpu_fpdscr;
#endif
	uint32_t sio_gpio_out;
	uint32_t sio_gpio_oe;
	uint32_t sio_gpio_hi_out;
	uint32_t sio_gpio_hi_oe;
	uint32_t io_ctrl[NUM_BANK0_GPIOS];
	uint32_t io_inte[S2RAM_IO_INTE_WORDS];
	uint32_t pads_voltage_select;
	uint32_t pads_io[NUM_BANK0_GPIOS];
	bool wdt_enabled;
	uint32_t wdt_ctrl;
	uint32_t wdt_load;
	uint32_t psm_wdsel;
#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(adc))
	bool adc_saved;
	uint32_t adc_cs;
	uint32_t adc_div;
	uint32_t adc_fcs;
	uint32_t adc_inte;
#endif
} ctx;

static bool powered_down;

static bool sleep_start_valid;
static uint64_t sleep_start_ms;

static bool block_out_of_reset(uint32_t reset_bit)
{
	return (resets_hw->reset & reset_bit) == 0U;
}

void rp2350_s2ram_ctx_set_powered_down(bool value)
{
	powered_down = value;
}

bool rp2350_powman_s2ram_powered_down(void)
{
	return powered_down;
}

static void core_save(void)
{
	for (size_t i = 0; i < S2RAM_NVIC_WORDS; i++) {
		ctx.nvic_iser[i] = NVIC->ISER[i];
	}

	for (size_t i = 0; i < CONFIG_NUM_IRQS; i++) {
		ctx.nvic_ipr[i] = NVIC->IPR[i];
	}

	ctx.scb_ccr = SCB->CCR;
	ctx.scb_shcsr = SCB->SHCSR & S2RAM_SHCSR_ENA_MASK;
#if defined(CONFIG_FPU)
	ctx.fpu_fpccr = FPU->FPCCR;
	ctx.fpu_fpdscr = FPU->FPDSCR;
#endif
}

static void core_restore_scb(void)
{
	SCB->CCR = ctx.scb_ccr;
	SCB->SHCSR = (SCB->SHCSR & ~S2RAM_SHCSR_ENA_MASK) | ctx.scb_shcsr;
#if defined(CONFIG_FPU)
	FPU->FPCCR = ctx.fpu_fpccr;
	FPU->FPDSCR = ctx.fpu_fpdscr;
#endif
	__DSB();
	__ISB();
}

static void core_restore_nvic(void)
{
	for (size_t i = 0; i < CONFIG_NUM_IRQS; i++) {
		NVIC->IPR[i] = ctx.nvic_ipr[i];
	}

	/* Drop whatever latched while the peripherals were being restored. */
	for (size_t i = 0; i < S2RAM_NVIC_WORDS; i++) {
		NVIC->ICPR[i] = UINT32_MAX;
	}

	for (size_t i = 0; i < S2RAM_NVIC_WORDS; i++) {
		NVIC->ISER[i] = ctx.nvic_iser[i];
	}

	__DSB();
	__ISB();
}

static void gpio_save(void)
{
	ctx.sio_gpio_out = sio_hw->gpio_out;
	ctx.sio_gpio_oe = sio_hw->gpio_oe;
	ctx.sio_gpio_hi_out = sio_hw->gpio_hi_out;
	ctx.sio_gpio_hi_oe = sio_hw->gpio_hi_oe;

	for (size_t i = 0; i < NUM_BANK0_GPIOS; i++) {
		ctx.io_ctrl[i] = io_bank0_hw->io[i].ctrl;
		ctx.pads_io[i] = pads_bank0_hw->io[i];
	}

	for (size_t i = 0; i < ARRAY_SIZE(ctx.io_inte); i++) {
		ctx.io_inte[i] = io_bank0_hw->proc0_irq_ctrl.inte[i];
	}

	ctx.pads_voltage_select = pads_bank0_hw->voltage_select;
}

static void gpio_restore(void)
{
	/*
	 * Drive the outputs before muxing the pins back to SIO, and only then
	 * release the pad isolation (the saved pad values have ISO cleared), so the
	 * pins go straight from their latched sleep level to the saved level.
	 */
	sio_hw->gpio_out = ctx.sio_gpio_out;
	sio_hw->gpio_oe = ctx.sio_gpio_oe;
	sio_hw->gpio_hi_out = ctx.sio_gpio_hi_out;
	sio_hw->gpio_hi_oe = ctx.sio_gpio_hi_oe;

	for (size_t i = 0; i < NUM_BANK0_GPIOS; i++) {
		io_bank0_hw->io[i].ctrl = ctx.io_ctrl[i];
	}

	/* Clear edges latched during the restore before re-enabling them. */
	for (size_t i = 0; i < ARRAY_SIZE(ctx.io_inte); i++) {
		io_bank0_hw->intr[i] = UINT32_MAX;
		io_bank0_hw->proc0_irq_ctrl.inte[i] = ctx.io_inte[i];
	}

	pads_bank0_hw->voltage_select = ctx.pads_voltage_select;

	for (size_t i = 0; i < NUM_BANK0_GPIOS; i++) {
		pads_bank0_hw->io[i] = ctx.pads_io[i];
	}
}

static void uart_save(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(uart_ctx); i++) {
		struct s2ram_uart_ctx *u = &uart_ctx[i];

		u->saved = (u->hw != NULL) && block_out_of_reset(u->reset_bit);
		if (!u->saved) {
			continue;
		}

		u->ibrd = u->hw->ibrd;
		u->fbrd = u->hw->fbrd;
		u->lcr_h = u->hw->lcr_h;
		u->cr = u->hw->cr;
		u->ifls = u->hw->ifls;
		u->imsc = u->hw->imsc;
		u->dmacr = u->hw->dmacr;
	}
}

static void uart_restore(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(uart_ctx); i++) {
		struct s2ram_uart_ctx *u = &uart_ctx[i];

		if (!u->saved) {
			continue;
		}

		/* PL011: the baud divisors only latch on a LCR_H write; enable last. */
		u->hw->cr = 0U;
		u->hw->ibrd = u->ibrd;
		u->hw->fbrd = u->fbrd;
		u->hw->lcr_h = u->lcr_h;
		u->hw->ifls = u->ifls;
		u->hw->dmacr = u->dmacr;
		u->hw->imsc = u->imsc;
		u->hw->cr = u->cr;
	}
}

/*
 * Shortest load the watchdog is re-armed with: the time left at power-down can
 * be close to zero, and the feeder thread only runs once the kernel is back.
 */
#define S2RAM_WDT_MIN_LOAD_US (5U * USEC_PER_SEC)

static void watchdog_save(void)
{
	ctx.wdt_enabled = (watchdog_hw->ctrl & WATCHDOG_CTRL_ENABLE_BITS) != 0U;
	if (!ctx.wdt_enabled) {
		return;
	}

	/* LOAD is write-only: re-arm with the time that was left, but not less than the minimum. */
	ctx.wdt_ctrl = watchdog_hw->ctrl;
	ctx.wdt_load = MAX(watchdog_hw->ctrl & WATCHDOG_CTRL_TIME_BITS, S2RAM_WDT_MIN_LOAD_US);
	ctx.psm_wdsel = psm_hw->wdsel;
}

/*
 * The watchdog sits in SWCORE: it stopped during the power-down and comes back
 * disabled. Re-arm it right away, so a hang anywhere later in the resume path
 * still reboots the device instead of leaving it dead until a power-on reset.
 * Its tick generator was already restarted with the clocks.
 */
static void watchdog_restore(void)
{
	if (!ctx.wdt_enabled) {
		return;
	}

	psm_hw->wdsel = ctx.psm_wdsel;
	watchdog_hw->load = ctx.wdt_load;
	watchdog_hw->ctrl = ctx.wdt_ctrl & ~(WATCHDOG_CTRL_TRIGGER_BITS | WATCHDOG_CTRL_TIME_BITS);
}

static void adc_save(void)
{
#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(adc))
	ctx.adc_saved = block_out_of_reset(RESETS_RESET_ADC_BITS);
	if (!ctx.adc_saved) {
		return;
	}

	ctx.adc_cs = adc_hw->cs & ~ADC_CS_START_ONCE_BITS;
	ctx.adc_div = adc_hw->div;
	ctx.adc_fcs = adc_hw->fcs;
	ctx.adc_inte = adc_hw->inte;
#endif
}

static void adc_restore(void)
{
#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(adc))
	if (!ctx.adc_saved) {
		return;
	}

	adc_hw->div = ctx.adc_div;
	adc_hw->fcs = ctx.adc_fcs;
	adc_hw->inte = ctx.adc_inte;
	adc_hw->cs = ctx.adc_cs;
#endif
}

uint64_t rp2350_powman_s2ram_slept_ms(void)
{
	if (!sleep_start_valid) {
		return 0U;
	}

	return powman_timer_get_ms() - sleep_start_ms;
}

void rp2350_s2ram_ctx_save(void)
{
	sleep_start_valid = powman_timer_is_running();
	if (sleep_start_valid) {
		sleep_start_ms = powman_timer_get_ms();
	}

	core_save();
	watchdog_save();
	gpio_save();
	uart_save();
	adc_save();
}

void rp2350_s2ram_ctx_restore(void)
{
	const struct device *clocks = DEVICE_DT_GET(DT_NODELABEL(clocks));

	core_restore_scb();

	/*
	 * Re-run the clock controller init: it brings XOSC, both PLLs, every
	 * clk_* (sys, peri, usb, adc, ...) and the tick generators (TIMER0/1,
	 * watchdog) back exactly as at boot, and takes all peripheral blocks out
	 * of reset. It only touches registers, so it is safe with IRQs locked.
	 */
	(void)clocks->ops.init(clocks);

	watchdog_restore();
	gpio_restore();
	uart_restore();
	adc_restore();

	core_restore_nvic();
}
