/*
 * Copyright © 2010 Anthony Towns <aj@erisian.com.au>
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

static __xdata union {
	struct ao_telemetry telemetry;
	uint8_t raw[sizeof (struct ao_telemetry)]; /* cf max packet length in ao_radio */
} packet;
static size_t packet_size = 0;
static uint16_t packet_delay;

static int next_c = -1;

static void
nextchar(void)
{
	if (next_c < 0)
		return;
	next_c = getchar();
	if (next_c == '\r')
		next_c = '\n';
}

static void
skip_white(void)
{
	while (next_c == ' ' || next_c == '\t')
		nextchar();
}

static void
read_white(void)
{
	if (next_c != ' ' && next_c != '\t')
		next_c = -2;
	skip_white();
}

static uint8_t
read_char(void)
{
	uint8_t ch;
	read_white();
	ch = next_c;
	nextchar();
	return ch;
}

static uint8_t
read_hex(void)
{
	uint8_t o;
	if ('0' <= next_c && next_c <= '9')
		o = next_c - '0';
	else if ('a' <= next_c && next_c <= 'f')
		o = next_c - 'a' + 10;
	else if ('A' <= next_c && next_c <= 'F')
		o = next_c - 'A' + 10;
	else {
		next_c = -2;
		o = 0;
	}
	nextchar();
	return o;
}

static uint8_t
read_hexbyte(void)
{
	uint8_t ch;
	skip_white();
	ch  = read_hex() * 0x10;
	ch |= read_hex();
	return ch;
}

static void
read_string(char *str, size_t max)
{
	read_white();
	while (next_c > 0 && next_c != ' ' && next_c != '\n') {
		if (max > 1) {
			*str++ = (char) next_c;
			max--;
		}
		nextchar();
	}
	*str = '\0';
}

static void
read_literal(char *str)
{
	read_white();
	while (*str && next_c == *str) {
		str++;
		nextchar();
	}
	if (*str)
		next_c = -2;
}

static long
read_decimal(void)
{
	int neg = 0;
	long out = 0;

	read_white();

	if (next_c == '-') {
		neg = 1;
		nextchar();
	} else if (next_c == '+') {
		nextchar();
	}
	out = 0;
	while ('0' <= next_c && next_c <= '9') {
		out = (10 * out) + (next_c - '0');
		nextchar();
	}
	return neg ? -out : out;
}

static void
parse_telem(void)
{
	int n_sats, v_sats, i;
	/* stdin will contain something like:
	 * N0CALL 224 10 7 18075 15536 26672 21416 25172 4500 7000 15544 15527 -2997858 26642 26828 15551 16086 9 GPS V 2010 17 17 3 17 53 -278481050 1529577250 75 0 0 0 4 0 0 SAT 11 32 40 20 40 31 43 1 36 16 33 23 40 11 41 24 40 13 39 14 32 30 40
	 */

	read_string(packet.telemetry.callsign, AO_MAX_CALLSIGN);

	packet.telemetry.serial = read_decimal();
	packet.telemetry.flight = read_decimal();
	packet.telemetry.flight_state = read_decimal();

	packet.telemetry.adc.tick = read_decimal();
	packet.telemetry.adc.accel = read_decimal();
	packet.telemetry.adc.pres = read_decimal();
	packet.telemetry.adc.temp = read_decimal();
	packet.telemetry.adc.v_batt = read_decimal();
	packet.telemetry.adc.sense_d = read_decimal();
	packet.telemetry.adc.sense_m = read_decimal();

	packet.telemetry.flight_accel = read_decimal();
	packet.telemetry.ground_accel = read_decimal();
	packet.telemetry.flight_vel = read_decimal();
	packet.telemetry.flight_pres = read_decimal();
	packet.telemetry.ground_pres = read_decimal();
	packet.telemetry.accel_plus_g = read_decimal();
	packet.telemetry.accel_minus_g = read_decimal();

	n_sats = read_decimal();
	read_literal("GPS");

	switch (read_char()) {
	case 'V': /* valid */
		packet.telemetry.gps.flags = AO_GPS_VALID | AO_GPS_DATE_VALID;
		break;
	case 'D': /* valid, bad data */
		packet.telemetry.gps.flags = AO_GPS_VALID;
		break;
	case 'R': /* unlocked */
		packet.telemetry.gps.flags = AO_GPS_RUNNING;
		break;
	case 'N': /* not connected */
	default:
		packet.telemetry.gps.flags = 0;
		break;
	}
	packet.telemetry.gps.flags |=
		(n_sats << AO_GPS_NUM_SAT_SHIFT) & AO_GPS_NUM_SAT_MASK;

	packet.telemetry.gps.year = read_decimal() - 2000;
	packet.telemetry.gps.month = read_decimal();
	packet.telemetry.gps.day = read_decimal();
	packet.telemetry.gps.hour = read_decimal();
	packet.telemetry.gps.minute = read_decimal();
	packet.telemetry.gps.second = read_decimal();

	packet.telemetry.gps.latitude = read_decimal();
	packet.telemetry.gps.longitude = read_decimal();
	packet.telemetry.gps.altitude = read_decimal();
	packet.telemetry.gps.ground_speed = read_decimal();
	packet.telemetry.gps.course = read_decimal();
	packet.telemetry.gps.climb_rate = read_decimal();
	packet.telemetry.gps.hdop = read_decimal();
	packet.telemetry.gps.h_error = read_decimal();
	packet.telemetry.gps.v_error = read_decimal();

	read_literal("SAT");

	v_sats = read_decimal();

	if (v_sats > n_sats)
		n_sats = v_sats;
	if (n_sats > AO_MAX_GPS_TRACKING)
		n_sats = AO_MAX_GPS_TRACKING;

	packet.telemetry.gps_tracking.channels = n_sats;
	for (i = 0; i < n_sats; i++) {
		if (i < v_sats) {
			packet.telemetry.gps_tracking.sats[i].svid = read_decimal();
			packet.telemetry.gps_tracking.sats[i].c_n_1 = read_decimal();
		} else {
			packet.telemetry.gps_tracking.sats[i].svid = 0;
			packet.telemetry.gps_tracking.sats[i].c_n_1 = 0;
		}
	}
	packet_delay = packet.telemetry.adc.tick;
}

