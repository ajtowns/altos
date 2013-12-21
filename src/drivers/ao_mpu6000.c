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

static uint8_t	ao_mpu6000_configured;

#ifndef AO_MPU6000_I2C_INDEX
#define AO_MPU6000_SPI	1
#else
#define AO_MPU6000_SPI	0
#endif

#if AO_MPU6000_SPI

#define ao_mpu6000_spi_get()	ao_spi_get(AO_MPU6000_SPI_BUS, AO_SPI_SPEED_1MHz)
#define ao_mpu6000_spi_put()	ao_spi_put(AO_MPU6000_SPI_BUS)

#define ao_mpu6000_spi_start() 	ao_spi_set_cs(AO_MPU6000_SPI_CS_PORT,	\
					      (1 << AO_MPU6000_SPI_CS_PIN))

#define ao_mpu6000_spi_end() 	ao_spi_clr_cs(AO_MPU6000_SPI_CS_PORT,	\
					      (1 << AO_MPU6000_SPI_CS_PIN))

#endif


static void
_ao_mpu6000_reg_write(uint8_t addr, uint8_t value)
{
	uint8_t	d[2] = { addr, value };
#if AO_MPU6000_SPI
	ao_mpu6000_spi_start();
	ao_spi_send(d, 2, AO_MPU6000_SPI_BUS);
	ao_mpu6000_spi_end();
#else
	ao_i2c_get(AO_MPU6000_I2C_INDEX);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_WRITE);
	ao_i2c_send(d, 2, AO_MPU6000_I2C_INDEX, TRUE);
	ao_i2c_put(AO_MPU6000_I2C_INDEX);
#endif
}

static void
_ao_mpu6000_read(uint8_t addr, void *data, uint8_t len)
{
#if AO_MPU6000_SPI
	addr |= 0x80;
	ao_mpu6000_spi_start();
	ao_spi_send(&addr, 1, AO_MPU6000_SPI_BUS);
	ao_spi_recv(data, len, AO_MPU6000_SPI_BUS);
	ao_mpu6000_spi_end();
#else
	ao_i2c_get(AO_MPU6000_I2C_INDEX);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_WRITE);
	ao_i2c_send(&addr, 1, AO_MPU6000_I2C_INDEX, FALSE);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_READ);
	ao_i2c_recv(data, len, AO_MPU6000_I2C_INDEX, TRUE);
	ao_i2c_put(AO_MPU6000_I2C_INDEX);
#endif
}

static uint8_t
_ao_mpu6000_reg_read(uint8_t addr)
{
	uint8_t	value;
#if AO_MPU6000_SPI
	addr |= 0x80;
	ao_mpu6000_spi_start();
	ao_spi_send(&addr, 1, AO_MPU6000_SPI_BUS);
	ao_spi_recv(&value, 1, AO_MPU6000_SPI_BUS);
	ao_mpu6000_spi_end();
#else
	ao_i2c_get(AO_MPU6000_I2C_INDEX);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_WRITE);
	ao_i2c_send(&addr, 1, AO_MPU6000_I2C_INDEX, FALSE);
	ao_i2c_start(AO_MPU6000_I2C_INDEX, MPU6000_ADDR_READ);
	ao_i2c_recv(&value, 1, AO_MPU6000_I2C_INDEX, TRUE);
	ao_i2c_put(AO_MPU6000_I2C_INDEX);
#endif
	return value;
}

static void
_ao_mpu6000_sample(struct ao_mpu6000_sample *sample)
{
	uint16_t	*d = (uint16_t *) sample;
	int		i = sizeof (*sample) / 2;

	_ao_mpu6000_read(MPU6000_ACCEL_XOUT_H, sample, sizeof (*sample));
#if __BYTE_ORDER == __LITTLE_ENDIAN
	/* byte swap */
	while (i--) {
		uint16_t	t = *d;
		*d++ = (t >> 8) | (t << 8);
	}
#endif
}

#define G	981	/* in cm/s² */

#if 0
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
#endif

static uint8_t
ao_mpu6000_accel_check(int16_t normal, int16_t test)
{
	int16_t	diff = test - normal;

	if (diff < MPU6000_ST_ACCEL(16) / 4) {
		return 1;
	}
	if (diff > MPU6000_ST_ACCEL(16) * 4) {
		return 1;
	}
	return 0;
}

static uint8_t
ao_mpu6000_gyro_check(int16_t normal, int16_t test)
{
	int16_t	diff = test - normal;

	if (diff < 0)
		diff = -diff;
	if (diff < MPU6000_ST_GYRO(2000) / 4) {
		return 1;
	}
	if (diff > MPU6000_ST_GYRO(2000) * 4) {
		return 1;
	}
	return 0;
}

