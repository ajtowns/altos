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

/*
 * ao_log_single.c
 *
 * Stores a sequence of fixed-size (32 byte) chunks
 * without splitting memory up into separate flights
 */

#include "ao.h"
#include "ao_product.h"

static __xdata struct ao_task ao_log_single_task;

__xdata uint8_t 	ao_log_running;
__xdata uint8_t		ao_log_mutex;
__pdata uint32_t	ao_log_start_pos;
__pdata uint32_t	ao_log_end_pos;
__pdata uint32_t	ao_log_current_pos;

__xdata union ao_log_single ao_log_single_write_data;
__xdata union ao_log_single ao_log_single_read_data;

uint8_t
ao_log_single_write(void)
{
	uint8_t wrote = 0;

	ao_mutex_get(&ao_log_mutex); {
		if (ao_log_current_pos >= ao_log_end_pos && ao_log_running)
			ao_log_single_stop();
		if (ao_log_running) {
			wrote = 1;
			ao_storage_write(ao_log_current_pos,
					 &ao_log_single_write_data,
					 AO_LOG_SINGLE_SIZE);
			ao_log_current_pos += AO_LOG_SINGLE_SIZE;
		}
	} ao_mutex_put(&ao_log_mutex);
	return wrote;
}

static uint8_t
ao_log_single_valid(void)
{
	__xdata uint8_t	*d = ao_log_single_read_data.bytes;
	uint8_t	i;
	for (i = 0; i < AO_LOG_SINGLE_SIZE; i++)
		if (*d++ != 0xff)
			return 1;
	return 0;
}

uint8_t
ao_log_single_read(uint32_t pos)
{
	if (!ao_storage_read(pos, &ao_log_single_read_data, AO_LOG_SINGLE_SIZE))
		return 0;
	return ao_log_single_valid();
}

void
ao_log_single_start(void)
{
	if (!ao_log_running) {
		ao_log_running = 1;
		ao_wakeup(&ao_log_running);
	}
}

void
ao_log_single_stop(void)
{
	if (ao_log_running) {
		ao_log_running = 0;
	}
}

void
ao_log_single_restart(void)
{
	/* Find end of data */
	ao_log_end_pos = ao_storage_config;
	for (ao_log_current_pos = 0;
	     ao_log_current_pos < ao_storage_config;
	     ao_log_current_pos += ao_storage_block)
	{
		if (!ao_log_single_read(ao_log_current_pos))
			break;
	}
	if (ao_log_current_pos > 0) {
		ao_log_current_pos -= ao_storage_block;
		for (; ao_log_current_pos < ao_storage_config;
		     ao_log_current_pos += sizeof (struct ao_log_telescience))
		{
			if (!ao_log_single_read(ao_log_current_pos))
				break;
		}
	}
}

void
ao_log_single_set(void)
{
	printf("Logging currently %s\n", ao_log_running ? "on" : "off");
	ao_cmd_hex();
	if (ao_cmd_status == ao_cmd_success) {
		if (ao_cmd_lex_i) {
			printf("Logging from %ld to %ld\n", ao_log_current_pos, ao_log_end_pos);
			ao_log_single_start();
		} else {
			printf ("Log stopped at %ld\n", ao_log_current_pos);
			ao_log_single_stop();
		}
	}
	ao_cmd_status = ao_cmd_success;
}

void
ao_log_single_delete(void)
{
	uint32_t	pos;

	ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ao_cmd_lex_i != 1) {
		ao_cmd_status = ao_cmd_syntax_error;
		printf("No such flight: %d\n", ao_cmd_lex_i);
		return;
	}
	ao_log_single_stop();
	for (pos = 0; pos < ao_storage_config; pos += ao_storage_block) {
		if (!ao_log_single_read(pos))
			break;
		ao_storage_erase(pos);
	}
	ao_log_current_pos = ao_log_start_pos = 0;
	if (pos == 0)
		printf("No such flight: %d\n", ao_cmd_lex_i);
	else
		printf ("Erased\n");
}

uint8_t
ao_log_full(void)
{
	return ao_log_current_pos >= ao_log_end_pos;
}

uint8_t
ao_log_present(void)
{
	return ao_log_single_read(0);
}

static void
ao_log_single_query(void)
{
	printf("Logging enabled: %d\n", ao_log_running);
	printf("Log start: %ld\n", ao_log_start_pos);
	printf("Log cur: %ld\n", ao_log_current_pos);
	printf("Log end: %ld\n", ao_log_end_pos);
	ao_log_single_extra_query();
}

const struct ao_cmds ao_log_single_cmds[] = {
	{ ao_log_single_set,	"L <0 off, 1 on>\0Set logging" },
	{ ao_log_single_list,	"l\0List stored logs" },
	{ ao_log_single_delete, "d 1\0Delete all stored logs" },
	{ ao_log_single_query, "q\0Query log status" },
	{ 0,	NULL },
};

void
ao_log_single_init(void)
{
	ao_log_running = 0;

	ao_cmd_register(&ao_log_single_cmds[0]);

	ao_add_task(&ao_log_single_task, ao_log_single, "log");
}
