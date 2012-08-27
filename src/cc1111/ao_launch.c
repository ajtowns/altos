/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
#include <ao_radio_cmac.h>

__xdata uint16_t ao_launch_ignite;

#if 0
#define PRINTD(...) printf(__VA_ARGS__)
#else
#define PRINTD(...) 
#endif

static void
ao_launch_run(void)
{
	for (;;) {
		while (!ao_launch_ignite)
			ao_sleep(&ao_launch_ignite);
		ao_ignition[ao_igniter_drogue].firing = 1;
		ao_ignition[ao_igniter_main].firing = 1;
		AO_IGNITER_DIR |= AO_IGNITER_DROGUE_BIT | AO_IGNITER_MAIN_BIT;
		AO_IGNITER_DROGUE = 1;
		while (ao_launch_ignite) {
			ao_launch_ignite = 0;
			ao_delay(AO_MS_TO_TICKS(500));
		}
		AO_IGNITER_DROGUE = 0;
		ao_ignition[ao_igniter_drogue].firing = 0;
		ao_ignition[ao_igniter_main].firing = 0;
	}
}

static void
ao_launch_status(void)
{
	uint8_t	i;
	for (;;) {
		ao_delay(AO_SEC_TO_TICKS(1));
		if (ao_igniter_status(ao_igniter_drogue) == ao_igniter_ready) {
			if (ao_igniter_status(ao_igniter_main) == ao_igniter_ready) {
				for (i = 0; i < 5; i++) {
					ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(50));
					ao_delay(AO_MS_TO_TICKS(100));
				}
			} else {
				ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
			}
		}
	}
}

static __pdata uint8_t	ao_launch_armed;
static __pdata uint16_t	ao_launch_arm_time;

static void
ao_launch(void)
{
	static __xdata struct ao_launch_command	command;
	static __xdata struct ao_launch_query	query;
	int16_t	time_difference;

	ao_led_off(AO_LED_RED);
	ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
	for (;;) {
		flush();
		if (ao_radio_cmac_recv(&command, sizeof (command), 0) != AO_RADIO_CMAC_OK)
			continue;
		
		PRINTD ("tick %d serial %d cmd %d channel %d\n",
			command.tick, command.serial, command.cmd, command.channel);

		switch (command.cmd) {
		case AO_LAUNCH_QUERY:
			if (command.serial != ao_serial_number) {
				PRINTD ("serial number mismatch\n");
				break;
			}

			if (command.channel == 0) {
				query.valid = 1;
				query.arm_status = ao_igniter_status(ao_igniter_drogue);
				query.igniter_status = ao_igniter_status(ao_igniter_main);
			} else {
				query.valid = 0;
			}
			query.tick = ao_time();
			query.serial = ao_serial_number;
			query.channel = command.channel;
			PRINTD ("query tick %d serial %d channel %d valid %d arm %d igniter %d\n",
				query.tick, query.serial, query.channel, query.valid, query.arm_status,
				query.igniter_status);
			ao_radio_cmac_send(&query, sizeof (query));
			break;
		case AO_LAUNCH_ARM:
			if (command.serial != ao_serial_number) {
				PRINTD ("serial number mismatch\n");
				break;
			}

			if (command.channel != 0)
				break;
			time_difference = command.tick - ao_time();
			PRINTD ("arm tick %d local tick %d\n", command.tick, ao_time());
			if (time_difference < 0)
				time_difference = -time_difference;
			if (time_difference > 10) {
				PRINTD ("time difference too large %d\n", time_difference);
				break;
			}
			PRINTD ("armed\n");
			ao_launch_armed = 1;
			ao_launch_arm_time = ao_time();
			break;
		case AO_LAUNCH_FIRE:
			if (!ao_launch_armed) {
				PRINTD ("not armed\n");
				break;
			}
			if ((uint16_t) (ao_time() - ao_launch_arm_time) > AO_SEC_TO_TICKS(20)) {
				PRINTD ("late launch arm_time %d time %d\n",
					ao_launch_arm_time, ao_time());
				break;
			}
			time_difference = command.tick - ao_time();
			if (time_difference < 0)
				time_difference = -time_difference;
			if (time_difference > 10) {
				PRINTD ("time different too large %d\n", time_difference);
				break;
			}
			PRINTD ("ignite\n");
			ao_launch_ignite = 1;
			ao_wakeup(&ao_launch_ignite);
			break;
		}
	}
}

void
ao_launch_test(void)
{
	switch (ao_igniter_status(ao_igniter_drogue)) {
	case ao_igniter_ready:
	case ao_igniter_active:
		printf ("Armed: ");
		switch (ao_igniter_status(ao_igniter_main)) {
		default:
			printf("unknown status\n");
			break;
		case ao_igniter_ready:
			printf("igniter good\n");
			break;
		case ao_igniter_open:
			printf("igniter bad\n");
			break;
		}
		break;
	default:
		printf("Disarmed\n");
	}
}

void
ao_launch_manual(void)
{
	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	ao_cmd_white();
	ao_launch_ignite = 1;
	ao_wakeup(&ao_launch_ignite);
}

static __xdata struct ao_task ao_launch_task;
static __xdata struct ao_task ao_launch_ignite_task;
static __xdata struct ao_task ao_launch_status_task;

__code struct ao_cmds ao_launch_cmds[] = {
	{ ao_launch_test,	"t\0Test launch continuity" },
	{ ao_launch_manual,	"i <key>\0Fire igniter. <key> is doit with D&I" },
	{ 0, NULL }
};

void
ao_launch_init(void)
{
	AO_IGNITER_DROGUE = 0;
	AO_IGNITER_MAIN = 0;
	AO_IGNITER_DIR |= AO_IGNITER_DROGUE_BIT | AO_IGNITER_MAIN_BIT;
	ao_cmd_register(&ao_launch_cmds[0]);
	ao_add_task(&ao_launch_task, ao_launch, "launch listener");
	ao_add_task(&ao_launch_ignite_task, ao_launch_run, "launch igniter");
	ao_add_task(&ao_launch_status_task, ao_launch_status, "launch status");
}
