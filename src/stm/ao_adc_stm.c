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

static uint8_t			ao_adc_ready;

#define AO_ADC_CR2_VAL		((0 << STM_ADC_CR2_SWSTART) |		\
				 (STM_ADC_CR2_EXTEN_DISABLE << STM_ADC_CR2_EXTEN) | \
				 (0 << STM_ADC_CR2_EXTSEL) |		\
				 (0 << STM_ADC_CR2_JWSTART) |		\
				 (STM_ADC_CR2_JEXTEN_DISABLE << STM_ADC_CR2_JEXTEN) | \
				 (0 << STM_ADC_CR2_JEXTSEL) |		\
				 (0 << STM_ADC_CR2_ALIGN) |		\
				 (0 << STM_ADC_CR2_EOCS) |		\
				 (1 << STM_ADC_CR2_DDS) |		\
				 (1 << STM_ADC_CR2_DMA) |		\
				 (STM_ADC_CR2_DELS_UNTIL_READ << STM_ADC_CR2_DELS) | \
				 (0 << STM_ADC_CR2_CONT) |		\
				 (1 << STM_ADC_CR2_ADON))

/*
 * Callback from DMA ISR
 *
 * Mark time in ring, shut down DMA engine
 */
static void ao_adc_done(int index)
{
	(void) index;
	AO_DATA_PRESENT(AO_DATA_ADC);
	ao_dma_done_transfer(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC1));
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
 * Start the ADC sequence using the DMA engine
 */
void
ao_adc_poll(void)
{
	if (!ao_adc_ready)
		return;
	ao_adc_ready = 0;
	stm_adc.sr = 0;
	ao_dma_set_transfer(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC1),
			    &stm_adc.dr,
			    (void *) (&ao_data_ring[ao_data_head].adc),
			    AO_NUM_ADC,
			    (0 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_HIGH << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_16 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_16 << STM_DMA_CCR_PSIZE) |
			    (1 << STM_DMA_CCR_MINC) |
			    (0 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_PER_TO_MEM << STM_DMA_CCR_DIR));
	ao_dma_set_isr(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC1), ao_adc_done);
	ao_dma_start(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC1));

	stm_adc.cr2 = AO_ADC_CR2_VAL | (1 << STM_ADC_CR2_SWSTART);
}

/*
 * Fetch a copy of the most recent ADC data
 */
void
ao_adc_get(__xdata struct ao_adc *packet)
{
#if HAS_FLIGHT
	uint8_t	i = ao_data_ring_prev(ao_sample_data);
#else
	uint8_t	i = ao_data_ring_prev(ao_data_head);
#endif
	memcpy(packet, (void *) &ao_data_ring[i].adc, sizeof (struct ao_adc));
}

#ifdef AO_ADC_SQ1_NAME
static const char *ao_adc_name[AO_NUM_ADC] = {
	AO_ADC_SQ1_NAME,
#ifdef AO_ADC_SQ2_NAME
	AO_ADC_SQ2_NAME,
#endif
#ifdef AO_ADC_SQ3_NAME
	AO_ADC_SQ3_NAME,
#endif
#ifdef AO_ADC_SQ4_NAME
	AO_ADC_SQ4_NAME,
#endif
#ifdef AO_ADC_SQ5_NAME
	AO_ADC_SQ5_NAME,
#endif
#ifdef AO_ADC_SQ6_NAME
	AO_ADC_SQ6_NAME,
#endif
#ifdef AO_ADC_SQ7_NAME
	AO_ADC_SQ7_NAME,
#endif
#ifdef AO_ADC_SQ8_NAME
	AO_ADC_SQ8_NAME,
#endif
#ifdef AO_ADC_SQ9_NAME
	AO_ADC_SQ9_NAME,
#endif
#ifdef AO_ADC_SQ10_NAME
	AO_ADC_SQ10_NAME,
#endif
#ifdef AO_ADC_SQ11_NAME
	AO_ADC_SQ11_NAME,
#endif
#ifdef AO_ADC_SQ12_NAME
	AO_ADC_SQ12_NAME,
#endif
#ifdef AO_ADC_SQ13_NAME
	AO_ADC_SQ13_NAME,
#endif
#ifdef AO_ADC_SQ14_NAME
	AO_ADC_SQ14_NAME,
#endif
#ifdef AO_ADC_SQ15_NAME
	AO_ADC_SQ15_NAME,
#endif
#ifdef AO_ADC_SQ16_NAME
	AO_ADC_SQ16_NAME,
#endif
#ifdef AO_ADC_SQ17_NAME
	AO_ADC_SQ17_NAME,
#endif
#ifdef AO_ADC_SQ18_NAME
	AO_ADC_SQ18_NAME,
#endif
#ifdef AO_ADC_SQ19_NAME
	AO_ADC_SQ19_NAME,
#endif
#ifdef AO_ADC_SQ20_NAME
	AO_ADC_SQ20_NAME,
#endif
#ifdef AO_ADC_SQ21_NAME
	#error "too many ADC names"
#endif
};
#endif

