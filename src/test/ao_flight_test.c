/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#define GRAVITY 9.80665

#define AO_HERTZ	100

#define HAS_ADC 1
#define AO_DATA_RING	64
#define ao_data_ring_next(n)	(((n) + 1) & (AO_DATA_RING - 1))
#define ao_data_ring_prev(n)	(((n) - 1) & (AO_DATA_RING - 1))

#define AO_M_TO_HEIGHT(m)	((int16_t) (m))
#define AO_MS_TO_SPEED(ms)	((int16_t) ((ms) * 16))
#define AO_MSS_TO_ACCEL(mss)	((int16_t) ((mss) * 16))

#define AO_GPS_NEW_DATA		1
#define AO_GPS_NEW_TRACKING	2

int ao_gps_new;

#if TELEMEGA
#define AO_ADC_NUM_SENSE	6
#define HAS_MS5607		1
#define HAS_MPU6000		1
#define HAS_MMA655X		1
#define HAS_HMC5883 		1
#define HAS_BEEP		1

struct ao_adc {
	int16_t			sense[AO_ADC_NUM_SENSE];
	int16_t			v_batt;
	int16_t			v_pbatt;
	int16_t			temp;
};
#else
/*
 * One set of samples read from the A/D converter
 */
struct ao_adc {
	int16_t		accel;		/* accelerometer */
	int16_t		pres;		/* pressure sensor */
	int16_t		pres_real;	/* unclipped */
	int16_t		temp;		/* temperature sensor */
	int16_t		v_batt;		/* battery voltage */
	int16_t		sense_d;	/* drogue continuity sense */
	int16_t		sense_m;	/* main continuity sense */
};

#ifndef HAS_ACCEL
#define HAS_ACCEL 1
#define HAS_ACCEL_REF 0
#endif

#endif

#define __pdata
#define __data
#define __xdata
#define __code
#define __reentrant

#define HAS_FLIGHT 1
#define HAS_IGNITE 1
#define HAS_USB 1
#define HAS_GPS 1

#include <ao_data.h>
#include <ao_log.h>
#include <ao_telemetry.h>

#if TELEMEGA
int ao_gps_count;
struct ao_telemetry_location ao_gps_first;
struct ao_telemetry_location ao_gps_prev;
struct ao_telemetry_location ao_gps_static;

struct ao_telemetry_satellite ao_gps_tracking;

static inline double sqr(double a) { return a * a; }

void
cc_great_circle (double start_lat, double start_lon,
		 double end_lat, double end_lon,
		 double *dist, double *bearing)
{
	const double rad = M_PI / 180;
	const double earth_radius = 6371.2 * 1000;	/* in meters */
	double lat1 = rad * start_lat;
	double lon1 = rad * -start_lon;
	double lat2 = rad * end_lat;
	double lon2 = rad * -end_lon;

//	double d_lat = lat2 - lat1;
	double d_lon = lon2 - lon1;

	/* From http://en.wikipedia.org/wiki/Great-circle_distance */
	double vdn = sqrt(sqr(cos(lat2) * sin(d_lon)) +
			  sqr(cos(lat1) * sin(lat2) -
			      sin(lat1) * cos(lat2) * cos(d_lon)));
	double vdd = sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(d_lon);
	double d = atan2(vdn,vdd);
	double course;

	if (cos(lat1) < 1e-20) {
		if (lat1 > 0)
			course = M_PI;
		else
			course = -M_PI;
	} else {
		if (d < 1e-10)
			course = 0;
		else
			course = acos((sin(lat2)-sin(lat1)*cos(d)) /
				      (sin(d)*cos(lat1)));
		if (sin(lon2-lon1) > 0)
			course = 2 * M_PI-course;
	}
	*dist = d * earth_radius;
	*bearing = course * 180/M_PI;
}

double
ao_distance_from_pad(void)
{
	double	dist, bearing;
	if (!ao_gps_count)
		return 0;
	
	cc_great_circle(ao_gps_first.latitude / 1e7,
			ao_gps_first.longitude / 1e7,
			ao_gps_static.latitude / 1e7,
			ao_gps_static.longitude / 1e7,
			&dist, &bearing);
	return dist;
}

