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

#ifndef _AO_MPU6000_H_
#define _AO_MPU6000_H_

#ifndef M_PI
#define M_PI 3.1415926535897832384626433
#endif

#define MPU6000_ADDR_WRITE	0xd0
#define MPU6000_ADDR_READ	0xd1

/* From Tridge */
#define MPUREG_XG_OFFS_TC 0x00
#define MPUREG_YG_OFFS_TC 0x01
#define MPUREG_ZG_OFFS_TC 0x02
#define MPUREG_X_FINE_GAIN 0x03
#define MPUREG_Y_FINE_GAIN 0x04
#define MPUREG_Z_FINE_GAIN 0x05
#define MPUREG_XA_OFFS_H 0x06 // X axis accelerometer offset (high byte)
#define MPUREG_XA_OFFS_L 0x07 // X axis accelerometer offset (low byte)
#define MPUREG_YA_OFFS_H 0x08 // Y axis accelerometer offset (high byte)
#define MPUREG_YA_OFFS_L 0x09 // Y axis accelerometer offset (low byte)
#define MPUREG_ZA_OFFS_H 0x0A // Z axis accelerometer offset (high byte)
#define MPUREG_ZA_OFFS_L 0x0B // Z axis accelerometer offset (low byte)
#define MPUREG_PRODUCT_ID 0x0C // Product ID Register
#define MPUREG_XG_OFFS_USRH 0x13 // X axis gyro offset (high byte)
#define MPUREG_XG_OFFS_USRL 0x14 // X axis gyro offset (low byte)
#define MPUREG_YG_OFFS_USRH 0x15 // Y axis gyro offset (high byte)
#define MPUREG_YG_OFFS_USRL 0x16 // Y axis gyro offset (low byte)
#define MPUREG_ZG_OFFS_USRH 0x17 // Z axis gyro offset (high byte)
#define MPUREG_ZG_OFFS_USRL 0x18 // Z axis gyro offset (low byte)

#define MPU6000_SMPRT_DIV	0x19

#define MPU6000_CONFIG		0x1a

#define  MPU6000_CONFIG_EXT_SYNC_SET	3
#define  MPU6000_CONFIG_EXT_SYNC_SET_DISABLED		0
#define  MPU6000_CONFIG_EXT_SYNC_SET_TEMP_OUT_L		1
#define  MPU6000_CONFIG_EXT_SYNC_SET_GYRO_XOUT_L	2
#define  MPU6000_CONFIG_EXT_SYNC_SET_GYRO_YOUT_L	3
#define  MPU6000_CONFIG_EXT_SYNC_SET_GYRO_ZOUT_L	4
#define  MPU6000_CONFIG_EXT_SYNC_SET_ACCEL_XOUT_L	5
#define  MPU6000_CONFIG_EXT_SYNC_SET_ACCEL_YOUT_L	6
#define  MPU6000_CONFIG_EXT_SYNC_SET_ACCEL_ZOUT_L	7
#define  MPU6000_CONFIG_EXT_SYNC_SET_MASK		7

#define  MPU6000_CONFIG_DLPF_CFG	0
#define  MPU6000_CONFIG_DLPF_CFG_260_256		0
#define  MPU6000_CONFIG_DLPF_CFG_184_188		1
#define  MPU6000_CONFIG_DLPF_CFG_94_98			2
#define  MPU6000_CONFIG_DLPF_CFG_44_42			3
#define  MPU6000_CONFIG_DLPF_CFG_21_20			4
#define  MPU6000_CONFIG_DLPF_CFG_10_10			5
#define  MPU6000_CONFIG_DLPF_CFG_5_5			6
#define  MPU6000_CONFIG_DLPF_CFG_MASK			7

