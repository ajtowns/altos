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

#include "ao.h"
#include <ao_flight.h>
#include <math.h>

static __xdata struct ao_telemetry_sensor		ao_tel_sensor;
static __xdata struct ao_telemetry_location		ao_tel_location;
static __xdata struct ao_telemetry_configuration	ao_tel_config;
static __xdata int16_t 					ao_tel_max_speed;
static __xdata int16_t					ao_tel_max_height;
static int8_t ao_tel_rssi;

static __xdata char ao_lcd_line[17];
static __xdata char ao_state_name[] = "SIPBFCDMLI";

static void
ao_terraui_line(uint8_t addr)
{
	ao_lcd_goto(addr);
	ao_lcd_putstring(ao_lcd_line);
}

#define ao_terraui_state()	(ao_state_name[ao_tel_sensor.state])

static char
ao_terraui_igniter(int16_t sense)
{
	if (sense < AO_IGNITER_OPEN)
		return '-';
	if (sense > AO_IGNITER_CLOSED)
		return '+';
	return '?';
}

static char
ao_terraui_battery(void)
{
	if (ao_tel_sensor.v_batt > 25558)
		return '+';
	return '-';
}

static char
ao_terraui_gps(void)
{
	if (ao_tel_location.flags & (1 << 4)) {
		if ((ao_tel_location.flags & 0xf) >= 4)
			return '+';
	}
	return '-';
}

static char
ao_terraui_local_gps(void)
{
	if (ao_gps_data.flags & (1 << 4)) {
		if ((ao_gps_data.flags & 0xf) >= 4)
			return '+';
	}
	return '-';
}

static char
ao_terraui_logging(void)
{
	if (ao_tel_config.flight != 0)
		return '+';
	return '-';
}

static __code char ao_progress[4] = { '\011', '\012', '\014', '\013' };

static uint8_t	ao_telem_progress;
static uint8_t	ao_gps_progress;

static void
ao_terraui_startup(void)
{
	sprintf(ao_lcd_line, "%-16.16s", ao_product);
	ao_terraui_line(AO_LCD_ADDR(0,0));
	sprintf(ao_lcd_line, "%-8.8s %5u  ", ao_version, ao_serial_number);
	ao_terraui_line(AO_LCD_ADDR(1,0));
}

static void
ao_terraui_info_firstline(void)
{
	sprintf(ao_lcd_line, "S %4d %7.7s %c",
		ao_tel_sensor.serial,
		ao_tel_config.callsign,
		ao_terraui_state());
	ao_terraui_line(AO_LCD_ADDR(0,0));
}
	
static void
ao_terraui_info(void)
{
	ao_terraui_info_firstline();
	sprintf(ao_lcd_line, "F %4d RSSI%4d%c",
		ao_tel_config.flight,
		ao_tel_rssi,
		ao_progress[ao_telem_progress]);
	ao_terraui_line(AO_LCD_ADDR(1,0));
}

static void
ao_terraui_pad(void)
{
	sprintf(ao_lcd_line, "B%c A%c M%c L%c G%c %c",
		ao_terraui_battery(),
		ao_terraui_igniter(ao_tel_sensor.sense_d),
		ao_terraui_igniter(ao_tel_sensor.sense_m),
		ao_terraui_logging(),
		ao_terraui_gps(),
		ao_terraui_state());
	ao_terraui_line(AO_LCD_ADDR(0,0));
	sprintf(ao_lcd_line, "SAT %2d RSSI%4d%c",
		ao_tel_location.flags & 0xf,
		ao_tel_rssi,
		ao_progress[ao_telem_progress]);
	ao_terraui_line(AO_LCD_ADDR(1,0));
}

static void
ao_terraui_ascent(void)
{
	sprintf(ao_lcd_line, "S %5d S\011%5d%c",
		ao_tel_sensor.speed >> 4,
		ao_tel_max_speed >> 4,
		ao_terraui_state());
	ao_terraui_line(AO_LCD_ADDR(0,0));
	sprintf(ao_lcd_line, "H %5d H\011%5d%c",
		ao_tel_sensor.height >> 4,
		ao_tel_max_height >> 4,
		ao_progress[ao_telem_progress]);
	ao_terraui_line(AO_LCD_ADDR(1,0));
}

static int16_t mag(int32_t d)
{
	if (d < 0)
		d = -d;
	if (d > 0x7fff)
		d = 0x7fff;
	return d;
}

static int32_t
dist(int32_t d)
{
	__pdata uint32_t m;
	uint8_t neg = 0;

	if (d < 0) {
		d = -d;
		neg = 1;
	}

	m = 10000000;
	while (d >= (2147483647 / 111198)) {
		d /= 10;
		m /= 10;
	}
	d = (d * 111198) / m;
	if (neg)
		d = -d;
	return d;
}

