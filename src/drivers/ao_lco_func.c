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
#include <ao_radio_cmac.h>
#include <ao_lco_func.h>

static __xdata struct ao_pad_command	command;
static __xdata uint8_t			ao_lco_mutex;

int8_t
ao_lco_query(uint16_t box, struct ao_pad_query *query, uint16_t *tick_offset)
{
	int8_t		r;
	uint16_t	sent_time;

	ao_mutex_get(&ao_lco_mutex);
	command.tick = ao_time() - *tick_offset;
	command.box = box;
	command.cmd = AO_LAUNCH_QUERY;
	command.channels = 0;
	ao_radio_cmac_send(&command, sizeof (command));
	sent_time = ao_time();
	r = ao_radio_cmac_recv(query, sizeof (*query), AO_MS_TO_TICKS(10));
	if (r == AO_RADIO_CMAC_OK)
		*tick_offset = sent_time - query->tick;
	ao_mutex_put(&ao_lco_mutex);
	return r;
}

void
ao_lco_arm(uint16_t box, uint8_t channels, uint16_t tick_offset)
{
	ao_mutex_get(&ao_lco_mutex);
	command.tick = ao_time() - tick_offset;
	command.box = box;
	command.cmd = AO_LAUNCH_ARM;
	command.channels = channels;
	ao_radio_cmac_send(&command, sizeof (command));
	ao_mutex_put(&ao_lco_mutex);
}

void
ao_lco_ignite(uint16_t box, uint8_t channels, uint16_t tick_offset)
{
	ao_mutex_get(&ao_lco_mutex);
	command.tick = ao_time() - tick_offset;
	command.box = box;
	command.cmd = AO_LAUNCH_FIRE;
	command.channels = channels;
	ao_radio_cmac_send(&command, sizeof (command));
	ao_mutex_put(&ao_lco_mutex);
}