double
ao_gps_angle(void)
{
	double	dist, bearing;
	double	height;
	double	angle;

	if (ao_gps_count < 2)
		return 0;

	cc_great_circle(ao_gps_prev.latitude / 1e7,
			ao_gps_prev.longitude / 1e7,
			ao_gps_static.latitude / 1e7,
			ao_gps_static.longitude / 1e7,
			&dist, &bearing);
	height = ao_gps_static.altitude - ao_gps_prev.altitude;

	angle = atan2(dist, height);
	return angle * 180/M_PI;
}
#endif

#define to_fix16(x) ((int16_t) ((x) * 65536.0 + 0.5))
#define to_fix32(x) ((int32_t) ((x) * 65536.0 + 0.5))
#define from_fix(x)	((x) >> 16)

/*
 * Above this height, the baro sensor doesn't work
 */
#define AO_BARO_SATURATE	13000
#define AO_MIN_BARO_VALUE	ao_altitude_to_pres(AO_BARO_SATURATE)

/*
 * Above this speed, baro measurements are unreliable
 */
#define AO_MAX_BARO_SPEED	200

#define ACCEL_NOSE_UP	(ao_accel_2g >> 2)

extern enum ao_flight_state ao_flight_state;

#define FALSE 0
#define TRUE 1

volatile struct ao_data ao_data_ring[AO_DATA_RING];
volatile uint8_t ao_data_head;
int	ao_summary = 0;

#define ao_led_on(l)
#define ao_led_off(l)
#define ao_timer_set_adc_interval(i)
#define ao_wakeup(wchan) ao_dump_state()
#define ao_cmd_register(c)
#define ao_usb_disable()
#define ao_telemetry_set_interval(x)
#define ao_rdf_set(rdf)
#define ao_packet_slave_start()
#define ao_packet_slave_stop()
#define flush()

enum ao_igniter {
	ao_igniter_drogue = 0,
	ao_igniter_main = 1
};

struct ao_data ao_data_static;

int	drogue_height;
double	drogue_time;
int	main_height;
double	main_time;

int	tick_offset;

static int32_t	ao_k_height;

int16_t
ao_time(void)
{
	return ao_data_static.tick;
}

void
ao_delay(int16_t interval)
{
	return;
}

void
ao_ignite(enum ao_igniter igniter)
{
	double time = (double) (ao_data_static.tick + tick_offset) / 100;

	if (igniter == ao_igniter_drogue) {
		drogue_time = time;
		drogue_height = ao_k_height >> 16;
	} else {
		main_time = time;
		main_height = ao_k_height >> 16;
	}
}

struct ao_task {
	int dummy;
};

#define ao_add_task(t,f,n) ((void) (t))

#define ao_log_start()
#define ao_log_stop()

#define AO_MS_TO_TICKS(ms)	((ms) / 10)
#define AO_SEC_TO_TICKS(s)	((s) * 100)

#define AO_FLIGHT_TEST

int	ao_flight_debug;

FILE *emulator_in;
char *emulator_app;
char *emulator_name;
char *emulator_info;
double emulator_error_max = 4;
double emulator_height_error_max = 20;	/* noise in the baro sensor */

void
ao_dump_state(void);

void
ao_sleep(void *wchan);

const char const * const ao_state_names[] = {
	"startup", "idle", "pad", "boost", "fast",
	"coast", "drogue", "main", "landed", "invalid"
};

struct ao_cmds {
	void		(*func)(void);
	const char	*help;
};

#define ao_xmemcpy(d,s,c) memcpy(d,s,c)
#define ao_xmemset(d,v,c) memset(d,v,c)
#define ao_xmemcmp(d,s,c) memcmp(d,s,c)

#define AO_NEED_ALTITUDE_TO_PRES 1
#if TELEMEGA
#include "ao_convert_pa.c"
#include <ao_ms5607.h>
struct ao_ms5607_prom	ao_ms5607_prom;
#include "ao_ms5607_convert.c"
#define AO_PYRO_NUM	4
#include <ao_pyro.h>
#else
#include "ao_convert.c"
#endif

#include <ao_config.h>
#include <ao_fake_flight.h>

#define ao_config_get()

struct ao_config ao_config;

#define DATA_TO_XDATA(x) (x)


