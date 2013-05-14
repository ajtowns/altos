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

#if HAS_MPU6000

static uint8_t	ao_mpu6000_wake;
static uint8_t	ao_mpu6000_configured;

#define ao_mpu6000_spi_get() 	ao_spi_get_bit(AO_MPU6000_SPI_CS_PORT,	\
					       AO_MPU6000_SPI_CS_PIN,	\
					       AO_MPU6000_SPI_CS,	\
					       AO_MPU6000_SPI_BUS,	\
					       AO_SPI_SPEED_1MHz)

#define ao_mpu6000_spi_put() 	ao_spi_put_bit(AO_MPU6000_SPI_CS_PORT,	\
					       AO_MPU6000_SPI_CS_PIN,	\
					       AO_MPU6000_SPI_CS,	\
					       AO_MPU6000_SPI_BUS)


static void
ao_mpu6000_reg_write(uint8_t addr, uint8_t value)
{
	uint8_t	d[2] = { addr, value };
#ifdef AO_MPU6000_I2C_INDEX
	ao_i2c_get(AO_MPU6000_I2C_INDEX);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_WRITE);
	ao_i2c_send(d, 2, AO_MPU6000_I2C_INDEX, TRUE);
	ao_i2c_put(AO_MPU6000_I2C_INDEX);
#else
	ao_mpu6000_spi_get();
	ao_spi_send(d, 2, AO_MPU6000_SPI_BUS);
	ao_mpu6000_spi_put();
#endif
}

static void
ao_mpu6000_read(uint8_t addr, void *data, uint8_t len)
{
#ifdef AO_MPU6000_I2C_INDEX
	ao_i2c_get(AO_MPU6000_I2C_INDEX);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_WRITE);
	ao_i2c_send(&addr, 1, AO_MPU6000_I2C_INDEX, FALSE);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_READ);
	ao_i2c_recv(data, len, AO_MPU6000_I2C_INDEX, TRUE);
	ao_i2c_put(AO_MPU6000_I2C_INDEX);
#else
	addr |= 0x80;
	ao_mpu6000_spi_get();
	ao_spi_send(&addr, 1, AO_MPU6000_SPI_BUS);
	ao_spi_recv(data, len, AO_MPU6000_SPI_BUS);
	ao_mpu6000_spi_put();
#endif
}

static uint8_t
ao_mpu6000_reg_read(uint8_t addr)
{
	uint8_t	value;
#ifdef AO_MPU6000_I2C_INDEX
	ao_i2c_get(AO_MPU6000_I2C_INDEX);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_WRITE);
	ao_i2c_send(&addr, 1, AO_MPU6000_I2C_INDEX, FALSE);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_READ);
	ao_i2c_recv(&value, 1, AO_MPU6000_I2C_INDEX, TRUE);
	ao_i2c_put(AO_MPU6000_I2C_INDEX);
#else
	addr |= 0x80;
	ao_mpu6000_spi_get();
	ao_spi_send(&addr, 1, AO_MPU6000_SPI_BUS);
	ao_spi_recv(&value, 1, AO_MPU6000_SPI_BUS);
	ao_mpu6000_spi_put();
#endif
	return value;
}

static void
ao_mpu6000_sample(struct ao_mpu6000_sample *sample)
{
	uint16_t	*d = (uint16_t *) sample;
	int		i = sizeof (*sample) / 2;

	ao_mpu6000_read(MPU6000_ACCEL_XOUT_H, sample, sizeof (*sample));
#if __BYTE_ORDER == __LITTLE_ENDIAN
	/* byte swap */
	while (i--) {
		uint16_t	t = *d;
		*d++ = (t >> 8) | (t << 8);
	}
#endif
}

#define G	981	/* in cm/s² */

static int16_t /* cm/s² */
ao_mpu6000_accel(int16_t v)
{
	return (int16_t) ((v * (int32_t) (16.0 * 980.665 + 0.5)) / 32767);
}

static int16_t	/* deg*10/s */
ao_mpu6000_gyro(int16_t v)
{
	return (int16_t) ((v * (int32_t) 20000) / 32767);
}

static uint8_t
ao_mpu6000_accel_check(int16_t normal, int16_t test, char *which)
{
	int16_t	diff = test - normal;

	if (diff < MPU6000_ST_ACCEL(16) / 2) {
		return 1;
	}
	if (diff > MPU6000_ST_ACCEL(16) * 2) {
		return 1;
	}
	return 0;
}

static uint8_t
ao_mpu6000_gyro_check(int16_t normal, int16_t test, char *which)
{
	int16_t	diff = test - normal;

	if (diff < 0)
		diff = -diff;
	if (diff < MPU6000_ST_GYRO(2000) / 2) {
		return 1;
	}
	if (diff > MPU6000_ST_GYRO(2000) * 2) {
		return 1;
	}
	return 0;
}

