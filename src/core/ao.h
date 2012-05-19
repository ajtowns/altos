/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License
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

#ifndef _AO_H_
#define _AO_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "ao_pins.h"
#include <ao_arch.h>

#define TRUE 1
#define FALSE 0

/* Convert a __data pointer into an __xdata pointer */
#ifndef DATA_TO_XDATA
#define DATA_TO_XDATA(a)	(a)
#endif
#ifndef PDATA_TO_XDATA
#define PDATA_TO_XDATA(a)	(a)
#endif
#ifndef CODE_TO_XDATA
#define CODE_TO_XDATA(a)	(a)
#endif

/* An AltOS task */
struct ao_task {
	__xdata void *wchan;		/* current wait channel (NULL if running) */
	uint16_t alarm;			/* abort ao_sleep time */
	ao_arch_task_members		/* any architecture-specific fields */
	uint8_t task_id;		/* unique id */
	__code char *name;		/* task name */
	uint8_t	stack[AO_STACK_SIZE];	/* saved stack */
};

extern __xdata struct ao_task *__data ao_cur_task;

#define AO_NUM_TASKS		16	/* maximum number of tasks */
#define AO_NO_TASK		0	/* no task id */

/*
 ao_task.c
 */

/* Suspend the current task until wchan is awoken.
 * returns:
 *  0 on normal wake
 *  1 on alarm
 */
uint8_t
ao_sleep(__xdata void *wchan);

/* Wake all tasks sleeping on wchan */
void
ao_wakeup(__xdata void *wchan);

/* set an alarm to go off in 'delay' ticks */
void
ao_alarm(uint16_t delay);

/* Clear any pending alarm */
void
ao_clear_alarm(void);

/* Yield the processor to another task */
void
ao_yield(void) ao_arch_naked_declare;

/* Add a task to the run queue */
void
ao_add_task(__xdata struct ao_task * task, void (*start)(void), __code char *name) __reentrant;

/* Terminate the current task */
void
ao_exit(void);

/* Dump task info to console */
void
ao_task_info(void);

/* Start the scheduler. This will not return */
void
ao_start_scheduler(void);

/*
 * ao_panic.c
 */

#define AO_PANIC_NO_TASK	1	/* AO_NUM_TASKS is not large enough */
#define AO_PANIC_DMA		2	/* Attempt to start DMA while active */
#define AO_PANIC_MUTEX		3	/* Mis-using mutex API */
#define AO_PANIC_EE		4	/* Mis-using eeprom API */
#define AO_PANIC_LOG		5	/* Failing to read/write log data */
#define AO_PANIC_CMD		6	/* Too many command sets registered */
#define AO_PANIC_STDIO		7	/* Too many stdio handlers registered */
#define AO_PANIC_REBOOT		8	/* Reboot failed */
#define AO_PANIC_FLASH		9	/* Invalid flash part (or wrong blocksize) */
#define AO_PANIC_USB		10	/* Trying to send USB packet while busy */
#define AO_PANIC_BT		11	/* Communications with bluetooth device failed */
#define AO_PANIC_STACK		12	/* Stack overflow */
#define AO_PANIC_SPI		13	/* SPI communication failure */

/* Stop the operating system, beeping and blinking the reason */
void
ao_panic(uint8_t reason);

/*
 * ao_timer.c
 */

/* Our timer runs at 100Hz */
#define AO_HERTZ		100
#define AO_MS_TO_TICKS(ms)	((ms) / (1000 / AO_HERTZ))
#define AO_SEC_TO_TICKS(s)	((s) * AO_HERTZ)

/* Returns the current time in ticks */
uint16_t
ao_time(void);

/* Suspend the current task until ticks time has passed */
void
ao_delay(uint16_t ticks);

/* Set the ADC interval */
void
ao_timer_set_adc_interval(uint8_t interval) __critical;

/* Timer interrupt */
void
ao_timer_isr(void) ao_arch_interrupt(9);

/* Initialize the timer */
void
ao_timer_init(void);

/* Initialize the hardware clock. Must be called first */
void
ao_clock_init(void);

