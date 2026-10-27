/*
 * Copyright (c) 2026 Centro de Inovação EDGE
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Save/restore of the switched-core (SWCORE) hardware state around S2RAM.
 *
 * Suspend-to-RAM powers the whole SWCORE domain off: only SRAM and the XIP cache
 * keep their contents. Every peripheral, the clock tree, the IO/pad configuration
 * and the NVIC come back in their reset state, while the kernel and the drivers
 * resume believing they are still configured. This module snapshots what the
 * running drivers configured before the power-down and writes it back on resume,
 * before interrupts are re-enabled.
 *
 * Only blocks whose registers can be read back are covered here. Hardware with
 * write-only state (PIO instruction memory) or that must re-enumerate (USB) is
 * restored by its own driver or owner.
 */

#ifndef SOC_RASPBERRYPI_RPI_PICO_RP2350_S2RAM_CTX_H_
#define SOC_RASPBERRYPI_RPI_PICO_RP2350_S2RAM_CTX_H_

#include <stdbool.h>

/**
 * @brief Snapshot the SWCORE hardware state. Call with interrupts locked, right
 *        before the power-down.
 */
void rp2350_s2ram_ctx_save(void);

/**
 * @brief Bring the SWCORE hardware back to the saved state. Call with interrupts
 *        locked, only after a real power-down (resume path).
 */
void rp2350_s2ram_ctx_restore(void);

/**
 * @brief Record whether the last S2RAM attempt actually powered SWCORE down.
 */
void rp2350_s2ram_ctx_set_powered_down(bool powered_down);

#endif /* SOC_RASPBERRYPI_RPI_PICO_RP2350_S2RAM_CTX_H_ */
