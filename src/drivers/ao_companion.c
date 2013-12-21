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
#include <ao_companion.h>

#ifdef TELEMEGA
#define ao_spi_slow(b)
#define ao_spi_fast(b)
#endif

#define COMPANION_SELECT()	do {			\
		ao_spi_get_bit(AO_COMPANION_CS_PORT,	\
			       AO_COMPANION_CS_PIN,	\
			       AO_COMPANION_CS,		\
			       AO_COMPANION_SPI_BUS,	\
			       AO_SPI_SPEED_200kHz);	\
	} while (0)

#define COMPANION_DESELECT()	do {			\
		ao_spi_put_bit(AO_COMPANION_CS_PORT,	\
			       AO_COMPANION_CS_PIN,	\
			       AO_COMPANION_CS,		\
			       AO_COMPANION_SPI_BUS);	\
	} while (0)

__xdata struct ao_companion_command		ao_companion_command;
__xdata struct ao_companion_setup		ao_companion_setup;

__xdata uint16_t	ao_companion_data[AO_COMPANION_MAX_CHANNELS];
__pdata uint8_t		ao_companion_running;
__xdata uint8_t		ao_companion_mutex;

static void
ao_companion_send_command(uint8_t command)
{
	ao_companion_command.command = command;
	ao_companion_command.flight_state = ao_flight_state;
	ao_companion_command.tick = ao_time();
	ao_companion_command.serial = ao_serial_number;
	ao_companion_command.flight = ao_flight_number;
	ao_companion_command.accel = ao_accel;
	ao_companion_command.speed = ao_speed;
	ao_companion_command.height = ao_height;
	ao_companion_command.motor_number = ao_motor_number;
	ao_spi_send(&ao_companion_command, sizeof (ao_companion_command), AO_COMPANION_SPI_BUS);
}

static uint8_t
ao_companion_get_setup(void)
{
	COMPANION_SELECT();
	ao_companion_send_command(AO_COMPANION_SETUP);
	ao_spi_recv(&ao_companion_setup, sizeof (ao_companion_setup), AO_COMPANION_SPI_BUS);
	COMPANION_DESELECT();
	return ((int16_t) ao_companion_setup.board_id ==
		(int16_t) (uint16_t) (~ao_companion_setup.board_id_inverse));
}

static void
ao_companion_get_data(void)
{
	COMPANION_SELECT();
	ao_companion_send_command(AO_COMPANION_FETCH);
	ao_mutex_get(&ao_companion_mutex);
	ao_spi_recv(&ao_companion_data, ao_companion_setup.channels * 2, AO_COMPANION_SPI_BUS);
	ao_mutex_put(&ao_companion_mutex);
	COMPANION_DESELECT();
}

static void
ao_companion_notify(void)
{
	COMPANION_SELECT();
	ao_companion_send_command(AO_COMPANION_NOTIFY);
	COMPANION_DESELECT();
}

void
ao_companion(void)
{
	uint8_t	i;
	while (!ao_flight_number)
		ao_sleep(&ao_flight_number);
	for (i = 0; i < 10; i++) {
		ao_delay(AO_SEC_TO_TICKS(1));
		if ((ao_companion_running = ao_companion_get_setup()))
		    break;
	}
	while (ao_companion_running) {
		ao_alarm(ao_companion_setup.update_period);
		if (ao_sleep(DATA_TO_XDATA(&ao_flight_state)))
			ao_companion_get_data();
		else
			ao_companion_notify();
	}
	ao_exit();
}

void
ao_companion_status(void) __reentrant
{
	uint8_t	i;
	printf("Companion running: %d\n", ao_companion_running);
	if (!ao_companion_running)
		return;
	printf("device: %d\n"
	       "update period: %d\n"
	       "channels: %d\n"
	       "data:",
	       ao_companion_setup.board_id,
	       ao_companion_setup.update_period,
	       ao_companion_setup.channels);
	for(i = 0; i < ao_companion_setup.channels; i++)
		printf(" %5u", ao_companion_data[i]);
	printf("\n");
}

__code struct ao_cmds ao_companion_cmds[] = {
	{ ao_companion_status,	"L\0Companion link status" },
	{ 0, NULL },
};

static __xdata struct ao_task ao_companion_task;

void
ao_companion_init(void)
{
	ao_enable_output(AO_COMPANION_CS_PORT, AO_COMPANION_CS_PIN, AO_COMPANION_CS, 1);
	ao_cmd_register(&ao_companion_cmds[0]);
	ao_add_task(&ao_companion_task, ao_companion, "companion");
}
