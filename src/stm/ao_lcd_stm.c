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

struct ao_lcd_segment {
	uint8_t	reg;
	uint8_t	bit;
};

#define A	0
#define B	1
#define C	2
#define D	3
#define E	4

static struct stm_gpio *gpios[] = {
	&stm_gpioa,
	&stm_gpiob,
	&stm_gpioc,
	&stm_gpiod,
	&stm_gpioe
};

static inline int ao_lcd_stm_seg_enabled(int seg) {
	if (seg < 32)
		return (AO_LCD_STM_SEG_ENABLED_0 >> seg) & 1;
	else
		return (AO_LCD_STM_SEG_ENABLED_1 >> (seg - 32)) & 1;
}

static inline int ao_lcd_stm_com_enabled(int com) {
	return (AO_LCD_STM_COM_ENABLED >> com) & 1;
}

#define AO_LCD_STM_GPIOA_SEGS_0	(		\
		(1 << 0) |			\
		(1 << 1) |			\
		(1 << 2) |			\
		(1 << 3) |			\
		(1 << 4) |			\
		(1 << 17))

#define AO_LCD_STM_GPIOA_SEGS_1 0

#define AO_LCD_STM_USES_GPIOA	(!!((AO_LCD_STM_GPIOA_SEGS_0 & AO_LCD_STM_SEG_ENABLED_0) | \
				    (AO_LCD_STM_GPIOA_SEGS_1 & AO_LCD_STM_SEG_ENABLED_1)))


#define AO_LCD_STM_GPIOB_SEGS_0	(		\
		(1 << 5) |			\
		(1 << 6) |			\
		(1 << 7) |			\
		(1 << 8) |			\
		(1 << 9) |			\
		(1 << 10) |			\
		(1 << 11) |			\
		(1 << 12) |			\
		(1 << 13) |			\
		(1 << 14) |			\
		(1 << 15) |			\
		(1 << 16))

#define AO_LCD_STM_GPIOB_SEGS_1 0

#if AO_LCD_28_ON_C

#define AO_LCD_STM_GPIOC_28_SEGS	(	\
		(1 << 28) |			\
		(1 << 29) |			\
		(1 << 30))

#define AO_LCD_STM_GPIOD_28_SEGS	(	\
		(1 << 31))

#else
#define AO_LCD_STM_GPIOC_28_C_SEGS	0

#define AO_LCD_STM_GPIOD_28_SEGS	(	\
		(1 << 28) |			\
		(1 << 29) |			\
		(1 << 30) |			\
		(1 << 31))
#endif

#define AO_LCD_STM_GPIOC_SEGS_0	(		\
		(1 << 18) |			\
		(1 << 19) |			\
		(1 << 20) |			\
		(1 << 21) |			\
		(1 << 22) |			\
		(1 << 23) |			\
		(1 << 24) |			\
		(1 << 25) |			\
		(1 << 26) |			\
		(1 << 27) |			\
		AO_LCD_STM_GPIOC_28_SEGS)

#define AO_LCD_STM_GPIOC_SEGS_1 (		\
		(1 << (40 - 32)) |		\
		(1 << (41 - 32)) |		\
		(1 << (42 - 32)))

#define AO_LCD_STM_GPIOD_SEGS_0	(		\
		AO_LCD_STM_GPIOD_28_SEGS)

#define AO_LCD_STM_GPIOD_SEGS_1 (		\
		(1 << (32 - 32)) |		\
		(1 << (33 - 32)) |		\
		(1 << (34 - 32)) |		\
		(1 << (35 - 32)) |		\
		(1 << (43 - 32)))

#define AO_LCD_STM_GPIOE_SEGS_0	0

#define AO_LCD_STM_GPIOE_SEGS_1 (		\
		(1 << (36 - 32)) |		\
		(1 << (37 - 32)) |		\
		(1 << (38 - 32)) |		\
		(1 << (39 - 32)))

#define AO_LCD_STM_USES_GPIOA	(!!((AO_LCD_STM_GPIOA_SEGS_0 & AO_LCD_STM_SEG_ENABLED_0) | \
				    (AO_LCD_STM_GPIOA_SEGS_1 & AO_LCD_STM_SEG_ENABLED_1)))

#define AO_LCD_STM_USES_GPIOB	(!!((AO_LCD_STM_GPIOB_SEGS_0 & AO_LCD_STM_SEG_ENABLED_0) | \
				    (AO_LCD_STM_GPIOB_SEGS_1 & AO_LCD_STM_SEG_ENABLED_1)))