static void
ao_adc_dump(void) __reentrant
{
	struct ao_data	packet;
#ifndef AO_ADC_DUMP
	uint8_t i;
	int16_t *d;
#endif

	ao_data_get(&packet);
#ifdef AO_ADC_DUMP
	AO_ADC_DUMP(&packet);
#else
	printf("tick: %5u",  packet.tick);
	d = (int16_t *) (&packet.adc);
	for (i = 0; i < AO_NUM_ADC; i++) {
#ifdef AO_ADC_SQ1_NAME
		if (ao_adc_name[i])
			printf (" %s: %5d", ao_adc_name[i], d[i]);
		else		
#endif
			printf (" %2d: %5d", i, d[i]);
	}
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
#ifdef AO_ADC_PIN0_PORT
	stm_rcc.ahbenr |= AO_ADC_RCC_AHBENR;
#endif

#ifdef AO_ADC_PIN0_PORT
	stm_moder_set(AO_ADC_PIN0_PORT, AO_ADC_PIN0_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN1_PORT
	stm_moder_set(AO_ADC_PIN1_PORT, AO_ADC_PIN1_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN2_PORT
	stm_moder_set(AO_ADC_PIN2_PORT, AO_ADC_PIN2_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN3_PORT
	stm_moder_set(AO_ADC_PIN3_PORT, AO_ADC_PIN3_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN4_PORT
	stm_moder_set(AO_ADC_PIN4_PORT, AO_ADC_PIN4_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN5_PORT
	stm_moder_set(AO_ADC_PIN5_PORT, AO_ADC_PIN5_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN6_PORT
	stm_moder_set(AO_ADC_PIN6_PORT, AO_ADC_PIN6_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN7_PORT
	stm_moder_set(AO_ADC_PIN7_PORT, AO_ADC_PIN7_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN8_PORT
	stm_moder_set(AO_ADC_PIN8_PORT, AO_ADC_PIN8_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN9_PORT
	stm_moder_set(AO_ADC_PIN9_PORT, AO_ADC_PIN9_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN10_PORT
	stm_moder_set(AO_ADC_PIN10_PORT, AO_ADC_PIN10_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN11_PORT
	stm_moder_set(AO_ADC_PIN11_PORT, AO_ADC_PIN11_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN12_PORT
	stm_moder_set(AO_ADC_PIN12_PORT, AO_ADC_PIN12_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN13_PORT
	stm_moder_set(AO_ADC_PIN13_PORT, AO_ADC_PIN13_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN14_PORT
	stm_moder_set(AO_ADC_PIN14_PORT, AO_ADC_PIN14_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN15_PORT
	stm_moder_set(AO_ADC_PIN15_PORT, AO_ADC_PIN15_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN16_PORT
	stm_moder_set(AO_ADC_PIN16_PORT, AO_ADC_PIN16_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN17_PORT
	stm_moder_set(AO_ADC_PIN17_PORT, AO_ADC_PIN17_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN18_PORT
	stm_moder_set(AO_ADC_PIN18_PORT, AO_ADC_PIN18_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN19_PORT
	stm_moder_set(AO_ADC_PIN19_PORT, AO_ADC_PIN19_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN20_PORT
	stm_moder_set(AO_ADC_PIN20_PORT, AO_ADC_PIN20_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN21_PORT
	stm_moder_set(AO_ADC_PIN21_PORT, AO_ADC_PIN21_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN22_PORT
	stm_moder_set(AO_ADC_PIN22_PORT, AO_ADC_PIN22_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN23_PORT
	stm_moder_set(AO_ADC_PIN23_PORT, AO_ADC_PIN23_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN24_PORT
	#error "Too many ADC ports"
#endif

	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_ADC1EN);

	/* Turn off ADC during configuration */
	stm_adc.cr2 = 0;

	stm_adc.cr1 = ((0 << STM_ADC_CR1_OVRIE ) |
		       (STM_ADC_CR1_RES_12 << STM_ADC_CR1_RES ) |
		       (0 << STM_ADC_CR1_AWDEN ) |
		       (0 << STM_ADC_CR1_JAWDEN ) |
		       (0 << STM_ADC_CR1_PDI ) |
		       (0 << STM_ADC_CR1_PDD ) |
		       (0 << STM_ADC_CR1_DISCNUM ) |
		       (0 << STM_ADC_CR1_JDISCEN ) |
		       (0 << STM_ADC_CR1_DISCEN ) |
		       (0 << STM_ADC_CR1_JAUTO ) |
		       (0 << STM_ADC_CR1_AWDSGL ) |
		       (1 << STM_ADC_CR1_SCAN ) |
		       (0 << STM_ADC_CR1_JEOCIE ) |
		       (0 << STM_ADC_CR1_AWDIE ) |
		       (0 << STM_ADC_CR1_EOCIE ) |
		       (0 << STM_ADC_CR1_AWDCH ));

	/* 384 cycle sample time for everyone */
	stm_adc.smpr1 = 0x3ffff;
	stm_adc.smpr2 = 0x3fffffff;
	stm_adc.smpr3 = 0x3fffffff;

	stm_adc.sqr1 = ((AO_NUM_ADC - 1) << 20);
	stm_adc.sqr2 = 0;
	stm_adc.sqr3 = 0;
	stm_adc.sqr4 = 0;
	stm_adc.sqr5 = 0;
#if AO_NUM_ADC > 0
	stm_adc.sqr5 |= (AO_ADC_SQ1 << 0);
#endif
#if AO_NUM_ADC > 1
	stm_adc.sqr5 |= (AO_ADC_SQ2 << 5);
#endif
#if AO_NUM_ADC > 2
	stm_adc.sqr5 |= (AO_ADC_SQ3 << 10);
#endif
#if AO_NUM_ADC > 3
	stm_adc.sqr5 |= (AO_ADC_SQ4 << 15);
#endif
#if AO_NUM_ADC > 4
	stm_adc.sqr5 |= (AO_ADC_SQ5 << 20);
#endif
#if AO_NUM_ADC > 5
	stm_adc.sqr5 |= (AO_ADC_SQ6 << 25);
#endif
#if AO_NUM_ADC > 6
	stm_adc.sqr4 |= (AO_ADC_SQ7 << 0);
#endif
#if AO_NUM_ADC > 7
	stm_adc.sqr4 |= (AO_ADC_SQ8 << 5);
#endif
#if AO_NUM_ADC > 8
	stm_adc.sqr4 |= (AO_ADC_SQ9 << 10);
#endif
#if AO_NUM_ADC > 9
	stm_adc.sqr4 |= (AO_ADC_SQ10 << 15);
#endif
#if AO_NUM_ADC > 10
	stm_adc.sqr4 |= (AO_ADC_SQ11 << 20);
#endif
#if AO_NUM_ADC > 11
	stm_adc.sqr4 |= (AO_ADC_SQ12 << 25);
#endif
#if AO_NUM_ADC > 12
	stm_adc.sqr3 |= (AO_ADC_SQ13 << 0);
#endif
#if AO_NUM_ADC > 13
	stm_adc.sqr3 |= (AO_ADC_SQ14 << 5);
#endif
#if AO_NUM_ADC > 14
	stm_adc.sqr3 |= (AO_ADC_SQ15 << 10);
#endif
#if AO_NUM_ADC > 15
	stm_adc.sqr3 |= (AO_ADC_SQ16 << 15);
#endif
#if AO_NUM_ADC > 16
	stm_adc.sqr3 |= (AO_ADC_SQ17 << 20);
#endif
#if AO_NUM_ADC > 17
	stm_adc.sqr3 |= (AO_ADC_SQ18 << 25);
#endif
#if AO_NUM_ADC > 18
#error "need to finish stm_adc.sqr settings"
#endif
	
	/* Turn ADC on */
	stm_adc.cr2 = AO_ADC_CR2_VAL;

	/* Wait for ADC to be ready */
	while (!(stm_adc.sr & (1 << STM_ADC_SR_ADONS)))
		;

#ifndef HAS_ADC_TEMP
#error Please define HAS_ADC_TEMP
#endif
#if HAS_ADC_TEMP
	stm_adc.ccr = ((1 << STM_ADC_CCR_TSVREFE));
#else
	stm_adc.ccr = 0;
#endif
	/* Clear any stale status bits */
	stm_adc.sr = 0;

	ao_dma_alloc(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC1));

	ao_cmd_register(&ao_adc_cmds[0]);

	ao_adc_ready = 1;
}
