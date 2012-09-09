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

#ifndef _AO_PAD_H_
#define _AO_PAD_H_

#define AO_PAD_MAX_CHANNELS	8
#define AO_PAD_MAX_BOXES	100

struct ao_pad_command {
	uint16_t	tick;
	uint16_t	box;
	uint8_t		cmd;
	uint8_t		channels;
};

/* Report current telefire status.
 */

#define AO_PAD_QUERY		1

struct ao_pad_query {
	uint16_t	tick;		/* telefire tick */
	uint16_t	box;		/* telefire box number */
	uint8_t		channels;	/* which chanels are present */
	uint8_t		armed;		/* which channels are armed */
	uint8_t		arm_status;	/* status of arming switch */
	uint8_t		igniter_status[AO_PAD_MAX_CHANNELS];	/* status for each igniter */
};

/* Arm pads for 3 seconds, no report
 */
#define AO_PAD_ARM		2

#define AO_PAD_ARM_TIME		AO_SEC_TO_TICKS(3)

/* Fire current armed pads for 200ms, no report
 */
#define AO_PAD_FIRE		3

#define AO_PAD_FIRE_TIME	AO_MS_TO_TICKS(200)

#define AO_PAD_ARM_STATUS_DISARMED	0
#define AO_PAD_ARM_STATUS_ARMED		1
#define AO_PAD_ARM_STATUS_UNKNOWN	2

#define AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_OPEN	0
#define AO_PAD_IGNITER_STATUS_GOOD_IGNITER_RELAY_OPEN	1
#define AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_CLOSED	2
#define AO_PAD_IGNITER_STATUS_UNKNOWN			3

void
ao_pad_init(void);

void
ao_pad_disable(void);

void
ao_pad_enable(void);

#endif /* _AO_PAD_H_ */
