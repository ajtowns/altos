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

#if HAS_MMA655X

#if 1
#define PRINTD(...) do { printf ("\r%5u %s: ", ao_tick_count, __func__); printf(__VA_ARGS__); } while(0)
#else
#define PRINTD(...) 
#endif

static uint8_t	mma655x_configured;

uint8_t	ao_mma655x_spi_index = AO_MMA655X_SPI_INDEX;

static void
ao_mma655x_start(void) {
	ao_spi_get_bit(AO_MMA655X_CS_PORT,
		       AO_MMA655X_CS_PIN,
		       AO_MMA655X_CS,
		       AO_MMA655X_SPI_INDEX,
		       AO_SPI_SPEED_FAST);
}

static void
ao_mma655x_stop(void) {
	ao_spi_put_bit(AO_MMA655X_CS_PORT,
		       AO_MMA655X_CS_PIN,
		       AO_MMA655X_CS,
		       AO_MMA655X_SPI_INDEX);
}

static void
ao_mma655x_restart(void) {
	uint8_t	i;
	ao_gpio_set(AO_MMA655X_CS_PORT, AO_MMA655X_CS_PIN, AO_MMA655X_CS, 1);

	/* Emperical testing on STM32L151 at 32MHz for this delay amount */
	for (i = 0; i < 9; i++)
		ao_arch_nop();
	ao_gpio_set(AO_MMA655X_CS_PORT, AO_MMA655X_CS_PIN, AO_MMA655X_CS, 0);
}

static uint8_t
ao_parity(uint8_t v)
{
	uint8_t	p;
	/* down to four bits */
	p = (v ^ (v >> 4)) & 0xf;

	/* Cute lookup hack -- 0x6996 encodes the sixteen
	 * even parity values in order.
	 */
	p = (~0x6996 >> p) & 1;
	return p;
}

static void
ao_mma655x_cmd(uint8_t d[2])
{
	ao_mma655x_start();
	PRINTD("\tSEND %02x %02x\n", d[0], d[1]);
	ao_spi_duplex(d, d, 2, AO_MMA655X_SPI_INDEX);
	PRINTD("\t\tRECV %02x %02x\n", d[0], d[1]);
	ao_mma655x_stop();
}

static uint8_t
ao_mma655x_reg_read(uint8_t addr)
{
	uint8_t	d[2];
	ao_mma655x_start();
	d[0] = addr | (ao_parity(addr) << 7);
	d[1] = 0;
	ao_spi_send(&d, 2, AO_MMA655X_SPI_INDEX);
	ao_mma655x_restart();

	/* Send a dummy read of 00 to clock out the SPI data */
	d[0] = 0x80;
	d[1] = 0x00;
	ao_spi_duplex(&d, &d, 2, AO_MMA655X_SPI_INDEX);
	ao_mma655x_stop();
	return d[1];
}

static void
ao_mma655x_reg_write(uint8_t addr, uint8_t value)
{
	uint8_t	d[2];

	addr |= (1 << 6);	/* write mode */
	d[0] = addr | (ao_parity(addr^value) << 7);
	d[1] = value;
	ao_mma655x_start();
	ao_spi_send(d, 2, AO_MMA655X_SPI_INDEX);
	ao_mma655x_stop();

	addr &= ~(1 << 6);
	PRINTD("write %x %x = %x\n",
	       addr, value, ao_mma655x_reg_read(addr));
}

static uint16_t
ao_mma655x_value(void)
{
	uint8_t		d[2];
	uint16_t	v;

	d[0] = ((0 << 6) |	/* Axis selection (X) */
		(1 << 5) |	/* Acceleration operation */
		(1 << 4));	/* Raw data */
	d[1] = ((1 << 3) |	/* must be one */
		(1 << 2) |	/* Unsigned data */
		(0 << 1) |	/* Arm disabled */
		(1 << 0));	/* Odd parity */
	ao_mma655x_start();
	PRINTD("value SEND %02x %02x\n", d[0], d[1]);
	ao_spi_send(d, 2, AO_MMA655X_SPI_INDEX);
	ao_mma655x_restart();
	d[0] = 0x80;
	d[1] = 0x00;
	ao_spi_duplex(d, d, 2, AO_MMA655X_SPI_INDEX);
	ao_mma655x_stop();
	PRINTD("value RECV %02x %02x\n", d[0], d[1]);

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
	uint8_t		i;
	uint8_t	s0, s1, s2, s3;
	uint8_t	pn;
	uint32_t	lot;
	uint16_t	serial;


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
	for (i = 0; i < 10; i++) {
		a_st = ao_mma655x_value();
		printf ("SELF-TEST %2d = %6d\n", i, a_st);
	}

	stdefl = ao_mma655x_reg_read(AO_MMA655X_STDEFL);

	ao_mma655x_reg_write(AO_MMA655X_AXISCFG,
			     AXISCFG_VALUE |
			     (0 << AO_MMA655X_AXISCFG_ST));
	a = ao_mma655x_value();

	for (i = 0; i < 10; i++) {
		a = ao_mma655x_value();
		printf("NORMAL   %2d = %6d\n", i, a);
	}

	ao_mma655x_reg_write(AO_MMA655X_DEVCFG,
			     DEVCFG_VALUE | (1 << AO_MMA655X_DEVCFG_ENDINIT));
	s0 = ao_mma655x_reg_read(AO_MMA655X_SN0);
	s1 = ao_mma655x_reg_read(AO_MMA655X_SN1);
	s2 = ao_mma655x_reg_read(AO_MMA655X_SN2);
	s3 = ao_mma655x_reg_read(AO_MMA655X_SN3);
	lot = ((uint32_t) s3 << 24) | ((uint32_t) s2 << 16) |
		((uint32_t) s1 << 8) | ((uint32_t) s0);
	serial = lot & 0x1fff;
	lot >>= 12;
	pn = ao_mma655x_reg_read(AO_MMA655X_PN);
	printf ("MMA655X lot %d serial %d number %d\n", lot, serial, pn);

}

static void
ao_mma655x_dump(void)
{
	ao_mma655x_setup();
	printf ("MMA655X value %d\n", ao_mma655x_value());
}

__code struct ao_cmds ao_mma655x_cmds[] = {
	{ ao_mma655x_dump,	"A\0Display MMA655X data" },
	{ 0, NULL },
};

static void
ao_mma655x(void)
{
	ao_mma655x_setup();
	for (;;) {
		ao_data_ring[ao_data_head].mma655x = ao_mma655x_value();
		ao_arch_critical(
			AO_DATA_PRESENT(AO_DATA_MMA655X);
			AO_DATA_WAIT();
			);
	}
}

static __xdata struct ao_task ao_mma655x_task;

void
ao_mma655x_init(void)
{
	mma655x_configured = 0;

	ao_cmd_register(&ao_mma655x_cmds[0]);
	ao_spi_init_cs(AO_MMA655X_CS_PORT, (1 << AO_MMA655X_CS_PIN));

//	ao_add_task(&ao_mma655x_task, ao_mma655x, "mma655x");
}

#endif
