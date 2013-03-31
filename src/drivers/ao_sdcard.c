/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#include "ao.h"
#include "ao_sdcard.h"

#define ao_sdcard_get_slow()		ao_spi_get(AO_SDCARD_SPI_BUS, AO_SPI_SPEED_250kHz)
#define ao_sdcard_get()			ao_spi_get(AO_SDCARD_SPI_BUS, AO_SPI_SPEED_FAST)
#define ao_sdcard_put()			ao_spi_put(AO_SDCARD_SPI_BUS)
#define ao_sdcard_send_fixed(d,l)	ao_spi_send_fixed((d), (l), AO_SDCARD_SPI_BUS)
#define ao_sdcard_send(d,l)		ao_spi_send((d), (l), AO_SDCARD_SPI_BUS)
#define ao_sdcard_recv(d,l)		ao_spi_recv((d), (l), AO_SDCARD_SPI_BUS)
#define ao_sdcard_select()		ao_gpio_set(AO_SDCARD_SPI_CS_PORT,AO_SDCARD_SPI_CS_PIN,AO_SDCARD_SPI_CS,0)
#define ao_sdcard_deselect()		ao_gpio_set(AO_SDCARD_SPI_CS_PORT,AO_SDCARD_SPI_CS_PIN,AO_SDCARD_SPI_CS,1)

#define SDCARD_DEBUG	0

static uint8_t	initialized;
static uint8_t	present;
static uint8_t	mutex;
static enum ao_sdtype sdtype;

#define ao_sdcard_lock()	ao_mutex_get(&mutex)
#define ao_sdcard_unlock()	ao_mutex_put(&mutex)

#if 0
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

/*
 * Send an SD command and await the status reply
 */

static uint8_t
ao_sdcard_send_cmd(uint8_t cmd, uint32_t arg)
{
	uint8_t	data[6];
	uint8_t	reply;
	int i;

	DBG ("\tsend_cmd %d arg %08x\n", cmd, arg);
	if (cmd != SDCARD_GO_IDLE_STATE) {
		for (i = 0; i < SDCARD_CMD_TIMEOUT; i++) {
			ao_sdcard_recv(&reply, 1);
			if (reply == 0xff)
				break;
		}
		if (i == SDCARD_CMD_TIMEOUT)
			return SDCARD_STATUS_TIMEOUT;
	}
	
	data[0] = cmd & 0x3f | 0x40;
	data[1] = arg >> 24;
	data[2] = arg >> 16;
	data[3] = arg >> 8;
	data[4] = arg;
	if (cmd == SDCARD_GO_IDLE_STATE)
		data[5] = 0x95;	/* Valid for 0 arg */
	else if (cmd == SDCARD_SEND_IF_COND)
		data[5] = 0x87;	/* Valid for 0x1aa arg */
	else
		data[5] = 0xff;	/* no CRC */
	ao_sdcard_send(data, 6);

	/* The first reply byte will be the status,
	 * which must have the high bit clear
	 */
	for (i = 0; i < SDCARD_CMD_TIMEOUT; i++) {
		ao_sdcard_recv(&reply, 1);
		DBG ("\t\tgot byte %02x\n", reply);
		if ((reply & 0x80) == 0)
			return reply;
	}
	return SDCARD_STATUS_TIMEOUT;
}

/*
 * Retrieve any reply, discarding the trailing CRC byte
 */
static void
ao_sdcard_recv_reply(uint8_t *reply, int len)
{
	uint8_t	discard;

	if (len)
		ao_sdcard_recv(reply, len);
	/* trailing byte */
	ao_sdcard_recv(&discard, 1);
}

/*
 * Wait while the card is busy. The
 * card will return a stream of 0xff
 * until it isn't busy anymore
 */
static void
ao_sdcard_wait_busy(void)
{
	uint8_t	v;

	do {
		ao_sdcard_recv(&v, 1);
	} while (v != 0xff);
	ao_sdcard_send_fixed(0xff, 1);
}

static uint8_t
ao_sdcard_go_idle_state(void)
{
	uint8_t	ret;

	DBG ("go_idle_state\n");
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_GO_IDLE_STATE, 0);
	ao_sdcard_recv_reply(NULL, 0);
	ao_sdcard_deselect();
	DBG ("\tgo_idle_state status %02x\n", ret);
	return ret;
}

static uint8_t
ao_sdcard_send_op_cond(void)
{
	uint8_t	ret;

	DBG ("send_op_cond\n");
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_SEND_OP_COND, 0);
	ao_sdcard_recv_reply(NULL, 0);
	ao_sdcard_deselect();
	DBG ("\tsend_op_cond %02x\n", ret);
	return ret;
}

static uint8_t
ao_sdcard_send_if_cond(uint32_t arg, uint8_t send_if_cond_response[4])
{
	uint8_t ret;

	DBG ("send_if_cond\n");
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_SEND_IF_COND, arg);
	if (ret != SDCARD_STATUS_IDLE_STATE) {
		DBG ("\tsend_if_cond failed %02x\n", ret);
		return ret;
	}
	ao_sdcard_recv_reply(send_if_cond_response, 4);
	DBG ("send_if_cond status %02x response %02x %02x %02x %02x\n",
		ret,
		send_if_cond_response[0],
		send_if_cond_response[1],
		send_if_cond_response[2],
		send_if_cond_response[3]);
	ao_sdcard_deselect();
	return ret;
}