extern int16_t ao_ground_accel, ao_flight_accel;
extern int16_t ao_accel_2g;

typedef int16_t	accel_t;

extern uint16_t	ao_sample_tick;

extern alt_t	ao_sample_height;
extern accel_t	ao_sample_accel;
extern int32_t	ao_accel_scale;
extern alt_t	ao_ground_height;
extern alt_t	ao_sample_alt;

double ao_sample_qangle;

int ao_sample_prev_tick;
uint16_t	prev_tick;


#include "ao_kalman.c"
#include "ao_sqrt.c"
#include "ao_sample.c"
#include "ao_flight.c"
#if TELEMEGA
#define AO_PYRO_NUM	4

#define AO_PYRO_0	0
#define AO_PYRO_1	1
#define AO_PYRO_2	2
#define AO_PYRO_3	3

static void
ao_pyro_pin_set(uint8_t pin, uint8_t value)
{
	printf ("set pyro %d %d\n", pin, value);
}

#include "ao_pyro.c"
#endif

#define to_double(f)	((f) / 65536.0)

static int	ao_records_read = 0;
static int	ao_eof_read = 0;
static int	ao_flight_ground_accel;
static int	ao_flight_started = 0;
static int	ao_test_max_height;
static double	ao_test_max_height_time;
static int	ao_test_main_height;
static double	ao_test_main_height_time;
static double	ao_test_landed_time;
static double	ao_test_landed_height;
static double	ao_test_landed_time;
static int	landed_set;
static double	landed_time;
static double	landed_height;

#if HAS_MPU6000
static struct ao_mpu6000_sample	ao_ground_mpu6000;
#endif

void
ao_test_exit(void)
{
	double	drogue_error;
	double	main_error;
	double	landed_error;
	double	landed_time_error;

	if (!ao_test_main_height_time) {
		ao_test_main_height_time = ao_test_max_height_time;
		ao_test_main_height = ao_test_max_height;
	}
	drogue_error = fabs(ao_test_max_height_time - drogue_time);
	main_error = fabs(ao_test_main_height_time - main_time);
	landed_error = fabs(ao_test_landed_height - landed_height);
	landed_time_error = ao_test_landed_time - landed_time;
	if (drogue_error > emulator_error_max || main_error > emulator_error_max) {
		printf ("%s %s\n",
			emulator_app, emulator_name);
		if (emulator_info)
			printf ("\t%s\n", emulator_info);
		printf ("\tApogee error %g\n", drogue_error);
		printf ("\tMain error %g\n", main_error);
		printf ("\tLanded height error %g\n", landed_error);
		printf ("\tLanded time error %g\n", landed_time_error);
		printf ("\tActual: apogee: %d at %7.2f main: %d at %7.2f landed %7.2f at %7.2f\n",
			ao_test_max_height, ao_test_max_height_time,
			ao_test_main_height, ao_test_main_height_time,
			ao_test_landed_height, ao_test_landed_time);
		printf ("\tComputed: apogee: %d at %7.2f main: %d at %7.2f landed %7.2f at %7.2f\n",
			drogue_height, drogue_time, main_height, main_time,
			landed_height, landed_time);
		exit (1);
	}
	exit(0);
}

#ifdef TELEMEGA
struct ao_azel {
	int	az;
	int	el;
};

static void
azel (struct ao_azel *r, struct ao_quaternion *q)
{
	double	v;

	r->az = floor (atan2(q->y, q->x) * 180/M_PI + 0.5);
	v = sqrt (q->x*q->x + q->y*q->y);
	r->el = floor (atan2(q->z, v) * 180/M_PI + 0.5);
}
#endif

