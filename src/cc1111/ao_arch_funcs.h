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

extern __xdata uint8_t	ao_spi_mutex;

#define AO_SPI_SPEED_FAST	17
#define AO_SPI_SPEED_200kHz	13

#define ao_spi_set_speed(speed) (U0GCR = (UxGCR_CPOL_NEGATIVE |		\
					  UxGCR_CPHA_FIRST_EDGE |	\
					  UxGCR_ORDER_MSB |		\
					  ((speed) << UxGCR_BAUD_E_SHIFT)))

#define ao_spi_get_mask(reg,mask,bus,speed) do {	\
		ao_mutex_get(&ao_spi_mutex);		\
		ao_spi_set_speed(speed);		\
		(reg) &= ~(mask);			\
	} while (0)

#define ao_spi_put_mask(reg,mask,bus) do {		\
	(reg) |= (mask); \
	ao_mutex_put(&ao_spi_mutex); \
	} while (0)


#define ao_spi_get_bit(reg,bit,pin,bus,speed) do {	\
		ao_mutex_get(&ao_spi_mutex);	\
		ao_spi_set_speed(speed);	\
		pin = 0;			\
	} while (0)

#define ao_spi_put_bit(reg,bit,pin,bus) do {	\
		pin = 1;			\
		ao_mutex_put(&ao_spi_mutex);	\
	} while (0)


/*
 * The SPI mutex must be held to call either of these
 * functions -- this mutex covers the entire SPI operation,
 * from chip select low to chip select high
 */

void
ao_spi_send_bus(void __xdata *block, uint16_t len) __reentrant;

void
ao_spi_recv_bus(void __xdata *block, uint16_t len) __reentrant;

#define ao_spi_send(block, len, bus) ao_spi_send_bus(block, len)
#define ao_spi_recv(block, len, bus) ao_spi_recv_bus(block, len)

void
ao_spi_init(void);

#define ao_spi_init_cs(port, mask) do {		\
		SPI_CS_PORT |= mask;		\
		SPI_CS_DIR |= mask;		\
		SPI_CS_SEL &= ~mask;		\
	} while (0)

#define cc1111_enable_output(port,dir,sel,pin,bit,v) do {	\
		pin = v;					\
		dir |= (1 << bit);				\
		sel &= ~(1 << bit);				\
	} while (0)

#define disable_unreachable	_Pragma("disable_warning 126")

#define token_paster(x,y)	x ## y
#define token_evaluator(x,y)	token_paster(x,y)
#define ao_enable_output(port,bit,pin,v) cc1111_enable_output(port,token_evaluator(port,DIR), token_evaluator(port,SEL), pin, bit, v)
#define ao_gpio_set(port, bit, pin, v) ((pin) = (v))
