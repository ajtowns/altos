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

/*
 * ao_spi.c
 */

#define ao_spi_get_mask(reg,mask,bus,speed) do {	\
		(reg) &= ~(mask);			\
	} while (0)

#define ao_spi_put_mask(reg,mask,bus) do {	\
		(reg) |= (mask);		\
	} while (0)

#define ao_spi_get_bit(reg,bit,pin,bus,speed) ao_spi_get_mask(reg,(1<<(bit)),bus,speed)

#define ao_spi_put_bit(reg,bit,pin,bus) ao_spi_put_mask(reg,(1<<(bit)),bus)

#define ao_gpio_token_paster(x,y)		x ## y
#define ao_gpio_token_evaluator(x,y)	ao_gpio_token_paster(x,y)

#define ao_gpio_set(port, bit, pin, v) do {	\
		if (v)				\
			PORTB |= (1 << bit);	\
		else				\
			PORTB &= ~(1 << bit);	\
	} while (0)

#define ao_gpio_get(port, bit, pin)	((PORTB >> (bit)) & 1)

/*
 * The SPI mutex must be held to call either of these
 * functions -- this mutex covers the entire SPI operation,
 * from chip select low to chip select high
 */

#define ao_enable_output(port, bit, pin, v) do {			\
		ao_gpio_set(port, bit, pin, v);				\
		ao_gpio_token_evaluator(DDR,port) |= (1 << bit);	\
	} while (0)


void
ao_spi_send_bus(void __xdata *block, uint16_t len) __reentrant;

void
ao_spi_recv_bus(void __xdata *block, uint16_t len) __reentrant;

#define ao_spi_send(block, len, bus) ao_spi_send_bus(block, len)
#define ao_spi_recv(block, len, bus) ao_spi_recv_bus(block, len)

void
ao_spi_init(void);

#define ao_spi_get(bus, speed)
#define ao_spi_put(bus)

#define ao_spi_init_cs(port, mask) do {		\
		PORTB |= (mask);		\
		DDRB |= (mask);		\
	} while (0)

/* I2C */

void
ao_i2c_get(uint8_t i2c_index);

uint8_t
ao_i2c_start_bus(uint8_t address);

#define ao_i2c_start(i,a)	ao_i2c_start_bus(a)

void
ao_i2c_put(uint8_t i2c_index);

uint8_t
ao_i2c_send_bus(void *block, uint16_t len, uint8_t stop);

#define ao_i2c_send(b,l,i,s) ao_i2c_send_bus(b,l.s)

uint8_t
ao_i2c_send_fixed_bus(uint8_t value, uint16_t len, uint8_t stop);

#define ao_i2c_send_fixed(v,l,i,s) ao_i2c_send_fixed_bus(v,l.s)

uint8_t
ao_i2c_recv_bus(void *block, uint16_t len, uint8_t stop);

#define ao_i2c_recv(b,l,i,s) ao_i2c_recv_bus(b,l.s)

void
ao_i2c_init(void);

/* notask.c */

uint8_t
ao_sleep(__xdata void *wchan);

void
ao_wakeup(__xdata void *wchan);

extern alt_t	ao_max_height;

extern void ao_report_altitude(void);

void ao_delay_us(uint16_t us);