void
ao_insert(void)
{
	double	time;

	ao_data_ring[ao_data_head] = ao_data_static;
	ao_data_head = ao_data_ring_next(ao_data_head);
	if (ao_flight_state != ao_flight_startup) {
#if HAS_ACCEL
		double  accel = ((ao_flight_ground_accel - ao_data_accel_cook(&ao_data_static)) * GRAVITY * 2.0) /
			(ao_config.accel_minus_g - ao_config.accel_plus_g);
#else
		double	accel = 0.0;
#endif
#if TELEMEGA
		double	height;

		ao_ms5607_convert(&ao_data_static.ms5607_raw, &ao_data_static.ms5607_cooked);
		height = ao_pa_to_altitude(ao_data_static.ms5607_cooked.pres) - ao_ground_height;
#else
		double	height = ao_pres_to_altitude(ao_data_static.adc.pres_real) - ao_ground_height;
#endif

		(void) accel;
		if (!tick_offset)
			tick_offset = -ao_data_static.tick;
		if ((prev_tick - ao_data_static.tick) > 0x400)
			tick_offset += 65536;
		prev_tick = ao_data_static.tick;
		time = (double) (ao_data_static.tick + tick_offset) / 100;

		if (ao_test_max_height < height) {
			ao_test_max_height = height;
			ao_test_max_height_time = time;
			ao_test_landed_height = height;
			ao_test_landed_time = time;
		}
		if (height > ao_config.main_deploy) {
			ao_test_main_height_time = time;
			ao_test_main_height = height;
		}

		if (ao_test_landed_height > height) {
			ao_test_landed_height = height;
			ao_test_landed_time = time;
		}

		if (ao_flight_state == ao_flight_landed && !landed_set) {
			landed_set = 1;
			landed_time = time;
			landed_height = height;
		}

		if (!ao_summary) {
#if TELEMEGA
			static struct ao_quaternion	ao_ground_mag;
			static int			ao_ground_mag_set;

			if (!ao_ground_mag_set) {
				ao_quaternion_init_vector (&ao_ground_mag,
							   ao_data_mag_across(&ao_data_static),
							   ao_data_mag_through(&ao_data_static),
							   ao_data_mag_along(&ao_data_static));
				ao_quaternion_normalize(&ao_ground_mag, &ao_ground_mag);
				ao_quaternion_rotate(&ao_ground_mag, &ao_ground_mag, &ao_rotation);
				ao_ground_mag_set = 1;
			}

			struct ao_quaternion		ao_mag, ao_mag_rot;

			ao_quaternion_init_vector(&ao_mag,
						  ao_data_mag_across(&ao_data_static),
						  ao_data_mag_through(&ao_data_static),
						  ao_data_mag_along(&ao_data_static));

			ao_quaternion_normalize(&ao_mag, &ao_mag);
			ao_quaternion_rotate(&ao_mag_rot, &ao_mag, &ao_rotation);
			
			float				ao_dot;
			int				ao_mag_angle;

			ao_dot = ao_quaternion_dot(&ao_mag_rot, &ao_ground_mag);

			struct ao_azel			ground_azel, mag_azel, rot_azel;

			azel(&ground_azel, &ao_ground_mag);
			azel(&mag_azel, &ao_mag);
			azel(&rot_azel, &ao_mag_rot);

			ao_mag_angle = floor (acos(ao_dot) * 180 / M_PI + 0.5);

			(void) ao_mag_angle;

			static struct ao_quaternion	ao_x = { .r = 0, .x = 1, .y = 0, .z = 0 };
			struct ao_quaternion		ao_out;

			ao_quaternion_rotate(&ao_out, &ao_x, &ao_rotation);

			int	out = floor (atan2(ao_out.y, ao_out.x) * 180 / M_PI);

#if 0
			printf ("%7.2f state %-8.8s height %8.4f tilt %4d rot %4d mag_tilt %4d mag_rot %4d\n",
				time,
				ao_state_names[ao_flight_state],
				ao_k_height / 65536.0,
				ao_sample_orient, out,
				mag_azel.el,
				mag_azel.az);
#endif
			printf ("%7.2f state %-8.8s height %8.4f tilt %4d rot %4d dist %12.2f gps_tilt %4d gps_sats %2d\n",
				time,
				ao_state_names[ao_flight_state],
				ao_k_height / 65536.0,
				ao_sample_orient, out,
				ao_distance_from_pad(),
				(int) floor (ao_gps_angle() + 0.5),
				(ao_gps_static.flags & 0xf) * 10);

#if 0
			printf ("\t\tstate %-8.8s ground az: %4d el %4d mag az %4d el %4d rot az %4d el %4d el_diff %4d az_diff %4d angle %4d tilt %4d ground %8.5f %8.5f %8.5f cur %8.5f %8.5f %8.5f rot %8.5f %8.5f %8.5f\n",
				ao_state_names[ao_flight_state],
				ground_azel.az, ground_azel.el,
				mag_azel.az, mag_azel.el,
				rot_azel.az, rot_azel.el,
				ground_azel.el - rot_azel.el,
				ground_azel.az - rot_azel.az,
				ao_mag_angle,
				ao_sample_orient,
				ao_ground_mag.x,
				ao_ground_mag.y,
				ao_ground_mag.z,
				ao_mag.x,
				ao_mag.y,
				ao_mag.z,
				ao_mag_rot.x,
				ao_mag_rot.y,
				ao_mag_rot.z);
#endif
#endif

#if 0
			printf("%7.2f height %8.2f accel %8.3f "
#if TELEMEGA
			       "angle %5d "
			       "accel_x %8.3f accel_y %8.3f accel_z %8.3f gyro_x %8.3f gyro_y %8.3f gyro_z %8.3f mag_x %8d mag_y %8d, mag_z %8d mag_angle %4d "
#endif
			       "state %-8.8s k_height %8.2f k_speed %8.3f k_accel %8.3f avg_height %5d drogue %4d main %4d error %5d\n",
			       time,
			       height,
			       accel,
#if TELEMEGA
			       ao_sample_orient,

			       ao_mpu6000_accel(ao_data_static.mpu6000.accel_x),
			       ao_mpu6000_accel(ao_data_static.mpu6000.accel_y),
			       ao_mpu6000_accel(ao_data_static.mpu6000.accel_z),
			       ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_x - ao_ground_mpu6000.gyro_x),
			       ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_y - ao_ground_mpu6000.gyro_y),
			       ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_z - ao_ground_mpu6000.gyro_z),
			       ao_data_static.hmc5883.x,
			       ao_data_static.hmc5883.y,
			       ao_data_static.hmc5883.z,
			       ao_mag_angle,
#endif
			       ao_state_names[ao_flight_state],
			       ao_k_height / 65536.0,
			       ao_k_speed / 65536.0 / 16.0,
			       ao_k_accel / 65536.0 / 16.0,
			       ao_avg_height,
			       drogue_height,
			       main_height,
			       ao_error_h_sq_avg);
