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

#ifndef AO_FAT_TEST
#include "ao.h"
#endif

/* Include bufio commands */
#ifndef AO_FAT_TEST
#define BUFIO_COMMANDS	0
#endif

#include "ao_sdcard.h"
#include "ao_bufio.h"

#define AO_NUM_BUF		16
#define AO_BUFSIZ		512

struct ao_bufio {
	uint32_t	block;
	int16_t		seqno;
	uint8_t		busy;	
	uint8_t		dirty;
};

static struct ao_bufio	ao_bufio[AO_NUM_BUF];
static uint8_t		ao_buffer[AO_NUM_BUF][AO_BUFSIZ];
static int16_t		ao_seqno;
static uint8_t		ao_bufio_mutex;

#if 0
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...) (void) 0
#endif

static inline void
ao_bufio_lock(void)
{
	ao_mutex_get(&ao_bufio_mutex);
}

static inline void
ao_bufio_unlock(void)
{
	ao_mutex_put(&ao_bufio_mutex);
}

static inline int16_t
ao_seqno_age(int16_t s)
{
	return ao_seqno - s;
}

static inline int16_t
ao_seqno_next(void)
{
	return ++ao_seqno;
}

static inline int
ao_seqno_older(int16_t a, int16_t b)
{
	return ao_seqno_age(a) > ao_seqno_age(b);
}

static inline void
ao_validate_bufno(int b)
{
	if (b < 0 || AO_NUM_BUF <= b)
		ao_panic(AO_PANIC_BUFIO);
}

static inline int
ao_buf_to_num(uint8_t *buf)
{
	int b = (buf - &ao_buffer[0][0]) / AO_BUFSIZ;

	ao_validate_bufno(b);
	return b;
}

static inline int
ao_bufio_to_num(struct ao_bufio *bufio)
{
	int b = (bufio - ao_bufio);

	ao_validate_bufno(b);
	return b;
}

static inline struct ao_bufio *
ao_buf_to_bufio(uint8_t *buf)
{
	int b = ao_buf_to_num(buf);
	struct ao_bufio *bufio;

	bufio = &ao_bufio[b];
	DBG ("buf %08x is %d bufio %08x\n", buf, b, bufio);
	return bufio;
}

static inline uint8_t *
ao_bufio_to_buf(struct ao_bufio *bufio)
{
	int b = ao_bufio_to_num(bufio);
	uint8_t *buf;

	buf = &ao_buffer[b][0];
	DBG ("bufio %08x is %d buf %08x\n", bufio, b, buf);
	return buf;
}

/*
 * Write a buffer to storage if it is dirty
 */
static void
ao_bufio_write(struct ao_bufio *bufio)
{
	if (bufio->dirty) {
		ao_sdcard_write_block(bufio->block, ao_bufio_to_buf(bufio));
		bufio->dirty = 0;
	}
}

/*
 * Read a buffer from storage
 */
static uint8_t
ao_bufio_read(struct ao_bufio *bufio)
{
	uint8_t	*buf = ao_bufio_to_buf(bufio);

	return ao_sdcard_read_block(bufio->block, buf);
}

/*
 * Find a buffer containing the specified block
 */
static struct ao_bufio *
ao_bufio_find_block(uint32_t block)
{
	int b;

	for (b = 0; b < AO_NUM_BUF; b++) {
		struct ao_bufio *bufio = &ao_bufio[b];
		if (bufio->block == block) {
			DBG ("Found existing buffer %d (seqno %d)\n",
				ao_bufio_to_num(bufio), bufio->seqno);
			return bufio;
		}
	}
	return NULL;
}

/*
 * Find the least recently used idle buffer
 */
static struct ao_bufio *
ao_bufio_find_idle(void)
{
	int b;
	struct ao_bufio *oldest = NULL;

	for (b = 0; b < AO_NUM_BUF; b++) {
		struct ao_bufio *bufio = &ao_bufio[b];
		if (!bufio->busy)
			if (!oldest || ao_seqno_older(bufio->seqno, oldest->seqno))
				oldest = bufio;
	}
	if (oldest)
		DBG ("Using idle buffer %d (seqno %d)\n",
			ao_bufio_to_num(oldest), oldest->seqno);
	return oldest;
}

/*
 * Return a pointer to a buffer containing
 * the contents of the specified block
 */
uint8_t *
ao_bufio_get(uint32_t block)
{
	struct ao_bufio *bufio;
	uint8_t	*buf = NULL;

	ao_bufio_lock();
	bufio = ao_bufio_find_block(block);
	if (!bufio) {
		bufio = ao_bufio_find_idle();
		if (bufio) {
			ao_bufio_write(bufio);
			bufio->block = block;
			DBG ("read buffer\n");
			if (!ao_bufio_read(bufio)) {
				bufio->block = 0xffffffff;
				bufio = NULL;
			}
		} else
			ao_panic(AO_PANIC_BUFIO);
	}
	if (bufio) {
		bufio->busy++;
		if (!bufio->busy)
			ao_panic(AO_PANIC_BUFIO);
		buf = ao_bufio_to_buf(bufio);
	}
	ao_bufio_unlock();
	return buf;
}

/*
 * Release a buffer, marking it dirty
 * if it has been written to
 */
void
ao_bufio_put(uint8_t *buf, uint8_t write)
{
	struct ao_bufio *bufio;

	ao_bufio_lock();
	bufio = ao_buf_to_bufio(buf);
	
	if (!bufio->busy)
		ao_panic(AO_PANIC_BUFIO);

	DBG ("idle buffer %d write %d\n", ao_bufio_to_num(bufio), write);
	bufio->dirty |= write;
	if (!--bufio->busy) {
		bufio->seqno = ao_seqno_next();
		DBG ("not busy, seqno %d\n", bufio->seqno);
	}
	ao_bufio_unlock();
}

/*
 * Flush a single buffer immediately. Useful
 * if write order is important
 */
void
ao_bufio_flush_one(uint8_t *buf)
{
	ao_bufio_lock();
	ao_bufio_write(ao_buf_to_bufio(buf));
	ao_bufio_unlock();
}

/*
 * Flush all buffers to storage
 */
void
ao_bufio_flush(void)
{
	int	b;

	ao_bufio_lock();
	for (b = 0; b < AO_NUM_BUF; b++)
		ao_bufio_write(&ao_bufio[b]);
	ao_bufio_unlock();
}

#if BUFIO_COMMANDS
static void
ao_bufio_test_read(void)
{
	uint8_t	*buf;
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if ((buf = ao_bufio_get(ao_cmd_lex_u32))) {
		int i;
		for (i = 0; i < 512; i++) {
			printf (" %02x", buf[i]);
			if ((i & 0xf) == 0xf)
				printf("\n");
		}
		ao_bufio_put(buf, 0);
	}
}

static const struct ao_cmds ao_bufio_cmds[] = {
	{ ao_bufio_test_read,	"q\0Test bufio read" },
	{ 0, NULL },
};
#endif

void
ao_bufio_setup(void)
{
	int b;

	for (b = 0; b < AO_NUM_BUF; b++) {
		ao_bufio[b].dirty = 0;
		ao_bufio[b].busy = 0;
		ao_bufio[b].block = 0xffffffff;
	}
}

void
ao_bufio_init(void)
{
	ao_bufio_setup();
	ao_sdcard_init();
#if BUFIO_COMMANDS
	ao_cmd_register(&ao_bufio_cmds[0]);
#endif
}
