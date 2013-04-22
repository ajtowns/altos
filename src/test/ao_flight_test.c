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
#include <string.h>
#include <getopt.h>
#include <math.h>

#define AO_HERTZ	100

#define HAS_ADC 1
#define AO_DATA_RING	64
#define ao_data_ring_next(n)	(((n) + 1) & (AO_DATA_RING - 1))
#define ao_data_ring_prev(n)	(((n) - 1) & (AO_DATA_RING - 1))

#define AO_M_TO_HEIGHT(m)	((int16_t) (m))
#define AO_MS_TO_SPEED(ms)	((int16_t) ((ms) * 16))
#define AO_MSS_TO_ACCEL(mss)	((int16_t) ((mss) * 16))

#if TELEMEGA
#define AO_ADC_NUM_SENSE	6
#define HAS_MS5607		1
#define HAS_MPU6000		1
#define HAS_MMA655X		1

struct ao_adc {
	int16_t			sense[AO_ADC_NUM_SENSE];
	int16_t			v_batt;
	int16_t			v_pbatt;
	int16_t			accel_ref;
	int16_t			accel;
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
struct ao_ms5607_prom	ms5607_prom;
#include "ao_ms5607_convert.c"
#else
#include "ao_convert.c"
#endif

struct ao_config {
	uint16_t	main_deploy;
	int16_t		accel_plus_g;
	int16_t		accel_minus_g;
	uint8_t		pad_orientation;
	uint16_t	apogee_lockout;
};

#define AO_PAD_ORIENTATION_ANTENNA_UP	0
#define AO_PAD_ORIENTATION_ANTENNA_DOWN	1

#define ao_config_get()

struct ao_config ao_config;

#define DATA_TO_XDATA(x) (x)


#define GRAVITY 9.80665
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

#if HAS_MPU6000
static double
ao_mpu6000_accel(int16_t sensor)
{
	return sensor / 32767.0 * MPU6000_ACCEL_FULLSCALE * GRAVITY;
}

static double
ao_mpu6000_gyro(int32_t sensor)
{
	return sensor / 32767.0 * MPU6000_GYRO_FULLSCALE;
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
			printf("%7.2f height %8.2f accel %8.3f "
#if TELEMEGA
			       "roll %8.3f angle %8.3f qangle %8.3f "
			       "accel_x %8.3f accel_y %8.3f accel_z %8.3f gyro_x %8.3f gyro_y %8.3f gyro_z %8.3f "
#endif
			       "state %-8.8s k_height %8.2f k_speed %8.3f k_accel %8.3f avg_height %5d drogue %4d main %4d error %5d\n",
			       time,
			       height,
			       accel,
#if TELEMEGA
			       ao_mpu6000_gyro(ao_sample_roll_angle) / 100.0,
			       ao_mpu6000_gyro(ao_sample_angle) / 100.0,
			       ao_sample_qangle,
			       ao_mpu6000_accel(ao_data_static.mpu6000.accel_x),
			       ao_mpu6000_accel(ao_data_static.mpu6000.accel_y),
			       ao_mpu6000_accel(ao_data_static.mpu6000.accel_z),
			       ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_x - ao_ground_mpu6000.gyro_x),
			       ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_y - ao_ground_mpu6000.gyro_y),
			       ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_z - ao_ground_mpu6000.gyro_z),
#endif
			       ao_state_names[ao_flight_state],
			       ao_k_height / 65536.0,
			       ao_k_speed / 65536.0 / 16.0,
			       ao_k_accel / 65536.0 / 16.0,
			       ao_avg_height,
			       drogue_height,
			       main_height,
			       ao_error_h_sq_avg);
			
//			if (ao_flight_state == ao_flight_landed)
//				ao_test_exit();
		}
	}
}

#define AO_MAX_CALLSIGN			8
#define AO_MAX_VERSION			8
#define AO_MAX_TELEMETRY		128

struct ao_telemetry_generic {
	uint16_t	serial;		/* 0 */
	uint16_t	tick;		/* 2 */
	uint8_t		type;		/* 4 */
	uint8_t		payload[27];	/* 5 */
	/* 32 */
};

#define AO_TELEMETRY_SENSOR_TELEMETRUM	0x01
#define AO_TELEMETRY_SENSOR_TELEMINI	0x02
#define AO_TELEMETRY_SENSOR_TELENANO	0x03

struct ao_telemetry_sensor {
	uint16_t	serial;		/*  0 */
	uint16_t	tick;		/*  2 */
	uint8_t		type;		/*  4 */

