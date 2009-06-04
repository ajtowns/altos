/*
 * Copyright © 2008 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <stdint.h>

/*
 * Test ADC in DMA mode
 */

sfr at 0x80 P0;
sfr at 0x90 P1;
sfr at 0xA0 P2;
sfr at 0xC6 CLKCON;
sfr at 0xbe SLEEP;

# define SLEEP_USB_EN		(1 << 7)
# define SLEEP_XOSC_STB		(1 << 6)

sfr at 0xF1 PERCFG;
#define PERCFG_T1CFG_ALT_1	(0 << 6)
#define PERCFG_T1CFG_ALT_2	(1 << 6)

#define PERCFG_T3CFG_ALT_1	(0 << 5)
#define PERCFG_T3CFG_ALT_2	(1 << 5)

#define PERCFG_T4CFG_ALT_1	(0 << 4)
#define PERCFG_T4CFG_ALT_2	(1 << 4)

#define PERCFG_U1CFG_ALT_1	(0 << 1)
#define PERCFG_U1CFG_ALT_2	(1 << 1)

#define PERCFG_U0CFG_ALT_1	(0 << 0)
#define PERCFG_U0CFG_ALT_2	(1 << 0)

sfr at 0xF2 ADCCFG;
sfr at 0xF3 P0SEL;
sfr at 0xF4 P1SEL;
sfr at 0xF5 P2SEL;

sfr at 0xFD P0DIR;
sfr at 0xFE P1DIR;
sfr at 0xFF P2DIR;
sfr at 0x8F P0INP;
sfr at 0xF6 P1INP;
sfr at 0xF7 P2INP;

sfr at 0x89 P0IFG;
sfr at 0x8A P1IFG;
sfr at 0x8B P2IFG;

sbit at 0x90 P1_0;
sbit at 0x91 P1_1;
sbit at 0x92 P1_2;
sbit at 0x93 P1_3;
sbit at 0x94 P1_4;
sbit at 0x95 P1_5;
sbit at 0x96 P1_6;
sbit at 0x97 P1_7;

/*
 * UART registers
 */

sfr at 0x86 U0CSR;
sfr at 0xF8 U1CSR;

/*
 * IRCON2
 */
sfr at 0xE8 IRCON2;	/* CPU Interrupt Flag 5 */

sbit at 0xE8 USBIF;	/* USB interrupt flag (shared with Port2) */
sbit at 0xE8 P2IF;	/* Port2 interrupt flag (shared with USB) */
sbit at 0xE9 UTX0IF;	/* USART0 TX interrupt flag */
sbit at 0xEA UTX1IF;	/* USART1 TX interrupt flag (shared with I2S TX) */
sbit at 0xEA I2STXIF;	/* I2S TX interrupt flag (shared with USART1 TX) */
sbit at 0xEB P1IF;	/* Port1 interrupt flag */
sbit at 0xEC WDTIF;	/* Watchdog timer interrupt flag */

# define UxCSR_MODE_UART		(1 << 7)
# define UxCSR_MODE_SPI			(0 << 7)
# define UxCSR_RE			(1 << 6)
# define UxCSR_SLAVE			(1 << 5)
# define UxCSR_MASTER			(0 << 5)
# define UxCSR_FE			(1 << 4)
# define UxCSR_ERR			(1 << 3)
# define UxCSR_RX_BYTE			(1 << 2)
# define UxCSR_TX_BYTE			(1 << 1)
# define UxCSR_ACTIVE			(1 << 0)

sfr at 0xc4 U0UCR;
sfr at 0xfb U1UCR;

# define UxUCR_FLUSH			(1 << 7)
# define UxUCR_FLOW_DISABLE		(0 << 6)
# define UxUCR_FLOW_ENABLE		(1 << 6)
# define UxUCR_D9_EVEN_PARITY		(0 << 5)
# define UxUCR_D9_ODD_PARITY		(1 << 5)
# define UxUCR_BIT9_8_BITS		(0 << 4)
# define UxUCR_BIT9_9_BITS		(1 << 4)
# define UxUCR_PARITY_DISABLE		(0 << 3)
# define UxUCR_PARITY_ENABLE		(1 << 3)
# define UxUCR_SPB_1_STOP_BIT		(0 << 2)
# define UxUCR_SPB_2_STOP_BITS		(1 << 2)
# define UxUCR_STOP_LOW			(0 << 1)
# define UxUCR_STOP_HIGH		(1 << 1)
# define UxUCR_START_LOW		(0 << 0)
# define UxUCR_START_HIGH		(1 << 0)

