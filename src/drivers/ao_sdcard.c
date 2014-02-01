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

#if HAS_RADIO
extern uint8_t ao_radio_mutex;
#define get_radio()	ao_mutex_get(&ao_radio_mutex)
#define put_radio()	ao_mutex_put(&ao_radio_mutex)
#else
#define get_radio()
#define put_radio()
#endif

#define ao_sdcard_get_slow() do { get_radio(); ao_spi_get(AO_SDCARD_SPI_BUS, AO_SPI_SPEED_250kHz); } while (0)
#define ao_sdcard_get()	do { get_radio(); ao_spi_get(AO_SDCARD_SPI_BUS, AO_SPI_SPEED_FAST); } while (0)
#define ao_sdcard_put() do { ao_spi_put(AO_SDCARD_SPI_BUS); put_radio(); } while (0)
#define ao_sdcard_send_fixed(d,l)	ao_spi_send_fixed((d), (l), AO_SDCARD_SPI_BUS)
#define ao_sdcard_send(d,l)		ao_spi_send((d), (l), AO_SDCARD_SPI_BUS)
#define ao_sdcard_recv(d,l)		ao_spi_recv((d), (l), AO_SDCARD_SPI_BUS)
#define ao_sdcard_select()		ao_gpio_set(AO_SDCARD_SPI_CS_PORT,AO_SDCARD_SPI_CS_PIN,AO_SDCARD_SPI_CS,0)
#define ao_sdcard_deselect()		ao_gpio_set(AO_SDCARD_SPI_CS_PORT,AO_SDCARD_SPI_CS_PIN,AO_SDCARD_SPI_CS,1)

/* Include SD card commands */
#define SDCARD_DEBUG	0

/* Spew SD tracing */
#define SDCARD_TRACE	0

/* Emit error and warning messages */
#define SDCARD_WARN	0

static uint8_t	initialized;
static uint8_t	present;
static uint8_t	mutex;
static enum ao_sdtype sdtype;

#define ao_sdcard_lock()	ao_mutex_get(&mutex)
#define ao_sdcard_unlock()	ao_mutex_put(&mutex)

#if SDCARD_TRACE
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...) (void) 0
#endif

#if SDCARD_WARN
#define WARN(...) printf(__VA_ARGS__)
#else
#define WARN(...) (void) 0
#endif

#define later(x,y)	((int16_t) ((x) - (y)) >= 0)

/*
 * Wait while the card is busy. The card will return a stream of 0xff
 * when it is ready to accept a command
 */

static uint8_t
ao_sdcard_wait_busy(void)
{
	uint16_t	timeout = ao_time() + SDCARD_BUSY_TIMEOUT;
	uint8_t		reply;
	for (;;) {
		ao_sdcard_recv(&reply, 1);
		DBG("\t\twait busy %02x\n", reply);
		if (reply == 0xff)
			break;
		if (later(ao_time(), timeout)) {
			WARN("wait busy timeout\n");
			return 0;
		}
	}
	return 1;
}


/*
 * Send an SD command and await the status reply
 */

