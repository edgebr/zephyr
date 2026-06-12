/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/toolchain.h>

#ifdef CONFIG_MODEM_CELLULAR
#include <zephyr/modem/chat.h>
#include <zephyr/drivers/modem/modem_cellular.h>

/* Validate the board-init registration macro expands, links, and binds a
 * board-supplied chat script to a modem instance.
 */
MODEM_CHAT_MATCH_DEFINE(board_init_ok_match, "OK", "", NULL);
MODEM_CHAT_SCRIPT_CMDS_DEFINE(board_init_script_cmds,
			      MODEM_CHAT_SCRIPT_CMD_RESP("AT", board_init_ok_match));
MODEM_CHAT_SCRIPT_NO_ABORT_DEFINE(board_init_script, board_init_script_cmds, NULL, 4);
MODEM_CELLULAR_BOARD_INIT_DEFINE(DT_NODELABEL(test_quectel_eg25_g), &board_init_script);
#endif /* CONFIG_MODEM_CELLULAR */

int main(void)
{
	return 0;
}

#ifdef CONFIG_CONNECTIVITY_WIFI_MGMT_APPLICATION

#include <zephyr/net/conn_mgr_connectivity_impl.h>
#include <zephyr/net/conn_mgr/connectivity_wifi_mgmt.h>

/* Bind L2 connectity APIs. */
static struct conn_mgr_conn_api conn_api = { 0 };

CONN_MGR_CONN_DEFINE(CONNECTIVITY_WIFI_MGMT, &conn_api);

#endif /* CONFIG_CONNECTIVITY_WIFI_MGMT_APPLICATION */
