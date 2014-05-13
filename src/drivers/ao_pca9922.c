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

/*
 * PCA9922 LED driver. This uses SPI to send a single byte to the device to
 * set the current state of the LEDs using the existing LED interface
 */

#include <ao.h>

static __xdata uint8_t	ao_led_state;

static void
ao_led_apply(void)
{
	/* Don't try the SPI bus during initialization */
	if (!ao_cur_task)
		return;
	ao_spi_get(AO_PCA9922_SPI_BUS);
	ao_spi_set_speed(AO_PCA9922_SPI_BUS,AO_SPI_SPEED_FAST);
	AO_PCA9922_CS = 1;
	ao_spi_send(&ao_led_state, 1, AO_PCA9922_SPI_BUS);
	AO_PCA9922_CS = 0;
	ao_spi_put(AO_PCA9922_SPI_BUS);
}

void
ao_led_on(uint8_t colors)
{
	ao_led_state |= colors;
	ao_led_apply();
}

void
ao_led_off(uint8_t colors)
{
	ao_led_state &= ~colors;
	ao_led_apply();
}

void
ao_led_set(uint8_t colors)
{
	ao_led_state = colors;
	ao_led_apply();
}

void
ao_led_set_mask(uint8_t colors, uint8_t mask)
{
	ao_led_state = (ao_led_state & ~mask) | (colors & mask);
	ao_led_apply();
}

#define LED_TEST	1
#if LED_TEST
static void
ao_led_test(void)
{
	ao_cmd_hexbyte();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_led_set(ao_cmd_lex_i);
	printf("LEDs set to %02x\n", ao_cmd_lex_i);
}

static const struct ao_cmds ao_led_cmds[] = {
	{ ao_led_test,	"l <value>\0Set LEDs to <value>" },
	{ 0, NULL }
};
#endif

void
ao_led_toggle(uint8_t colors)
{
	ao_led_state ^= colors;
	ao_led_apply();
}

void
ao_led_for(uint8_t colors, uint16_t ticks) __reentrant
{
	ao_led_on(colors);
	ao_delay(ticks);
	ao_led_off(colors);
}

void
ao_led_init(uint8_t enable)
{
	(void) enable;
	ao_enable_output(AO_PCA9922_CS_PORT, AO_PCA9922_CS_PIN, AO_PCA9922_CS, 1);
#if LED_TEST
	ao_cmd_register(&ao_led_cmds[0]);
#endif
}
