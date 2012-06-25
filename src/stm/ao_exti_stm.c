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
#include <ao_exti.h>

static void	(*ao_exti_callback[16])(void);

static void
ao_exti_isr(void) {
	uint32_t	pending = stm_exti.pr;
	uint8_t		pin;

	/* Clear pending interrupts */
	stm_exti.pr = pending;
	for (pin = 0; pin < 16 && pending; pin++) {
		uint32_t	mask = (1 << pin);

		if (pending & mask) {
			pending &= ~mask;
			if (ao_exti_callback[pin])
				(*ao_exti_callback[pin])();
		}
	}
}

void stm_exti0_isr(void) { ao_exti_isr(); }
void stm_exti1_isr(void) { ao_exti_isr(); }
void stm_exti2_isr(void) { ao_exti_isr(); }
void stm_exti3_isr(void) { ao_exti_isr(); }
void stm_exti4_isr(void) { ao_exti_isr(); }
void stm_exti9_5_isr(void) { ao_exti_isr(); }
void stm_exti15_10_isr(void) { ao_exti_isr(); }

void
ao_exti_setup (struct stm_gpio *gpio, uint8_t pin, uint8_t mode, void (*callback)(void)) {
	uint32_t	mask = 1 << pin;
	uint32_t	pupdr;
	uint8_t		irq;

	ao_exti_callback[pin] = callback;

	/* configure gpio to interrupt routing */
	stm_exticr_set(gpio, pin);

	/* configure pin as input, setting selected pull-up/down mode */
	stm_moder_set(gpio, pin, STM_MODER_INPUT);
	switch (mode & (AO_EXTI_MODE_PULL_UP|AO_EXTI_MODE_PULL_DOWN)) {
	case 0:
	default:
		pupdr  = STM_PUPDR_NONE;
		break;
	case AO_EXTI_MODE_PULL_UP:
		pupdr = STM_PUPDR_PULL_UP;
		break;
	case AO_EXTI_MODE_PULL_DOWN:
		pupdr = STM_PUPDR_PULL_DOWN;
		break;
	}
	stm_pupdr_set(gpio, pin, pupdr);

	/* Set interrupt mask and rising/falling mode */
	stm_exti.imr &= ~mask;
	if (mode & AO_EXTI_MODE_RISING)
		stm_exti.rtsr |= mask;
	else
		stm_exti.rtsr &= ~mask;
	if (mode & AO_EXTI_MODE_FALLING)
		stm_exti.ftsr |= mask;
	else
		stm_exti.ftsr &= ~mask;

	if (pin <= 4)
		irq = STM_ISR_EXTI0_POS + pin;
	else if (pin <= 9)
		irq = STM_ISR_EXTI9_5_POS;
	else
		irq = STM_ISR_EXTI15_10_POS;
	stm_nvic_set_priority(irq, 10);
	stm_nvic_set_enable(irq);
}

void
ao_exti_set_callback(struct stm_gpio *gpio, uint8_t pin, void (*callback)()) {
	ao_exti_callback[pin] = callback;
}

void
ao_exti_enable(struct stm_gpio *gpio, uint8_t pin) {
	uint32_t	mask = (1 << pin);
	stm_exti.pr = mask;
	stm_exti.imr |= (1 << pin);
}

void
ao_exti_disable(struct stm_gpio *gpio, uint8_t pin) {
	uint32_t	mask = (1 << pin);
	stm_exti.imr &= ~mask;
	stm_exti.pr = mask;
}

void
ao_exti_init(void)
{
	stm_nvic_set_priority(STM_ISR_EXTI1_POS, 10);
	stm_nvic_set_priority(STM_ISR_EXTI2_POS, 10);
	stm_nvic_set_priority(STM_ISR_EXTI3_POS, 10);
	stm_nvic_set_priority(STM_ISR_EXTI4_POS, 10);
	stm_nvic_set_priority(STM_ISR_EXTI9_5_POS, 10);
	stm_nvic_set_priority(STM_ISR_EXTI15_10_POS, 10);
}
