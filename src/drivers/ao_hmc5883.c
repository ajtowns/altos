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

static uint8_t	ao_hmc5883_wake;
static uint8_t	ao_hmc5883_configured;

static void
ao_hmc5883_isr(void)
{
	ao_exti_disable(&AO_HMC5883_INT_PORT, AO_HMC5883_INT_PIN);
	ao_hmc5883_wake = 1;
	ao_wakeup(&ao_hmc5883_wake);
}

static uint8_t	ao_hmc5883_addr_reg;

static void
ao_hmc5883_update_addr (uint8_t len)
{
	ao_hmc5883_addr_reg += len;
	if (ao_hmc5883_addr_reg == 9)
		ao_hmc5883_addr_reg = 3;
	else if (ao_hmc5883_addr_reg > 12)
		ao_hmc5883_addr_reg = 0;
		    
}

static void
ao_hmc5883_write(uint8_t addr, uint8_t *data, uint8_t len)
{
	ao_i2c_get(AO_HMC5883_I2C_INDEX);
	ao_i2c_start(AO_HMC5883_I2C_INDEX, HMC5883_ADDR_WRITE);
	ao_i2c_send(&addr, 1, AO_HMC5883_I2C_INDEX);
	ao_hmc5883_addr_reg = addr;
	if (len) {
		ao_i2c_send(data, len, AO_HMC5883_I2C_INDEX);
		ao_hmc5883_update_addr(len);
	}
	ao_i2c_stop(AO_HMC5883_I2C_INDEX);
	ao_i2c_put(AO_HMC5883_I2C_INDEX);
}

static void
ao_hmc5883_read(uint8_t addr, uint8_t *data, uint8_t len)
{
	ao_i2c_get(AO_HMC5883_I2C_INDEX);
	if (addr != ao_hmc5883_addr_reg) {
		ao_i2c_start(AO_HMC5883_I2C_INDEX, HMC5883_ADDR_WRITE);
		ao_i2c_send(&addr, 1, AO_HMC5883_I2C_INDEX);
		ao_hmc5883_addr_reg = addr;
	}
	ao_i2c_start(AO_HMC5883_I2C_INDEX, HMC5883_ADDR_READ);
	if (len) {
		ao_i2c_recv(data, len, AO_HMC5883_I2C_INDEX);
		ao_hmc5883_update_addr(len);
	}
	ao_i2c_stop(AO_HMC5883_I2C_INDEX);
	ao_i2c_put(AO_HMC5883_I2C_INDEX);
}

static uint8_t
ao_hmc5883_setup(void)
{
	uint8_t	present;
	if (ao_hmc5883_configured)
		return 1;

	/* Enable the EXTI interrupt for the appropriate pin */
	ao_enable_port(AO_HMC5883_INT_PORT);
	ao_exti_setup(&AO_HMC5883_INT_PORT, AO_HMC5883_INT_PIN,
		      AO_EXTI_MODE_FALLING, ao_hmc5883_isr);

	ao_hmc5883_addr_reg = 0xff;

	ao_i2c_get(AO_HMC5883_I2C_INDEX);
	present = ao_i2c_start(AO_HMC5883_I2C_INDEX, HMC5883_ADDR_READ);
	ao_i2c_stop(AO_HMC5883_I2C_INDEX);
	ao_i2c_put(AO_HMC5883_I2C_INDEX);
	if (!present)
		return 0;
	ao_hmc5883_configured = 1;
	return 1;
}

static void
ao_hmc5883_show(void)
{
	uint8_t	addr, data;

	for (addr = 0x00; addr <= 0x7f; addr++)
	{
		ao_i2c_get(AO_HMC5883_I2C_INDEX);
		data = ao_i2c_start(AO_HMC5883_I2C_INDEX, addr << 1);
		ao_i2c_stop(AO_HMC5883_I2C_INDEX);
		ao_i2c_put(AO_HMC5883_I2C_INDEX);
		if (data)
			printf("address %02x responds\n", addr << 1);
	}
	if (!ao_hmc5883_setup()) {
		printf("hmc5883 not present\n");
		return;
	}
	for (addr = 0; addr <= 12; addr++) {
		ao_hmc5883_read(addr, &data, 1);
		printf ("hmc5883 register %2d: %02x\n",
			addr, data);
	}
}

static const struct ao_cmds ao_hmc5883_cmds[] = {
	{ ao_hmc5883_show,	"M\0Show HMC5883 status" },
	{ 0, NULL }
};

void
ao_hmc5883_init(void)
{
	ao_hmc5883_configured = 0;

	ao_cmd_register(&ao_hmc5883_cmds[0]);
}