#define MPU6000_GYRO_CONFIG	0x1b
# define MPU600_GYRO_CONFIG_XG_ST	7
# define MPU600_GYRO_CONFIG_YG_ST	6
# define MPU600_GYRO_CONFIG_ZG_ST	5
# define MPU600_GYRO_CONFIG_FS_SEL	3
# define MPU600_GYRO_CONFIG_FS_SEL_250		0
# define MPU600_GYRO_CONFIG_FS_SEL_500		1
# define MPU600_GYRO_CONFIG_FS_SEL_1000		2
# define MPU600_GYRO_CONFIG_FS_SEL_2000		3
# define MPU600_GYRO_CONFIG_FS_SEL_MASK		3

#define MPU6000_ACCEL_CONFIG	0x1c
# define MPU600_ACCEL_CONFIG_XA_ST	7
# define MPU600_ACCEL_CONFIG_YA_ST	6
# define MPU600_ACCEL_CONFIG_ZA_ST	5
# define MPU600_ACCEL_CONFIG_AFS_SEL	3
# define MPU600_ACCEL_CONFIG_AFS_SEL_2G		0
# define MPU600_ACCEL_CONFIG_AFS_SEL_4G		1
# define MPU600_ACCEL_CONFIG_AFS_SEL_8G		2
# define MPU600_ACCEL_CONFIG_AFS_SEL_16G	3
# define MPU600_ACCEL_CONFIG_AFS_SEL_MASK	3
# define MPU600_ACCEL_CONFIG_ACCEL_HPF	0
# define MPU600_ACCEL_CONFIG_ACCEL_HPF_RESET	0
# define MPU600_ACCEL_CONFIG_ACCEL_HPF_5Hz	1
# define MPU600_ACCEL_CONFIG_ACCEL_HPF_2_5Hz	2
# define MPU600_ACCEL_CONFIG_ACCEL_HPF_1_25Hz	3
# define MPU600_ACCEL_CONFIG_ACCEL_HPF_0_63Hz	4
# define MPU600_ACCEL_CONFIG_ACCEL_HPF_HOLD	7
# define MPU600_ACCEL_CONFIG_ACCEL_HPF_MASK	7

#define MPU6000_INT_ENABLE	0x38
#define  MPU6000_INT_ENABLE_FF_EN		7
#define  MPU6000_INT_ENABLE_MOT_EN		6
#define  MPU6000_INT_ENABLE_ZMOT_EN		5
#define  MPU6000_INT_ENABLE_FIFO_OFLOW_EN	4
#define  MPU6000_INT_ENABLE_I2C_MST_INT_EN	3
#define  MPU6000_INT_ENABLE_DATA_RDY_EN		0

#define MPU6000_INT_STATUS	0x3a
#define  MPU6000_INT_STATUS_FF_EN		7
#define  MPU6000_INT_STATUS_MOT_EN		6
#define  MPU6000_INT_STATUS_ZMOT_EN		5
#define  MPU6000_INT_STATUS_FIFO_OFLOW_EN	4
#define  MPU6000_INT_STATUS_I2C_MST_INT_EN	3
#define  MPU6000_INT_STATUS_DATA_RDY_EN		0

#define MPU6000_ACCEL_XOUT_H		0x3b
#define MPU6000_ACCEL_XOUT_L		0x3c
#define MPU6000_ACCEL_YOUT_H		0x3d
#define MPU6000_ACCEL_YOUT_L		0x3e
#define MPU6000_ACCEL_ZOUT_H		0x3f
#define MPU6000_ACCEL_ZOUT_L		0x40
#define MPU6000_TEMP_H			0x41
#define MPU6000_TEMP_L			0x42
#define MPU6000_GYRO_XOUT_H		0x43
#define MPU6000_GYRO_XOUT_L		0x44
#define MPU6000_GYRO_YOUT_H		0x45
#define MPU6000_GYRO_YOUT_L		0x46
#define MPU6000_GYRO_ZOUT_H		0x47
#define MPU6000_GYRO_ZOUT_L		0x48

#define MPU6000_SIGNAL_PATH_RESET	0x68
#define MPU6000_SIGNAL_PATH_RESET_GYRO_RESET	2
#define MPU6000_SIGNAL_PATH_RESET_ACCEL_RESET	1
#define MPU6000_SIGNAL_PATH_RESET_TEMP_RESET	0