#define AO_LCD_STM_USES_GPIOC	(!!((AO_LCD_STM_GPIOC_SEGS_0 & AO_LCD_STM_SEG_ENABLED_0) | \
				    (AO_LCD_STM_GPIOC_SEGS_1 & AO_LCD_STM_SEG_ENABLED_1)))


#define AO_LCD_STM_USES_GPIOD	(!!((AO_LCD_STM_GPIOD_SEGS_0 & AO_LCD_STM_SEG_ENABLED_0) | \
				    (AO_LCD_STM_GPIOD_SEGS_1 & AO_LCD_STM_SEG_ENABLED_1)))

#define AO_LCD_STM_USES_GPIOE	(!!((AO_LCD_STM_GPIOE_SEGS_0 & AO_LCD_STM_SEG_ENABLED_0) | \
				    (AO_LCD_STM_GPIOE_SEGS_1 & AO_LCD_STM_SEG_ENABLED_1)))


static const struct ao_lcd_segment segs[] = {
	{ A, 1 },	/* 0 */
	{ A, 2 },
	{ A, 3 },
	{ A, 6 },
		
	{ A, 7 },	/* 4 */
	{ B, 0 },
	{ B, 1 },
	{ B, 3 },

	{ B, 4 },	/* 8 */
	{ B, 5 },
	{ B, 10 },
	{ B, 11 },

	{ B, 12 },	/* 12 */
	{ B, 13 },
	{ B, 14 },
	{ B, 15 },

	{ B, 8 },	/* 16 */
	{ A, 15 },
	{ C, 0 },
	{ C, 1 },

	{ C, 2 },	/* 20 */
	{ C, 3 },
	{ C, 4 },
	{ C, 5 },

	{ C, 6 },	/* 24 */
	{ C, 7 },
	{ C, 8 },
	{ C, 9 },

#if AO_LCD_28_ON_C
	{ C, 10 },	/* 28 */
	{ C, 11 },
	{ C, 12 },
	{ D, 2 },
#else
	{ D, 8 },	/* 28 */
	{ D, 9 },
	{ D, 10 },
	{ D, 11 },
#endif
	{ D, 12 },	/* 32 */
	{ D, 13 },
	{ D, 14 },
	{ D, 15 },
		
	{ E, 0 },	/* 36 */
	{ E, 1 },
	{ E, 2 },
	{ E, 3 },

	{ C, 10 },	/* 40 */
	{ C, 11 },
	{ C, 12 },
	{ D, 2 },
};

static const struct ao_lcd_segment coms[] = {
	{ A, 8 },	/* 0 */
	{ A, 9 },	/* 1 */
	{ A, 10 }, 	/* 2 */
	{ B, 9 },	/* 3 */
	{ C, 10 },	/* 4 */
	{ C, 11 },	/* 5 */
	{ C, 12 },	/* 6 */
};

#define NSEG	(sizeof segs/sizeof segs[0])
#define NCOM	(sizeof coms/sizeof coms[0])

static void
ao_lcd_stm_fcr_sync(void)
{
	while ((stm_lcd.sr & (1 << STM_LCD_SR_FCRSF)) == 0)
		asm("nop");
}

static void
ao_lcd_stm_seg_set(void)
{
	int	com, seg, val;
	int	n, bit;
	ao_cmd_decimal();
	com = ao_cmd_lex_i;
	ao_cmd_decimal();
	seg = ao_cmd_lex_u32;
	ao_cmd_decimal();
	val = ao_cmd_lex_i;
	printf ("com: %d seg: %d val: %d\n", com, seg, val);
	n = (seg >> 5) & 1;
	if (com >= NCOM)
		com = NCOM-1;
	if (seg >= NSEG)
		seg = NSEG-1;
	if (val)
		stm_lcd.ram[com * 2 + n] |= (1 << (seg & 0x1f));
	else
		stm_lcd.ram[com * 2 + n] &= ~(1 << (seg & 0x1f));
	stm_lcd.sr = (1 << STM_LCD_SR_UDR);
}

static void
ao_lcd_stm_clear(void)
{
	int	i;

	for (i = 0; i < sizeof (stm_lcd.ram) / 4; i++)
		stm_lcd.ram[i] = 0;
	stm_lcd.sr = (1 << STM_LCD_SR_UDR);
}


const struct ao_cmds ao_lcd_stm_cmds[] = {
	{ ao_lcd_stm_seg_set,	"s <com> <seg> <value>\0Set LCD segment" },
	{ ao_lcd_stm_clear,	"C\0Clear LCD" },
	{ 0, NULL },
};

