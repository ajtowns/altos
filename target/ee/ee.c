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

sfr at 0x80 P0;
sfr at 0x90 P1;
sfr at 0xA0 P2;
sfr at 0xC6 CLKCON;

sfr at 0xF1 PERCFG;
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

#define MOSI	P1_5
#define MISO	P1_4
#define SCK	P1_3
#define CS	P1_2

#define DEBUG	P1_1

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

void
cs(uint8_t b)
{
	CS = b;
	delay(1);
}

void
out_bit(uint8_t b)
{
	MOSI = b;
	SCK = 1;
	delay(1);
	SCK = 0;
	delay(1);
}

void
out_byte(uint8_t byte)
{
	uint8_t s;

	for (s = 0; s < 8; s++) {
		uint8_t b = (byte & 0x80) ? 1 : 0;
		out_bit(b);
		byte <<= 1;
	}
}

uint8_t
in_bit(void)
{
	uint8_t	b;
	SCK = 1;
	delay(1);
	b = MISO;
	SCK = 0;
	delay(1);
	return b;
}

uint8_t
in_byte(void)
{
	uint8_t byte = 0;
	uint8_t s;
	uint8_t b;

	for (s = 0; s < 8; s++) {
		b = in_bit();
		byte = byte << 1;
		byte |= s;
	}
	return byte;
}

uint8_t
rdsr(void)
{
	uint8_t status;
	cs(0);
	out_byte(0x05);
	status = in_byte();
	cs(1);
	return status;
}

void
wrsr(uint8_t status)
{
	cs(0);
	out_byte(0x01);
	out_byte(status);
	cs(1);
}
	
void
wren(void)
{
	cs(0);
	out_byte(0x06);
	cs(1);
}

void
write(uint32_t addr, uint8_t *bytes, uint16_t len)
{
	wren();
	cs(0);
	out_byte(0x02);
	out_byte(addr >> 16);
	out_byte(addr >> 8);
	out_byte(addr);
	while (len-- > 0)
		out_byte(*bytes++);
	cs(1);
	for (;;) {
		uint8_t status = rdsr();
		if ((status & (1 << 0)) == 0)
			break;
	}
}

void
read(uint32_t addr, uint8_t *bytes, uint16_t len)
{
	cs(0);
	out_byte(0x03);
	out_byte(addr >> 16);
	out_byte(addr >> 8);
	out_byte(addr);
	while (len-- > 0)
		*bytes++ = in_byte();
	cs(1);
}

void
debug_byte(uint8_t byte)
{
	uint8_t s;

	for (s = 0; s < 8; s++) {
		DEBUG = byte & 1;
		delay(2);
		byte >>= 1;
	}
}

#define STRING	"hi"
#define LENGTH	2

main ()
{
	uint8_t	status;
	uint8_t buf[LENGTH];
	int i;

	CLKCON = 0;
	
	CS = 1;
	SCK = 0;
	P1DIR = ((1 << 5) |
		 (1 << 4) |
		 (1 << 3) |
		 (1 << 2) |
		 (1 << 1));
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
