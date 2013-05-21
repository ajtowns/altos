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

#define AO_EXTI_MODE_RISING	1
#define AO_EXTI_MODE_FALLING	2
#define AO_EXTI_MODE_PULL_UP	4
#define AO_EXTI_MODE_PULL_DOWN	8
#define AO_EXTI_PRIORITY_LOW	16
#define AO_EXTI_PRIORITY_MED	0
#define AO_EXTI_PRIORITY_HIGH	32
#define AO_EXTI_PIN_NOCONFIGURE	64

void
ao_exti_setup(struct stm_gpio *gpio, uint8_t pin, uint8_t mode, void (*callback)());

void
ao_exti_set_mode(struct stm_gpio *gpio, uint8_t pin, uint8_t mode);

void
ao_exti_set_callback(struct stm_gpio *gpio, uint8_t pin, void (*callback)());

void
ao_exti_enable(struct stm_gpio *gpio, uint8_t pin);

void
ao_exti_disable(struct stm_gpio *gpio, uint8_t pin);

void
ao_exti_init(void);

#endif /* _AO_EXTI_H_ */