sfr at 0xc5 U0GCR;
sfr at 0xfc U1GCR;

# define UxGCR_CPOL_NEGATIVE		(0 << 7)
# define UxGCR_CPOL_POSITIVE		(1 << 7)
# define UxGCR_CPHA_FIRST_EDGE		(0 << 6)
# define UxGCR_CPHA_SECOND_EDGE		(1 << 6)
# define UxGCR_ORDER_LSB		(0 << 5)
# define UxGCR_ORDER_MSB		(1 << 5)
# define UxGCR_BAUD_E_MASK		(0x1f)
# define UxGCR_BAUD_E_SHIFT		0

sfr at 0xc1 U0DBUF;
sfr at 0xf9 U1DBUF;
sfr at 0xc2 U0BAUD;
sfr at 0xfa U1BAUD;

#define DEBUG	P1_1


# define DMA_LEN_HIGH_VLEN_MASK		(7 << 5)
# define DMA_LEN_HIGH_VLEN_LEN		(0 << 5)
# define DMA_LEN_HIGH_VLEN_PLUS_1	(1 << 5)
# define DMA_LEN_HIGH_VLEN		(2 << 5)
# define DMA_LEN_HIGH_VLEN_PLUS_2	(3 << 5)
# define DMA_LEN_HIGH_VLEN_PLUS_3	(4 << 5)
# define DMA_LEN_HIGH_MASK		(0x1f)

# define DMA_CFG0_WORDSIZE_8		(0 << 7)
# define DMA_CFG0_WORDSIZE_16		(1 << 7)
# define DMA_CFG0_TMODE_MASK		(3 << 5)
# define DMA_CFG0_TMODE_SINGLE		(0 << 5)
# define DMA_CFG0_TMODE_BLOCK		(1 << 5)
# define DMA_CFG0_TMODE_REPEATED_SINGLE	(2 << 5)
# define DMA_CFG0_TMODE_REPEATED_BLOCK	(3 << 5)

/*
 * DMA triggers
 */
# define DMA_CFG0_TRIGGER_NONE		0
# define DMA_CFG0_TRIGGER_PREV		1
# define DMA_CFG0_TRIGGER_T1_CH0	2
# define DMA_CFG0_TRIGGER_T1_CH1	3
# define DMA_CFG0_TRIGGER_T1_CH2	4
# define DMA_CFG0_TRIGGER_T2_OVFL	6
# define DMA_CFG0_TRIGGER_T3_CH0	7
# define DMA_CFG0_TRIGGER_T3_CH1	8
# define DMA_CFG0_TRIGGER_T4_CH0	9
# define DMA_CFG0_TRIGGER_T4_CH1	10
# define DMA_CFG0_TRIGGER_IOC_0		12
# define DMA_CFG0_TRIGGER_IOC_1		13
# define DMA_CFG0_TRIGGER_URX0		14
# define DMA_CFG0_TRIGGER_UTX0		15
# define DMA_CFG0_TRIGGER_URX1		16
# define DMA_CFG0_TRIGGER_UTX1		17
# define DMA_CFG0_TRIGGER_FLASH		18
# define DMA_CFG0_TRIGGER_RADIO		19
# define DMA_CFG0_TRIGGER_ADC_CHALL	20
# define DMA_CFG0_TRIGGER_ADC_CH0	21
# define DMA_CFG0_TRIGGER_ADC_CH1	22
# define DMA_CFG0_TRIGGER_ADC_CH2	23
# define DMA_CFG0_TRIGGER_ADC_CH3	24
# define DMA_CFG0_TRIGGER_ADC_CH4	25
# define DMA_CFG0_TRIGGER_ADC_CH5	26
# define DMA_CFG0_TRIGGER_ADC_CH6	27
# define DMA_CFG0_TRIGGER_I2SRX		27
# define DMA_CFG0_TRIGGER_ADC_CH7	28
# define DMA_CFG0_TRIGGER_I2STX		28
# define DMA_CFG0_TRIGGER_ENC_DW	29
# define DMA_CFG0_TRIGGER_DNC_UP	30

