/*
 * Copyright (c) 2026 Centro de Inovação EDGE
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file power.c
 * @brief RP2350 SoC Power Management hooks.
 *
 * Light-sleep (RUNTIME_IDLE / SUSPEND_TO_IDLE) is handled by a plain WFI
 * with clock-gating; IRQs are re-enabled immediately on wakeup.
 *
 * Deep sleep (P1.0 / P1.7 via POWMAN) is NOT driven by the Zephyr PM
 * framework.  The parking service calls the POWMAN API directly, saves
 * application state to flash or POWMAN scratch registers, and lets the
 * bootrom cold-boot the device on wakeup.  pm_state_set is therefore
 * not involved in deep sleep.
 */

#include <zephyr/pm/pm.h>
#include <zephyr/logging/log.h>

#include <cmsis_core.h>

LOG_MODULE_DECLARE(soc, CONFIG_SOC_LOG_LEVEL);

void pm_state_set(enum pm_state state, uint8_t substate_id)
{
	ARG_UNUSED(substate_id);

	switch (state) {
	case PM_STATE_RUNTIME_IDLE:
	case PM_STATE_SUSPEND_TO_IDLE:
		__disable_irq();
		__WFI();
		break;
	default:
		LOG_DBG("PM state not supported: %u", state);
		break;
	}
}

void pm_state_exit_post_ops(enum pm_state state, uint8_t substate_id)
{
	ARG_UNUSED(state);
	ARG_UNUSED(substate_id);
	__enable_irq();
}
