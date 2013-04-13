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

#ifndef _AO_SDCARD_H_
#define _AO_SDCARD_H_

uint8_t
ao_sdcard_read_block(uint32_t block, uint8_t *data);

uint8_t
ao_sdcard_write_block(uint32_t block, uint8_t *data);

void
ao_sdcard_init(void);

/* Commands */
#define SDCARD_GO_IDLE_STATE		0
#define SDCARD_SEND_OP_COND		1
#define SDCARD_SEND_IF_COND		8
#define SDCARD_SEND_CSD			9
#define SDCARD_SEND_CID			10
#define SDCARD_SEND_STATUS		13
#define SDCARD_SET_BLOCKLEN		16
#define SDCARD_READ_BLOCK		17
#define SDCARD_WRITE_BLOCK		24
#define SDCARD_WRITE_MULTIPLE_BLOCK	25
#define SDCARD_ERASE_WR_BLK_START	32
#define SDCARD_ERASE_WR_BLK_END		33
#define SDCARD_ERASE			38
#define SDCARD_APP_CMD			55
#define SDCARD_READ_OCR			58

/* App commands */
#define SDCARD_APP_SET_WR_BLK_ERASE_COUNT	23
#define SDCARD_APP_SEND_OP_COMD			41

/* Status */
#define SDCARD_STATUS_READY_STATE	0x00
#define SDCARD_STATUS_IDLE_STATE	0x01
#define SDCARD_STATUS_ERASE_RESET	0x02
#define SDCARD_STATUS_ILLEGAL_COMMAND	0x04
#define SDCARD_STATUS_COM_CRC_ERROR	0x08
#define SDCARD_STATUS_ERASE_SEQ_ERROR	0x10
#define SDCARD_STATUS_ADDRESS_ERROR	0x20
#define SDCARD_STATUS_PARAMETER_ERROR	0x40
#define SDCARD_STATUS_TIMEOUT		0xff

#define SDCARD_DATA_START_BLOCK		0xfe
#define SDCARD_STOP_TRAN_TOKEN		0xfd
#define SDCARD_WRITE_MULTIPLE_TOKEN	0xfc
#define SDCARD_DATA_RES_MASK		0x1f
#define SDCARD_DATA_RES_ACCEPTED	0x05

#define SDCARD_CMD_TIMEOUT		AO_MS_TO_TICKS(20)
#define SDCARD_BUSY_TIMEOUT		AO_MS_TO_TICKS(20)
#define SDCARD_BLOCK_TIMEOUT		AO_MS_TO_TICKS(200)
#define SDCARD_IDLE_RETRY		10
#define SDCARD_OP_COND_RETRY		10

enum ao_sdtype {
	ao_sdtype_unknown,
	ao_sdtype_mmc3,
	ao_sdtype_sd1,
	ao_sdtype_sd2byte,
	ao_sdtype_sd2block,
};

#endif /* _AO_SDCARD_H_ */