#endif
			
//			if (ao_flight_state == ao_flight_landed)
//				ao_test_exit();
		}
	}
}


uint16_t
uint16(uint8_t *bytes, int off)
{
	return (uint16_t) bytes[off] | (((uint16_t) bytes[off+1]) << 8);
}

int16_t
int16(uint8_t *bytes, int off)
{
	return (int16_t) uint16(bytes, off);
}

uint32_t
uint32(uint8_t *bytes, int off)
{
	return (uint32_t) bytes[off] | (((uint32_t) bytes[off+1]) << 8) |
		(((uint32_t) bytes[off+2]) << 16) |
		(((uint32_t) bytes[off+3]) << 24);
}

int32_t
int32(uint8_t *bytes, int off)
{
	return (int32_t) uint32(bytes, off);
}

static int log_format;

void
ao_sleep(void *wchan)
{
	if (wchan == &ao_data_head) {
		char		type = 0;
		uint16_t	tick = 0;
		uint16_t	a = 0, b = 0;
		uint8_t		bytes[1024];
		union ao_telemetry_all	telem;
		char		line[1024];
		char		*saveptr;
		char		*l;
		char		*words[64];
		int		nword;

#if TELEMEGA
		if (ao_flight_state >= ao_flight_boost && ao_flight_state < ao_flight_landed)
			ao_pyro_check();
#endif
		for (;;) {
			if (ao_records_read > 2 && ao_flight_state == ao_flight_startup)
			{
#if TELEMEGA
				ao_data_static.mpu6000 = ao_ground_mpu6000;
#else
				ao_data_static.adc.accel = ao_flight_ground_accel;
#endif
				ao_insert();
				return;
			}

			if (!fgets(line, sizeof (line), emulator_in)) {
				if (++ao_eof_read >= 1000) {
					if (!ao_summary)
						printf ("no more data, exiting simulation\n");
					ao_test_exit();
				}
				ao_data_static.tick += 10;
				ao_insert();
				return;
			}
			l = line;
			for (nword = 0; nword < 64; nword++) {
				words[nword] = strtok_r(l, " \t\n", &saveptr);
				l = NULL;
				if (words[nword] == NULL)
					break;
			}
#if TELEMEGA
			if (log_format == AO_LOG_FORMAT_TELEMEGA && nword == 30 && strlen(words[0]) == 1) {
				int	i;
				struct ao_ms5607_value	value;

				type = words[0][0];
				tick = strtoul(words[1], NULL, 16);
//				printf ("%c %04x", type, tick);
				for (i = 2; i < nword; i++) {
					bytes[i - 2] = strtoul(words[i], NULL, 16);
//					printf(" %02x", bytes[i-2]);
				}
//				printf ("\n");
				switch (type) {
				case 'F':
					ao_flight_ground_accel = int16(bytes, 2);
					ao_flight_started = 1;
					ao_ground_pres = int32(bytes, 4);
					ao_ground_height = ao_pa_to_altitude(ao_ground_pres);
					ao_ground_accel_along = int16(bytes, 8);
					ao_ground_accel_across = int16(bytes, 10);
					ao_ground_accel_through = int16(bytes, 12);
					ao_ground_roll = int16(bytes, 14);
					ao_ground_pitch = int16(bytes, 16);
					ao_ground_yaw = int16(bytes, 18);
					ao_ground_mpu6000.accel_x = ao_ground_accel_across;
					ao_ground_mpu6000.accel_y = ao_ground_accel_along;
					ao_ground_mpu6000.accel_z = ao_ground_accel_through;
					ao_ground_mpu6000.gyro_x = ao_ground_pitch >> 9;
					ao_ground_mpu6000.gyro_y = ao_ground_roll >> 9;
					ao_ground_mpu6000.gyro_z = ao_ground_yaw >> 9;
					break;
				case 'A':
					ao_data_static.tick = tick;
					ao_data_static.ms5607_raw.pres = int32(bytes, 0);
					ao_data_static.ms5607_raw.temp = int32(bytes, 4);
					ao_ms5607_convert(&ao_data_static.ms5607_raw, &value);
					ao_data_static.mpu6000.accel_x = int16(bytes, 8);
					ao_data_static.mpu6000.accel_y = int16(bytes, 10);
					ao_data_static.mpu6000.accel_z = int16(bytes, 12);
					ao_data_static.mpu6000.gyro_x = int16(bytes, 14);
					ao_data_static.mpu6000.gyro_y = int16(bytes, 16);
					ao_data_static.mpu6000.gyro_z = int16(bytes, 18);
					ao_data_static.hmc5883.x = int16(bytes, 20);
					ao_data_static.hmc5883.y = int16(bytes, 22);
					ao_data_static.hmc5883.z = int16(bytes, 24);
#if HAS_MMA655X
					ao_data_static.mma655x = int16(bytes, 26);
#endif
					ao_records_read++;
					ao_insert();
					return;
				case 'G':
					ao_gps_prev = ao_gps_static;
					ao_gps_static.tick = tick;
					ao_gps_static.latitude = int32(bytes, 0);
					ao_gps_static.longitude = int32(bytes, 4);
					ao_gps_static.altitude = int32(bytes, 8);
					ao_gps_static.flags = bytes[13];
					if (!ao_gps_count)
						ao_gps_first = ao_gps_static;
					ao_gps_count++;
					break;
				}
				continue;
			} else if (nword == 3 && strcmp(words[0], "ms5607") == 0) {
				if (strcmp(words[1], "reserved:") == 0)
					ao_ms5607_prom.reserved = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "sens:") == 0)
					ao_ms5607_prom.sens = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "off:") == 0)
					ao_ms5607_prom.off = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "tcs:") == 0)
					ao_ms5607_prom.tcs = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "tco:") == 0)
					ao_ms5607_prom.tco = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "tref:") == 0)
					ao_ms5607_prom.tref = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "tempsens:") == 0)
					ao_ms5607_prom.tempsens = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "crc:") == 0)
					ao_ms5607_prom.crc = strtoul(words[2], NULL, 10);
				continue;
			} else if (nword >= 3 && strcmp(words[0], "Pyro") == 0) {
				int	p = strtoul(words[1], NULL, 10);
				int	i, j;
				struct ao_pyro	*pyro = &ao_config.pyro[p];

				for (i = 2; i < nword; i++) {
					for (j = 0; j < NUM_PYRO_VALUES; j++)
						if (!strcmp (words[i], ao_pyro_values[j].name))
							break;
					if (j == NUM_PYRO_VALUES)
						continue;
					pyro->flags |= ao_pyro_values[j].flag;
					if (ao_pyro_values[j].offset != NO_VALUE && i + 1 < nword) {
						int16_t	val = strtoul(words[++i], NULL, 10);
						*((int16_t *) ((char *) pyro + ao_pyro_values[j].offset)) = val;
					}
				}
			}