static __code uint8_t cos_table[] = {
   0, /*  0 */
   0, /*  1 */
   0, /*  2 */
 255, /*  3 */
 254, /*  4 */
 253, /*  5 */
 252, /*  6 */
 251, /*  7 */
 249, /*  8 */
 247, /*  9 */
 245, /* 10 */
 243, /* 11 */
 240, /* 12 */
 238, /* 13 */
 235, /* 14 */
 232, /* 15 */
 228, /* 16 */
 225, /* 17 */
 221, /* 18 */
 217, /* 19 */
 213, /* 20 */
 209, /* 21 */
 205, /* 22 */
 200, /* 23 */
 195, /* 24 */
 190, /* 25 */
 185, /* 26 */
 180, /* 27 */
 175, /* 28 */
 169, /* 29 */
 163, /* 30 */
 158, /* 31 */
 152, /* 32 */
 145, /* 33 */
 139, /* 34 */
 133, /* 35 */
 126, /* 36 */
 120, /* 37 */
 113, /* 38 */
 106, /* 39 */
 100, /* 40 */
  93, /* 41 */
  86, /* 42 */
  79, /* 43 */
  71, /* 44 */
  64, /* 45 */
  57, /* 46 */
  49, /* 47 */
  42, /* 48 */
  35, /* 49 */
  27, /* 50 */
  20, /* 51 */
  12, /* 52 */
   5, /* 53 */
   1, /* 54 */
};

static __code uint8_t tan_table[] = {
    0, /*  0 */
    4, /*  1 */
    9, /*  2 */
   13, /*  3 */
   18, /*  4 */
   22, /*  5 */
   27, /*  6 */
   31, /*  7 */
   36, /*  8 */
   41, /*  9 */
   45, /* 10 */
   50, /* 11 */
   54, /* 12 */
   59, /* 13 */
   64, /* 14 */
   69, /* 15 */
   73, /* 16 */
   78, /* 17 */
   83, /* 18 */
   88, /* 19 */
   93, /* 20 */
   98, /* 21 */
  103, /* 22 */
  109, /* 23 */
  114, /* 24 */
  119, /* 25 */
  125, /* 26 */
  130, /* 27 */
  136, /* 28 */
  142, /* 29 */
  148, /* 30 */
  154, /* 31 */
  160, /* 32 */
  166, /* 33 */
  173, /* 34 */
  179, /* 35 */
  186, /* 36 */
  193, /* 37 */
  200, /* 38 */
  207, /* 39 */
  215, /* 40 */
  223, /* 41 */
  231, /* 42 */
  239, /* 43 */
  247, /* 44 */
};
	
int16_t ao_atan2(int32_t dy, int32_t dx) __reentrant
{
	int8_t	m = 1;
	int16_t	a = 0;
	uint8_t	r;
	int8_t	t;

	if (dx == 0) {
		if (dy > 0)
			return 90;
		if (dy < 0)
			return -90;
		return 0;
	}

	if (dx < 0) {
		a = 180;
		m = -m;
		dx = -dx;
	}

	if (dy < 0) {
		m = -m;
		a = -a;
		dy = -dy;
	}

	if (dy > dx) {
		int32_t	t;

		t = dy; dy = dx; dx = t;
		a = a + m * 90;
		m = -m;
	}

	dy = dy << 8;
	dy = (dy + (dx >> 1)) / dx;
	r = dy;
	for (t = 0; t < 44; t++)
		if (tan_table[t] >= r)
			break;
	return t * m + a;
}

static __pdata int32_t	lon_dist, lat_dist;
static __pdata uint32_t	ground_dist, range;
static __pdata uint8_t dist_in_km;
static __pdata int16_t bearing, elevation;

static void
ao_terraui_lat_dist(void)
{
	lat_dist = dist (ao_tel_location.latitude - ao_gps_data.latitude);
}

static void
ao_terraui_lon_dist(void) __reentrant
{
	uint8_t	c = cos_table[ao_gps_data.latitude >> 24];
	lon_dist = ao_tel_location.longitude;

	/* check if it's shorter to go the other way around */
	if ((int16_t) (lon_dist >> 24) < (int16_t) (ao_gps_data.longitude >> 24) - (int16_t) (1800000000 >> 24))
		lon_dist += 3600000000ul;
	lon_dist = dist(lon_dist - ao_gps_data.longitude);
	if (c) {
		if (lon_dist & 0x7f800000)
			lon_dist = (lon_dist >> 8) * c;
		else
			lon_dist = (lon_dist * (int16_t) c) >> 8;
	}
}