/*
 * ao_mutex.c
 */

void
ao_mutex_get(__xdata uint8_t *ao_mutex) __reentrant;

void
ao_mutex_put(__xdata uint8_t *ao_mutex) __reentrant;

/*
 * ao_cmd.c
 */

enum ao_cmd_status {
	ao_cmd_success = 0,
	ao_cmd_lex_error = 1,
	ao_cmd_syntax_error = 2,
};

extern __pdata uint16_t ao_cmd_lex_i;
extern __pdata uint32_t ao_cmd_lex_u32;
extern __pdata char	ao_cmd_lex_c;
extern __pdata enum ao_cmd_status ao_cmd_status;

void
ao_cmd_lex(void);

void
ao_cmd_put8(uint8_t v);

void
ao_cmd_put16(uint16_t v);

uint8_t
ao_cmd_is_white(void);

void
ao_cmd_white(void);

int8_t
ao_cmd_hexchar(char c);

void
ao_cmd_hexbyte(void);

void
ao_cmd_hex(void);

void
ao_cmd_decimal(void);

uint8_t
ao_match_word(__code char *word);

struct ao_cmds {
	void		(*func)(void);
	__code char	*help;
};

void
ao_cmd_register(const __code struct ao_cmds *cmds);

void
ao_cmd_init(void);

#if HAS_CMD_FILTER
/*
 * Provided by an external module to filter raw command lines
 */
uint8_t
ao_cmd_filter(void);
#endif

/*
 * Various drivers
 */
#if HAS_ADC
#include <ao_adc.h>
#endif

#if HAS_BEEP
#include <ao_beep.h>
#endif

#if LEDS_AVAILABLE
#include <ao_led.h>
#endif

#if HAS_USB
#include <ao_usb.h>
#endif

#if HAS_EEPROM
#include <ao_storage.h>
#endif

#if HAS_LOG
#include <ao_log.h>
#endif

#if HAS_FLIGHT
#include <ao_flight.h>
#include <ao_sample.h>
#endif

/*
 * ao_report.c
 */

void
ao_report_init(void);

/*
 * ao_convert.c
 *
 * Given raw data, convert to SI units
 */

/* pressure from the sensor to altitude in meters */
int16_t
ao_pres_to_altitude(int16_t pres) __reentrant;

int16_t
ao_altitude_to_pres(int16_t alt) __reentrant;

int16_t
ao_temp_to_dC(int16_t temp) __reentrant;

/*
 * ao_convert_pa.c
 *
 * Convert between pressure in Pa and altitude in meters
 */

int32_t
ao_pa_to_altitude(int32_t pa);

int32_t
ao_altitude_to_pa(int32_t alt);

#if HAS_DBG
#include <ao_dbg.h>
#endif

#if HAS_SERIAL_0 || HAS_SERIAL_1 || HAS_SERIAL_2 || HAS_SERIAL_3
#include <ao_serial.h>
#endif


/*
 * ao_spi_slave.c
 */

uint8_t
ao_spi_slave_recv(uint8_t *buf, uint8_t len);

void
ao_spi_slave_send(uint8_t *buf, uint8_t len);

void
ao_spi_slave_init(void);

/* This must be defined by the product; it will get called when chip
 * select goes low, at which point it should use ao_spi_read and
 * ao_spi_write to deal with the request
 */

void
ao_spi_slave(void);

/*
 * ao_telemetry.c
 */
#define AO_MAX_CALLSIGN			8
#define AO_MAX_VERSION			8
#if LEGACY_MONITOR
#define AO_MAX_TELEMETRY		128
#else
#define AO_MAX_TELEMETRY		32
#endif

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

#define AO_TELEMETRY_COMPANION		0x07

#define AO_COMPANION_MAX_CHANNELS	12

struct ao_telemetry_companion {
	uint16_t				serial;		/*  0 */
	uint16_t				tick;		/*  2 */
	uint8_t					type;		/*  4 */
	uint8_t					board_id;	/*  5 */