	uint8_t         state;          /*  5 flight state */
	int16_t		accel;		/*  6 accelerometer (TM only) */
	int16_t		pres;		/*  8 pressure sensor */
	int16_t		temp;		/* 10 temperature sensor */
	int16_t		v_batt;		/* 12 battery voltage */
	int16_t		sense_d;	/* 14 drogue continuity sense (TM/Tm) */
	int16_t		sense_m;	/* 16 main continuity sense (TM/Tm) */

	int16_t         acceleration;   /* 18 m/s² * 16 */
	int16_t         speed;          /* 20 m/s * 16 */
	int16_t         height;         /* 22 m */

	int16_t		ground_pres;	/* 24 average pres on pad */
	int16_t		ground_accel;	/* 26 average accel on pad */
	int16_t		accel_plus_g;	/* 28 accel calibration at +1g */
	int16_t		accel_minus_g;	/* 30 accel calibration at -1g */
	/* 32 */
};

#define AO_TELEMETRY_CONFIGURATION	0x04

struct ao_telemetry_configuration {
	uint16_t	serial;				/*  0 */
	uint16_t	tick;				/*  2 */
	uint8_t		type;				/*  4 */

	uint8_t         device;         		/*  5 device type */
	uint16_t        flight;				/*  6 flight number */
	uint8_t		config_major;			/*  8 Config major version */
	uint8_t		config_minor;			/*  9 Config minor version */
	uint16_t	apogee_delay;			/* 10 Apogee deploy delay in seconds */
	uint16_t	main_deploy;			/* 12 Main deploy alt in meters */
	uint16_t	flight_log_max;			/* 14 Maximum flight log size in kB */
	char		callsign[AO_MAX_CALLSIGN];	/* 16 Radio operator identity */
	char		version[AO_MAX_VERSION];	/* 24 Software version */
	/* 32 */
};

#define AO_TELEMETRY_LOCATION		0x05

#define AO_GPS_MODE_NOT_VALID		'N'
#define AO_GPS_MODE_AUTONOMOUS		'A'
#define AO_GPS_MODE_DIFFERENTIAL	'D'
#define AO_GPS_MODE_ESTIMATED		'E'
#define AO_GPS_MODE_MANUAL		'M'
#define AO_GPS_MODE_SIMULATED		'S'

struct ao_telemetry_location {
	uint16_t	serial;		/*  0 */
	uint16_t	tick;		/*  2 */
	uint8_t		type;		/*  4 */

	uint8_t         flags;		/*  5 Number of sats and other flags */
	int16_t         altitude;	/*  6 GPS reported altitude (m) */
	int32_t         latitude;	/*  8 latitude (degrees * 10⁷) */
	int32_t         longitude;	/* 12 longitude (degrees * 10⁷) */
	uint8_t         year;		/* 16 (- 2000) */
	uint8_t         month;		/* 17 (1-12) */
	uint8_t         day;		/* 18 (1-31) */
	uint8_t         hour;		/* 19 (0-23) */
	uint8_t         minute;		/* 20 (0-59) */
	uint8_t         second;		/* 21 (0-59) */
	uint8_t         pdop;		/* 22 (m * 5) */
	uint8_t         hdop;		/* 23 (m * 5) */
	uint8_t         vdop;		/* 24 (m * 5) */
	uint8_t         mode;		/* 25 */
	uint16_t	ground_speed;	/* 26 cm/s */
	int16_t		climb_rate;	/* 28 cm/s */
	uint8_t		course;		/* 30 degrees / 2 */
	uint8_t		unused[1];	/* 31 */
	/* 32 */
};

#define AO_TELEMETRY_SATELLITE		0x06

struct ao_telemetry_satellite_info {
	uint8_t		svid;
	uint8_t		c_n_1;
};

struct ao_telemetry_satellite {
	uint16_t				serial;		/*  0 */
	uint16_t				tick;		/*  2 */
	uint8_t					type;		/*  4 */
	uint8_t					channels;	/*  5 number of reported sats */

	struct ao_telemetry_satellite_info	sats[12];	/* 6 */
	uint8_t					unused[2];	/* 30 */
	/* 32 */
};

union ao_telemetry_all {
	struct ao_telemetry_generic		generic;
	struct ao_telemetry_sensor		sensor;
	struct ao_telemetry_configuration	configuration;
	struct ao_telemetry_location		location;
	struct ao_telemetry_satellite		satellite;
};

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

#if TELEMEGA

static double
ao_vec_norm(double x, double y, double z)
{
	return x*x + y*y + z*z;
}

static void
ao_vec_normalize(double *x, double *y, double *z)
{
	double	scale = 1/sqrt(ao_vec_norm(*x, *y, *z));

	*x *= scale;
	*y *= scale;
	*z *= scale;
}