static void
ao_mpu6000_setup(void)
{
	struct ao_mpu6000_sample	normal_mode, test_mode;
	int				errors =0;

	if (ao_mpu6000_configured)
		return;

	/* Reset the whole chip */
	
	ao_mpu6000_reg_write(MPU6000_PWR_MGMT_1,
			     (1 << MPU6000_PWR_MGMT_1_DEVICE_RESET));

	/* Wait for it to reset. If we talk too quickly, it appears to get confused */
	ao_delay(AO_MS_TO_TICKS(100));

	/* Reset signal conditioning */
	ao_mpu6000_reg_write(MPU6000_USER_CONTROL,
			     (0 << MPU6000_USER_CONTROL_FIFO_EN) |
			     (0 << MPU6000_USER_CONTROL_I2C_MST_EN) |
			     (0 << MPU6000_USER_CONTROL_I2C_IF_DIS) |
			     (0 << MPU6000_USER_CONTROL_FIFO_RESET) |
			     (0 << MPU6000_USER_CONTROL_I2C_MST_RESET) |
			     (1 << MPU6000_USER_CONTROL_SIG_COND_RESET));

	while (ao_mpu6000_reg_read(MPU6000_USER_CONTROL) & (1 << MPU6000_USER_CONTROL_SIG_COND_RESET))
		ao_yield();

	/* Reset signal paths */
	ao_mpu6000_reg_write(MPU6000_SIGNAL_PATH_RESET,
			     (1 << MPU6000_SIGNAL_PATH_RESET_GYRO_RESET) |
			     (1 << MPU6000_SIGNAL_PATH_RESET_ACCEL_RESET) |
			     (1 << MPU6000_SIGNAL_PATH_RESET_TEMP_RESET));

	ao_mpu6000_reg_write(MPU6000_SIGNAL_PATH_RESET,
			     (0 << MPU6000_SIGNAL_PATH_RESET_GYRO_RESET) |
			     (0 << MPU6000_SIGNAL_PATH_RESET_ACCEL_RESET) |
			     (0 << MPU6000_SIGNAL_PATH_RESET_TEMP_RESET));

	/* Select clocks, disable sleep */
	ao_mpu6000_reg_write(MPU6000_PWR_MGMT_1,
			     (0 << MPU6000_PWR_MGMT_1_DEVICE_RESET) |
			     (0 << MPU6000_PWR_MGMT_1_SLEEP) |
			     (0 << MPU6000_PWR_MGMT_1_CYCLE) |
			     (0 << MPU6000_PWR_MGMT_1_TEMP_DIS) |
			     (MPU6000_PWR_MGMT_1_CLKSEL_PLL_X_AXIS << MPU6000_PWR_MGMT_1_CLKSEL));

	/* Set sample rate divider to sample at full speed
	ao_mpu6000_reg_write(MPU6000_SMPRT_DIV, 0);

	/* Disable filtering */
	ao_mpu6000_reg_write(MPU6000_CONFIG,
			     (MPU6000_CONFIG_EXT_SYNC_SET_DISABLED << MPU6000_CONFIG_EXT_SYNC_SET) |
			     (MPU6000_CONFIG_DLPF_CFG_260_256 << MPU6000_CONFIG_DLPF_CFG));

	/* Configure accelerometer to +/-16G in self-test mode */
	ao_mpu6000_reg_write(MPU6000_ACCEL_CONFIG,
			     (1 << MPU600_ACCEL_CONFIG_XA_ST) |
			     (1 << MPU600_ACCEL_CONFIG_YA_ST) |
			     (1 << MPU600_ACCEL_CONFIG_ZA_ST) |
			     (MPU600_ACCEL_CONFIG_AFS_SEL_16G << MPU600_ACCEL_CONFIG_AFS_SEL));

	/* Configure gyro to +/- 2000°/s in self-test mode */
	ao_mpu6000_reg_write(MPU6000_GYRO_CONFIG,
			     (1 << MPU600_GYRO_CONFIG_XG_ST) |
			     (1 << MPU600_GYRO_CONFIG_YG_ST) |
			     (1 << MPU600_GYRO_CONFIG_ZG_ST) |
			     (MPU600_GYRO_CONFIG_FS_SEL_2000 << MPU600_GYRO_CONFIG_FS_SEL));

	ao_delay(AO_MS_TO_TICKS(200));
	ao_mpu6000_sample(&test_mode);

#if TRIDGE
	// read the product ID rev c has 1/2 the sensitivity of rev d
    _mpu6000_product_id = _register_read(MPUREG_PRODUCT_ID);
    //Serial.printf("Product_ID= 0x%x\n", (unsigned) _mpu6000_product_id);

    if ((_mpu6000_product_id == MPU6000ES_REV_C4) || (_mpu6000_product_id == MPU6000ES_REV_C5) ||
        (_mpu6000_product_id == MPU6000_REV_C4) || (_mpu6000_product_id == MPU6000_REV_C5)) {
        // Accel scale 8g (4096 LSB/g)
        // Rev C has different scaling than rev D
        register_write(MPUREG_ACCEL_CONFIG,1<<3);
    } else {
        // Accel scale 8g (4096 LSB/g)
        register_write(MPUREG_ACCEL_CONFIG,2<<3);
    }
    hal.scheduler->delay(1);

#endif

	/* Configure accelerometer to +/-16G */
	ao_mpu6000_reg_write(MPU6000_ACCEL_CONFIG,
			     (0 << MPU600_ACCEL_CONFIG_XA_ST) |
			     (0 << MPU600_ACCEL_CONFIG_YA_ST) |
			     (0 << MPU600_ACCEL_CONFIG_ZA_ST) |
			     (MPU600_ACCEL_CONFIG_AFS_SEL_16G << MPU600_ACCEL_CONFIG_AFS_SEL));

	/* Configure gyro to +/- 2000°/s */
	ao_mpu6000_reg_write(MPU6000_GYRO_CONFIG,
			     (0 << MPU600_GYRO_CONFIG_XG_ST) |
			     (0 << MPU600_GYRO_CONFIG_YG_ST) |
			     (0 << MPU600_GYRO_CONFIG_ZG_ST) |
			     (MPU600_GYRO_CONFIG_FS_SEL_2000 << MPU600_GYRO_CONFIG_FS_SEL));

	ao_delay(AO_MS_TO_TICKS(10));
	ao_mpu6000_sample(&normal_mode);
	
	errors += ao_mpu6000_accel_check(normal_mode.accel_x, test_mode.accel_x, "x");
	errors += ao_mpu6000_accel_check(normal_mode.accel_y, test_mode.accel_y, "y");
	errors += ao_mpu6000_accel_check(normal_mode.accel_z, test_mode.accel_z, "z");

	errors += ao_mpu6000_gyro_check(normal_mode.gyro_x, test_mode.gyro_x, "x");
	errors += ao_mpu6000_gyro_check(normal_mode.gyro_y, test_mode.gyro_y, "y");
	errors += ao_mpu6000_gyro_check(normal_mode.gyro_z, test_mode.gyro_z, "z");

	if (errors)
		ao_panic(AO_PANIC_SELF_TEST_MPU6000);

	/* Filter to about 100Hz, which also sets the gyro rate to 1000Hz */
	ao_mpu6000_reg_write(MPU6000_CONFIG,
			     (MPU6000_CONFIG_EXT_SYNC_SET_DISABLED << MPU6000_CONFIG_EXT_SYNC_SET) |
			     (MPU6000_CONFIG_DLPF_CFG_94_98 << MPU6000_CONFIG_DLPF_CFG));

	/* Set sample rate divider to sample at 200Hz (v = gyro/rate - 1) */
	ao_mpu6000_reg_write(MPU6000_SMPRT_DIV,
			     1000 / 200 - 1);
	
	ao_delay(AO_MS_TO_TICKS(100));
	ao_mpu6000_configured = 1;
}