	uint8_t					update_period;	/*  6 */
	uint8_t					channels;	/*  7 */
	uint16_t				companion_data[AO_COMPANION_MAX_CHANNELS];	/*  8 */
	/* 32 */
};
	
/* #define AO_SEND_ALL_BARO */

#define AO_TELEMETRY_BARO		0x80

/*
 * This packet allows the full sampling rate baro
 * data to be captured over the RF link so that the
 * flight software can be tested using 'real' data.
 *
 * Along with this telemetry packet, the flight
 * code is modified to send full-rate telemetry all the time
 * and never send an RDF tone; this ensure that the full radio
 * link is available.
 */
struct ao_telemetry_baro {
	uint16_t				serial;		/*  0 */
	uint16_t				tick;		/*  2 */
	uint8_t					type;		/*  4 */
	uint8_t					samples;	/*  5 number samples */

	int16_t					baro[12];	/* 6 samples */
	/* 32 */
};

union ao_telemetry_all {
	struct ao_telemetry_generic		generic;
	struct ao_telemetry_sensor		sensor;
	struct ao_telemetry_configuration	configuration;
	struct ao_telemetry_location		location;
	struct ao_telemetry_satellite		satellite;
	struct ao_telemetry_companion		companion;
	struct ao_telemetry_baro		baro;
};

struct ao_telemetry_all_recv {
	union ao_telemetry_all		telemetry;
	int8_t				rssi;
	uint8_t				status;
};

/*
 * ao_gps.c
 */

#define AO_GPS_NUM_SAT_MASK	(0xf << 0)
#define AO_GPS_NUM_SAT_SHIFT	(0)

#define AO_GPS_VALID		(1 << 4)
#define AO_GPS_RUNNING		(1 << 5)
#define AO_GPS_DATE_VALID	(1 << 6)
#define AO_GPS_COURSE_VALID	(1 << 7)

extern __pdata uint16_t ao_gps_tick;
extern __xdata uint8_t ao_gps_mutex;
extern __xdata struct ao_telemetry_location ao_gps_data;
extern __xdata struct ao_telemetry_satellite ao_gps_tracking_data;

struct ao_gps_orig {
	uint8_t			year;
	uint8_t			month;
	uint8_t			day;
	uint8_t			hour;
	uint8_t			minute;
	uint8_t			second;
	uint8_t			flags;
	int32_t			latitude;	/* degrees * 10⁷ */
	int32_t			longitude;	/* degrees * 10⁷ */
	int16_t			altitude;	/* m */
	uint16_t		ground_speed;	/* cm/s */
	uint8_t			course;		/* degrees / 2 */
	uint8_t			hdop;		/* * 5 */
	int16_t			climb_rate;	/* cm/s */
	uint16_t		h_error;	/* m */
	uint16_t		v_error;	/* m */
};

struct ao_gps_sat_orig {
	uint8_t		svid;
	uint8_t		c_n_1;
};

#define AO_MAX_GPS_TRACKING	12

struct ao_gps_tracking_orig {
	uint8_t			channels;
	struct ao_gps_sat_orig	sats[AO_MAX_GPS_TRACKING];
};

void
ao_gps(void);

void
ao_gps_print(__xdata struct ao_gps_orig *gps_data);

void
ao_gps_tracking_print(__xdata struct ao_gps_tracking_orig *gps_tracking_data);

void
ao_gps_init(void);

/*
 * ao_gps_report.c
 */

void
ao_gps_report(void);

void
ao_gps_report_init(void);

/*
 * ao_telemetry_orig.c
 */

struct ao_telemetry_orig {
	uint16_t		serial;
	uint16_t		flight;
	uint8_t			flight_state;
	int16_t			accel;
	int16_t			ground_accel;
	union {
		struct {
			int16_t			speed;
			int16_t			unused;
		} k;
		int32_t		flight_vel;
	} u;
	int16_t			height;
	int16_t			ground_pres;
	int16_t			accel_plus_g;
	int16_t			accel_minus_g;
	struct ao_adc		adc;
	struct ao_gps_orig	gps;
	char			callsign[AO_MAX_CALLSIGN];
	struct ao_gps_tracking_orig	gps_tracking;
};