static void
_ao_mpu6000_wait_alive(void)
{
	uint8_t	i;

	/* Wait for the chip to wake up */
	for (i = 0; i < 30; i++) {
		ao_delay(AO_MS_TO_TICKS(100));
		if (_ao_mpu6000_reg_read(MPU6000_WHO_AM_I) == 0x68)
			break;
	}
	if (i == 30)
		ao_panic(AO_PANIC_SELF_TEST_MPU6000);
}

#define ST_TRIES	10

static void
_ao_mpu6000_setup(void)
{
	struct ao_mpu6000_sample	normal_mode, test_mode;
	int				errors;
	int				st_tries;

	if (ao_mpu6000_configured)
		return;

	_ao_mpu6000_wait_alive();

	/* Reset the whole chip */
	
	_ao_mpu6000_reg_write(MPU6000_PWR_MGMT_1,
			      (1 << MPU6000_PWR_MGMT_1_DEVICE_RESET));

	/* Wait for it to reset. If we talk too quickly, it appears to get confused */

	_ao_mpu6000_wait_alive();

	/* Reset signal conditioning, disabling I2C on SPI systems */
	_ao_mpu6000_reg_write(MPU6000_USER_CTRL,
			      (0 << MPU6000_USER_CTRL_FIFO_EN) |
			      (0 << MPU6000_USER_CTRL_I2C_MST_EN) |
			      (AO_MPU6000_SPI << MPU6000_USER_CTRL_I2C_IF_DIS) |
			      (0 << MPU6000_USER_CTRL_FIFO_RESET) |
			      (0 << MPU6000_USER_CTRL_I2C_MST_RESET) |
			      (1 << MPU6000_USER_CTRL_SIG_COND_RESET));

	while (_ao_mpu6000_reg_read(MPU6000_USER_CTRL) & (1 << MPU6000_USER_CTRL_SIG_COND_RESET))
		ao_delay(AO_MS_TO_TICKS(10));

	/* Reset signal paths */
	_ao_mpu6000_reg_write(MPU6000_SIGNAL_PATH_RESET,
			      (1 << MPU6000_SIGNAL_PATH_RESET_GYRO_RESET) |
			      (1 << MPU6000_SIGNAL_PATH_RESET_ACCEL_RESET) |
			      (1 << MPU6000_SIGNAL_PATH_RESET_TEMP_RESET));

	_ao_mpu6000_reg_write(MPU6000_SIGNAL_PATH_RESET,
			      (0 << MPU6000_SIGNAL_PATH_RESET_GYRO_RESET) |
			      (0 << MPU6000_SIGNAL_PATH_RESET_ACCEL_RESET) |
			      (0 << MPU6000_SIGNAL_PATH_RESET_TEMP_RESET));

	/* Select clocks, disable sleep */
	_ao_mpu6000_reg_write(MPU6000_PWR_MGMT_1,
			      (0 << MPU6000_PWR_MGMT_1_DEVICE_RESET) |
			      (0 << MPU6000_PWR_MGMT_1_SLEEP) |
			      (0 << MPU6000_PWR_MGMT_1_CYCLE) |
			      (0 << MPU6000_PWR_MGMT_1_TEMP_DIS) |
			      (MPU6000_PWR_MGMT_1_CLKSEL_PLL_X_AXIS << MPU6000_PWR_MGMT_1_CLKSEL));

	/* Set sample rate divider to sample at full speed */
	_ao_mpu6000_reg_write(MPU6000_SMPRT_DIV, 0);

	/* Disable filtering */
	_ao_mpu6000_reg_write(MPU6000_CONFIG,
			      (MPU6000_CONFIG_EXT_SYNC_SET_DISABLED << MPU6000_CONFIG_EXT_SYNC_SET) |
			      (MPU6000_CONFIG_DLPF_CFG_260_256 << MPU6000_CONFIG_DLPF_CFG));

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

	for (st_tries = 0; st_tries < ST_TRIES; st_tries++) {
		errors = 0;

		/* Configure accelerometer to +/-16G in self-test mode */
		_ao_mpu6000_reg_write(MPU6000_ACCEL_CONFIG,
				      (1 << MPU600_ACCEL_CONFIG_XA_ST) |
				      (1 << MPU600_ACCEL_CONFIG_YA_ST) |
				      (1 << MPU600_ACCEL_CONFIG_ZA_ST) |
				      (MPU600_ACCEL_CONFIG_AFS_SEL_16G << MPU600_ACCEL_CONFIG_AFS_SEL));

		/* Configure gyro to +/- 2000°/s in self-test mode */
		_ao_mpu6000_reg_write(MPU6000_GYRO_CONFIG,
				      (1 << MPU600_GYRO_CONFIG_XG_ST) |
				      (1 << MPU600_GYRO_CONFIG_YG_ST) |
				      (1 << MPU600_GYRO_CONFIG_ZG_ST) |
				      (MPU600_GYRO_CONFIG_FS_SEL_2000 << MPU600_GYRO_CONFIG_FS_SEL));

		ao_delay(AO_MS_TO_TICKS(200));
		_ao_mpu6000_sample(&test_mode);

		/* Configure accelerometer to +/-16G */
		_ao_mpu6000_reg_write(MPU6000_ACCEL_CONFIG,
				      (0 << MPU600_ACCEL_CONFIG_XA_ST) |
				      (0 << MPU600_ACCEL_CONFIG_YA_ST) |
				      (0 << MPU600_ACCEL_CONFIG_ZA_ST) |
				      (MPU600_ACCEL_CONFIG_AFS_SEL_16G << MPU600_ACCEL_CONFIG_AFS_SEL));

		/* Configure gyro to +/- 2000°/s */
		_ao_mpu6000_reg_write(MPU6000_GYRO_CONFIG,
				      (0 << MPU600_GYRO_CONFIG_XG_ST) |
				      (0 << MPU600_GYRO_CONFIG_YG_ST) |
				      (0 << MPU600_GYRO_CONFIG_ZG_ST) |
				      (MPU600_GYRO_CONFIG_FS_SEL_2000 << MPU600_GYRO_CONFIG_FS_SEL));

		ao_delay(AO_MS_TO_TICKS(200));
		_ao_mpu6000_sample(&normal_mode);
	
		errors += ao_mpu6000_accel_check(normal_mode.accel_x, test_mode.accel_x);
		errors += ao_mpu6000_accel_check(normal_mode.accel_y, test_mode.accel_y);
		errors += ao_mpu6000_accel_check(normal_mode.accel_z, test_mode.accel_z);

		errors += ao_mpu6000_gyro_check(normal_mode.gyro_x, test_mode.gyro_x);
		errors += ao_mpu6000_gyro_check(normal_mode.gyro_y, test_mode.gyro_y);
		errors += ao_mpu6000_gyro_check(normal_mode.gyro_z, test_mode.gyro_z);
		if (!errors)
			break;
	}

	if (st_tries == ST_TRIES)
		ao_sensor_errors = 1;

	/* Filter to about 100Hz, which also sets the gyro rate to 1000Hz */
	_ao_mpu6000_reg_write(MPU6000_CONFIG,
			      (MPU6000_CONFIG_EXT_SYNC_SET_DISABLED << MPU6000_CONFIG_EXT_SYNC_SET) |
			      (MPU6000_CONFIG_DLPF_CFG_94_98 << MPU6000_CONFIG_DLPF_CFG));

	/* Set sample rate divider to sample at 200Hz (v = gyro/rate - 1) */
	_ao_mpu6000_reg_write(MPU6000_SMPRT_DIV,
			      1000 / 200 - 1);
	
	ao_delay(AO_MS_TO_TICKS(100));
	ao_mpu6000_configured = 1;
}

struct ao_mpu6000_sample	ao_mpu6000_current;

static void
ao_mpu6000(void)
{
	/* ao_mpu6000_init already grabbed the SPI bus and mutex */
	_ao_mpu6000_setup();
#if AO_MPU6000_SPI
	ao_mpu6000_spi_put();
#endif
	for (;;)
	{
#if AO_MPU6000_SPI
		ao_mpu6000_spi_get();
#endif
		_ao_mpu6000_sample(&ao_mpu6000_current);
#if AO_MPU6000_SPI
		ao_mpu6000_spi_put();
#endif	
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
	
#if AO_MPU6000_SPI
	ao_spi_init_cs(AO_MPU6000_SPI_CS_PORT, (1 << AO_MPU6000_SPI_CS_PIN));

	/* Pretend to be the mpu6000 task. Grab the SPI bus right away and
	 * hold it for the task so that nothing else uses the SPI bus before
	 * we get the I2C mode disabled in the chip
	 */

	ao_cur_task = &ao_mpu6000_task;
	ao_spi_get(AO_MPU6000_SPI_BUS, AO_SPI_SPEED_1MHz);
	ao_cur_task = NULL;
#endif	

	ao_cmd_register(&ao_mpu6000_cmds[0]);
}
#endif
