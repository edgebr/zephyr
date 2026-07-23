/*
 * Copyright (c) 2026 Centro de Inovação EDGE
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief RP2350 POWMAN low-power wakeup helpers.
 *
 * Encapsulates the POWMAN hardware needed to enter suspend-to-RAM (P1.x) with a
 * periodic alarm and/or GPIO wakeup, and to query why the switched core last
 * powered up. Applications arm the wakeup sources through this API and then
 * enter S2RAM via the Zephyr PM framework (e.g. pm_state_force()); no direct
 * POWMAN register access is needed at the application layer.
 *
 * The RP2350 low-power oscillator (LPOSC) that clocks the always-on POWMAN
 * timer is imprecise (nominal ~32 kHz but well off in practice). This module
 * measures its real frequency once and configures the timer divider
 * accordingly, so alarm intervals are accurate.
 */

#ifndef SOC_RASPBERRYPI_RPI_PICO_RP2350_POWMAN_H_
#define SOC_RASPBERRYPI_RPI_PICO_RP2350_POWMAN_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Source that last powered the switched core back up.
 */
enum rp2350_pm_wakeup_source {
	RP2350_PM_WAKEUP_NONE = 0, /**< No parking/S2RAM wakeup source identified */
	RP2350_PM_WAKEUP_ALARM,    /**< POWMAN timer alarm */
	RP2350_PM_WAKEUP_GPIO,     /**< POWMAN GPIO wakeup (PWRUP0) */
};

/**
 * @brief Ensure the POWMAN timer runs on the calibrated LPOSC 1 kHz tick source.
 *
 * Measures the real LPOSC frequency once (cached) and selects it as the POWMAN
 * timer tick source, starting the timer if needed. Idempotent: subsequent calls
 * do not recalibrate nor reset the timer count. Called automatically by
 * rp2350_powman_arm_alarm().
 */
void rp2350_powman_timer_init(void);

/**
 * @brief Arm a POWMAN alarm wakeup @p seconds from now.
 *
 * @param seconds Delay until wakeup, in seconds.
 */
void rp2350_powman_arm_alarm(uint32_t seconds);

/**
 * @brief Arm a POWMAN GPIO (edge-detected) wakeup.
 *
 * @param gpio         Absolute GPIO number (bank 0).
 * @param wake_on_high true to wake on a rising edge, false on a falling edge.
 *
 * @retval true  Wakeup armed.
 * @retval false @p gpio is out of range for this chip; nothing armed.
 */
bool rp2350_powman_arm_gpio_wakeup(uint32_t gpio, bool wake_on_high);

/**
 * @brief Disarm all POWMAN wakeup sources (alarm and GPIOs).
 */
void rp2350_powman_disarm_wakeups(void);

/**
 * @brief Decode the source of the last switched-core power-up.
 *
 * Reads POWMAN last_swcore_pwrup. Valid after resuming from S2RAM.
 *
 * @return The wakeup source.
 */
enum rp2350_pm_wakeup_source rp2350_powman_wakeup_source(void);

/**
 * @brief Whether this boot was preceded by a switched-core power-down.
 *
 * @return true if the POWMAN HAD_SWCORE_PD reset flag is set.
 */
bool rp2350_powman_had_powerdown(void);

#ifdef __cplusplus
}
#endif

#endif /* SOC_RASPBERRYPI_RPI_PICO_RP2350_POWMAN_H_ */