struct ao_mpu6000_sample	ao_mpu6000_current;

static void
ao_mpu6000(void)
{
	ao_mpu6000_setup();
	for (;;)
	{
		ao_mpu6000_sample(&ao_mpu6000_current);
		ao_arch_critical(
			AO_DATA_PRESENT(AO_DATA_MPU6000);
			AO_DATA_WAIT();
			);
	}
}

static struct ao_task ao_mpu6000_task;

static void
ao_mpu6000_show(void)
{
	struct ao_data	sample;

	ao_data_get(&sample);
	printf ("Accel: %7d %7d %7d Gyro: %7d %7d %7d\n",
		sample.mpu6000.accel_x,
		sample.mpu6000.accel_y,
		sample.mpu6000.accel_z,
		sample.mpu6000.gyro_x,
		sample.mpu6000.gyro_y,
		sample.mpu6000.gyro_z);
}

static const struct ao_cmds ao_mpu6000_cmds[] = {
	{ ao_mpu6000_show,	"I\0Show MPU6000 status" },
	{ 0, NULL }
};

void
ao_mpu6000_init(void)
{
	ao_mpu6000_configured = 0;

	ao_add_task(&ao_mpu6000_task, ao_mpu6000, "mpu6000");
#ifndef AO_MPU6000_I2C_INDEX
	ao_spi_init_cs(AO_MPU6000_SPI_CS_PORT, (1 << AO_MPU6000_SPI_CS_PIN));
#endif	
	ao_cmd_register(&ao_mpu6000_cmds[0]);
}
#endif