#else
			if (nword == 4 && log_format != AO_LOG_FORMAT_TELEMEGA) {
				type = words[0][0];
				tick = strtoul(words[1], NULL, 16);
				a = strtoul(words[2], NULL, 16);
				b = strtoul(words[3], NULL, 16);
				if (type == 'P')
					type = 'A';
			}
#endif
			else if (nword == 2 && strcmp(words[0], "log-format") == 0) {
				log_format = strtoul(words[1], NULL, 10);
			} else if (nword >= 6 && strcmp(words[0], "Accel") == 0) {
				ao_config.accel_plus_g = atoi(words[3]);
				ao_config.accel_minus_g = atoi(words[5]);
#ifdef TELEMEGA
			} else if (nword >= 8 && strcmp(words[0], "IMU") == 0) {
				ao_config.accel_zero_along = atoi(words[3]);
				ao_config.accel_zero_across = atoi(words[5]);
				ao_config.accel_zero_through = atoi(words[7]);
				printf ("%d %d %d\n", ao_config.accel_zero_along, ao_config.accel_zero_across, ao_config.accel_zero_through);
#endif
			} else if (nword >= 4 && strcmp(words[0], "Main") == 0) {
				ao_config.main_deploy = atoi(words[2]);
			} else if (nword >= 3 && strcmp(words[0], "Apogee") == 0 &&
				   strcmp(words[1], "lockout:") == 0) {
				ao_config.apogee_lockout = atoi(words[2]);
			} else if (nword >= 36 && strcmp(words[0], "CALL") == 0) {
				tick = atoi(words[10]);
				if (!ao_flight_started) {
					type = 'F';
					a = atoi(words[26]);
					ao_flight_started = 1;
				} else {
					type = 'A';
					a = atoi(words[12]);
					b = atoi(words[14]);
				}
			} else if (nword == 3 && strcmp(words[0], "BARO") == 0) {
				tick = strtol(words[1], NULL, 16);
				a = 16384 - 328;
				b = strtol(words[2], NULL, 10);
				type = 'A';
				if (!ao_flight_started) {
					ao_flight_ground_accel = 16384 - 328;
					ao_config.accel_plus_g = 16384 - 328;
					ao_config.accel_minus_g = 16384 + 328;
					ao_flight_started = 1;
				}
			} else if (nword == 2 && strcmp(words[0], "TELEM") == 0) {
				__xdata char	*hex = words[1];
				char	elt[3];
				int	i, len;
				uint8_t	sum;

				len = strlen(hex);
				if (len > sizeof (bytes) * 2) {
					len = sizeof (bytes)*2;
					hex[len] = '\0';
				}
				for (i = 0; i < len; i += 2) {
					elt[0] = hex[i];
					elt[1] = hex[i+1];
					elt[2] = '\0';
					bytes[i/2] = (uint8_t) strtol(elt, NULL, 16);
				}
				len = i/2;
				if (bytes[0] != len - 2) {
					printf ("bad length %d != %d\n", bytes[0], len - 2);
					continue;
				}
				sum = 0x5a;
				for (i = 1; i < len-1; i++)
					sum += bytes[i];
				if (sum != bytes[len-1]) {
					printf ("bad checksum\n");
					continue;
				}
				if ((bytes[len-2] & 0x80) == 0) {
					continue;
				}
				if (len == 36) {
					ao_xmemcpy(&telem, bytes + 1, 32);
					tick = telem.generic.tick;
					switch (telem.generic.type) {
					case AO_TELEMETRY_SENSOR_TELEMETRUM:
					case AO_TELEMETRY_SENSOR_TELEMINI:
					case AO_TELEMETRY_SENSOR_TELENANO:
						if (!ao_flight_started) {
							ao_flight_ground_accel = telem.sensor.ground_accel;
							ao_config.accel_plus_g = telem.sensor.accel_plus_g;
							ao_config.accel_minus_g = telem.sensor.accel_minus_g;
							ao_flight_started = 1;
						}
						type = 'A';
						a = telem.sensor.accel;
						b = telem.sensor.pres;
						break;
					}
				} else if (len == 99) {
					ao_flight_started = 1;
					tick = uint16(bytes+1, 21);
					ao_flight_ground_accel = int16(bytes+1, 7);
					ao_config.accel_plus_g = int16(bytes+1, 17);
					ao_config.accel_minus_g = int16(bytes+1, 19);
					type = 'A';
					a = int16(bytes+1, 23);
					b = int16(bytes+1, 25);
				} else if (len == 98) {
					ao_flight_started = 1;
					tick = uint16(bytes+1, 20);
					ao_flight_ground_accel = int16(bytes+1, 6);
					ao_config.accel_plus_g = int16(bytes+1, 16);
					ao_config.accel_minus_g = int16(bytes+1, 18);
					type = 'A';
					a = int16(bytes+1, 22);
					b = int16(bytes+1, 24);
				} else {
					printf("unknown len %d\n", len);
					continue;
				}
			}
			if (type != 'F' && !ao_flight_started)
				continue;

