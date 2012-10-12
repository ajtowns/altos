/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

/*
 * ATtiny USI as an I2C interface
 */

#define I2C_USICR ((0 << USISIE) |	/* No start condition interrupt */ \
		   (0 << USIOIE) |	/* No counter overflow interrupt */ \
		   (1 << USIWM1) |	/* Two-wire mode */		\
		   (0 << USIWM0) |	/*   ... */			\
		   (1 << USICS1) |	/* Software clock strobe */	\
		   (0 << USICS0) |	/*   ... */			\
		   (1 << USICLK))	/*   ... */			\

#define I2C_USICR_TICK (I2C_USICR | (1 << USITC))	/* Toggle the clock on every write */

#define I2C_USISR_1BIT	((1<<USISIF)|	/* Clear start condition flag */ \
			 (1<<USIOIF)|	/* Clear overflow flag */	\
			 (1<<USIPF)|	/* Clear stop condition flag */	\
			 (1<<USIDC)|	/* Clear data collision flag */	\
			 (0xE<<USICNT0)) /* Set counter value to 0xe */

#define I2C_USISR_8BIT	((1<<USISIF)|	/* Clear start condition flag */ \
			 (1<<USIOIF)|	/* Clear overflow flag */	\
			 (1<<USIPF)|	/* Clear stop condition flag */	\
			 (1<<USIDC)|	/* Clear data collision flag */	\
			 (0x0<<USICNT0)) /* Set counter value to 0 */

#define T2_TWI    5 		/* >4.7μs */
#define T4_TWI    4 		/* >4.0μs */

static inline void ao_i2c_transfer(uint8_t sr)
{
	USISR = sr;
	for (;;) {
		ao_delay_us(T2_TWI);

		/* Clock high */
		USICR = I2C_USICR_TICK;

		/* Wait for clock high (clock stretching) */
		ao_delay_us(T4_TWI);
		while(!(I2C_PIN & (1<<I2C_PIN_SCL)))
			;

		/* Clock low */
		USICR = I2C_USICR_TICK;

		/* Check for transfer complete */
		if (USISR & (1 << USIOIF))
			break;
	}
	ao_delay_us(T2_TWI);
}

static inline uint8_t ao_i2c_get_byte(uint8_t sr)
{
	uint8_t	ret;

	/* Set SDA to input */
	I2C_DIR &= ~(1<<I2C_PIN_SDA);

	ao_i2c_transfer(sr);

	ret = USIDR;
	USIDR = 0xff;

	/* Set SDA to output */
	I2C_DIR |= (1<<I2C_PIN_SDA);

	return ret;
}

static uint8_t
ao_i2c_write_byte(uint8_t byte)
{
	/* Pull SCL low */
	I2C_PORT &= ~(1<<I2C_PIN_SCL);

	/* Write the byte */
	USIDR = byte;
      
	/* Clock and verify (N)ACK from slave */

	ao_i2c_transfer(I2C_USISR_8BIT);

	if (ao_i2c_get_byte(I2C_USISR_1BIT) & 0x80)
		return 0;

	return 1;
}

static uint8_t
ao_i2c_read_byte(uint8_t ack)
{
	uint8_t	ret;

	/* Read the data */
	ret = ao_i2c_get_byte(I2C_USISR_8BIT);

	/* Ack it */
	USIDR = ack;
	ao_i2c_transfer(I2C_USISR_8BIT);

	return ret;
}

uint8_t
ao_i2c_start_bus(uint8_t address)
{
	/* Release SCL to ensure that (repeated) Start can be performed */

	I2C_PORT |= (1<<I2C_PIN_SCL);

	while( !(I2C_PORT & (1<<I2C_PIN_SCL)) )
		;
	ao_delay_us(T2_TWI);

	/* Generate Start Condition */

	/* Pull SDA low */
	I2C_PORT &= ~(1<<I2C_PIN_SDA);
	ao_delay_us(T4_TWI);                         

	/* Pull SCL low */
	I2C_PORT &= ~(1<<I2C_PIN_SCL);

	/* Raise SDA */
	I2C_PORT |= (1<<I2C_PIN_SDA);

	return ao_i2c_write_byte(address);
}

static void
ao_i2c_stop_bus(void)
{
	/* Pull SDA low. */
	I2C_PORT &= ~(1<<I2C_PIN_SDA);

	/* Release SCL. */
	I2C_PORT |= (1<<I2C_PIN_SCL);

	/* Wait for SCL to go high */
	while( !(I2C_PIN & (1<<I2C_PIN_SCL)) );
	ao_delay_us(T4_TWI);

	/* Raise SDA */
	I2C_PORT |= (1<<I2C_PIN_SDA);
	ao_delay_us(T2_TWI);
}

/* Send bytes over SPI.
 *
 * This just polls; the SPI is set to go as fast as possible,
 * so using interrupts would take way too long
 */
uint8_t
ao_i2c_send_bus(void __xdata *block, uint16_t len, uint8_t stop)
{
	uint8_t	*d = block;

	while (len--)
		if (!ao_i2c_write_byte (*d++))
			return 0;
	if (stop)
		ao_i2c_stop_bus();
	return 1;
}

/* Send bytes over SPI.
 *
 * This just polls; the SPI is set to go as fast as possible,
 * so using interrupts would take way too long
 */
uint8_t
ao_i2c_send_fixed_bus(uint8_t d, uint16_t len, uint8_t stop)
{
	while (len--)
		if (!ao_i2c_write_byte (d))
			return 0;
	if (stop)
		ao_i2c_stop_bus();
	return 1;
}

/* Receive bytes over SPI.
 *
 * Poll, sending zeros and reading data back
 */
uint8_t
ao_i2c_recv_bus(void __xdata *block, uint16_t len, uint8_t stop)
{
	uint8_t	*d = block;

	while (len--)
		*d++ = ao_i2c_read_byte (len ? 0x00 : 0xff);
	if (stop)
		ao_i2c_stop_bus();
	return 1;
}

/*
 * Initialize USI
 *
 * Chip select is the responsibility of the caller
 */

void
ao_i2c_init(void)
{
	/* Pull-ups on SDA and SCL */
	I2C_PORT |= (1<<I2C_PIN_SDA);
	I2C_PORT |= (1<<I2C_PIN_SCL);
  
	/* SCL and SDA are outputs */
	I2C_DIR  |= (1<<I2C_PIN_SCL);
	I2C_DIR  |= (1<<I2C_PIN_SDA);
  
	USIDR =  0xFF;
	USICR =  I2C_USICR;
}