# define DMA_CFG1_SRCINC_MASK		(3 << 6)
# define DMA_CFG1_SRCINC_0		(0 << 6)
# define DMA_CFG1_SRCINC_1		(1 << 6)
# define DMA_CFG1_SRCINC_2		(2 << 6)
# define DMA_CFG1_SRCINC_MINUS_1	(3 << 6)

# define DMA_CFG1_DESTINC_MASK		(3 << 4)
# define DMA_CFG1_DESTINC_0		(0 << 4)
# define DMA_CFG1_DESTINC_1		(1 << 4)
# define DMA_CFG1_DESTINC_2		(2 << 4)
# define DMA_CFG1_DESTINC_MINUS_1	(3 << 4)

# define DMA_CFG1_IRQMASK		(1 << 3)
# define DMA_CFG1_M8			(1 << 2)

# define DMA_CFG1_PRIORITY_MASK		(3 << 0)
# define DMA_CFG1_PRIORITY_LOW		(0 << 0)
# define DMA_CFG1_PRIORITY_NORMAL	(1 << 0)
# define DMA_CFG1_PRIORITY_HIGH		(2 << 0)

/*
 * DMAARM - DMA Channel Arm
 */

sfr at 0xD6 DMAARM;

# define DMAARM_ABORT			(1 << 7)
# define DMAARM_DMAARM4			(1 << 4)
# define DMAARM_DMAARM3			(1 << 3)
# define DMAARM_DMAARM2			(1 << 2)
# define DMAARM_DMAARM1			(1 << 1)
# define DMAARM_DMAARM0			(1 << 0)

/*
 * DMAREQ - DMA Channel Start Request and Status
 */

sfr at 0xD7 DMAREQ;

# define DMAREQ_DMAREQ4			(1 << 4)
# define DMAREQ_DMAREQ3			(1 << 3)
# define DMAREQ_DMAREQ2			(1 << 2)
# define DMAREQ_DMAREQ1			(1 << 1)
# define DMAREQ_DMAREQ0			(1 << 0)

/*
 * DMA configuration 0 address
 */

sfr at 0xD5 DMA0CFGH;
sfr at 0xD4 DMA0CFGL;

/*
 * DMA configuration 1-4 address
 */

sfr at 0xD3 DMA1CFGH;
sfr at 0xD2 DMA1CFGL;

/*
 * DMAIRQ - DMA Interrupt Flag
 */

sfr at 0xD1 DMAIRQ;

# define DMAIRQ_DMAIF4			(1 << 4)
# define DMAIRQ_DMAIF3			(1 << 3)
# define DMAIRQ_DMAIF2			(1 << 2)
# define DMAIRQ_DMAIF1			(1 << 1)
# define DMAIRQ_DMAIF0			(1 << 0)

struct cc_dma_channel {
	uint8_t	src_high;
	uint8_t	src_low;
	uint8_t	dst_high;
	uint8_t	dst_low;
	uint8_t	len_high;
	uint8_t	len_low;
	uint8_t	cfg0;
	uint8_t	cfg1;
};

/*
 * ADC Data register, low and high
 */
sfr at 0xBA ADCL;
sfr at 0xBB ADCH;
__xdata __at (0xDFBA) volatile uint16_t ADCXDATA;

/*
 * ADC Control Register 1
 */
sfr at 0xB4 ADCCON1;

# define ADCCON1_EOC		(1 << 7)	/* conversion complete */
# define ADCCON1_ST		(1 << 6)	/* start conversion */

