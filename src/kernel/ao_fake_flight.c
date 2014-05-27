/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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
#include <ao_fake_flight.h>
#if HAS_MS5607 || HAS_MS5611
#include <ao_ms5607.h>
#endif

uint8_t			ao_fake_flight_active;

static uint8_t		ao_fake_has_cur;
static volatile uint8_t	ao_fake_has_next;
static uint8_t		ao_fake_has_offset;
static uint16_t		ao_fake_tick_offset;
static struct ao_data	ao_fake_cur, ao_fake_next;

void
ao_fake_flight_poll(void)
{
	if (ao_fake_has_next && (ao_tick_count - ao_fake_next.tick) >= 0) {
		ao_fake_cur = ao_fake_next;
		ao_fake_has_next = 0;
		ao_wakeup((void *) &ao_fake_has_next);
		ao_fake_has_cur = 1;
	}
	if (!ao_fake_has_cur)
		return;
	ao_data_ring[ao_data_head] = ao_fake_cur;
	ao_data_ring[ao_data_head].tick = ao_tick_count;
	ao_data_head = ao_data_ring_next(ao_data_head);
	ao_wakeup((void *) &ao_data_head);
}

static uint8_t
ao_fake_data_read(void)
{
	uint8_t	i;
	uint8_t	*d = (void *) &ao_fake_next;

	if (getchar() == 0)
		return FALSE;
	for (i = 0; i < sizeof (struct ao_data); i++)
		*d++ = getchar();
	if (!ao_fake_has_offset) {
		ao_fake_tick_offset = (ao_tick_count + 1000) - ao_fake_next.tick;
		ao_fake_next.tick = ao_tick_count;
		ao_fake_has_offset = 1;
	} else
		ao_fake_next.tick += ao_fake_tick_offset;
	ao_fake_has_next = 1;
	return TRUE;
}

static void
ao_fake_calib_get(struct ao_fake_calib *calib)
{
#if HAS_ACCEL
	calib->accel_plus_g = ao_config.accel_plus_g;
	calib->accel_minus_g = ao_config.accel_minus_g;
#endif
#if HAS_GYRO
	calib->accel_zero_along = ao_config.accel_zero_along;
	calib->accel_zero_across = ao_config.accel_zero_across;
	calib->accel_zero_through = ao_config.accel_zero_through;
#endif
#if HAS_MS5607 || HAS_MS5611
	calib->ms5607_prom = ao_ms5607_prom;
#endif
}

static void
ao_fake_calib_set(struct ao_fake_calib *calib)
{
#if HAS_ACCEL
	ao_config.accel_plus_g = calib->accel_plus_g;
	ao_config.accel_minus_g = calib->accel_minus_g;
#endif
#if HAS_GYRO
	ao_config.accel_zero_along = calib->accel_zero_along;
	ao_config.accel_zero_across = calib->accel_zero_across;
	ao_config.accel_zero_through = calib->accel_zero_through;
#endif
#if HAS_MS5607 || HAS_MS5611
	ao_ms5607_prom = calib->ms5607_prom;
#endif
}

static uint8_t
ao_fake_calib_read(void)
{
	struct ao_fake_calib	ao_calib;
	uint8_t			*d = (void *) &ao_calib;
	uint16_t		i;

	/* Read calibration data */
	for (i = 0; i < sizeof (struct ao_fake_calib); i++)
		*d++ = getchar();
	if (ao_calib.major != AO_FAKE_CALIB_MAJOR
#if AO_FAKE_CALIB_MINOR != 0
	    || ao_calib.minor < AO_FAKE_CALIB_MINOR
#endif
		) {
		printf ("Calibration data major version mismatch %d.%d <= %d.%d\n",
			ao_calib.major, ao_calib.minor, AO_FAKE_CALIB_MAJOR, AO_FAKE_CALIB_MINOR);
		return FALSE;
	}
	ao_fake_calib_set(&ao_calib);
	return TRUE;
}

static void
ao_fake_flight(void)
{
	int16_t			calib_size, data_size;
	struct ao_fake_calib	save_calib;
	uint16_t		my_pyro_fired = 0;
	enum ao_flight_state	my_state = ao_flight_invalid;
	int			i;

	ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	calib_size = ao_cmd_lex_i;
	ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	data_size = ao_cmd_lex_i;
	if ((unsigned) calib_size != sizeof (struct ao_fake_calib)) {
		printf ("calib size %d larger than actual size %d\n",
			calib_size, sizeof (struct ao_fake_calib));
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	if (data_size != sizeof (struct ao_data)) {
		printf ("data size %d doesn't match actual size %d\n",
			data_size, sizeof (struct ao_data));
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	ao_fake_calib_get(&save_calib);
	if (!ao_fake_calib_read())
		return;

	ao_fake_has_next = 0;
	ao_fake_has_cur = 0;
	ao_fake_flight_active = 1;
	ao_sample_init();
#if PACKET_HAS_SLAVE
	ao_packet_slave_stop();
#endif
#if AO_LED_RED
	/* Turn on the LED to indicate startup */
	ao_led_on(AO_LED_RED);
#endif
	ao_flight_state = ao_flight_startup;
	for (;;) {
		if (my_state != ao_flight_state) {
			printf("state %d\n", ao_flight_state);
			my_state = ao_flight_state;
			flush();
		}
		if (my_pyro_fired != ao_pyro_fired) {
			int	pyro;

			for (pyro = 0; pyro < AO_PYRO_NUM; pyro++) {
				uint16_t	bit = (1 << pyro);
				if (!(my_pyro_fired & bit) && (ao_pyro_fired & bit))
					printf ("fire %d\n", pyro);
			}
			my_pyro_fired = ao_pyro_fired;
		}
		while (ao_fake_has_next)
			ao_sleep((void *) &ao_fake_has_next);
		if (!ao_fake_data_read())
			break;
	}

	/* Wait 20 seconds to see if we enter landed state */
	for (i = 0; i < 200; i++)
	{
		if (ao_flight_state == ao_flight_landed)
			break;
		ao_delay(AO_MS_TO_TICKS(100));
	}
#if AO_LED_RED
	/* Turn on the LED to indicate startup */
	ao_led_on(AO_LED_RED);
#endif
	ao_fake_flight_active = 0;
	ao_flight_state = ao_flight_startup;
	ao_sample_init();
	ao_fake_calib_set(&save_calib);
}

static const struct ao_cmds ao_fake_flight_cmds[] = {
	{ ao_fake_flight,	"F <calib-size> <data-size>\0Start fake flight" },
	{ 0, NULL }
};

void
ao_fake_flight_init(void)
{
	ao_cmd_register(&ao_fake_flight_cmds[0]);
}
