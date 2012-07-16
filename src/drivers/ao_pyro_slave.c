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
#include <ao_product.h>
#include <ao_companion.h>
#include <ao_flight.h>
#include <ao_pyro.h>

struct ao_companion_command	ao_companion_command;

static const struct ao_companion_setup	ao_telepyro_setup = {
	.board_id 		= AO_idProduct_NUMBER,
	.board_id_inverse	= ~AO_idProduct_NUMBER,
	.update_period		= 50,
	.channels		= AO_TELEPYRO_NUM_ADC,
};

struct ao_config ao_config;

extern volatile __data uint16_t ao_tick_count;
uint16_t ao_boost_tick;

void ao_spi_slave(void)
{
	if (!ao_spi_slave_recv((uint8_t *) &ao_companion_command,
			       sizeof (ao_companion_command)))
		return;

	/* Figure out the outbound data */
	switch (ao_companion_command.command) {
	case AO_COMPANION_SETUP:
		ao_spi_slave_send((uint8_t *) &ao_telepyro_setup,
				  sizeof (ao_telepyro_setup));
		break;
	case AO_COMPANION_FETCH:
		ao_spi_slave_send((uint8_t *) &ao_data_ring[ao_data_ring_prev(ao_data_head)].adc.adc,
				  AO_TELEPYRO_NUM_ADC * sizeof (uint16_t));
		break;
	case AO_COMPANION_NOTIFY:
		/* Can't use ao_time because we have interrupts suspended */
		if (ao_companion_command.flight_state < ao_flight_boost && ao_companion_command.flight_state >= ao_flight_boost)
			ao_boost_tick = ao_tick_count;
		ao_wakeup(&ao_pyro_wakeup);
		break;
	default:
		return;
	}
}
