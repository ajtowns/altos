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
#include <ao_data.h>

#ifndef AO_ADC_0
#define AO_ADC_0	0
#endif

#ifndef AO_ADC_1
#define AO_ADC_1	0
#endif

#ifndef AO_ADC_2
#define AO_ADC_2	0
#endif

#ifndef AO_ADC_3
#define AO_ADC_3	0
#endif

#ifndef AO_ADC_4
#define AO_ADC_4	0
#endif

#ifndef AO_ADC_5
#define AO_ADC_5	0
#endif

#ifndef AO_ADC_6
#define AO_ADC_6	0
#endif

#ifndef AO_ADC_7
#define AO_ADC_7	0
#endif

#define AO_ADC_NUM	(AO_ADC_0 + AO_ADC_1 + AO_ADC_2 + AO_ADC_3 + \
			 AO_ADC_4 + AO_ADC_5 + AO_ADC_6 + AO_ADC_7)

/* ADC clock is divided by this value + 1, which ensures that
 * the ADC clock will be strictly less than 4.5MHz as required
 */
#define AO_ADC_CLKDIV	(AO_LPC_SYSCLK / 450000)

static uint8_t		ao_adc_ready;
static uint8_t		ao_adc_sequence;

static const uint8_t	ao_adc_mask_seq[AO_ADC_NUM] = {
#if AO_ADC_0
	1 << 0,
#endif
#if AO_ADC_1
	1 << 1,
#endif
#if AO_ADC_2
	1 << 2,
#endif
#if AO_ADC_3
	1 << 3,
#endif
#if AO_ADC_4
	1 << 4,
#endif
#if AO_ADC_5
	1 << 6,
#endif
#if AO_ADC_6
	1 << 6,
#endif
#if AO_ADC_7
	1 << 7,
#endif
};

#define sample(id)	(*out++ = (uint16_t) lpc_adc.dr[id] >> 1)

static inline void lpc_adc_start(void) {
	lpc_adc.cr = ((ao_adc_mask_seq[ao_adc_sequence] << LPC_ADC_CR_SEL) |
		      (AO_ADC_CLKDIV << LPC_ADC_CR_CLKDIV) |
		      (0 << LPC_ADC_CR_BURST) |
		      (LPC_ADC_CR_CLKS_11 << LPC_ADC_CR_CLKS) |
		      (LPC_ADC_CR_START_NOW << LPC_ADC_CR_START));
}

void  lpc_adc_isr(void)
{
	uint16_t	*out;

	/* Store converted value in packet */
	out = (uint16_t *) &ao_data_ring[ao_data_head].adc;
	out[ao_adc_sequence] = (uint16_t) lpc_adc.gdr >> 1;
	if (++ao_adc_sequence < AO_ADC_NUM) {
		lpc_adc_start();
		return;
	}

	AO_DATA_PRESENT(AO_DATA_ADC);
	if (ao_data_present == AO_DATA_ALL) {
#if HAS_MS5607
		ao_data_ring[ao_data_head].ms5607_raw = ao_ms5607_current;
#endif
#if HAS_MMA655X
		ao_data_ring[ao_data_head].mma655x = ao_mma655x_current;
#endif
#if HAS_HMC5883
		ao_data_ring[ao_data_head].hmc5883 = ao_hmc5883_current;
#endif
#if HAS_MPU6000
		ao_data_ring[ao_data_head].mpu6000 = ao_mpu6000_current;
#endif
		ao_data_ring[ao_data_head].tick = ao_tick_count;
		ao_data_head = ao_data_ring_next(ao_data_head);
		ao_wakeup((void *) &ao_data_head);
	}
	ao_adc_ready = 1;
}


/*
 * Start the ADC sequence using burst mode
 */
void
ao_adc_poll(void)
{
	if (!ao_adc_ready)
		return;
	ao_adc_ready = 0;
	ao_adc_sequence = 0;
	lpc_adc_start();
}

static void
ao_adc_dump(void) __reentrant
{
	struct ao_data	packet;
#ifndef AO_ADC_DUMP
	int16_t *d;
	uint8_t i;
#endif

	ao_data_get(&packet);
#ifdef AO_ADC_DUMP
	AO_ADC_DUMP(&packet);
#else
	printf("tick: %5u",  packet.tick);
	d = (int16_t *) (&packet.adc);
	for (i = 0; i < AO_ADC_NUM; i++)
		printf (" %2d: %5d", i, d[i]);
	printf("\n");
#endif
}

__code struct ao_cmds ao_adc_cmds[] = {
	{ ao_adc_dump,	"a\0Display current ADC values" },
	{ 0, NULL },
};

void
ao_adc_init(void)
{
	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_ADC);
	lpc_scb.pdruncfg &= ~(1 << LPC_SCB_PDRUNCFG_ADC_PD);

	/* Enable interrupt when channel is complete */
	lpc_adc.inten = (1 << LPC_ADC_INTEN_ADGINTEN);

	lpc_nvic_set_enable(LPC_ISR_ADC_POS);
	lpc_nvic_set_priority(LPC_ISR_ADC_POS, AO_LPC_NVIC_CLOCK_PRIORITY);
#if AO_ADC_0
	ao_enable_analog(0, 11, 0);
#endif
#if AO_ADC_1
	ao_enable_analog(0, 12, 1);
#endif
#if AO_ADC_2
	ao_enable_analog(0, 13, 2);
#endif
#if AO_ADC_3
	ao_enable_analog(0, 14, 3);
#endif
#if AO_ADC_4
	ao_enable_analog(0, 15, 4);
#endif
#if AO_ADC_5
	ao_enable_analog(0, 16, 5);
#endif
#if AO_ADC_6
	ao_enable_analog(0, 22, 6);
#endif
#if AO_ADC_7
	ao_enable_analog(0, 23, 7);
#endif

	lpc_adc.cr = ((0 << LPC_ADC_CR_SEL) |
		      (AO_ADC_CLKDIV << LPC_ADC_CR_CLKDIV) |
		      (0 << LPC_ADC_CR_BURST) |
		      (LPC_ADC_CR_CLKS_11 << LPC_ADC_CR_CLKS));

	ao_cmd_register(&ao_adc_cmds[0]);

	ao_adc_ready = 1;
}
