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

#ifndef _AO_EXTI_H_
#define _AO_EXTI_H_

void
ao_exti_setup_port(uint8_t pin, uint8_t mode, void (*callback)(void));

#define ao_exti_setup(port, pin, mode, callback)	ao_exti_setup_port(pin, mode, callback)

#define ao_exti_enable(gpio, pin) 	(PCMSK |= (1 << (pin)))

#define ao_exti_disable(gpio, pin)	(PCMSK &= ~(1 << (pin)))

#define ao_exti_init()

#define AO_EXTI_MODE_RISING	1
#define AO_EXTI_PIN_NOCONFIGURE	0

#endif /* _AO_EXTI_H_ */
