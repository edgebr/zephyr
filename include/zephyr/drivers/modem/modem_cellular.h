/*
 * Copyright (c) 2026 Zuq Performance
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_MODEM_MODEM_CELLULAR_H_
#define ZEPHYR_INCLUDE_DRIVERS_MODEM_MODEM_CELLULAR_H_

#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the APN username/password used for PDP context authentication at run-time.
 *
 * @details Only used by modems whose AT command set carries authentication alongside the
 * APN (e.g. Quectel's AT+QICSGP). Must be called before cellular_set_apn(), since it is
 * cellular_set_apn() that wakes the state machine out of MODEM_CELLULAR_STATE_WAIT_FOR_APN
 * and triggers the dial-time AT script build.
 *
 * @param dev Cellular device.
 * @param username Zero-terminated username string, or NULL/empty if not required.
 * @param password Zero-terminated password string, or NULL/empty if not required.
 *
 * @return 0 on success, negative errno value on failure.
 * @retval -EINVAL username or password too long.
 * @retval -EBUSY Modem is already dialled, credentials cannot be changed.
 */
int modem_cellular_set_apn_credentials(const struct device *dev, const char *username,
					const char *password);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DRIVERS_MODEM_MODEM_CELLULAR_H_ */
