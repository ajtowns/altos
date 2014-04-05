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

#ifndef _AO_LED_H_
#define _AO_LED_H_

/*
 * ao_led.c
 */

#define AO_LED_NONE	0

#ifndef AO_LED_TYPE
#define AO_LED_TYPE uint8_t
#endif

/* Turn on the specified LEDs */
void
ao_led_on(AO_LED_TYPE colors);

/* Turn off the specified LEDs */
void
ao_led_off(AO_LED_TYPE colors);

/* Set all of the LEDs to the specified state */
void
ao_led_set(AO_LED_TYPE colors);

/* Set all LEDs in 'mask' to the specified state */
void
ao_led_set_mask(uint8_t colors, uint8_t mask);

/* Toggle the specified LEDs */
void
ao_led_toggle(AO_LED_TYPE colors);

/* Turn on the specified LEDs for the indicated interval */
void
ao_led_for(AO_LED_TYPE colors, uint16_t ticks) __reentrant;

/* Initialize the LEDs */
void
ao_led_init(AO_LED_TYPE enable);

#endif /* _AO_LED_H_ */