#if TELEMEGA
			(void) a;
			(void) b;
#else
			switch (type) {
			case 'F':
				ao_flight_ground_accel = a;
				if (ao_config.accel_plus_g == 0) {
					ao_config.accel_plus_g = a;
					ao_config.accel_minus_g = a + 530;
				}
				if (ao_config.main_deploy == 0)
					ao_config.main_deploy = 250;
				ao_flight_started = 1;
				break;
			case 'S':
				break;
			case 'A':
				ao_data_static.tick = tick;
				ao_data_static.adc.accel = a;
				ao_data_static.adc.pres_real = b;
				if (b < AO_MIN_BARO_VALUE)
					b = AO_MIN_BARO_VALUE;
				ao_data_static.adc.pres = b;
				ao_records_read++;
				ao_insert();
				return;
			case 'T':
				ao_data_static.tick = tick;
				ao_data_static.adc.temp = a;
				ao_data_static.adc.v_batt = b;
				break;
			case 'D':
			case 'G':
			case 'N':
			case 'W':
			case 'H':
				break;
			}
#endif
		}

	}
}
#define COUNTS_PER_G 264.8

void
ao_dump_state(void)
{
}

static const struct option options[] = {
	{ .name = "summary", .has_arg = 0, .val = 's' },
	{ .name = "debug", .has_arg = 0, .val = 'd' },
	{ .name = "info", .has_arg = 1, .val = 'i' },
	{ 0, 0, 0, 0},
};

void run_flight_fixed(char *name, FILE *f, int summary, char *info)
{
	emulator_name = name;
	emulator_in = f;
	emulator_info = info;
	ao_summary = summary;

	ao_flight_init();
	ao_flight();
}

int
main (int argc, char **argv)
{
	int	summary = 0;
	int	c;
	int	i;
	char	*info = NULL;

#if HAS_ACCEL
	emulator_app="full";
#else
	emulator_app="baro";
#endif
	while ((c = getopt_long(argc, argv, "sdi:", options, NULL)) != -1) {
		switch (c) {
		case 's':
			summary = 1;
			break;
		case 'd':
			ao_flight_debug = 1;
			break;
		case 'i':
			info = optarg;
			break;
		}
	}

	if (optind == argc)
		run_flight_fixed("<stdin>", stdin, summary, info);
	else
		for (i = optind; i < argc; i++) {
			FILE	*f = fopen(argv[i], "r");
			if (!f) {
				perror(argv[i]);
				continue;
			}
			run_flight_fixed(argv[i], f, summary, info);
			fclose(f);
		}
	exit(0);
}
