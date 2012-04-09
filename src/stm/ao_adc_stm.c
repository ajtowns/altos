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


volatile __xdata struct ao_adc	ao_adc_ring[AO_ADC_RING];
volatile __data uint8_t		ao_adc_head;
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
static void ao_adc_done(void)
{
	ao_adc_ring[ao_adc_head].tick = ao_time();
	ao_adc_head = ao_adc_ring_next(ao_adc_head);
	ao_wakeup((void *) &ao_adc_head);
	ao_dma_done_transfer(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC1));
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
			    (void *) (&ao_adc_ring[ao_adc_head].tick + 1),
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
	uint8_t	i = ao_adc_ring_prev(ao_adc_head);
	memcpy(packet, (void *) &ao_adc_ring[i], sizeof (struct ao_adc));
}

static void
ao_adc_dump(void) __reentrant
{
	struct ao_adc	packet;
	int16_t *d;
	uint8_t i;

	ao_adc_get(&packet);
	printf("tick: %5u",  packet.tick);
	d = (int16_t *) (&packet.tick + 1);
	for (i = 0; i < AO_NUM_ADC; i++)
		printf (" %2d: %5d", i, d[i]);
	printf("\n");
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
	stm_moder_set(&AO_ADC_PIN0_PORT, AO_ADC_PIN0_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN1_PORT
	stm_moder_set(&AO_ADC_PIN1_PORT, AO_ADC_PIN1_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN2_PORT
	stm_moder_set(&AO_ADC_PIN2_PORT, AO_ADC_PIN2_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN3_PORT
	stm_moder_set(&AO_ADC_PIN3_PORT, AO_ADC_PIN3_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN4_PORT
	stm_moder_set(&AO_ADC_PIN4_PORT, AO_ADC_PIN4_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN5_PORT
	stm_moder_set(&AO_ADC_PIN5_PORT, AO_ADC_PIN5_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN6_PORT
	stm_moder_set(&AO_ADC_PIN6_PORT, AO_ADC_PIN6_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN7_PORT
	stm_moder_set(&AO_ADC_PIN7_PORT, AO_ADC_PIN7_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN8_PORT
	stm_moder_set(&AO_ADC_PIN8_PORT, AO_ADC_PIN8_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN9_PORT
	stm_moder_set(&AO_ADC_PIN9_PORT, AO_ADC_PIN9_PIN, STM_MODER_ANALOG);
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

	/* Turn ADC on */
	stm_adc.cr2 = AO_ADC_CR2_VAL;

	/* Wait for ADC to be ready */
	while (!(stm_adc.sr & (1 << STM_ADC_SR_ADONS)))
		;

#if HAS_ADC_TEMP
	stm_adc.ccr = ((1 << STM_ADC_CCR_TSVREFE));
#else
	stm_adc.ccr = 0;
#endif
	/* Clear any stale status bits */
	stm_adc.sr = 0;
	ao_adc_ready = 1;

	ao_dma_alloc(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC1));
	ao_cmd_register(&ao_adc_cmds[0]);
}