void
ao_lcd_stm_init(void)
{
	int s, c;
	int r;
	uint32_t	csr;

	stm_rcc.ahbenr |= ((AO_LCD_STM_USES_GPIOA << STM_RCC_AHBENR_GPIOAEN) |
			   (AO_LCD_STM_USES_GPIOB << STM_RCC_AHBENR_GPIOBEN) |
			   (AO_LCD_STM_USES_GPIOC << STM_RCC_AHBENR_GPIOCEN) |
			   (AO_LCD_STM_USES_GPIOD << STM_RCC_AHBENR_GPIODEN) |
			   (AO_LCD_STM_USES_GPIOE << STM_RCC_AHBENR_GPIOEEN));

	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_LCDEN);

	/* Turn on the LSI clock */
	if ((stm_rcc.csr & (1 << STM_RCC_CSR_LSIRDY)) == 0) {
		stm_rcc.csr |= (1 << STM_RCC_CSR_LSION);
		while ((stm_rcc.csr & (1 << STM_RCC_CSR_LSIRDY)) == 0)
			asm("nop");
	}

	/* Enable RTC clock config (required to change the RTC/LCD clock */

	stm_pwr.cr |= (1 << STM_PWR_CR_DBP);

	/* Configure the LCDCLK - use the LSI clock */

	csr = stm_rcc.csr;
	csr &= ~(STM_RCC_CSR_RTCSEL_MASK << STM_RCC_CSR_RTCSEL);
	csr |= (STM_RCC_CSR_RTCSEL_LSI << STM_RCC_CSR_RTCSEL);
	stm_rcc.csr = csr;

	for (s = 0; s < NSEG; s++) {
		uint8_t	reg = segs[s].reg;
		uint8_t bit = segs[s].bit;
			
		if (ao_lcd_stm_seg_enabled(s)) {
			stm_moder_set(gpios[reg], bit, STM_MODER_ALTERNATE);
			stm_afr_set(gpios[reg], bit, STM_AFR_AF11);
		}
	}

	for (c = 0; c < NCOM; c++) {
		uint8_t	reg = coms[c].reg;
		uint8_t bit = coms[c].bit;
			
		if (ao_lcd_stm_com_enabled(c)) {
			stm_moder_set(gpios[reg], bit, STM_MODER_ALTERNATE);
			stm_afr_set(gpios[reg], bit, STM_AFR_AF11);
		}
	}

	/* duty cycle 1/3, radio 352, frame rate about 33Hz */
	stm_lcd.fcr = ((STM_LCD_FCR_PS_1 << STM_LCD_FCR_PS) |
		       (STM_LCD_FCR_DIV_31 << STM_LCD_FCR_DIV) |
		       (4 << STM_LCD_FCR_CC) |
		       (4 << STM_LCD_FCR_PON) |
		       (0 << STM_LCD_FCR_UDDIE) |
		       (0 << STM_LCD_FCR_SOFIE) |
		       (0 << STM_LCD_FCR_HD));

	ao_lcd_stm_fcr_sync();

	/* Program desired DUTY in LCD_CR */
	/* Program desired BIAS in LCD_CR */
	/* Enable mux seg */
	/* Internal voltage source */
	stm_lcd.cr = ((STM_LCD_CR_DUTY_1_4 << STM_LCD_CR_DUTY) |
		      (STM_LCD_CR_BIAS_1_3 << STM_LCD_CR_BIAS) |
		      (0 << STM_LCD_CR_VSEL) |
		      (1 << STM_LCD_CR_MUX_SEG));

	ao_lcd_stm_fcr_sync();

	/* Enable the display (LCDEN bit in LCD_CR) */
	stm_lcd.cr |= (1 << STM_LCD_CR_LCDEN);

	while ((stm_lcd.sr & (1 << STM_LCD_SR_RDY)) == 0)
		asm("nop");

	/* Load initial data into LCD_RAM and set the
	 * UDR bit in the LCD_SR register */
	for (r = 0; r < NCOM; r++) {
		stm_lcd.ram[r*2] = 0;
		stm_lcd.ram[r*2 + 1] = 0;
	}

	stm_lcd.sr = (1 << STM_LCD_SR_UDR);

	/* Program desired frame rate (PS and DIV bits in LCD_FCR) */

	/* Program the contrast (CC bits in LCD_FCR) */

	/* Program optional features (BLINK, BLINKF, PON, DEAD, HD) */

	/* Program the required interrupts */

	/* All done */
	ao_cmd_register(ao_lcd_stm_cmds);
}
