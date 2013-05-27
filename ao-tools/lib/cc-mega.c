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

#include "cc.h"
#include <string.h>
#include <ctype.h>

static const char *
parse_hex(const char *data, int *result)
{
	char	d[12];
	int 	x;
	int	i;

	while (isspace (*data))
		data++;
	for (i = 0; i < sizeof (d) - 1 && isxdigit(*data); i++)
		d[i] = *data++;
	d[i] = '\0';
	if (sscanf(d, "%x", &x) != 1)
		return NULL;
	*result = x;
	return data;
}

static const char *
parse_uint16(const char *data, uint16_t *result)
{
	int	x;
	data = parse_hex(data, &x);
	*result =x;
	return data;
}

static const char *
parse_uint8(const char *data, uint8_t *result)
{
	int	x;
	data = parse_hex(data, &x);
	*result =x;
	return data;
}

static int
parse_eeprom(const char *input_line, struct ao_log_mega *l) {
	const char	*line;
	int	b;

	if (input_line[1] != ' ')
		return 0;
	if (!isupper(input_line[0]))
		return 0;

	l->type = input_line[0];
	l->is_config = 0;
	line = input_line + 2;

	line = parse_uint16(line, &l->tick);
	for (b = 0; b < 28; b++) {
		if (!line)
			return 0;
		line = parse_uint8(line, &l->u.bytes[b]);
	}
	return 1;
}

#define YUP(t) do {				\
		l->u.config_int.kind = (t);	\
		l->is_config = 1;		\
		return 1;			\
	} while (0);

static int
parse_config(const char *input_line, struct ao_log_mega *l) {
	if (sscanf (input_line, "Config version: %d.%d",
		    &l->u.config_int.data[0],
		    &l->u.config_int.data[1]))
		YUP(AO_CONFIG_CONFIG);
	if (sscanf (input_line, "Main deploy: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MAIN);
	if (sscanf (input_line, "Apogee delay: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_APOGEE);
	if (sscanf (input_line, "Apogee lockout: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_LOCKOUT);
	if (sscanf (input_line, "Frequency: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_FREQUENCY);
	if (sscanf (input_line, "Radio enable:  %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_RADIO_ENABLE);
	if (sscanf (input_line, "Accel cal +1g: %d -1g: %d",
		    &l->u.config_int.data[0],
		    &l->u.config_int.data[1]))
		YUP(AO_CONFIG_ACCEL_CAL);
	if (sscanf (input_line, "Radio cal: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_RADIO_CAL);
	if (sscanf (input_line, "Max flight log: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MAX_LOG);
	if (sscanf (input_line, "Ignite mode: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_IGNITE_MODE);
	if (sscanf (input_line, "Pad orientation: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_PAD_ORIENTATION);
	if (sscanf (input_line, "serial-number %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_SERIAL_NUMBER);
	if (sscanf (input_line, "log-format %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_LOG_FORMAT);
	if (sscanf (input_line, "ms5607 reserved: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MS5607_RESERVED);
	if (sscanf (input_line, "ms5607 sens: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MS5607_SENS);
	if (sscanf (input_line, "ms5607 off: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MS5607_OFF);
	if (sscanf (input_line, "ms5607 tcs: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MS5607_TCS);
	if (sscanf (input_line, "ms5607 tco: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MS5607_TCO);
	if (sscanf (input_line, "ms5607 tref: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MS5607_TREF);
	if (sscanf (input_line, "ms5607 tempsens: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MS5607_TEMPSENS);
	if (sscanf (input_line, "ms5607 crc: %d",
		    &l->u.config_int.data[0]))
		YUP(AO_CONFIG_MS5607_CRC);
	return 0;
}

int
cc_mega_parse(const char *input_line, struct ao_log_mega *l) {
	return parse_eeprom(input_line, l) || parse_config(input_line, l);
}
