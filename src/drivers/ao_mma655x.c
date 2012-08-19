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
#include <ao_mma655x.h>

static uint8_t	mma655x_configured;
uint8_t		ao_mma655x_valid;

static void
ao_mma655x_start(void) {
	ao_spi_get_bit(AO_MMA655X_CS_GPIO,
		       AO_MMA655X_CS,
		       AO_MMA655X_CS_PIN,
		       AO_MMA655X_SPI_INDEX,
		       AO_SPI_SPEED_FAST);
}

static void
ao_mma655x_stop(void) {
	ao_spi_put_bit(AO_MMA655X_CS_GPIO,
		       AO_MMA655X_CS,
		       AO_MMA655X_CS_PIN,
		       AO_MMA655X_SPI_INDEX);
}

static uint8_t
ao_parity(uint8_t v)
{
	/* down to four bits */
	v = (v ^ (v >> 4)) & 0xf;

	/* Cute lookup hack -- 0x6996 encodes the sixteen
	 * even parity values in order.
	 */
	return (~0x6996 >> v) & 1;
}

static void
ao_mma655x_cmd(uint8_t d[2])
{
	ao_mma655x_start();
	ao_spi_send(d, 2, AO_MMA655X_SPI_INDEX);
	ao_spi_recv(d, 2, AO_MMA655X_SPI_INDEX);
	ao_mma655x_stop();
}

static uint8_t
ao_mma655x_reg_write(uint8_t addr, uint8_t value)
{
	uint8_t	d[2];

	addr |= (1 << 6);	/* write mode */
	d[0] = addr | (ao_parity(addr^value) << 7);
	d[1] = value;
	ao_mma655x_cmd(d);
	return d[1];
}

static uint8_t
ao_mma655x_reg_read(uint8_t addr)
{
	uint8_t	d[2];

	d[0] = addr | (ao_parity(addr) << 7);
	d[1] = 0;
	ao_mma655x_cmd(d);
	return d[1];
}

static uint16_t
ao_mma655x_value(void)
{
	uint8_t		d[2];
	uint16_t	v;

	d[0] = ((0 << 7) |	/* Axis selection (X) */
		(1 << 6) |	/* Acceleration operation */
		(1 << 5));	/* Raw data */
	d[1] = ((1 << 3) |	/* must be one */
		(1 << 2) |	/* Unsigned data */
		(0 << 1) |	/* Arm disabled */
		(1 << 0));	/* Odd parity */
	ao_mma655x_cmd(d);
	v = (uint16_t) d[1] << 2;
	v |= d[0] >> 6;
	v |= (uint16_t) (d[0] & 3) << 10;
	return v;
}

static void
ao_mma655x_reset(void) {
	ao_mma655x_reg_write(AO_MMA655X_DEVCTL,
			     (0 << AO_MMA655X_DEVCTL_RES_1) |
			     (0 << AO_MMA655X_DEVCTL_RES_1));
	ao_mma655x_reg_write(AO_MMA655X_DEVCTL,
			     (1 << AO_MMA655X_DEVCTL_RES_1) |
			     (1 << AO_MMA655X_DEVCTL_RES_1));
	ao_mma655x_reg_write(AO_MMA655X_DEVCTL,
			     (0 << AO_MMA655X_DEVCTL_RES_1) |
			     (1 << AO_MMA655X_DEVCTL_RES_1));
}

#define DEVCFG_VALUE	(\
	(1 << AO_MMA655X_DEVCFG_OC) |		/* Disable offset cancelation */ \
	(1 << AO_MMA655X_DEVCFG_SD) |		/* Receive unsigned data */ \
	(0 << AO_MMA655X_DEVCFG_OFMON) |	/* Disable offset monitor */ \
	(AO_MMA655X_DEVCFG_A_CFG_DISABLE << AO_MMA655X_DEVCFG_A_CFG))

#define AXISCFG_VALUE	(\
		(0 << AO_MMA655X_AXISCFG_LPF))	/* 100Hz 4-pole filter */


static void
ao_mma655x_setup(void)
{
	uint8_t		v;
	uint16_t	a, a_st;
	uint8_t		stdefl;

	if (mma655x_configured)
		return;
	mma655x_configured = 1;
	ao_delay(AO_MS_TO_TICKS(10));	/* Top */
	ao_mma655x_reset();
	ao_delay(AO_MS_TO_TICKS(10));	/* Top */
	(void) ao_mma655x_reg_read(AO_MMA655X_DEVSTAT);
	v = ao_mma655x_reg_read(AO_MMA655X_DEVSTAT);

	/* Configure R/W register values.
	 * Most of them relate to the arming feature, which
	 * we don't use, so the only registers we need to
	 * write are DEVCFG and AXISCFG
	 */

	ao_mma655x_reg_write(AO_MMA655X_DEVCFG,
			     DEVCFG_VALUE | (0 << AO_MMA655X_DEVCFG_ENDINIT));

	/* Test X axis
	 */
	
	ao_mma655x_reg_write(AO_MMA655X_AXISCFG,
			     AXISCFG_VALUE |
			     (1 << AO_MMA655X_AXISCFG_ST));
	a_st = ao_mma655x_value();

	stdefl = ao_mma655x_reg_read(AO_MMA655X_STDEFL);

	ao_mma655x_reg_write(AO_MMA655X_AXISCFG,
			     AXISCFG_VALUE |
			     (0 << AO_MMA655X_AXISCFG_ST));
	a = ao_mma655x_value();
	printf ("normal: %u self_test: %u stdefl: %u\n",
		a, a_st, stdefl);

	ao_mma655x_reg_write(AO_MMA655X_DEVCFG,
			     DEVCFG_VALUE | (1 << AO_MMA655X_DEVCFG_ENDINIT));
}

static void
ao_mma655x_dump(void)
{
	uint8_t	s0, s1, s2, s3;
	uint32_t	lot;
	uint16_t	serial;

	ao_mma655x_setup();

	s0 = ao_mma655x_reg_read(AO_MMA655X_SN0);
	s1 = ao_mma655x_reg_read(AO_MMA655X_SN1);
	s2 = ao_mma655x_reg_read(AO_MMA655X_SN2);
	s3 = ao_mma655x_reg_read(AO_MMA655X_SN3);
	lot = ((uint32_t) s3 << 24) | ((uint32_t) s2 << 16) |
		((uint32_t) s1 << 8) | ((uint32_t) s0);
	serial = lot & 0x1fff;
	lot >>= 12;
	printf ("MMA655X lot %d serial %d\n", lot, serial);
	mma655x_configured = 0;
}

__code struct ao_cmds ao_mma655x_cmds[] = {
	{ ao_mma655x_dump,	"A\0Display MMA655X data" },
	{ 0, NULL },
};

void
ao_mma655x_init(void)
{
	mma655x_configured = 0;
	ao_mma655x_valid = 0;

	ao_cmd_register(&ao_mma655x_cmds[0]);
	ao_spi_init_cs(AO_MMA655X_CS_GPIO, (1 << AO_MMA655X_CS));

//	ao_add_task(&ao_mma655x_task, ao_mma655x, "mma655x");
}