struct ao_telemetry_tiny {
	uint16_t		serial;
	uint16_t		flight;
	uint8_t			flight_state;
	int16_t			height;		/* AGL in meters */
	int16_t			speed;		/* in m/s * 16 */
	int16_t			accel;		/* in m/s² * 16 */
	int16_t			ground_pres;	/* sensor units */
	struct ao_adc		adc;		/* raw ADC readings */
	char			callsign[AO_MAX_CALLSIGN];
};

struct ao_telemetry_orig_recv {
	struct ao_telemetry_orig	telemetry_orig;
	int8_t				rssi;
	uint8_t				status;
};

struct ao_telemetry_tiny_recv {
	struct ao_telemetry_tiny	telemetry_tiny;
	int8_t				rssi;
	uint8_t				status;
};

/*
 * ao_radio_recv tacks on rssi and status bytes
 */

struct ao_telemetry_raw_recv {
	uint8_t			packet[AO_MAX_TELEMETRY + 2];
};

/* Set delay between telemetry reports (0 to disable) */

#ifdef AO_SEND_ALL_BARO
#define AO_TELEMETRY_INTERVAL_PAD	AO_MS_TO_TICKS(100)
#define AO_TELEMETRY_INTERVAL_FLIGHT	AO_MS_TO_TICKS(100)
#define AO_TELEMETRY_INTERVAL_RECOVER	AO_MS_TO_TICKS(100)
#else
#define AO_TELEMETRY_INTERVAL_PAD	AO_MS_TO_TICKS(1000)
#define AO_TELEMETRY_INTERVAL_FLIGHT	AO_MS_TO_TICKS(100)
#define AO_TELEMETRY_INTERVAL_RECOVER	AO_MS_TO_TICKS(1000)
#endif

void
ao_telemetry_set_interval(uint16_t interval);

void
ao_rdf_set(uint8_t rdf);

void
ao_telemetry_init(void);

void
ao_telemetry_orig_init(void);

void
ao_telemetry_tiny_init(void);

/*
 * ao_radio.c
 */

extern __xdata uint8_t	ao_radio_dma;
extern __xdata uint8_t ao_radio_dma_done;
extern __xdata uint8_t ao_radio_done;
extern __xdata uint8_t ao_radio_mutex;

void
ao_radio_general_isr(void) ao_arch_interrupt(16);

void
ao_radio_get(uint8_t len);

#define ao_radio_put() ao_mutex_put(&ao_radio_mutex)

void
ao_radio_set_packet(void);

void
ao_radio_send(__xdata void *d, uint8_t size) __reentrant;

uint8_t
ao_radio_recv(__xdata void *d, uint8_t size) __reentrant;

void
ao_radio_recv_abort(void);

/*
 * Compute the packet length as follows:
 *
 * 2000 bps (for a 1kHz tone)
 * so, for 'ms' milliseconds, we need
 * 2 * ms bits, or ms / 4 bytes
 */

#define AO_MS_TO_RDF_LEN(ms) ((ms) > 255 * 4 ? 255 : ((ms) >> 2))

void
ao_radio_rdf(uint8_t pkt_len);

void
ao_radio_rdf_abort(void);

void
ao_radio_idle(void);

void
ao_radio_init(void);

/*
 * ao_monitor.c
 */

extern const char const * const ao_state_names[];

#define AO_MONITOR_RING	8

union ao_monitor {
	struct ao_telemetry_raw_recv	raw;
	struct ao_telemetry_all_recv	all;
	struct ao_telemetry_orig_recv	orig;
	struct ao_telemetry_tiny_recv	tiny;
};

extern __xdata union ao_monitor ao_monitor_ring[AO_MONITOR_RING];

#define ao_monitor_ring_next(n)	(((n) + 1) & (AO_MONITOR_RING - 1))

extern __data uint8_t ao_monitoring;
extern __data uint8_t ao_monitor_head;

void
ao_monitor(void);

#define AO_MONITORING_OFF	0
#define AO_MONITORING_ORIG	1

