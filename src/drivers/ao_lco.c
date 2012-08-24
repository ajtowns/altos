/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <ao.h>
#include <ao_lco.h>

static __xdata struct ao_launch_command	command;
static __xdata struct ao_launch_query	query;
static __pdata uint16_t	launch_serial;
static __pdata uint8_t	launch_channel;
static __pdata uint16_t	tick_offset;

static void
launch_args(void) __reentrant
{
	ao_cmd_decimal();
	launch_serial = ao_cmd_lex_i;
	ao_cmd_decimal();
	launch_channel = ao_cmd_lex_i;
}

static int8_t
launch_query(void)
{
	uint8_t	i;
	int8_t	r = AO_RADIO_CMAC_OK;

	tick_offset = ao_time();
	for (i = 0; i < 10; i++) {
		printf ("."); flush();
		command.tick = ao_time();
		command.serial = launch_serial;
		command.cmd = AO_LAUNCH_QUERY;
		command.channel = launch_channel;
		ao_radio_cmac_send(&command, sizeof (command));
		r = ao_radio_cmac_recv(&query, sizeof (query), AO_MS_TO_TICKS(500));
		if (r == AO_RADIO_CMAC_OK)
			break;
	}
	tick_offset -= query.tick;
	printf("\n"); flush();
	return r;
}

static void
launch_report_cmd(void) __reentrant
{
	int8_t		r;

	launch_args();
	if (ao_cmd_status != ao_cmd_success)
		return;
	r = launch_query();
	switch (r) {
	case AO_RADIO_CMAC_OK:
		if (query.valid) {
			switch (query.arm_status) {
			case ao_igniter_ready:
			case ao_igniter_active:
				printf ("Armed: ");
				break;
			default:
				printf("Disarmed: ");
			}
			switch (query.igniter_status) {
			default:
				printf("unknown\n");
				break;
			case ao_igniter_ready:
				printf("igniter good\n");
				break;
			case ao_igniter_open:
				printf("igniter bad\n");
				break;
			}
		} else {
			printf("Invalid channel %d\n", launch_channel);
		}
		printf("Rssi: %d\n", ao_radio_cmac_rssi);
		break;
	default:
		printf("Error %d\n", r);
		break;
	}
}

static void
launch_arm(void) __reentrant
{
	command.tick = ao_time() - tick_offset;
	command.serial = launch_serial;
	command.cmd = AO_LAUNCH_ARM;
	command.channel = launch_channel;
	ao_radio_cmac_send(&command, sizeof (command));
}

static void
launch_ignite(void) __reentrant
{
	command.tick = ao_time() - tick_offset;
	command.serial = launch_serial;
	command.cmd = AO_LAUNCH_FIRE;
	command.channel = 0;
	ao_radio_cmac_send(&command, sizeof (command));
}

static void
launch_fire_cmd(void) __reentrant
{
	static __xdata struct ao_launch_command	command;
	uint8_t		secs;
	uint8_t		i;
	int8_t		r;

	launch_args();
	ao_cmd_decimal();
	secs = ao_cmd_lex_i;
	if (ao_cmd_status != ao_cmd_success)
		return;
	r = launch_query();
	if (r != AO_RADIO_CMAC_OK) {
		printf("query failed %d\n", r);
		return;
	}

	for (i = 0; i < 4; i++) {
		printf("arm %d\n", i); flush();
		launch_arm();
	}

	secs = secs * 10 - 5;
	if (secs > 100)
		secs = 100;
	for (i = 0; i < secs; i++) {
		printf("fire %d\n", i); flush();
		launch_ignite();
		ao_delay(AO_MS_TO_TICKS(100));
	}
}

static void
launch_arm_cmd(void) __reentrant
{
	uint8_t	i;
	int8_t  r;
	launch_args();
	r = launch_query();
	if (r != AO_RADIO_CMAC_OK) {
		printf("query failed %d\n", r);
		return;
	}
	for (i = 0; i < 4; i++)
		launch_arm();
}

static void
launch_ignite_cmd(void) __reentrant
{
	uint8_t i;
	launch_args();
	for (i = 0; i < 4; i++)
		launch_ignite();
}

static __code struct ao_cmds ao_lco_cmds[] = {
	{ radio_cmac_send_cmd,	"s <length>\0Send AES-CMAC packet. Bytes to send follow on next line" },
	{ radio_cmac_recv_cmd,	"S <length> <timeout>\0Receive AES-CMAC packet. Timeout in ms" },
	{ launch_report_cmd,    "l <serial> <channel>\0Get remote launch status" },
	{ launch_fire_cmd,	"f <serial> <channel> <secs>\0Fire remote igniter" },
	{ launch_arm_cmd,	"a <serial> <channel>\0Arm remote igniter" },
	{ launch_ignite_cmd,	"i <serial> <channel>\0Pulse remote igniter" },
	{ 0, NULL },
};

void
ao_lco_init(void)
{
	ao_cmd_register(&ao_lco_cmds[0]);
}