# define ADCCON1_STSEL_MASK	(3 << 4)	/* start select */
# define ADCCON1_STSEL_EXTERNAL	(0 << 4)	/* P2_0 pin triggers */
# define ADCCON1_STSEL_FULLSPEED (1 << 4)	/* full speed, no waiting */
# define ADCCON1_STSEL_TIMER1	(2 << 4)	/* timer 1 channel 0 */
# define ADCCON1_STSEL_START	(3 << 4)	/* set start bit */

# define ADCCON1_RCTRL_MASK	(3 << 2)	/* random number control */
# define ADCCON1_RCTRL_COMPLETE	(0 << 2)	/* operation completed */
# define ADCCON1_RCTRL_CLOCK_LFSR (1 << 2)	/* Clock the LFSR once */

/*
 * ADC Control Register 2
 */
sfr at 0xB5 ADCCON2;

# define ADCCON2_SREF_MASK	(3 << 6)	/* reference voltage */
# define ADCCON2_SREF_1_25V	(0 << 6)	/* internal 1.25V */
# define ADCCON2_SREF_EXTERNAL	(1 << 6)	/* external on AIN7 cc1110 */
# define ADCCON2_SREF_VDD	(2 << 6)	/* VDD on the AVDD pin */
# define ADCCON2_SREF_EXTERNAL_DIFF (3 << 6)	/* external on AIN6-7 cc1110 */

# define ADCCON2_SDIV_MASK	(3 << 4)	/* decimation rate */
# define ADCCON2_SDIV_64	(0 << 4)	/* 7 bits */
# define ADCCON2_SDIV_128	(1 << 4)	/* 9 bits */
# define ADCCON2_SDIV_256	(2 << 4)	/* 10 bits */
# define ADCCON2_SDIV_512	(3 << 4)	/* 12 bits */

# define ADCCON2_SCH_MASK	(0xf << 0)	/* Sequence channel select */
# define ADCCON2_SCH_SHIFT	0
# define ADCCON2_SCH_AIN0	(0 << 0)
# define ADCCON2_SCH_AIN1	(1 << 0)
# define ADCCON2_SCH_AIN2	(2 << 0)
# define ADCCON2_SCH_AIN3	(3 << 0)
# define ADCCON2_SCH_AIN4	(4 << 0)
# define ADCCON2_SCH_AIN5	(5 << 0)
# define ADCCON2_SCH_AIN6	(6 << 0)
# define ADCCON2_SCH_AIN7	(7 << 0)
# define ADCCON2_SCH_AIN0_AIN1	(8 << 0)
# define ADCCON2_SCH_AIN2_AIN3	(9 << 0)
# define ADCCON2_SCH_AIN4_AIN5	(0xa << 0)
# define ADCCON2_SCH_AIN6_AIN7	(0xb << 0)
# define ADCCON2_SCH_GND	(0xc << 0)
# define ADCCON2_SCH_VREF	(0xd << 0)
# define ADCCON2_SCH_TEMP	(0xe << 0)
# define ADCCON2_SCH_VDD_3	(0xf << 0)


/*
 * ADC Control Register 3
 */

sfr at 0xB6 ADCCON3;

