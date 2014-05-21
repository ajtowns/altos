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

#define LPC_NUM_PINS	56
#define LPC_NUM_PINT	8

static void	(*ao_exti_callback[LPC_NUM_PINT])(void);

static uint8_t	ao_pint_map[LPC_NUM_PINS];
static uint8_t	ao_pint_mode[LPC_NUM_PINS];
static uint8_t	ao_pint_inuse;
static uint8_t	ao_pint_enabled;

static void
ao_exti_isr(uint8_t pint)
{
	uint8_t	mask = 1 << pint;

	if (lpc_gpio_pin.ist & mask) {
		lpc_gpio_pin.ist = mask;
		lpc_gpio_pin.rise = mask;
		lpc_gpio_pin.fall = mask;

		(*ao_exti_callback[pint]) ();
	}
}

#define pin_isr(n)	void lpc_pin_int ## n ## _isr(void) { ao_exti_isr(n); }
pin_isr(0)
pin_isr(1)
pin_isr(2)
pin_isr(3)
pin_isr(4)
pin_isr(5)
pin_isr(6)
pin_isr(7)

#define pin_id(port,pin)	((port) * 24 + (pin));

static void
_ao_exti_set_enable(uint8_t pint)
{
	uint8_t		mask = 1 << pint;
	uint8_t		mode;

	if (ao_pint_enabled & mask)
		mode = ao_pint_mode[pint];
	else
		mode = 0;

	if (mode & AO_EXTI_MODE_RISING)
		lpc_gpio_pin.sienr = mask;
	else
		lpc_gpio_pin.cienr = mask;

	if (mode & AO_EXTI_MODE_FALLING)
		lpc_gpio_pin.sienf = mask;
	else
		lpc_gpio_pin.cienf = mask;
	lpc_gpio_pin.rise = mask;
	lpc_gpio_pin.fall = mask;
}

void
ao_exti_setup (uint8_t port, uint8_t pin, uint8_t mode, void (*callback)(void)) {
	uint8_t		id = pin_id(port,pin);
	uint8_t	       	pint;
	uint32_t	mask;
	uint8_t		prio;

	for (pint = 0; pint < LPC_NUM_PINT; pint++)
		if ((ao_pint_inuse & (1 << pint)) == 0)
			break;
	if (pint == LPC_NUM_PINT)
		ao_panic(AO_PANIC_EXTI);

	if (!(mode & AO_EXTI_PIN_NOCONFIGURE))
		ao_enable_input(port, pin, mode);

	ao_arch_block_interrupts();
	mask = (1 << pint);
	ao_pint_inuse |= mask;
	ao_pint_enabled &= ~mask;

	ao_pint_map[id] = pint;
	ao_exti_callback[pint] = callback;

	/* configure gpio to interrupt routing */
	lpc_scb.pintsel[pint] = id;

	/* Set edge triggered */
	lpc_gpio_pin.isel &= ~mask;

	ao_pint_enabled &= ~mask;
	ao_pint_mode[pint] = mode;
	_ao_exti_set_enable(pint);

	/* Set interrupt mask and rising/falling mode */

	prio = AO_LPC_NVIC_MED_PRIORITY;
	if (mode & AO_EXTI_PRIORITY_LOW)
		prio = AO_LPC_NVIC_LOW_PRIORITY;
	else if (mode & AO_EXTI_PRIORITY_HIGH)
		prio = AO_LPC_NVIC_HIGH_PRIORITY;

	/* Set priority and enable */
	lpc_nvic_set_priority(LPC_ISR_PIN_INT0_POS + pint, prio);
	lpc_nvic_set_enable(LPC_ISR_PIN_INT0_POS + pint);
	ao_arch_release_interrupts();
}

void
ao_exti_set_mode(uint8_t port, uint8_t pin, uint8_t mode)
{
	uint8_t		id = pin_id(port,pin);
	uint8_t		pint = ao_pint_map[id];

	ao_arch_block_interrupts();
	ao_pint_mode[pint] = mode;
	_ao_exti_set_enable(pint);
	ao_arch_release_interrupts();
}

void
ao_exti_set_callback(uint8_t port, uint8_t pin, void (*callback)()) {
	uint8_t		id = pin_id(port,pin);
	uint8_t		pint = ao_pint_map[id];

	ao_exti_callback[pint] = callback;
}

void
ao_exti_enable(uint8_t port, uint8_t pin)
{
	uint8_t		id = pin_id(port,pin);
	uint8_t		pint = ao_pint_map[id];
	uint8_t		mask = 1 << pint;

	ao_arch_block_interrupts();
	ao_pint_enabled |= mask;
	_ao_exti_set_enable(pint);
	ao_arch_release_interrupts();
}

void
ao_exti_disable(uint8_t port, uint8_t pin) {
	uint8_t		id = pin_id(port,pin);
	uint8_t		pint = ao_pint_map[id];
	uint8_t		mask = 1 << pint;

	ao_arch_block_interrupts();
	ao_pint_enabled &= ~mask;
	_ao_exti_set_enable(pint);
	ao_arch_release_interrupts();
}

void
ao_exti_init(void)
{
	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_PINT);
}