static uint8_t
ao_sdcard_send_cmd(uint8_t cmd, uint32_t arg)
{
	uint8_t	data[6];
	uint8_t	reply;
	uint16_t timeout;

	DBG ("\tsend_cmd %d arg %08x\n", cmd, arg);

	/* Wait for the card to not be busy */
	if (cmd != SDCARD_GO_IDLE_STATE) {
		if (!ao_sdcard_wait_busy())
			return SDCARD_STATUS_TIMEOUT;
	}
	
	data[0] = (cmd & 0x3f) | 0x40;
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
	timeout = ao_time() + SDCARD_CMD_TIMEOUT;
	for (;;) {
		ao_sdcard_recv(&reply, 1);
		DBG ("\t\tgot byte %02x\n", reply);
		if ((reply & 0x80) == 0)
			break;
		if (later(ao_time(), timeout)) {
			WARN("send_cmd %02x timeout\n", cmd);
			return SDCARD_STATUS_TIMEOUT;
		}
	}
#if SDCARD_WARN
	if (reply != SDCARD_STATUS_READY_STATE && reply != SDCARD_STATUS_IDLE_STATE)
		WARN("send_cmd %d failed %02x\n", cmd, reply);
#endif
	return reply;
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
 * Switch to 'idle' state. This is used to get the card into SPI mode
 */
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

/*
 * _ao_sdcard_send_status
 *
 * Get the 2-byte status value.
 *
 * Called from other functions with CS held low already,
 * hence prefixing the name with '_'
 */
static uint16_t
_ao_sdcard_send_status(void)
{
	uint8_t ret;
	uint8_t extra;

	DBG ("send_status\n");
	ret = ao_sdcard_send_cmd(SDCARD_SEND_STATUS, 0);
	ao_sdcard_recv_reply(&extra, 1);
	if (ret != SDCARD_STATUS_READY_STATE)
		DBG ("\tsend_if_cond failed %02x\n", ret);
	return ret | (extra << 8);
}

/*
 * ao_sdcard_set_blocklen
 *
 * Set the block length for future read and write commands
 */
static uint8_t
ao_sdcard_set_blocklen(uint32_t blocklen)
{
	uint8_t ret;

	DBG ("set_blocklen %d\n", blocklen);
	ao_sdcard_select();
	ret = ao_sdcard_send_cmd(SDCARD_SET_BLOCKLEN, blocklen);
	ao_sdcard_recv_reply(NULL, 0);
	ao_sdcard_deselect();
	if (ret != SDCARD_STATUS_READY_STATE)
		DBG ("\tsend_if_cond failed %02x\n", ret);
	return ret;
}

/*
 * _ao_sdcard_app_cmd
 *
 * Send the app command prefix
 *
 * Called with the CS held low, hence
 * the '_' prefix
 */
static uint8_t
_ao_sdcard_app_cmd(void)
{
	uint8_t	ret;

	DBG ("app_cmd\n");
	ret = ao_sdcard_send_cmd(SDCARD_APP_CMD, 0);
	ao_sdcard_recv_reply(NULL, 0);
	DBG ("\tapp_cmd status %02x\n");
	return ret;
}

static uint8_t
ao_sdcard_app_send_op_cond(uint32_t arg)
{
	uint8_t	ret;

	DBG("send_op_comd\n");
	ao_sdcard_select();
	ret = _ao_sdcard_app_cmd();
	if (ret != SDCARD_STATUS_IDLE_STATE)
		goto bail;
	ret = ao_sdcard_send_cmd(SDCARD_APP_SEND_OP_COMD, arg);
	ao_sdcard_recv_reply(NULL, 0);
bail:
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

/*
 * Follow the flow-chart defined by the SD group to
 * initialize the card and figure out what kind it is
 */
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

	/* Reset the card and get it into SPI mode */
	for (i = 0; i < SDCARD_IDLE_RETRY; i++) {
		if (ao_sdcard_go_idle_state() == SDCARD_STATUS_IDLE_STATE)
			break;
	}
	if (i == SDCARD_IDLE_RETRY)
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

		for (i = 0; i < SDCARD_OP_COND_RETRY; i++) {
			ao_delay(AO_MS_TO_TICKS(10));
			ret = ao_sdcard_app_send_op_cond(arg);
			if (ret != SDCARD_STATUS_IDLE_STATE)
				break;
		}
		if (ret != SDCARD_STATUS_READY_STATE) {
			/* MMC */
			for (i = 0; i < SDCARD_OP_COND_RETRY; i++) {
				ao_delay(AO_MS_TO_TICKS(10));
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
_ao_sdcard_reset(void)
{
	int i;
	uint8_t	ret = 0x3f;
	uint8_t	response[10];

	for (i = 0; i < SDCARD_IDLE_RETRY; i++) {
		if (ao_sdcard_go_idle_state() == SDCARD_STATUS_IDLE_STATE)
			break;
	}
	if (i == SDCARD_IDLE_RETRY) {
		ret = 0x3f;
		goto bail;
	}

	/* Follow the setup path to get the card out of idle state and
	 * up and running again
	 */
	if (ao_sdcard_send_if_cond(0x1aa, response) == SDCARD_STATUS_IDLE_STATE) {
		uint32_t	arg = 0;
//		uint8_t		sdver2 = 0;

		/* Check for SD version 2 */
		if ((response[2] & 0xf) == 1 && response[3] == 0xaa) {
			arg = 0x40000000;
//			sdver2 = 1;
		}

		for (i = 0; i < SDCARD_IDLE_RETRY; i++) {
			ret = ao_sdcard_app_send_op_cond(arg);
			if (ret != SDCARD_STATUS_IDLE_STATE)
				break;
		}

		if (ret != SDCARD_STATUS_READY_STATE) {
			/* MMC */
			for (i = 0; i < SDCARD_IDLE_RETRY; i++) {
				ret = ao_sdcard_send_op_cond();
				if (ret != SDCARD_STATUS_IDLE_STATE)
					break;
			}
			if (ret != SDCARD_STATUS_READY_STATE)
				goto bail;
		}

		/* For everything but SDHC cards, set the block length */
		if (sdtype != ao_sdtype_sd2block) {
			ret = ao_sdcard_set_blocklen(512);
			if (ret != SDCARD_STATUS_READY_STATE)
				DBG ("set_blocklen failed, ignoring\n");
		}
	}
bail:
	return ret;
}

/*
 * The card will send 0xff until it is ready to send
 * the data block at which point it will send the START_BLOCK
 * marker followed by the data. This function waits while
 * the card is sending 0xff
 */
static uint8_t
ao_sdcard_wait_block_start(void)
{
	uint8_t		v;
	uint16_t	timeout = ao_time() + SDCARD_BLOCK_TIMEOUT;

	DBG ("\twait_block_start\n");
	for (;;) {
		ao_sdcard_recv(&v, 1);
		DBG("\t\trecv %02x\n", v);
		if (v != 0xff)
			break;
		if (later(ao_time(), timeout)) {
			printf ("wait block start timeout\n");
			return 0xff;
		}
	}
	return v;
}

/*
 * Read a block of 512 bytes from the card
 */
uint8_t
ao_sdcard_read_block(uint32_t block, uint8_t *data)
{
	uint8_t	ret = 0x3f;
	uint8_t start_block;
	uint8_t crc[2];
	int tries;

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
	DBG("read block %d\n", block);
	if (sdtype != ao_sdtype_sd2block)
		block <<= 9;

	ao_sdcard_get();
	for (tries = 0; tries < 10; tries++) {
		ao_sdcard_select();

		ret = ao_sdcard_send_cmd(SDCARD_READ_BLOCK, block);
		ao_sdcard_recv_reply(NULL, 0);
		if (ret != SDCARD_STATUS_READY_STATE) {
			uint16_t	status;
			WARN ("read block command failed %d status %02x\n", block, ret);
			status = _ao_sdcard_send_status();
			WARN ("\tstatus now %04x\n", status);
			(void) status;
			goto bail;
		}

		ao_sdcard_send_fixed(0xff, 1);

		/* Wait for the data start block marker */
		start_block = ao_sdcard_wait_block_start();
		if (start_block != SDCARD_DATA_START_BLOCK) {
			WARN ("wait block start failed %02x\n", start_block);
			ret = 0x3f;
			goto bail;
		}

		ao_sdcard_recv(data, 512);
		ao_sdcard_recv(crc, 2);
	bail:
		ao_sdcard_deselect();
		if (ret == SDCARD_STATUS_READY_STATE)
			break;
		if (ret == SDCARD_STATUS_IDLE_STATE) {
			ret = _ao_sdcard_reset();
			if (ret != SDCARD_STATUS_READY_STATE)
				break;
		}
	}
	ao_sdcard_put();
	ao_sdcard_unlock();

#if SDCARD_WARN
	if (ret != SDCARD_STATUS_READY_STATE)
		WARN("read failed\n");
	else if (tries)
		WARN("took %d tries to read %d\n", tries + 1, block);
#endif

	DBG("read %s\n", ret == SDCARD_STATUS_READY_STATE ? "success" : "failure");
	return ret == SDCARD_STATUS_READY_STATE;
}

/*
 * Write a block of 512 bytes to the card
 */
uint8_t
ao_sdcard_write_block(uint32_t block, uint8_t *data)
{
	uint8_t	ret;
	uint8_t	response[1];
	uint8_t	start_block[8];
	uint16_t status;
	int	tries;

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
	DBG("write block %d\n", block);
	if (sdtype != ao_sdtype_sd2block)
		block <<= 9;

	ao_sdcard_get();

	for (tries = 0; tries < 10; tries++) {
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
		ao_sdcard_recv(response, sizeof (response));
		if ((response[0] & SDCARD_DATA_RES_MASK) != SDCARD_DATA_RES_ACCEPTED) {
			int i;
			WARN("Data not accepted, response");
			for (i = 0; i < (int) sizeof (response); i++)
				WARN(" %02x", response[i]);
			WARN("\n");
			ret = 0x3f;
			goto bail;
		}
		
		/* Wait for the bus to go idle (should be done with an interrupt?) */
		if (!ao_sdcard_wait_busy()) {
			ret = 0x3f;
			goto bail;
		}

		/* Check the current status after the write completes */
		status = _ao_sdcard_send_status();
		if ((status & 0xff) != SDCARD_STATUS_READY_STATE) {
			WARN ("send status after write %04x\n", status);
			ret = status & 0xff;
			goto bail;
		}
	bail:
		ao_sdcard_deselect();
		DBG("write %s\n", ret == SDCARD_STATUS_READY_STATE ? "success" : "failure");
		if (ret == SDCARD_STATUS_READY_STATE)
			break;
	}
	ao_sdcard_put();
	ao_sdcard_unlock();
	if (tries)
		WARN("took %d tries to write %d\n", tries + 1, block);

	return ret == SDCARD_STATUS_READY_STATE;
}

#if SDCARD_DEBUG
static uint8_t	test_data[512];

static void
ao_sdcard_test_read(void)
{
	int i;

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	
	for (i = 0; i < 100; i++) {
		printf ("."); flush();
		if (!ao_sdcard_read_block(ao_cmd_lex_u32+i, test_data)) {
			printf ("read error %d\n", i);
			return;
		}
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
	stm_pupdr_set(AO_SDCARD_SPI_PORT, AO_SDCARD_SPI_SCK_PIN, STM_PUPDR_PULL_UP);
	stm_pupdr_set(AO_SDCARD_SPI_PORT, AO_SDCARD_SPI_MISO_PIN, STM_PUPDR_PULL_UP);
	stm_pupdr_set(AO_SDCARD_SPI_PORT, AO_SDCARD_SPI_MOSI_PIN, STM_PUPDR_PULL_UP);
	ao_spi_init_cs(AO_SDCARD_SPI_CS_PORT, (1 << AO_SDCARD_SPI_CS_PIN));
#if SDCARD_DEBUG
	ao_cmd_register(&ao_sdcard_cmds[0]);
#endif
}