#define MPU6000_USER_CTRL		0x6a
#define MPU6000_USER_CTRL_FIFO_EN		6
#define MPU6000_USER_CTRL_I2C_MST_EN		5
#define MPU6000_USER_CTRL_I2C_IF_DIS		4
#define MPU6000_USER_CTRL_FIFO_RESET		2
#define MPU6000_USER_CTRL_I2C_MST_RESET		1
#define MPU6000_USER_CTRL_SIG_COND_RESET	0

#define MPU6000_PWR_MGMT_1	0x6b
#define MPU6000_PWR_MGMT_1_DEVICE_RESET		7
#define MPU6000_PWR_MGMT_1_SLEEP		6
#define MPU6000_PWR_MGMT_1_CYCLE		5
#define MPU6000_PWR_MGMT_1_TEMP_DIS		3
#define MPU6000_PWR_MGMT_1_CLKSEL		0
#define MPU6000_PWR_MGMT_1_CLKSEL_INTERNAL		0
#define MPU6000_PWR_MGMT_1_CLKSEL_PLL_X_AXIS		1
#define MPU6000_PWR_MGMT_1_CLKSEL_PLL_Y_AXIS		2
#define MPU6000_PWR_MGMT_1_CLKSEL_PLL_Z_AXIS		3
#define MPU6000_PWR_MGMT_1_CLKSEL_PLL_EXTERNAL_32K	4
#define MPU6000_PWR_MGMT_1_CLKSEL_PLL_EXTERNAL_19M	5
#define MPU6000_PWR_MGMT_1_CLKSEL_STOP			7
#define MPU6000_PWR_MGMT_1_CLKSEL_MASK			7

#define MPU6000_PWR_MGMT_2	0x6c

#define MPU6000_WHO_AM_I	0x75

/* Self test acceleration is approximately 0.5g */
#define MPU6000_ST_ACCEL(full_scale)	(32767 / ((full_scale) * 2))

/* Self test gyro is approximately 50°/s */
#define MPU6000_ST_GYRO(full_scale)	((int16_t) (((int32_t) 32767 * (int32_t) 50) / (full_scale)))

#define MPU6000_GYRO_FULLSCALE	((float) 2000 * M_PI/180.0)

static inline float
ao_mpu6000_gyro(float sensor) {
	return sensor * ((float) (MPU6000_GYRO_FULLSCALE / 32767.0));
}

#define MPU6000_ACCEL_FULLSCALE	16

static inline float
ao_mpu6000_accel(int16_t sensor) {
	return (float) sensor * ((float) (MPU6000_ACCEL_FULLSCALE * GRAVITY / 32767.0));
}

struct ao_mpu6000_sample {
	int16_t		accel_x;
	int16_t		accel_y;
	int16_t		accel_z;
	int16_t		temp;
	int16_t		gyro_x;
	int16_t		gyro_y;
	int16_t		gyro_z;
};

extern struct ao_mpu6000_sample	ao_mpu6000_current;

void
ao_mpu6000_init(void);

/* Product ID Description for MPU6000
 * high 4 bits low 4 bits
 * Product Name Product Revision
 */
#define MPU6000ES_REV_C4 0x14	/* 0001 0100 */
#define MPU6000ES_REV_C5 0x15	/* 0001 0101 */
#define MPU6000ES_REV_D6 0x16	/* 0001 0110 */
#define MPU6000ES_REV_D7 0x17	/* 0001 0111 */
#define MPU6000ES_REV_D8 0x18	/* 0001 1000 */
#define MPU6000_REV_C4 0x54	/* 0101 0100 */
#define MPU6000_REV_C5 0x55	/* 0101 0101 */
#define MPU6000_REV_D6 0x56	/* 0101 0110 */
#define MPU6000_REV_D7 0x57	/* 0101 0111 */
#define MPU6000_REV_D8 0x58	/* 0101 1000 */
#define MPU6000_REV_D9 0x59	/* 0101 1001 */

#endif /* _AO_MPU6000_H_ */
