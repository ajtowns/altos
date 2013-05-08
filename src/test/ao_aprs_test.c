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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <ao_telemetry.h>

struct ao_telemetry_location ao_gps_data;

#define AO_APRS_TEST

typedef int16_t (*ao_radio_fill_func)(uint8_t *buffer, int16_t len);

#define DEBUG 0
#if DEBUG
void
ao_aprs_bit(uint8_t bit)
{
	static int	seq = 0;
	printf ("%6d %d\n", seq++, bit ? 1 : 0);
}
#else
void
ao_aprs_bit(uint8_t bit)
{
	putchar (bit ? 0xc0 : 0x40);
}
#endif

void
ao_radio_send_aprs(ao_radio_fill_func fill);

#include <ao_aprs.c>

/*
 * @section copyright_sec Copyright
 *
 * Copyright (c) 2001-2009 Michael Gray, KD7LMO


 *
 *
 * @section gpl_sec GNU General Public License
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  

 */

static void
audio_gap(int secs)
{
#if !DEBUG
	int	samples = secs * 9600;

	while (samples--)
		ao_aprs_bit(0);
#endif
}

// This is where we go after reset.
int main(int argc, char **argv)
{
    audio_gap(1);

    ao_gps_data.latitude = (45.0 + 28.25 / 60.0) * 10000000;
    ao_gps_data.longitude = (-(122 + 44.2649 / 60.0)) * 10000000;
    ao_gps_data.altitude = 84;

    /* Transmit one packet */
    ao_aprs_send();

    tncBuffer[strlen((char *) tncBuffer) - 2] = '\0';
    fprintf(stderr, "packet: %s\n", tncBuffer);

    exit(0);
}

void
ao_radio_send_aprs(ao_radio_fill_func fill)
{
	int16_t	len;
	uint8_t	done = 0;
	uint8_t	buf[16], *b, c;
	uint8_t bit;

	while (!done) {
		len = (*fill)(buf, sizeof (buf));
		if (len < 0) {
			done = 1;
			len = -len;
		}
		b = buf;
		while (len--) {
			c = *b++;
			for (bit = 0; bit < 8; bit++) {
				ao_aprs_bit(c & 0x80);
				c <<= 1;
			}
		}
	}
}
