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

#include "ao.h"
#include "ao_product.h"
#include "ao_flight.h"
#include "ao_companion.h"

struct ao_companion_command	ao_companion_command;

static const struct ao_companion_setup	ao_telescience_setup = {
	.board_id 		= AO_idProduct_NUMBER,
	.board_id_inverse	= ~AO_idProduct_NUMBER,
	.update_period		= 50,
	.channels		= AO_LOG_TELESCIENCE_NUM_ADC,
};

void ao_spi_slave(void)
{
	if (!ao_spi_slave_recv((uint8_t *) &ao_companion_command,
			       sizeof (ao_companion_command)))
		return;

	/* Figure out the outbound data */
	switch (ao_companion_command.command) {
	case AO_COMPANION_SETUP:
		ao_spi_slave_send((uint8_t *) &ao_telescience_setup,
				  sizeof (ao_telescience_setup));
		break;
	case AO_COMPANION_FETCH:
		ao_spi_slave_send((uint8_t *) &ao_data_ring[ao_data_ring_prev(ao_data_head)].adc,
				  AO_LOG_TELESCIENCE_NUM_ADC * sizeof (uint16_t));
		break;
	case AO_COMPANION_NOTIFY:
		break;
	default:
		return;
	}

#if HAS_LOG
	ao_log_single_write_data.telescience.tm_tick = ao_companion_command.tick;
	if (ao_log_single_write_data.telescience.tm_state != ao_companion_command.flight_state) {
		ao_log_single_write_data.telescience.tm_state = ao_companion_command.flight_state;
		if (ao_flight_boost <= ao_log_single_write_data.telescience.tm_state) {
			if (ao_log_single_write_data.telescience.tm_state < ao_flight_landed)
				ao_log_single_start();
			else
				ao_log_single_stop();
		}
	}
#endif
}
