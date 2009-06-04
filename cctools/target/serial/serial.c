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
 * Validate UART1
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

#define MOSI	P1_5
#define MISO	P1_4
#define SCK	P1_3
#define CS	P1_2

#define DEBUG	P1_1

#define USART	1

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

/*
 * This version uses the USART in SPI mode
 */
void
usart_init(void)
{
	P1DIR |= (1 << 2);
	/*
	 * Configure the peripheral pin choices
	 * for both of the serial ports
	 *
	 * Note that telemetrum will use U1CFG_ALT_2
	 * but that overlaps with SPI ALT_2, so until
	 * we can test that this works, we'll set this
	 * to ALT_1
	 */
	PERCFG = (PERCFG_U1CFG_ALT_2 |
		  PERCFG_U0CFG_ALT_1);

	/*
	 * Make the UART pins controlled by the UART
	 * hardware
	 */
	P1SEL |= ((1 << 6) | (1 << 7));

	/*
	 * UART mode with the receiver enabled
	 */
	U1CSR = (UxCSR_MODE_UART |
		 UxCSR_RE);
	/*
	 * Pick a 38.4kbaud rate
	 */
	U1BAUD = 163;
	U1GCR = 10 << UxGCR_BAUD_E_SHIFT;	/* 38400 */
//	U1GCR = 3 << UxGCR_BAUD_E_SHIFT;	/* 300 */
	/*
	 * Reasonable serial parameters
	 */
	U1UCR = (UxUCR_FLUSH |
		 UxUCR_FLOW_DISABLE |
		 UxUCR_D9_ODD_PARITY |
		 UxUCR_BIT9_8_BITS |
		 UxUCR_PARITY_DISABLE |
		 UxUCR_SPB_2_STOP_BITS |
		 UxUCR_STOP_HIGH |
		 UxUCR_START_LOW);
}

void
usart_out_byte(uint8_t byte)
{
	U1DBUF = byte;
	while (!UTX1IF)
		;
	UTX1IF = 0;
}

void
usart_out_string(uint8_t *string)
{
	uint8_t	b;

	while (b = *string++)
		usart_out_byte(b);
}

uint8_t
usart_in_byte(void)
{
	uint8_t b;
	while ((U1CSR & UxCSR_RX_BYTE) == 0)
		;
	b = U1DBUF;
	U1CSR &= ~UxCSR_RX_BYTE;
	return b;
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

main ()
{
	P1DIR |= 2;
	CLKCON = 0;
	while (!(SLEEP & SLEEP_XOSC_STB))
		;

	usart_init();

	for (;;) {
		usart_out_string("hello world\r\n");
		debug_byte(usart_in_byte());
	}

}
