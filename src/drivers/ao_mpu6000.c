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
#include <ao_mpu6000.h>
#include <ao_exti.h>

static uint8_t	ao_mpu6000_wake;
static uint8_t	ao_mpu6000_configured;

static void
ao_mpu6000_isr(void)
{
	ao_exti_disable(&AO_MPU6000_INT_PORT, AO_MPU6000_INT_PIN);
	ao_mpu6000_wake = 1;
	ao_wakeup(&ao_mpu6000_wake);
}

static void
ao_mpu6000_write(uint8_t addr, uint8_t *data, uint8_t len)
{
	ao_i2c_get(AO_MPU6000_I2C_INDEX);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_WRITE);
	ao_i2c_send(&addr, 1, AO_MPU6000_I2C_INDEX, FALSE);
	ao_i2c_send(data, len, AO_MPU6000_I2C_INDEX, TRUE);
	ao_i2c_put(AO_MPU6000_I2C_INDEX);
}

static void
ao_mpu6000_read(uint8_t addr, uint8_t *data, uint8_t len)
{
	ao_i2c_get(AO_MPU6000_I2C_INDEX);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_WRITE);
	ao_i2c_send(&addr, 1, AO_MPU6000_I2C_INDEX, FALSE);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_READ);
	ao_i2c_recv(data, len, AO_MPU6000_I2C_INDEX, TRUE);
	ao_i2c_put(AO_MPU6000_I2C_INDEX);
}

static void
ao_mpu6000_setup(void)
{
	if (ao_mpu6000_configured)
		return;

	/* Enable the EXTI interrupt for the appropriate pin */
	ao_enable_port(AO_MPU6000_INT_PORT);
	ao_exti_setup(&AO_MPU6000_INT_PORT, AO_MPU6000_INT_PIN,
		      AO_EXTI_MODE_FALLING, ao_mpu6000_isr);

	ao_mpu6000_configured = 1;
}

static void
ao_mpu6000_show(void)
{
	uint8_t	addr;
	uint8_t	data[14];
	uint8_t i;

	ao_mpu6000_read(MPU6000_WHO_AM_I, data, 1);
	printf ("mpu6000 WHO_AM_I: %02x\n", data[0]);
#if 0
	ao_mpu6000_read(MPU6000_ACCEL_XOUT_H, data, 14);
	for (i = 0; i < 14; i++)
		printf ("reg %02x: %02x\n", i + MPU6000_ACCEL_XOUT_H, data[i]);
#endif
}

static const struct ao_cmds ao_mpu6000_cmds[] = {
	{ ao_mpu6000_show,	"I\0Show MPU6000 status" },
	{ 0, NULL }
};

void
ao_mpu6000_init(void)
{
	ao_mpu6000_configured = 0;

	ao_cmd_register(&ao_mpu6000_cmds[0]);
}
