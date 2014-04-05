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

#ifndef _AO_LOG_MICRO_H_
#define _AO_LOG_MICRO_H_

#define PA_GROUND_OFFSET	0
#define PA_MIN_OFFSET		4
#define N_SAMPLES_OFFSET	8
#define STARTING_LOG_OFFSET	10
#define MAX_LOG_OFFSET		512

void
ao_log_micro_save(void);

void
ao_log_micro_restore(void);

void
ao_log_micro_data(void);

void
ao_log_micro_dump(void);

#endif /* _AO_LOG_MICRO_H_ */