static uint8_t
ao_sdcard_send_status(void)
{
	uint8_t ret;

	DBG ("send_status\n");
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_SEND_STATUS, 0);
	ao_sdcard_recv_reply(NULL, 0);
	if (ret != SDCARD_STATUS_READY_STATE)
		DBG ("\tsend_if_cond failed %02x\n", ret);
	return ret;
}

static uint8_t
ao_sdcard_set_blocklen(uint32_t blocklen)
{
	uint8_t ret;

	DBG ("set_blocklen %d\n", blocklen);
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_SET_BLOCKLEN, blocklen);
	ao_sdcard_recv_reply(NULL, 0);
	if (ret != SDCARD_STATUS_READY_STATE)
		DBG ("\tsend_if_cond failed %02x\n", ret);
	return ret;
}

static uint8_t
ao_sdcard_app_cmd(void)
{
	uint8_t	ret;

	DBG ("app_cmd\n");
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_APP_CMD, 0);
	ao_sdcard_recv_reply(NULL, 0);
	ao_sdcard_deselect();
	DBG ("\tapp_cmd status %02x\n");
	return ret;
}

static uint8_t
ao_sdcard_app_send_op_cond(uint32_t arg)
{
	uint8_t	ret;

	ret = ao_sdcard_app_cmd();
	if (ret != SDCARD_STATUS_IDLE_STATE)
		return ret;
	DBG("send_op_comd\n");
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_APP_SEND_OP_COMD, arg);
	ao_sdcard_recv_reply(NULL, 0);
	ao_sdcard_deselect();
	DBG ("\tapp_send_op_cond status %02x\n", ret);
	return ret;
}

static uint8_t
ao_sdcard_read_ocr(uint8_t read_ocr_response[4])
{
	uint8_t	ret;

	DBG ("read_ocr\n");
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_READ_OCR, 0);
	if (ret != SDCARD_STATUS_READY_STATE)
		DBG ("\tread_ocr failed %02x\n", ret);
	else {
		ao_sdcard_recv_reply(read_ocr_response, 4);
		DBG ("\tread_ocr status %02x response %02x %02x %02x %02x\n", ret,
			read_ocr_response[0], read_ocr_response[1],
			read_ocr_response[2], read_ocr_response[3]);
	}
	ao_sdcard_deselect();
	return ret;
}

static void
ao_sdcard_setup(void)
{
	int	i;
	uint8_t	ret;
	uint8_t	response[10];

	DBG ("Testing sdcard\n");

	ao_sdcard_get_slow();
	/*
	 * min 74 clocks with CS high
	 */
	ao_sdcard_send_fixed(0xff, 10);

	ao_delay(AO_MS_TO_TICKS(10));

	/* Reset the card and get it into SPI mode */

	for (i = 0; i < SDCARD_IDLE_WAIT; i++) {
		if (ao_sdcard_go_idle_state() == SDCARD_STATUS_IDLE_STATE)
			break;
	}
	if (i == SDCARD_IDLE_WAIT)
		goto bail;

	/* Figure out what kind of card we have */

	sdtype = ao_sdtype_unknown;

	if (ao_sdcard_send_if_cond(0x1aa, response) == SDCARD_STATUS_IDLE_STATE) {
		uint32_t	arg = 0;
		uint8_t		sdver2 = 0;

		/* Check for SD version 2 */
		if ((response[2] & 0xf) == 1 && response[3] == 0xaa) {
			arg = 0x40000000;
			sdver2 = 1;
		}

		for (i = 0; i < SDCARD_IDLE_WAIT; i++) {
			ret = ao_sdcard_app_send_op_cond(arg);
			if (ret != SDCARD_STATUS_IDLE_STATE)
				break;
		}
		if (ret != SDCARD_STATUS_READY_STATE) {
			/* MMC */
			for (i = 0; i < SDCARD_IDLE_WAIT; i++) {
				ret = ao_sdcard_send_op_cond();
				if (ret != SDCARD_STATUS_IDLE_STATE)
					break;
			}
			if (ret != SDCARD_STATUS_READY_STATE)
				goto bail;
			sdtype = ao_sdtype_mmc3;
		} else {
			/* SD */
			if (sdver2 != 0) {
				ret = ao_sdcard_read_ocr(response);
				if (ret != SDCARD_STATUS_READY_STATE)
					goto bail;
				if ((response[0] & 0xc0) == 0xc0)
					sdtype = ao_sdtype_sd2block;
				else
					sdtype = ao_sdtype_sd2byte;
			} else {
				sdtype = ao_sdtype_sd1;
			}
		}

		/* For everything but SDHC cards, set the block length */
		if (sdtype != ao_sdtype_sd2block) {
			ret = ao_sdcard_set_blocklen(512);
			if (ret != SDCARD_STATUS_READY_STATE)
				DBG ("set_blocklen failed, ignoring\n");
		}
	}

	DBG ("SD card detected, type %d\n", sdtype);
bail:
	ao_sdcard_put();
}

