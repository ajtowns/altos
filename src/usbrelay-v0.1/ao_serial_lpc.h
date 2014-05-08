#define AO_LPC_USARTCLK 12000000

static const struct {
	uint16_t dl;
	uint8_t divaddval;
	uint8_t mulval;
} ao_usart_speeds[] = {
	[AO_SERIAL_SPEED_4800] = { /* actual =  4800.00 */
		.dl = 125,
		.divaddval = 1,
		.mulval = 4
	},
	[AO_SERIAL_SPEED_9600] = { /* actual =  9603.07 */
		.dl = 71,
		.divaddval = 1,
		.mulval = 10
	},
	[AO_SERIAL_SPEED_19200] = { /* actual = 19181.59 */
		.dl = 23,
		.divaddval = 7,
		.mulval = 10
	},
	[AO_SERIAL_SPEED_57600] = { /* actual = 57692.31 */
		.dl = 7,
		.divaddval = 6,
		.mulval = 7
	},
	[AO_SERIAL_SPEED_115200] = { /* actual = 115384.6 */
		.dl = 4,
		.divaddval = 5,
		.mulval = 8
	},
};