# define ADCCON3_EREF_MASK	(3 << 6)	/* extra conversion reference */
# define ADCCON3_EREF_1_25	(0 << 6)	/* internal 1.25V */
# define ADCCON3_EREF_EXTERNAL	(1 << 6)	/* external AIN7 cc1110 */
# define ADCCON3_EREF_VDD	(2 << 6)	/* VDD on the AVDD pin */
# define ADCCON3_EREF_EXTERNAL_DIFF (3 << 6)	/* external AIN6-7 cc1110 */
# define ADCCON2_EDIV_MASK	(3 << 4)	/* extral decimation */
# define ADCCON2_EDIV_64	(0 << 4)	/* 7 bits */
# define ADCCON2_EDIV_128	(1 << 4)	/* 9 bits */
# define ADCCON2_EDIV_256	(2 << 4)	/* 10 bits */
# define ADCCON2_EDIV_512	(3 << 4)	/* 12 bits */
# define ADCCON3_ECH_MASK	(0xf << 0)	/* Sequence channel select */
# define ADCCON3_ECH_SHIFT	0
# define ADCCON3_ECH_AIN0	(0 << 0)
# define ADCCON3_ECH_AIN1	(1 << 0)
# define ADCCON3_ECH_AIN2	(2 << 0)
# define ADCCON3_ECH_AIN3	(3 << 0)
# define ADCCON3_ECH_AIN4	(4 << 0)
# define ADCCON3_ECH_AIN5	(5 << 0)
# define ADCCON3_ECH_AIN6	(6 << 0)
# define ADCCON3_ECH_AIN7	(7 << 0)
# define ADCCON3_ECH_AIN0_AIN1	(8 << 0)
# define ADCCON3_ECH_AIN2_AIN3	(9 << 0)
# define ADCCON3_ECH_AIN4_AIN5	(0xa << 0)
# define ADCCON3_ECH_AIN6_AIN7	(0xb << 0)
# define ADCCON3_ECH_GND	(0xc << 0)
# define ADCCON3_ECH_VREF	(0xd << 0)
# define ADCCON3_ECH_TEMP	(0xe << 0)
# define ADCCON3_ECH_VDD_3	(0xf << 0)

sfr at 0xF2 ADCCFG;

#define nop()	_asm nop _endasm;

void
delay (unsigned char n)
{
	unsigned char i = 0;
	unsigned char j = 0;

	n++;
	while (--n != 0)
		while (--i != 0)
			while (--j != 0)
				nop();
}

void
debug_byte(uint8_t byte)
{
	uint8_t s;

	for (s = 0; s < 8; s++) {
		DEBUG = byte & 1;
		delay(5);
		byte >>= 1;
	}
}

struct cc_dma_channel __xdata dma_config;

#define ADC_LEN	6

uint16_t __xdata adc_output[ADC_LEN];

#define ADDRH(a)	(((uint16_t) (a)) >> 8)
#define ADDRL(a)	(((uint16_t) (a)))

void
adc_init(void)
{
	dma_config.cfg0 = (DMA_CFG0_WORDSIZE_16 |
			   DMA_CFG0_TMODE_REPEATED_SINGLE |
			   DMA_CFG0_TRIGGER_ADC_CHALL);
	dma_config.cfg1 = (DMA_CFG1_SRCINC_0 |
			   DMA_CFG1_DESTINC_1 |
			   DMA_CFG1_PRIORITY_NORMAL);

	dma_config.src_high = ADDRH(&ADCXDATA);
	dma_config.src_low = ADDRL(&ADCXDATA);
	dma_config.dst_high = ADDRH(adc_output);
	dma_config.dst_low = ADDRL(adc_output);
	dma_config.len_high = 0;
	dma_config.len_low = ADC_LEN;
	DMA0CFGH = ADDRH(&dma_config);
	DMA0CFGL = ADDRL(&dma_config);
	ADCCFG = ((1 << 0) |	/* acceleration */
		  (1 << 1) |	/* pressure */
		  (1 << 2) |	/* temperature */
		  (1 << 3) |	/* battery voltage */
		  (1 << 4) |	/* drogue sense */
		  (1 << 5));	/* main sense */

	ADCCON1 = (ADCCON1_STSEL_START);	/* ST bit triggers */
	ADCCON2 = (ADCCON2_SREF_VDD |		/* reference voltage is VDD */
		   ADCCON2_SDIV_512 |		/* 12 bit ADC results */
		   ADCCON2_SCH_AIN5);		/* sample all 6 inputs */
}

void
adc_run(void)
{
	DMAIRQ &= ~1;
	DMAARM |= 1;
	ADCCON1 |= ADCCON1_ST;
	while ((DMAIRQ & 1) == 0)
		;
}

main ()
{
	int i;
	P1DIR |= 2;
	CLKCON = 0;
	while (!(SLEEP & SLEEP_XOSC_STB))
		;
	while (P1 & 0x4)
		;

	adc_init();
	for (;;) {
		adc_run();
		for (i = 0; i < ADC_LEN; i++)
			debug_byte(adc_output[i]);
	}
}