struct ao_quat {
	double	q0, q1, q2, q3;
};

static void
ao_quat_mul(struct ao_quat *r, struct ao_quat *a, struct ao_quat *b)
{
	r->q0 = a->q0 * b->q0 - a->q1 * b->q1 - a->q2 * b->q2 - a->q3 * b->q3;
	r->q1 = a->q0 * b->q1 + a->q1 * b->q0 + a->q2 * b->q3 - a->q3 * b->q2;
	r->q2 = a->q0 * b->q2 - a->q1 * b->q3 + a->q2 * b->q0 + a->q3 * b->q1;
	r->q3 = a->q0 * b->q3 + a->q1 * b->q2 - a->q2 * b->q1 + a->q3 * b->q0;
}

#if 0
static void
ao_quat_scale(struct ao_quat *r, struct ao_quat *a, double s)
{
	r->q0 = a->q0 * s;
	r->q1 = a->q1 * s;
	r->q2 = a->q2 * s;
	r->q3 = a->q3 * s;
}
#endif

static void
ao_quat_conj(struct ao_quat *r, struct ao_quat *a)
{
	r->q0 =  a->q0;
	r->q1 = -a->q1;
	r->q2 = -a->q2;
	r->q3 = -a->q3;
}

static void
ao_quat_rot(struct ao_quat *r, struct ao_quat *a, struct ao_quat *q)
{
	struct ao_quat	t;
	struct ao_quat	c;
	ao_quat_mul(&t, q, a);
	ao_quat_conj(&c, q);
	ao_quat_mul(r, &t, &c);
}

static void
ao_quat_from_angle(struct ao_quat *r,
		   double x_rad,
		   double y_rad,
		   double z_rad)
{
	double angle = sqrt (x_rad * x_rad + y_rad * y_rad + z_rad * z_rad);
	double s = sin(angle/2);
	double c = cos(angle/2);

	r->q0 = c;
	r->q1 = x_rad * s / angle;
	r->q2 = y_rad * s / angle;
	r->q3 = z_rad * s / angle;
}

static void
ao_quat_from_vector(struct ao_quat *r, double x, double y, double z)
{
	ao_vec_normalize(&x, &y, &z);
	double	x_rad = atan2(z, y);
	double	y_rad = atan2(x, z);
	double	z_rad = atan2(y, x);

	ao_quat_from_angle(r, x_rad, y_rad, z_rad);
}

static double
ao_quat_norm(struct ao_quat *a)
{
	return (a->q0 * a->q0 +
		a->q1 * a->q1 +
		a->q2 * a->q2 +
		a->q3 * a->q3);
}

static void
ao_quat_normalize(struct ao_quat *a)
{
	double	norm = ao_quat_norm(a);

	if (norm) {
		double m = 1/sqrt(norm);

		a->q0 *= m;
		a->q1 *= m;
		a->q2 *= m;
		a->q3 *= m;
	}
}

static struct ao_quat	ao_up, ao_current;
static struct ao_quat	ao_orient;
static int		ao_orient_tick;

void
set_orientation(double x, double y, double z, int tick)
{
	struct ao_quat	t;

	printf ("set_orientation %g %g %g\n", x, y, z);
	ao_quat_from_vector(&ao_orient, x, y, z);
	ao_up.q1 = ao_up.q2 = 0;
	ao_up.q0 = ao_up.q3 = sqrt(2)/2;
	ao_orient_tick = tick;

	ao_orient.q0 = 1;
	ao_orient.q1 = 0;
	ao_orient.q2 = 0;
	ao_orient.q3 = 0;

	printf ("orient (%g) %g %g %g up (%g) %g %g %g\n",
		ao_orient.q0,
		ao_orient.q1,
		ao_orient.q2,
		ao_orient.q3,
		ao_up.q0,
		ao_up.q1,
		ao_up.q2,
		ao_up.q3);

	ao_quat_rot(&t, &ao_up, &ao_orient);
	printf ("pad orient (%g) %g %g %g\n",
		t.q0,
		t.q1,
		t.q2,
		t.q3);

}

