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
#include <ao_pad.h>
#include <ao_lco_cmd.h>
#include <ao_radio_cmac.h>

static __xdata struct ao_pad_command	command;
static __xdata struct ao_pad_query	query;
static __pdata uint16_t	launch_box;
static __pdata uint8_t	launch_channels;
static __pdata uint16_t	tick_offset;

static void
launch_args(void) __reentrant
{
	ao_cmd_decimal();
	launch_box = ao_cmd_lex_i;
	ao_cmd_hex();
	launch_channels = ao_cmd_lex_i;
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
		command.box = launch_box;
		command.cmd = AO_LAUNCH_QUERY;
		command.channels = launch_channels;
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
	uint8_t		c;

	launch_args();
	if (ao_cmd_status != ao_cmd_success)
		return;
	r = launch_query();
	switch (r) {
	case AO_RADIO_CMAC_OK:
		switch (query.arm_status) {
		case ao_igniter_ready:
		case ao_igniter_active:
			printf ("Armed: ");
			break;
		default:
			printf("Disarmed: ");
		}
		for (c = 0; c < AO_PAD_MAX_CHANNELS; c++) {
			if (query.channels & (1 << c)) {
				printf (" pad %d ", c);
				switch (query.igniter_status[c]) {
				default:
					printf("unknown, ");
					break;
				case AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_OPEN:
					printf("bad-open, ");
					break;
				case AO_PAD_IGNITER_STATUS_GOOD_IGNITER_RELAY_OPEN:
					printf("good-igniter, ");
					break;
				case AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_CLOSED:
					printf("bad-closed, ");
					break;
				}
			}
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
	command.box = launch_box;
	command.cmd = AO_LAUNCH_ARM;
	command.channels = launch_channels;
	ao_radio_cmac_send(&command, sizeof (command));
}

static void
launch_ignite(void) __reentrant
{
	command.tick = ao_time() - tick_offset;
	command.box = launch_box;
	command.cmd = AO_LAUNCH_FIRE;
	command.channels = 0;
	ao_radio_cmac_send(&command, sizeof (command));
}

static void
launch_fire_cmd(void) __reentrant
{
	static __xdata struct ao_pad_command	command;
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
	{ launch_report_cmd,    "l <box> <channel>\0Get remote launch status" },
	{ launch_fire_cmd,	"F <box> <channel> <secs>\0Fire remote igniter" },
	{ launch_arm_cmd,	"a <box> <channel>\0Arm remote igniter" },
	{ launch_ignite_cmd,	"i <box> <channel>\0Pulse remote igniter" },
	{ 0, NULL },
};

void
ao_lco_cmd_init(void)
{
	ao_cmd_register(&ao_lco_cmds[0]);
}