void
ao_monitor_set(uint8_t monitoring);

void
ao_monitor_disable(void);

void
ao_monitor_enable(void);

void
ao_monitor_init(void) __reentrant;

/*
 * ao_stdio.c
 */

#define AO_READ_AGAIN	((char) -1)

struct ao_stdio {
	char	(*pollchar)(void);
	void	(*putchar)(char c) __reentrant;
	void	(*flush)(void);
	uint8_t	echo;
};

extern __xdata struct ao_stdio ao_stdios[];
extern __pdata int8_t ao_cur_stdio;
extern __pdata int8_t ao_num_stdios;

void
flush(void);

extern __xdata uint8_t ao_stdin_ready;

uint8_t
ao_echo(void);

int8_t
ao_add_stdio(char (*pollchar)(void),
	     void (*putchar)(char) __reentrant,
	     void (*flush)(void)) __reentrant;

/*
 * ao_ignite.c
 */

enum ao_igniter {
	ao_igniter_drogue = 0,
	ao_igniter_main = 1
};

void
ao_ignite(enum ao_igniter igniter);

enum ao_igniter_status {
	ao_igniter_unknown,	/* unknown status (ambiguous voltage) */
	ao_igniter_ready,	/* continuity detected */
	ao_igniter_active,	/* igniter firing */
	ao_igniter_open,	/* open circuit detected */
};

struct ao_ignition {
	uint8_t	request;
	uint8_t fired;
	uint8_t firing;
};

extern __xdata struct ao_ignition ao_ignition[2];

enum ao_igniter_status
ao_igniter_status(enum ao_igniter igniter);

extern __pdata uint8_t ao_igniter_present;

void
ao_ignite_set_pins(void);

void
ao_igniter_init(void);

/*
 * ao_config.c
 */

#define AO_CONFIG_MAJOR	1
#define AO_CONFIG_MINOR	11

#define AO_AES_LEN 16

struct ao_config {
	uint8_t		major;
	uint8_t		minor;
	uint16_t	main_deploy;
	int16_t		accel_plus_g;		/* changed for minor version 2 */
	uint8_t		_legacy_radio_channel;
	char		callsign[AO_MAX_CALLSIGN + 1];
	uint8_t		apogee_delay;		/* minor version 1 */
	int16_t		accel_minus_g;		/* minor version 2 */
	uint32_t	radio_cal;		/* minor version 3 */
	uint32_t	flight_log_max;		/* minor version 4 */
	uint8_t		ignite_mode;		/* minor version 5 */
	uint8_t		pad_orientation;	/* minor version 6 */
	uint32_t	radio_setting;		/* minor version 7 */
	uint8_t		radio_enable;		/* minor version 8 */
	uint8_t		aes_key[AO_AES_LEN];	/* minor version 9 */
	uint32_t	frequency;		/* minor version 10 */
	uint16_t	apogee_lockout;		/* minor version 11 */
};

#define AO_IGNITE_MODE_DUAL		0
#define AO_IGNITE_MODE_APOGEE		1
#define AO_IGNITE_MODE_MAIN		2

#define AO_PAD_ORIENTATION_ANTENNA_UP	0
#define AO_PAD_ORIENTATION_ANTENNA_DOWN	1

extern __xdata struct ao_config ao_config;

#define AO_CONFIG_MAX_SIZE	128

void
ao_config_get(void);

void
ao_config_put(void);

void
ao_config_set_radio(void);

void
ao_config_init(void);

/*
 * ao_rssi.c
 */

void
ao_rssi_set(int rssi_value);

void
ao_rssi_init(uint8_t rssi_led);

/*
 * ao_product.c
 *
 * values which need to be defined for
 * each instance of a product
 */

extern const char ao_version[];
extern const char ao_manufacturer[];
extern const char ao_product[];

/*
 * Fifos
 */

#define AO_FIFO_SIZE	32

struct ao_fifo {
	uint8_t	insert;
	uint8_t	remove;
	char	fifo[AO_FIFO_SIZE];
};