static void
parse_hex_packet(void)
{
	int i;
	for (i = 0; i < packet_size; i++) {
		packet.raw[i] = read_hexbyte();
	}
}

static void
parse_raw_packet(void)
{
	int i;
	for (i = 0; i < packet_size; i++) {
		packet.raw[i] = getchar();
	}
	next_c = '\n';
}

static void
parse_test_packet(void)
{
	int i;
	for (i = 0; i < packet_size; i++) {
		packet.raw[i] = i;
	}
	packet_delay += AO_MS_TO_TICKS(100);
	next_c = '\n';
}

static void
ao_senddata_start(void) __reentrant
{
	uint16_t packet_time = 0, last_send = 0, delay, packet_cnt;
	uint8_t c;
	void (*packet_parse)(void);

	/* args: number packets, type of packet, param */
	ao_cmd_decimal();
	packet_cnt = ao_cmd_lex_i;

	ao_cmd_white();
	c = ao_cmd_lex_c;
	ao_cmd_lex();

	if (c == 'H') {
		ao_cmd_decimal();
		packet_size = ao_cmd_lex_i;
		packet_parse = parse_hex_packet;
	} else if (c == 'R') {
		ao_cmd_decimal();
		packet_size = ao_cmd_lex_i;
		packet_parse = parse_raw_packet;
	} else if (c == 'T') {
		packet_size = sizeof (packet.telemetry);
		packet_parse = parse_telem;
	} else if (c == 'X') {
		ao_cmd_decimal();
		packet_size = ao_cmd_lex_i;
		packet_parse = parse_test_packet;
	} else {
		ao_cmd_status = ao_cmd_syntax_error;
		packet_parse = (void(*)(void)) NULL;
	}

	if (ao_cmd_status != ao_cmd_success) {
		return;
	}

	if (packet_size <= 0 || sizeof(packet.raw) < packet_size) {
		printf("Packet size must be between 1 and %d.\n", sizeof(packet.raw));
		flush();
		return;
	}

	ao_set_monitor(0);
	flush();

	ao_radio_set_fixed_pkt(packet_size);

	while (packet_cnt-- > 0) {
		packet_delay = packet_time;
		next_c = ' ';
		packet_parse();

		if (next_c < 0) {
			next_c = ' ';
			while (next_c != '\n')
				nextchar();
			break;
		}

		while (next_c != '\n')
			nextchar();

		delay = packet_delay - packet_time - ao_time() + last_send;
		if (delay < AO_SEC_TO_TICKS(3)) {
			ao_delay(delay);
		}
		packet_time = packet_delay;
		last_send = ao_time();

		ao_led_on(AO_LED_RED);
		ao_radio_send(&packet, packet_size);
		ao_led_off(AO_LED_RED);

		putchar('.');
		flush();
	}
	putchar('\n');
	flush();

	ao_radio_set_telemetry();
}

__code struct ao_cmds ao_senddata_cmds[] = {
	{ 'S',  ao_senddata_start,    "S cnt ([RHX] size|T)               Send fake telemetry data" },
	{ 0,    ao_senddata_start, NULL },
};


void
ao_senddata_init()
{
	ao_cmd_register(&ao_senddata_cmds[0]);
}
