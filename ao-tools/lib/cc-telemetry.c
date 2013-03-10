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

#include "cc.h"
#include <string.h>

static int
parse_byte(char *data, uint8_t *byte)
{
	char	d[3];
	int x;
	d[0] = data[0];
	d[1] = data[1];
	d[2] = '\0';

	if (sscanf(d, "%x", &x) != 1)
		return FALSE;
	*byte = x;
	return TRUE;
}

int
cc_telemetry_parse(const char *input_line, union ao_telemetry_all *telemetry)
{
	uint8_t	*byte;
	char *data;
	uint8_t hex[35];
	int i;

	if (strncmp(input_line, "TELEM", 5) != 0)
		return FALSE;

	data = strchr (input_line, ' ');
	if (!data)
		return FALSE;
	data++;
	byte = hex;
	for (i = 0; i < 35; i++) {
		if (!parse_byte(data, byte))
			return FALSE;
		data += 2;
		byte++;
	}
	if (hex[0] != 34)
		return FALSE;
	memcpy(telemetry, hex+1, 34);
	return TRUE;
}

uint8_t
cc_telemetry_cksum(const union ao_telemetry_all *telemetry)
{
	const uint8_t	*x = (const uint8_t *) telemetry;
	int i;
	uint8_t	sum = 0x5a;
	for (i = 0; i < 34; i++)
		sum += x[i];
	return sum;
}

void
cc_telemetry_unparse(const union ao_telemetry_all *telemetry, char output_line[CC_TELEMETRY_BUFSIZE])
{
	uint8_t hex[36];
	int i;
	int p;

	hex[0] = 34;
	memcpy(hex+1, telemetry, 34);
	hex[35] = cc_telemetry_cksum(telemetry);
	strcpy(output_line, "TELEM ");
	p = strlen(output_line);
	for (i = 0; i < 36; i++) {
		sprintf(output_line + p, "%02x", hex[i]);
		p += 2;
	}
}
		
