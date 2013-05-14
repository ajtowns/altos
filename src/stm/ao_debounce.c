/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
#include <ao_debounce.h>

static uint8_t			ao_debounce_initialized;
static uint8_t			ao_debounce_running;
static struct ao_debounce	*ao_debounce;

static void
ao_debounce_on(void)
{
	stm_tim6.cr1 = ((0 << STM_TIM67_CR1_ARPE) |
			(0 << STM_TIM67_CR1_OPM) |
			(1 << STM_TIM67_CR1_URS) |
			(0 << STM_TIM67_CR1_UDIS) |
			(1 << STM_TIM67_CR1_CEN));
}

static void
ao_debounce_off(void)
{
	stm_tim6.cr1 = ((0 << STM_TIM67_CR1_ARPE) |
			(0 << STM_TIM67_CR1_OPM) |
			(1 << STM_TIM67_CR1_URS) |
			(0 << STM_TIM67_CR1_UDIS) |
			(0 << STM_TIM67_CR1_CEN));
}

static void
_ao_debounce_set(struct ao_debounce *debounce, uint8_t value)
{
	if (value != debounce->value) {
		debounce->value = value;
		debounce->_set(debounce, value);
	}
	_ao_debounce_stop(debounce);
}

/*
 * Get the current value, set the result when we've
 * reached the debounce count limit
 */
static void
_ao_debounce_check(struct ao_debounce *debounce)
{
	if (debounce->_get(debounce)) {
		if (debounce->count < 0)
			debounce->count = 0;
		if (debounce->count < debounce->hold) {
			if (++debounce->count == debounce->hold)
				_ao_debounce_set(debounce, 1);
		}
	} else {
		if (debounce->count > 0)
			debounce->count = 0;
		if (debounce->count > -debounce->hold) {
			if (--debounce->count == -debounce->hold)
				_ao_debounce_set(debounce, 0);
		}
	}
}

/*
 * Start monitoring one pin
 */
void
_ao_debounce_start(struct ao_debounce *debounce)
{
	if (debounce->running)
		return;
	debounce->running = 1;

	/* Reset the counter */
	debounce->count = 0;

	/* Link into list */
	debounce->next = ao_debounce;
	ao_debounce = debounce;

	/* Make sure the timer is running */
	if (!ao_debounce_running++)
		ao_debounce_on();

	/* And go check the current value */
	_ao_debounce_check(debounce);
}

/*
 * Stop monitoring one pin
 */
void
_ao_debounce_stop(struct ao_debounce *debounce)
{
	struct ao_debounce **prev;
	if (!debounce->running)
		return;

	debounce->running = 0;

	/* Unlink */
	for (prev = &ao_debounce; (*prev); prev = &((*prev)->next)) {
		if (*prev == debounce) {
			*prev = debounce->next;
			break;
		}
	}
	debounce->next = NULL;

	/* Turn off the timer if possible */
	if (!--ao_debounce_running)
		ao_debounce_off();
}

void stm_tim6_isr(void)
{
	struct ao_debounce	*debounce, *next;
	if (stm_tim6.sr & (1 << STM_TIM67_SR_UIF)) {
		stm_tim6.sr = 0;

		/* Walk the current list, allowing the current
		 * object to be removed from the list
		 */
		for (debounce = ao_debounce; debounce; debounce = next) {
			next = debounce->next;
			_ao_debounce_check(debounce);
		}
	}
}

/*
 * According to the STM clock-configuration, timers run
 * twice as fast as the APB1 clock *if* the APB1 prescaler
 * is greater than 1.
 */

#if AO_APB1_PRESCALER > 1
#define TIMER_23467_SCALER 2
#else
#define TIMER_23467_SCALER 1
#endif

#define TIMER_100kHz	((AO_PCLK1 * TIMER_23467_SCALER) / 100000)

void
ao_debounce_init(void)
{
	if (ao_debounce_initialized)
		return;
	ao_debounce_initialized = 1;

	stm_nvic_set_enable(STM_ISR_TIM6_POS);
	stm_nvic_set_priority(STM_ISR_TIM6_POS, AO_STM_NVIC_CLOCK_PRIORITY);

	/* Turn on timer 6 */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_TIM6EN);

	stm_tim6.psc = TIMER_100kHz;
	stm_tim6.arr = 9;
	stm_tim6.cnt = 0;

	/* Enable update interrupt */
	stm_tim6.dier = (1 << STM_TIM67_DIER_UIE);

	/* Poke timer to reload values */
	stm_tim6.egr |= (1 << STM_TIM67_EGR_UG);

	stm_tim6.cr2 = (STM_TIM67_CR2_MMS_RESET << STM_TIM67_CR2_MMS);

	/* And turn it off (for now) */
	ao_debounce_off();
}