static int32_t sqr(int32_t x) { return x * x; }

static void
ao_terraui_compute(void)
{
	uint16_t	h = ao_tel_sensor.height;

	ao_terraui_lat_dist();
	ao_terraui_lon_dist();
	if (lat_dist > 32767 || lon_dist > 32767) {
		dist_in_km = 1;
		ground_dist = sqr(lat_dist/1000) + sqr(lon_dist/1000);
		h = h/1000;
	} else {
		dist_in_km = 0;
		ground_dist = sqr(lat_dist) + sqr(lon_dist);
	}
	ground_dist = ao_sqrt (ground_dist);
	range = ao_sqrt(ground_dist * ground_dist + ao_tel_sensor.height * ao_tel_sensor.height);
	bearing = ao_atan2(lon_dist, lat_dist);
	if (bearing < 0)
		bearing += 360;
	elevation = ao_atan2(ao_tel_sensor.height, ground_dist);
}

static void
ao_terraui_descent(void)
{
	ao_terraui_compute();
	sprintf(ao_lcd_line, "\007 %4d  \005 %3d  %c",
		bearing, elevation,
		ao_terraui_state());
	ao_terraui_line(AO_LCD_ADDR(0,0));
	sprintf(ao_lcd_line, "H:%5d S:%5d%c",
		ao_tel_sensor.height, ao_tel_sensor.speed >> 4,
		ao_progress[ao_telem_progress]);
	ao_terraui_line(AO_LCD_ADDR(1,0));
}

static void
ao_terraui_recover(void)
{
	ao_terraui_compute();
	sprintf(ao_lcd_line, "\007 %4d  \005 %3d  %c",
		bearing, elevation,
		ao_terraui_state());
	ao_terraui_line(AO_LCD_ADDR(0,0));
	sprintf(ao_lcd_line, "R%5ld%cRSSI%4d%c",
		ground_dist, dist_in_km ? 'k' : ' ',
		ao_tel_rssi,
		ao_progress[ao_telem_progress]);
	ao_terraui_line(AO_LCD_ADDR(1,0));
}

static void
ao_terraui_coord(int32_t c, char plus, char minus, char extra) __reentrant
{
	uint16_t	d;
	uint8_t		m;
	uint16_t	f;

	if (c < 0) {
		plus = minus;
		c = -c;
	}
	d = c / 10000000;
	c = c % 10000000;
	c = c * 60;
	m = c / 10000000;
	c = c % 10000000;
	f = (c + 500) / 1000;
	sprintf(ao_lcd_line, "%c %3d\362 %2d.%04d\"%c",
		plus, d, m, f, extra);
}

static void
ao_terraui_remote(void)
{
	ao_terraui_coord(ao_tel_location.latitude, 'N', 'S', ao_terraui_state());
	ao_terraui_line(AO_LCD_ADDR(0,0));
	ao_terraui_coord(ao_tel_location.longitude, 'E', 'W', ao_progress[ao_telem_progress]);
	ao_terraui_line(AO_LCD_ADDR(1,0));
}

static void
ao_terraui_local(void) __reentrant
{
	ao_terraui_coord(ao_gps_data.latitude, 'n', 's',
			 ao_terraui_local_gps());
	ao_terraui_line(AO_LCD_ADDR(0,0));
	ao_terraui_coord(ao_gps_data.longitude, 'e', 'w', ao_progress[ao_gps_progress]);
	ao_terraui_line(AO_LCD_ADDR(1,0));
}

static __pdata uint8_t ao_set_freq;
static __pdata uint32_t ao_set_freq_orig;

static void
ao_terraui_freq(void) __reentrant
{
	uint16_t	MHz;
	uint16_t	frac;
	
	MHz = ao_config.frequency / 1000;
	frac = ao_config.frequency % 1000;
	ao_terraui_info_firstline();
	sprintf(ao_lcd_line, "Freq: %3d.%03d MHz", MHz, frac);
	ao_terraui_line(AO_LCD_ADDR(1,0));
	ao_lcd_goto(AO_LCD_ADDR(1,11));
}

static void
ao_terraui_freq_start(void)
{
	ao_set_freq = 1;
	ao_set_freq_orig = ao_config.frequency;
	ao_lcd_cursor_on();
}

static void
ao_terraui_freq_button(char b) {

	switch (b) {
	case 0:
		return;
	case 1:
		if (ao_config.frequency > 430000)
			ao_config.frequency -= 100;
		break;
	case 2:
		ao_set_freq = 0;
		ao_lcd_cursor_off();
		if (ao_set_freq_orig != ao_config.frequency)
			ao_config_put();
		return;
	case 3:
		if (ao_config.frequency < 438000)
			ao_config.frequency += 100;
		break;

	}
	ao_config_set_radio();
	ao_radio_recv_abort();
}

