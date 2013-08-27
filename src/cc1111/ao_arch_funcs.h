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

#if !HAS_SPI_0 && !HAS_SPI_1
#define HAS_SPI_0	1
#define SPI_0_ALT_2	1
#endif

#if HAS_SPI_0 && HAS_SPI_1
#define MULTI_SPI	1
#define N_SPI		2
#else
#define MULTI_SPI	0
#define N_SPI		1
#endif

extern __xdata uint8_t	ao_spi_mutex[N_SPI];

#if MULTI_SPI
#define ao_spi_get(bus)	ao_mutex_get(&ao_spi_mutex[bus])
#define ao_spi_put(bus)	ao_mutex_put(&ao_spi_mutex[bus])
#else
#define ao_spi_get(bus)	ao_mutex_get(&ao_spi_mutex[0])
#define ao_spi_put(bus)	ao_mutex_put(&ao_spi_mutex[0])
#endif

#define AO_SPI_SPEED_FAST	17
#define AO_SPI_SPEED_200kHz	13

#if MULTI_SPI
#define ao_spi_set_speed(bus,speed) (*(bus ? &U1GCR : &U0GCR) =(UxGCR_CPOL_NEGATIVE | \
								UxGCR_CPHA_FIRST_EDGE |	\
								UxGCR_ORDER_MSB | \
								((speed) << UxGCR_BAUD_E_SHIFT)))
#else
#define ao_spi_set_speed(bus,speed) (U0GCR = (UxGCR_CPOL_NEGATIVE |	\
					      UxGCR_CPHA_FIRST_EDGE |	\
					      UxGCR_ORDER_MSB |		\
					      ((speed) << UxGCR_BAUD_E_SHIFT)))
#endif

#define ao_spi_get_slave(bus) do {			\
		ao_spi_get(bus);			\
		ao_spi_set_speed(bus,AO_SPI_SPEED_FAST);	\
	} while (0)

#define ao_spi_put_slave(bus) do {		\
		ao_spi_put(bus);		\
	} while (0)

#define ao_spi_get_mask(reg,mask,bus,speed) do {	\
		ao_spi_get(bus);			\
		ao_spi_set_speed(bus,speed);		\
		(reg) &= ~(mask);			\
	} while (0)

#define ao_spi_put_mask(reg,mask,bus) do {		\
	(reg) |= (mask); \
	ao_spi_put(bus); \
	} while (0)


#define ao_spi_get_bit(reg,bit,pin,bus,speed) do {	\
		ao_spi_get(bus);			\
		ao_spi_set_speed(bus,speed);		\
		pin = 0;				\
	} while (0)

#define ao_spi_put_bit(reg,bit,pin,bus) do {	\
		pin = 1;			\
		ao_spi_put(bus);		\
	} while (0)


/*
 * The SPI mutex must be held to call either of these
 * functions -- this mutex covers the entire SPI operation,
 * from chip select low to chip select high
 */

#if MULTI_SPI
void
ao_spi_send(void __xdata *block, uint16_t len, uint8_t bus) __reentrant;

void
ao_spi_recv(void __xdata *block, uint16_t len, uint8_t bus) __reentrant;
#else
void
ao_spi_send_bus(void __xdata *block, uint16_t len) __reentrant;

void
ao_spi_recv_bus(void __xdata *block, uint16_t len) __reentrant;

#define ao_spi_send(block, len, bus) ao_spi_send_bus(block, len)
#define ao_spi_recv(block, len, bus) ao_spi_recv_bus(block, len)
#endif

#if AO_SPI_SLAVE
void
ao_spi_send_wait(void);

void
ao_spi_recv_wait(void);
#endif

void
ao_spi_init(void);

#define token_paster(x,y)	x ## y
#define token_paster3(x,y,z)	x ## y ## z
#define token_evaluator(x,y)	token_paster(x,y)
#define token_evaluator3(x,y,z)	token_paster3(x,y,z)

#define ao_spi_init_cs(port, mask) do {			\
		port |= mask;				\
		token_evaluator(port,DIR) |= mask;	\
		token_evaluator(port,SEL) &= ~mask;	\
	} while (0)

#define cc1111_enable_output(port,dir,sel,pin,bit,v) do {	\
		pin = v;					\
		dir |= (1 << bit);				\
		sel &= ~(1 << bit);				\
	} while (0)

#define disable_unreachable	_Pragma("disable_warning 126")

#define ao_enable_output(port,bit,pin,v) cc1111_enable_output(port,token_evaluator(port,DIR), token_evaluator(port,SEL), pin, bit, v)
#define ao_gpio_set(port, bit, pin, v) ((pin) = (v))
#define ao_gpio_get(port, bit, pin) (pin)

