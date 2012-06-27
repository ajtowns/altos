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
#include <ao_hmc5883.h>
#include <ao_exti.h>

uint8_t	ao_hmc5883_valid;

static uint8_t	ao_hmc5883_configured;

static uint8_t	ao_hmc5883_addr;

static void
ao_hmc5883_reg_write(uint8_t addr, uint8_t data)
{
	uint8_t	d[2];

	d[0] = addr;
	d[1] = data;
	ao_i2c_get(AO_HMC5883_I2C_INDEX);
	ao_i2c_start(AO_HMC5883_I2C_INDEX, HMC5883_ADDR_WRITE);
	ao_i2c_send(d, 2, AO_HMC5883_I2C_INDEX, TRUE);
	ao_i2c_put(AO_HMC5883_I2C_INDEX);
	ao_hmc5883_addr = addr + 1;
}

static void
ao_hmc5883_read(uint8_t addr, uint8_t *data, uint8_t len)
{
	ao_i2c_get(AO_HMC5883_I2C_INDEX);
	if (addr != ao_hmc5883_addr) {
		ao_i2c_start(AO_HMC5883_I2C_INDEX, HMC5883_ADDR_WRITE);
		ao_i2c_send(&addr, 1, AO_HMC5883_I2C_INDEX, FALSE);
	}
	ao_i2c_start(AO_HMC5883_I2C_INDEX, HMC5883_ADDR_READ);
	ao_i2c_recv(data, len, AO_HMC5883_I2C_INDEX, TRUE);
	ao_i2c_put(AO_HMC5883_I2C_INDEX);
	ao_hmc5883_addr = 0xff;
}

static uint8_t ao_hmc5883_done;

static void
ao_hmc5883_isr(void)
{
	ao_exti_disable(AO_HMC5883_INT_PORT, AO_HMC5883_INT_PIN);
	ao_hmc5883_done = 1;
	ao_wakeup(&ao_hmc5883_done);
}

void
ao_hmc5883_sample(struct ao_hmc5883_sample *sample)
{
	uint16_t	*d = (uint16_t *) sample;
	int		i = sizeof (*sample) / 2;
	uint8_t		single = HMC5883_MODE_SINGLE;

	ao_hmc5883_done = 0;
	ao_exti_enable(AO_HMC5883_INT_PORT, AO_HMC5883_INT_PIN);
	ao_hmc5883_reg_write(HMC5883_MODE, HMC5883_MODE_SINGLE);

	cli();
	while (!ao_hmc5883_done)
		ao_sleep(&ao_hmc5883_done);
	sei();

	ao_hmc5883_read(HMC5883_X_MSB, (uint8_t *) sample, sizeof (struct ao_hmc5883_sample));
#if __BYTE_ORDER == __LITTLE_ENDIAN
	/* byte swap */
	while (i--) {
		uint16_t	t = *d;
		*d++ = (t >> 8) | (t << 8);
	}
#endif
}

static uint8_t
ao_hmc5883_setup(void)
{
	uint8_t d;
	uint8_t	present;

	if (ao_hmc5883_configured)
		return 1;

	ao_i2c_get(AO_HMC5883_I2C_INDEX);
	present = ao_i2c_start(AO_HMC5883_I2C_INDEX, HMC5883_ADDR_READ);
	ao_i2c_recv(&d, 1, AO_HMC5883_I2C_INDEX, TRUE);
	ao_i2c_put(AO_HMC5883_I2C_INDEX);

	if (!present)
		ao_panic(AO_PANIC_SELF_TEST);

	ao_hmc5883_reg_write(HMC5883_CONFIG_A,
			     (HMC5883_CONFIG_A_MA_8 << HMC5883_CONFIG_A_MA) |
			     (HMC5883_CONFIG_A_DO_15 << HMC5883_CONFIG_A_DO) |
			     (HMC5883_CONFIG_A_MS_NORMAL << HMC5883_CONFIG_A_MS));

	ao_hmc5883_reg_write(HMC5883_CONFIG_B,
			     (HMC5883_CONFIG_B_GN_1_3 << HMC5883_CONFIG_B_GN));

	ao_hmc5883_configured = 1;
	return 1;
}

struct ao_hmc5883_sample ao_hmc5883_current;

static void
ao_hmc5883(void)
{
	ao_hmc5883_setup();
	for (;;) {
		struct ao_hmc5883_sample ao_hmc5883_next;

		ao_hmc5883_sample(&ao_hmc5883_next);
		ao_arch_critical(
			ao_hmc5883_current = ao_hmc5883_next;
			ao_hmc5883_valid = 1;
			);
		ao_delay(0);
	}
}

static struct ao_task ao_hmc5883_task;

static void
ao_hmc5883_show(void)
{
	struct ao_hmc5883_sample	sample;

	sample = ao_hmc5883_current;
	printf ("X: %d Y: %d Z: %d\n", sample.x, sample.y, sample.z);
}

static const struct ao_cmds ao_hmc5883_cmds[] = {
	{ ao_hmc5883_show,	"M\0Show HMC5883 status" },
	{ 0, NULL }
};

void
ao_hmc5883_init(void)
{
	ao_hmc5883_configured = 0;
	ao_hmc5883_valid = 0;

	ao_enable_port(AO_HMC5883_INT_PORT);
	ao_exti_setup(AO_HMC5883_INT_PORT,
		      AO_HMC5883_INT_PIN,
		      AO_EXTI_MODE_FALLING | AO_EXTI_MODE_PULL_UP,
		      ao_hmc5883_isr);

	ao_add_task(&ao_hmc5883_task, ao_hmc5883, "hmc5883");
	ao_cmd_register(&ao_hmc5883_cmds[0]);
}