static __code void (*__code ao_terraui_page[])(void) = {
	ao_terraui_startup,
	ao_terraui_info,
	ao_terraui_pad,
	ao_terraui_ascent,
	ao_terraui_descent,
	ao_terraui_recover,
	ao_terraui_remote,
	ao_terraui_local,
};

#define NUM_PAGE	(sizeof (ao_terraui_page)/sizeof (ao_terraui_page[0]))

static __pdata uint8_t ao_current_page = 0;
static __pdata uint8_t ao_shown_about = 3;

static void
ao_terraui(void)
{
	ao_lcd_start();

	ao_delay(AO_MS_TO_TICKS(100));
	ao_button_clear();

	for (;;) {
		char	b;

		if (ao_set_freq)
			ao_terraui_freq();
		else
			ao_terraui_page[ao_current_page]();

		ao_alarm(AO_SEC_TO_TICKS(1));
		b = ao_button_get();
		ao_clear_alarm();

		if (b > 0) {
			ao_beep_for(AO_BEEP_HIGH, AO_MS_TO_TICKS(10));
			ao_shown_about = 0;
		}
		
		if (ao_set_freq) {
			ao_terraui_freq_button(b);
			continue;
		}
		switch (b) {
		case 0:
			if (ao_shown_about) {
				if (--ao_shown_about == 0)
					ao_current_page = 2;
			}
			break;
		case 1:
			ao_current_page++;
			if (ao_current_page >= NUM_PAGE)
				ao_current_page = 0;
			break;
		case 2:
			ao_terraui_freq_start();
			break;
		case 3:
			if (ao_current_page == 0)
				ao_current_page = NUM_PAGE;
			ao_current_page--;
			break;
		}
	}
}

__xdata static struct ao_task ao_terraui_task;

static void
ao_terramonitor(void)
{
	uint8_t	monitor;

	monitor = ao_monitor_head;
	ao_monitor_set(sizeof (struct ao_telemetry_generic));
	for (monitor = ao_monitor_head;;
	     monitor = ao_monitor_ring_next(monitor))
	{
		while (monitor == ao_monitor_head)
			ao_sleep(DATA_TO_XDATA(&ao_monitor_head));
		if (ao_monitoring != sizeof (union ao_telemetry_all))
			continue;
		if (!(ao_monitor_ring[monitor].all.status & PKT_APPEND_STATUS_1_CRC_OK))
			continue;
		ao_tel_rssi = AO_RSSI_FROM_RADIO(ao_monitor_ring[monitor].all.rssi);
		switch (ao_monitor_ring[monitor].all.telemetry.generic.type) {
		case AO_TELEMETRY_SENSOR_TELEMETRUM:
		case AO_TELEMETRY_SENSOR_TELEMINI:
		case AO_TELEMETRY_SENSOR_TELENANO:
			ao_xmemcpy(&ao_tel_sensor, &ao_monitor_ring[monitor], sizeof (ao_tel_sensor));
			if (ao_tel_sensor.state < ao_flight_boost) {
				ao_tel_max_speed = 0;
				ao_tel_max_height = 0;
			} else {
				if (ao_tel_sensor.speed > ao_tel_max_speed)
					ao_tel_max_speed = ao_tel_sensor.speed;
				if (ao_tel_sensor.height > ao_tel_max_height)
					ao_tel_max_height = ao_tel_sensor.height;
			}
			ao_telem_progress = (ao_telem_progress + 1) & 0x3;
			break;
		case AO_TELEMETRY_LOCATION:
			ao_xmemcpy(&ao_tel_location, &ao_monitor_ring[monitor], sizeof (ao_tel_location));
			break;
		case AO_TELEMETRY_CONFIGURATION:
			ao_xmemcpy(&ao_tel_config, &ao_monitor_ring[monitor], sizeof (ao_tel_config));
		}
	}
}

__xdata static struct ao_task ao_terramonitor_task;

static void
ao_terragps(void)
{
	uint16_t	gps_tick = ao_gps_progress;

	for (;;) {
		while (ao_gps_tick == gps_tick)
			ao_sleep(&ao_gps_new);
		gps_tick = ao_gps_tick;
		ao_gps_progress = (ao_gps_progress + 1) & 3;
	}
}

__xdata static struct ao_task ao_terragps_task;

void
ao_terraui_init(void)
{
	ao_add_task(&ao_terraui_task, ao_terraui, "ui");
	ao_add_task(&ao_terramonitor_task, ao_terramonitor, "monitor");
	ao_add_task(&ao_terragps_task, ao_terragps, "gps");
}
