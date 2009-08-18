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
 * Validate the SPI-connected EEPROM
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

#define BITBANG	0
#define USART	1

#define nop()	_asm nop _endasm;

void
delay (unsigned char n)
{
	unsigned char i = 0;
	unsigned char j = 0;

	while (--n != 0)
		while (--i != 0)
			while (--j != 0)
				nop();
}

#if BITBANG

/*
 * This version directly manipulates the GPIOs to synthesize SPI
 */

void
bitbang_cs(uint8_t b)
{
	SCK = 0;
	CS = b;
	delay(1);
}

void
bitbang_out_bit(uint8_t b)
{
	MOSI = b;
	delay(1);
	SCK = 1;
	delay(1);
	SCK = 0;
}

void
bitbang_out_byte(uint8_t byte)
{
	uint8_t s;

	for (s = 0; s < 8; s++) {
		uint8_t b = (byte & 0x80) ? 1 : 0;
		bitbang_out_bit(b);
		byte <<= 1;
	}
}

uint8_t
bitbang_in_bit(void)
{
	uint8_t	b;

	delay(1);
	SCK = 1;
	delay(1);
	b = MISO;
	SCK = 0;
	return b;
}

uint8_t
bitbang_in_byte(void)
{
	uint8_t byte = 0;
	uint8_t s;
	uint8_t b;

	for (s = 0; s < 8; s++) {
		b = bitbang_in_bit();
		byte = byte << 1;
		byte |= b;
	}
	return byte;
}

void
bit_bang_init(void)
{
	CS = 1;
	SCK = 0;
	P1DIR = ((1 << 5) |
		 (0 << 4) |
		 (1 << 3) |
		 (1 << 2) |
		 (1 << 1));
}

#define spi_init()	bitbang_init()
#define spi_out_byte(b)	bitbang_out_byte(b)
#define spi_in_byte()	bitbang_in_byte()
#define spi_cs(b)	bitbang_cs(b)
#endif

#if USART

/*
 * This version uses the USART in SPI mode
 */
void
usart_init(void)
{
	/*
	 * Configure our chip select line
	 */
	CS = 1;
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
	PERCFG = (PERCFG_U1CFG_ALT_1 |
		  PERCFG_U0CFG_ALT_2);

	/*
	 * Make the SPI pins controlled by the SPI
	 * hardware
	 */
	P1SEL |= ((1 << 5) | (1 << 4) | (1 << 3));

	/*
	 * SPI in master mode
	 */
	U0CSR = (UxCSR_MODE_SPI |
		 UxCSR_MASTER);

	/*
	 * The cc1111 is limited to a 24/8 MHz SPI clock,
	 * while the 25LC1024 is limited to 20MHz. So,
	 * use the 3MHz clock (BAUD_E 17, BAUD_M 0)
	 */
	U0BAUD = 0;
	U0GCR = (UxGCR_CPOL_NEGATIVE |
		 UxGCR_CPHA_FIRST_EDGE |
		 UxGCR_ORDER_MSB |
		 (17 << UxGCR_BAUD_E_SHIFT));
}

void
usart_cs(uint8_t b)
{
	CS = b;
}

uint8_t
usart_in_out(uint8_t byte)
{
	U0DBUF = byte;
	while ((U0CSR & UxCSR_TX_BYTE) == 0)
		;
	U0CSR &= ~UxCSR_TX_BYTE;
	return U0DBUF;
}

void
usart_out_byte(uint8_t byte)
{
	(void) usart_in_out(byte);
}

uint8_t
usart_in_byte(void)
{
	return usart_in_out(0xff);
}

#define spi_init()	usart_init()
#define spi_out_byte(b)	usart_out_byte(b)
#define spi_in_byte()	usart_in_byte()
#define spi_cs(b)	usart_cs(b)

#endif

uint8_t
rdsr(void)
{
	uint8_t status;
	spi_cs(0);
	spi_out_byte(0x05);
	status = spi_in_byte();
	spi_cs(1);
	return status;
}

void
wrsr(uint8_t status)
{
	spi_cs(0);
	spi_out_byte(0x01);
	spi_out_byte(status);
	spi_cs(1);
}

void
wren(void)
{
	spi_cs(0);
	spi_out_byte(0x06);
	spi_cs(1);
}

void
write(uint32_t addr, uint8_t *bytes, uint16_t len)
{
	wren();
	spi_cs(0);
	spi_out_byte(0x02);
	spi_out_byte(addr >> 16);
	spi_out_byte(addr >> 8);
	spi_out_byte(addr);
	while (len-- > 0)
		spi_out_byte(*bytes++);
	spi_cs(1);
	for (;;) {
		uint8_t status = rdsr();
		if ((status & (1 << 0)) == 0)
			break;
	}
}

void
read(uint32_t addr, uint8_t *bytes, uint16_t len)
{
	spi_cs(0);
	spi_out_byte(0x03);
	spi_out_byte(addr >> 16);
	spi_out_byte(addr >> 8);
	spi_out_byte(addr);
	while (len-- > 0)
		*bytes++ = spi_in_byte();
	spi_cs(1);
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

#define STRING	"\360\252"
#define LENGTH	2

main ()
{
	uint8_t	status;
	uint8_t buf[LENGTH];
	int i;

	P1DIR |= 2;
	CLKCON = 0;
	while (!(SLEEP & SLEEP_XOSC_STB))
		;

	spi_init();

	status = rdsr();
	/*
	 * Turn off both block-protect bits
	 */
	status &= ~((1 << 3) | (1 << 2));
	/*
	 * Turn off write protect enable
	 */
	status &= ~(1 << 7);
	wrsr(status);
	write(0x0, STRING, LENGTH);
	for (;;) {
		read(0x0, buf, LENGTH);
		for (i = 0; i < LENGTH; i++)
			debug_byte(buf[i]);
	}
}