void
update_orientation (double rate_x, double rate_y, double rate_z, int tick)
{
	struct ao_quat	q_dot;
	double		lambda;
	double		dt = (tick - ao_orient_tick) / 100.0;

	ao_orient_tick = tick;
 
//	lambda = 1 - ao_quat_norm(&ao_orient);
	lambda = 0;

	q_dot.q0 = -0.5 * (ao_orient.q1 * rate_x + ao_orient.q2 * rate_y + ao_orient.q3 * rate_z) + lambda * ao_orient.q0;
	q_dot.q1 =  0.5 * (ao_orient.q0 * rate_x + ao_orient.q2 * rate_z - ao_orient.q3 * rate_y) + lambda * ao_orient.q1;
	q_dot.q2 =  0.5 * (ao_orient.q0 * rate_y + ao_orient.q3 * rate_x - ao_orient.q1 * rate_z) + lambda * ao_orient.q2;
	q_dot.q3 =  0.5 * (ao_orient.q0 * rate_z + ao_orient.q1 * rate_y - ao_orient.q2 * rate_x) + lambda * ao_orient.q3;

#if 0
	printf ("update_orientation %g %g %g (%g s)\n", rate_x, rate_y, rate_z, dt);
	printf ("q_dot (%g) %g %g %g\n",
		q_dot.q0,
		q_dot.q1,
		q_dot.q2,
		q_dot.q3);
#endif

	ao_orient.q0 += q_dot.q0 * dt;
	ao_orient.q1 += q_dot.q1 * dt;
	ao_orient.q2 += q_dot.q2 * dt;
	ao_orient.q3 += q_dot.q3 * dt;

	ao_quat_normalize(&ao_orient);

	ao_quat_rot(&ao_current, &ao_up, &ao_orient);

	ao_sample_qangle = 180 / M_PI * acos(ao_current.q3 * sqrt(2));
#if 0
	printf ("orient (%g) %g %g %g current (%g) %g %g %g\n",
		ao_orient.q0,
		ao_orient.q1,
		ao_orient.q2,
		ao_orient.q3,
		ao_current.q0,
		ao_current.q1,
		ao_current.q2,
		ao_current.q3);
#endif
}
#endif

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
					break;
				case 'A':
					ao_data_static.tick = tick;
					ao_data_static.ms5607_raw.pres = int32(bytes, 0);
					ao_data_static.ms5607_raw.temp = int32(bytes, 4);
					ao_ms5607_convert(&ao_data_static.ms5607_raw, &value);
					ao_data_static.mpu6000.accel_x = int16(bytes, 8);
					ao_data_static.mpu6000.accel_y = -int16(bytes, 10);
					ao_data_static.mpu6000.accel_z = int16(bytes, 12);
					ao_data_static.mpu6000.gyro_x = int16(bytes, 14);
					ao_data_static.mpu6000.gyro_y = -int16(bytes, 16);
					ao_data_static.mpu6000.gyro_z = int16(bytes, 18);
#if HAS_MMA655X
					ao_data_static.mma655x = int16(bytes, 26);
#endif
					if (ao_records_read == 0)
						ao_ground_mpu6000 = ao_data_static.mpu6000;
					else if (ao_records_read < 10) {
#define f(f) ao_ground_mpu6000.f = ao_ground_mpu6000.f + ((ao_data_static.mpu6000.f - ao_ground_mpu6000.f) >> 2)
						f(accel_x);
						f(accel_y);
						f(accel_z);
						f(gyro_x);
						f(gyro_y);
						f(gyro_z);

						double		accel_x = ao_mpu6000_accel(ao_ground_mpu6000.accel_x);
						double		accel_y = ao_mpu6000_accel(ao_ground_mpu6000.accel_y);
						double		accel_z = ao_mpu6000_accel(ao_ground_mpu6000.accel_z);

						/* X and Y are in the ground plane, arbitraryily picked as MPU X and Z axes
						 * Z is normal to the ground, the MPU y axis
						 */
						set_orientation(accel_x, accel_z, accel_y, tick);
					} else {
						double		rate_x = ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_x - ao_ground_mpu6000.gyro_x);
						double		rate_y = ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_y - ao_ground_mpu6000.gyro_y);
						double		rate_z = ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_z - ao_ground_mpu6000.gyro_z);

						update_orientation(rate_x * M_PI / 180, rate_z * M_PI / 180, rate_y * M_PI / 180, tick);
					}
					ao_records_read++;
					ao_insert();
					return;
				}
				continue;
			} else if (nword == 3 && strcmp(words[0], "ms5607") == 0) {
				if (strcmp(words[1], "reserved:") == 0)
					ms5607_prom.reserved = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "sens:") == 0)
					ms5607_prom.sens = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "off:") == 0)
					ms5607_prom.off = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "tcs:") == 0)
					ms5607_prom.tcs = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "tco:") == 0)
					ms5607_prom.tco = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "tref:") == 0)
					ms5607_prom.tref = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "tempsens:") == 0)
					ms5607_prom.tempsens = strtoul(words[2], NULL, 10);
				else if (strcmp(words[1], "crc:") == 0)
					ms5607_prom.crc = strtoul(words[2], NULL, 10);
				continue;
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
