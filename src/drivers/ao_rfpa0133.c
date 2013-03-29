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

#include "ao.h"

static void
ao_rfpa0133_set_power(uint8_t power)
{
	ao_gpio_set(AO_PA_GAIN_8_GPIO, AO_PA_GAIN_8_PIN, AO_PA_GAIN_8, power & 1);
	ao_gpio_set(AO_PA_GAIN_16_GPIO, AO_PA_GAIN_16_PIN, AO_PA_GAIN_16, (power >> 1) & 1);
}

void
ao_radio_pa_on(void)
{
	ao_rfpa0133_set_power(ao_config.radio_amp);
	ao_gpio_set(AO_PA_POWER_GPIO, AO_PA_POWER_PIN, AO_PA_POWER, 1);
}

void
ao_radio_pa_off(void)
{
	ao_gpio_set(AO_PA_POWER_GPIO, AO_PA_POWER_PIN, AO_PA_POWER, 0);
	ao_rfpa0133_set_power(0);
}

void
ao_radio_pa_init(void)
{
	ao_enable_output(AO_PA_POWER_GPIO, AO_PA_POWER_PIN, AO_PA_POWER, 0);
	ao_enable_output(AO_PA_GAIN_8_GPIO, AO_PA_GAIN_8_PIN, AO_PA_GAIN_8, 0);
	ao_enable_output(AO_PA_GAIN_16_GPIO, AO_PA_GAIN_16_PIN, AO_PA_GAIN_16, 0);
}