static uint8_t
ao_sdcard_wait_block_start(void)
{
	int	i;
	uint8_t	v;

	DBG ("\twait_block_start\n");
	for (i = 0; i < SDCARD_BLOCK_TIMEOUT; i++) {
		ao_sdcard_recv(&v, 1);
		DBG("\t\trecv %02x\n", v);
		if (v != 0xff)
			break;
	}
	return v;
}

/*
 * Read a block of 512 bytes from the card
 */
uint8_t
ao_sdcard_read_block(uint32_t block, uint8_t *data)
{
	uint8_t	ret;
	uint8_t crc[2];

	ao_sdcard_lock();
	if (!initialized) {
		ao_sdcard_setup();
		initialized = 1;
		if (sdtype != ao_sdtype_unknown)
			present = 1;
	}
	if (!present) {
		ao_sdcard_unlock();
		return 0;
	}
	if (sdtype != ao_sdtype_sd2block)
		block <<= 9;
	ao_sdcard_get();
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_READ_BLOCK, block);
	ao_sdcard_recv_reply(NULL, 0);
	if (ret != SDCARD_STATUS_READY_STATE)
		goto bail;

	/* Wait for the data start block marker */
	if (ao_sdcard_wait_block_start() != SDCARD_DATA_START_BLOCK) {
		ret = 0x3f;
		goto bail;
	}

	ao_sdcard_recv(data, 512);
	ao_sdcard_recv(crc, 2);
bail:
	ao_sdcard_deselect();
	ao_sdcard_put();
	ao_sdcard_unlock();
	return ret == SDCARD_STATUS_READY_STATE;
}

/*
 * Write a block of 512 bytes to the card
 */
uint8_t
ao_sdcard_write_block(uint32_t block, uint8_t *data)
{
	uint8_t	ret;
	uint8_t	response;
	uint8_t	start_block[2];
	int	i;

	ao_sdcard_lock();
	if (!initialized) {
		ao_sdcard_setup();
		initialized = 1;
		if (sdtype != ao_sdtype_unknown)
			present = 1;
	}
	if (!present) {
		ao_sdcard_unlock();
		return 0;
	}
	if (sdtype != ao_sdtype_sd2block)
		block <<= 9;
	ao_sdcard_get();
	ao_sdcard_select();

	ret = ao_sdcard_send_cmd(SDCARD_WRITE_BLOCK, block);
	ao_sdcard_recv_reply(NULL, 0);
	if (ret != SDCARD_STATUS_READY_STATE)
		goto bail;

	/* Write a pad byte followed by the data start block marker */
	start_block[0] = 0xff;
	start_block[1] = SDCARD_DATA_START_BLOCK;
	ao_sdcard_send(start_block, 2);

	/* Send the data */
	ao_sdcard_send(data, 512);

	/* Fake the CRC */
	ao_sdcard_send_fixed(0xff, 2);

	/* See if the card liked the data */
	ao_sdcard_recv(&response, 1);
	if ((response & SDCARD_DATA_RES_MASK) != SDCARD_DATA_RES_ACCEPTED) {
		ret = 0x3f;
		goto bail;
	}
		
	/* Wait for the bus to go idle (should be done with an interrupt) */
	for (i = 0; i < SDCARD_IDLE_TIMEOUT; i++) {
		ao_sdcard_recv(&response, 1);
		if (response == 0xff)
			break;
	}
	if (i == SDCARD_IDLE_TIMEOUT)
		ret = 0x3f;
bail:
	ao_sdcard_deselect();
	ao_sdcard_put();
	ao_sdcard_unlock();
	return ret == SDCARD_STATUS_READY_STATE;
}

#if SDCARD_DEBUG
static uint8_t	test_data[512];

static void
ao_sdcard_test_read(void)
{
	int i;
	if (!ao_sdcard_read_block(1, test_data)) {
		printf ("read error\n");
		return;
	}
	printf ("data:");
	for (i = 0; i < 18; i++)
		printf (" %02x", test_data[i]);
	printf ("\n");
}

static void
ao_sdcard_test_write(void)
{
	int	i;
	printf ("data:");
	for (i = 0; i < 16; i++) {
		test_data[i]++;
		printf (" %02x", test_data[i]);
	}
	printf ("\n");
	if (!ao_sdcard_write_block(1, test_data)) {
		printf ("write error\n");
		return;
	}
}

static const struct ao_cmds ao_sdcard_cmds[] = {
	{ ao_sdcard_test_read,	"x\0Test read" },
	{ ao_sdcard_test_write,	"y\0Test read" },
	{ 0, NULL },
};
#endif

void
ao_sdcard_init(void)
{
	ao_spi_init_cs(AO_SDCARD_SPI_CS_PORT, (1 << AO_SDCARD_SPI_CS_PIN));
#if SDCARD_DEBUG
	ao_cmd_register(&ao_sdcard_cmds[0]);
#endif
}