#define ao_fifo_insert(f,c) do { \
	(f).fifo[(f).insert] = (c); \
	(f).insert = ((f).insert + 1) & (AO_FIFO_SIZE-1); \
} while(0)

#define ao_fifo_remove(f,c) do {\
	c = (f).fifo[(f).remove]; \
	(f).remove = ((f).remove + 1) & (AO_FIFO_SIZE-1); \
} while(0)

#define ao_fifo_full(f)		((((f).insert + 1) & (AO_FIFO_SIZE-1)) == (f).remove)
#define ao_fifo_empty(f)	((f).insert == (f).remove)

#if PACKET_HAS_MASTER || PACKET_HAS_SLAVE
#include <ao_packet.h>
#endif

#if HAS_BTM
#include <ao_btm.h>
#endif

#if HAS_COMPANION
#include <ao_companion.h>
#endif

#if HAS_LCD
#include <ao_lcd.h>
#endif

#if HAS_AES
#include <ao_aes.h>
#endif

/* ao_launch.c */

struct ao_launch_command {
	uint16_t	tick;
	uint16_t	serial;
	uint8_t		cmd;
	uint8_t		channel;
	uint16_t	unused;
};

#define AO_LAUNCH_QUERY		1

struct ao_launch_query {
	uint16_t	tick;
	uint16_t	serial;
	uint8_t		channel;
	uint8_t		valid;
	uint8_t		arm_status;
	uint8_t		igniter_status;
};

#define AO_LAUNCH_ARM		2
#define AO_LAUNCH_FIRE		3

void
ao_launch_init(void);

/*
 * ao_log_single.c
 */

#define AO_LOG_TELESCIENCE_START	((uint8_t) 's')
#define AO_LOG_TELESCIENCE_DATA		((uint8_t) 'd')

#define AO_LOG_TELESCIENCE_NUM_ADC	12

struct ao_log_telescience {
	uint8_t		type;
	uint8_t		csum;
	uint16_t	tick;
	uint16_t	tm_tick;
	uint8_t		tm_state;
	uint8_t		unused;
	uint16_t	adc[AO_LOG_TELESCIENCE_NUM_ADC];
};

#define AO_LOG_SINGLE_SIZE		32

union ao_log_single {
	struct ao_log_telescience	telescience;
	union ao_telemetry_all		telemetry;
	uint8_t				bytes[AO_LOG_SINGLE_SIZE];
};

extern __xdata union ao_log_single	ao_log_single_write_data;
extern __xdata union ao_log_single	ao_log_single_read_data;

void
ao_log_single_extra_query(void);

void
ao_log_single_list(void);

void
ao_log_single_main(void);

uint8_t
ao_log_single_write(void);

uint8_t
ao_log_single_read(uint32_t pos);

void
ao_log_single_start(void);

void
ao_log_single_stop(void);

void
ao_log_single_restart(void);

void
ao_log_single_set(void);

void
ao_log_single_delete(void);

void
ao_log_single_init(void);

void
ao_log_single(void);

/*
 * ao_pyro_slave.c
 */

#define AO_TELEPYRO_NUM_ADC	9

#ifndef ao_xmemcpy
#define ao_xmemcpy(d,s,c) memcpy(d,s,c)
#define ao_xmemset(d,v,c) memset(d,v,c)
#define ao_xmemcmp(d,s,c) memcmp(d,s,c)
#endif

/*
 * ao_terraui.c
 */

void
ao_terraui_init(void);

/*
 * ao_battery.c
 */

#ifdef BATTERY_PIN
void
ao_battery_isr(void) ao_arch_interrupt(1);

uint16_t
ao_battery_get(void);

void
ao_battery_init(void);
#endif /* BATTERY_PIN */

/*
 * ao_sqrt.c
 */

uint32_t
ao_sqrt(uint32_t op);

/*
 * ao_freq.c
 */

int32_t ao_freq_to_set(int32_t freq, int32_t cal) __reentrant;

#include <ao_arch_funcs.h>

/*
 * ao_ms5607.c
 */

void ao_ms5607_init(void);

#endif /* _AO_H_ */
