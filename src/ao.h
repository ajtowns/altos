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
#include "cc1111.h"

#define TRUE 1
#define FALSE 0

/* Convert a __data pointer into an __xdata pointer */
#define DATA_TO_XDATA(a)	((void __xdata *) ((uint8_t) (a) | 0xff00))

/* Stack runs from above the allocated __data space to 0xfe, which avoids
 * writing to 0xff as that triggers the stack overflow indicator
 */
#define AO_STACK_START	0x80
#define AO_STACK_END	0xfe
#define AO_STACK_SIZE	(AO_STACK_END - AO_STACK_START + 1)

/* An AltOS task */
struct ao_task {
	__xdata void *wchan;		/* current wait channel (NULL if running) */
	uint8_t	stack_count;		/* amount of saved stack */
	uint8_t task_id;		/* index in the task array */
	__code char *name;		/* task name */
	uint8_t	stack[AO_STACK_SIZE];	/* saved stack */
};

extern __xdata struct ao_task *__data ao_cur_task;

#define AO_NUM_TASKS		16	/* maximum number of tasks */
#define AO_NO_TASK		0	/* no task id */

/*
 ao_task.c
 */

/* Suspend the current task until wchan is awoken */
void
ao_sleep(__xdata void *wchan);

/* Wake all tasks sleeping on wchan */
void
ao_wakeup(__xdata void *wchan);

/* Yield the processor to another task */
void
ao_yield(void) _naked;

/* Add a task to the run queue */
void
ao_add_task(__xdata struct ao_task * task, void (*start)(void), __code char *name) __reentrant;

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

/* Stop the operating system, beeping and blinking the reason */
void
ao_panic(uint8_t reason);

/*
 * ao_timer.c
 */

/* Our timer runs at 100Hz */
#define AO_MS_TO_TICKS(ms)	((ms) / 10)
#define AO_SEC_TO_TICKS(s)	((s) * 100)

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
ao_timer_isr(void) interrupt 9;

/* Initialize the timer */
void
ao_timer_init(void);

/* Initialize the hardware clock. Must be called first */
void
ao_clock_init(void);

/*
 * ao_adc.c
 */

#define AO_ADC_RING	64
#define ao_adc_ring_next(n)	(((n) + 1) & (AO_ADC_RING - 1))
#define ao_adc_ring_prev(n)	(((n) - 1) & (AO_ADC_RING - 1))

/*
 * One set of samples read from the A/D converter
 */
struct ao_adc {
	uint16_t	tick;		/* tick when the sample was read */
	int16_t		accel;		/* accelerometer */
	int16_t		pres;		/* pressure sensor */
	int16_t		temp;		/* temperature sensor */
	int16_t		v_batt;		/* battery voltage */
	int16_t		sense_d;	/* drogue continuity sense */
	int16_t		sense_m;	/* main continuity sense */
};

/*
 * A/D data is stored in a ring, with the next sample to be written
 * at ao_adc_head
 */
extern volatile __xdata struct ao_adc	ao_adc_ring[AO_ADC_RING];
extern volatile __data uint8_t		ao_adc_head;

/* Trigger a conversion sequence (called from the timer interrupt) */
void
ao_adc_poll(void);

/* Suspend the current task until another A/D sample is converted */
void
ao_adc_sleep(void);

/* Get a copy of the last complete A/D sample set */
void
ao_adc_get(__xdata struct ao_adc *packet);

/* The A/D interrupt handler */
#if !AO_NO_ADC_ISR
void
ao_adc_isr(void) interrupt 1;
#endif

/* Initialize the A/D converter */
void
ao_adc_init(void);

/*
 * ao_beep.c
 */

/*
 * Various pre-defined beep frequencies
 *
 * frequency = 1/2 (24e6/32) / beep
 */

#define AO_BEEP_LOW	150	/* 2500Hz */
#define AO_BEEP_MID	94	/* 3989Hz */
#define AO_BEEP_HIGH	75	/* 5000Hz */
#define AO_BEEP_OFF	0	/* off */

#define AO_BEEP_g	240	/* 1562.5Hz */
#define AO_BEEP_gs	227	/* 1652Hz (1655Hz) */
#define AO_BEEP_aa	214	/* 1752Hz (1754Hz) */
#define AO_BEEP_bbf	202	/* 1856Hz (1858Hz) */
#define AO_BEEP_bb	190	/* 1974Hz (1969Hz) */
#define AO_BEEP_cc	180	/* 2083Hz (2086Hz) */
#define AO_BEEP_ccs	170	/* 2205Hz (2210Hz) */
#define AO_BEEP_dd	160	/* 2344Hz (2341Hz) */
#define AO_BEEP_eef	151	/* 2483Hz (2480Hz) */
#define AO_BEEP_ee	143	/* 2622Hz (2628Hz) */
#define AO_BEEP_ff	135	/* 2778Hz (2784Hz) */
#define AO_BEEP_ffs	127	/* 2953Hz (2950Hz) */
#define AO_BEEP_gg	120	/* 3125Hz */
#define AO_BEEP_ggs	113	/* 3319Hz (3311Hz) */
#define AO_BEEP_aaa	107	/* 3504Hz (3508Hz) */
#define AO_BEEP_bbbf	101	/* 3713Hz (3716Hz) */
#define AO_BEEP_bbb	95	/* 3947Hz (3937Hz) */
#define AO_BEEP_ccc	90	/* 4167Hz (4171Hz) */
#define AO_BEEP_cccs	85	/* 4412Hz (4419Hz) */
#define AO_BEEP_ddd	80	/* 4688Hz (4682Hz) */
#define AO_BEEP_eeef	76	/* 4934Hz (4961Hz) */
#define AO_BEEP_eee	71	/* 5282Hz (5256Hz) */
#define AO_BEEP_fff	67	/* 5597Hz (5568Hz) */
#define AO_BEEP_fffs	64	/* 5859Hz (5899Hz) */
#define AO_BEEP_ggg	60	/* 6250Hz */

/* Set the beeper to the specified tone */
void
ao_beep(uint8_t beep);

/* Turn on the beeper for the specified time */
void
ao_beep_for(uint8_t beep, uint16_t ticks) __reentrant;

/* Initialize the beeper */
void
ao_beep_init(void);

/*
 * ao_led.c
 */

#define AO_LED_NONE	0
#define AO_LED_GREEN	1
#define AO_LED_RED	2

/* Turn on the specified LEDs */
void
ao_led_on(uint8_t colors);

/* Turn off the specified LEDs */
void
ao_led_off(uint8_t colors);

/* Set all of the LEDs to the specified state */
void
ao_led_set(uint8_t colors);

/* Toggle the specified LEDs */
void
ao_led_toggle(uint8_t colors);

/* Turn on the specified LEDs for the indicated interval */
void
ao_led_for(uint8_t colors, uint16_t ticks) __reentrant;

/* Initialize the LEDs */
void
ao_led_init(uint8_t enable);

/*
 * ao_usb.c
 */

/* Put one character to the USB output queue */
void
ao_usb_putchar(char c);

/* Get one character from the USB input queue */
char
ao_usb_getchar(void);

/* Flush the USB output queue */
void
ao_usb_flush(void);

/* USB interrupt handler */
void
ao_usb_isr(void) interrupt 6;

/* Enable the USB controller */
void
ao_usb_enable(void);

/* Disable the USB controller */
void
ao_usb_disable(void);

/* Initialize the USB system */
void
ao_usb_init(void);

/*
 * ao_cmd.c
 */

enum ao_cmd_status {
	ao_cmd_success = 0,
	ao_cmd_lex_error = 1,
	ao_cmd_syntax_error = 2,
};

extern __xdata uint16_t ao_cmd_lex_i;
extern __xdata char	ao_cmd_lex_c;
extern __xdata enum ao_cmd_status ao_cmd_status;

void
ao_cmd_lex(void);

void
ao_cmd_put8(uint8_t v);

void
ao_cmd_put16(uint16_t v);

void
ao_cmd_white(void);

void
ao_cmd_hex(void);

void
ao_cmd_decimal(void);

struct ao_cmds {
	char		cmd;
	void		(*func)(void);
	const char	*help;
};

void
ao_cmd_register(__code struct ao_cmds *cmds);

void
ao_cmd_init(void);

/*
 * ao_dma.c
 */

/* Allocate a DMA channel. the 'done' parameter will be set to 1
 * when the dma is finished and will be used to wakeup any waiters
 */
uint8_t
ao_dma_alloc(__xdata uint8_t * done);

/* Setup a DMA channel */
void
ao_dma_set_transfer(uint8_t id,
		    void __xdata *srcaddr,
		    void __xdata *dstaddr,
		    uint16_t count,
		    uint8_t cfg0,
		    uint8_t cfg1);

/* Start a DMA channel */
void
ao_dma_start(uint8_t id);

/* Manually trigger a DMA channel */
void
ao_dma_trigger(uint8_t id);

/* Abort a running DMA transfer */
void
ao_dma_abort(uint8_t id);

/* DMA interrupt routine */
void
ao_dma_isr(void) interrupt 8;

/*
 * ao_mutex.c
 */

void
ao_mutex_get(__xdata uint8_t *ao_mutex) __reentrant;

void
ao_mutex_put(__xdata uint8_t *ao_mutex) __reentrant;

/*
 * ao_ee.c
 */

/*
 * We reserve the last block on the device for
 * configuration space. Writes and reads in this
 * area return errors.
 */

#define AO_EE_BLOCK_SIZE	((uint16_t) (256))
#define AO_EE_DEVICE_SIZE	((uint32_t) 128 * (uint32_t) 1024)
#define AO_EE_DATA_SIZE		(AO_EE_DEVICE_SIZE - (uint32_t) AO_EE_BLOCK_SIZE)
#define AO_EE_CONFIG_BLOCK	((uint16_t) (AO_EE_DATA_SIZE / AO_EE_BLOCK_SIZE))

void
ao_ee_flush(void) __reentrant;

/* Write to the eeprom */
uint8_t
ao_ee_write(uint32_t pos, uint8_t *buf, uint16_t len) __reentrant;

/* Read from the eeprom */
uint8_t
ao_ee_read(uint32_t pos, uint8_t *buf, uint16_t len) __reentrant;

/* Write the config block (at the end of the eeprom) */
uint8_t
ao_ee_write_config(uint8_t *buf, uint16_t len) __reentrant;

/* Read the config block (at the end of the eeprom) */
uint8_t
ao_ee_read_config(uint8_t *buf, uint16_t len) __reentrant;

/* Initialize the EEPROM code */
void
ao_ee_init(void);

/*
 * ao_log.c
 */

/*
 * The data log is recorded in the eeprom as a sequence
 * of data packets.
 *
 * Each packet starts with a 4-byte header that has the
 * packet type, the packet checksum and the tick count. Then
 * they all contain 2 16 bit values which hold packet-specific
 * data.
 *
 * For each flight, the first packet
 * is FLIGHT packet, indicating the serial number of the
 * device and a unique number marking the number of flights
 * recorded by this device.
 *
 * During flight, data from the accelerometer and barometer
 * are recorded in SENSOR packets, using the raw 16-bit values
 * read from the A/D converter.
 *
 * Also during flight, but at a lower rate, the deployment
 * sensors are recorded in DEPLOY packets. The goal here is to
 * detect failure in the deployment circuits.
 *
 * STATE packets hold state transitions as the flight computer
 * transitions through different stages of the flight.
 */
#define AO_LOG_FLIGHT		'F'
#define AO_LOG_SENSOR		'A'
#define AO_LOG_TEMP_VOLT	'T'
#define AO_LOG_DEPLOY		'D'
#define AO_LOG_STATE		'S'
#define AO_LOG_GPS_TIME		'G'
#define AO_LOG_GPS_LAT		'N'
#define AO_LOG_GPS_LON		'W'
#define AO_LOG_GPS_ALT		'H'
#define AO_LOG_GPS_SAT		'V'

#define AO_LOG_POS_NONE		(~0UL)

struct ao_log_record {
	char			type;
	uint8_t			csum;
	uint16_t		tick;
	union {
		struct {
			int16_t		ground_accel;
			uint16_t	flight;
		} flight;
		struct {
			int16_t		accel;
			int16_t		pres;
		} sensor;
		struct {
			int16_t		temp;
			int16_t		v_batt;
		} temp_volt;
		struct {
			int16_t		drogue;
			int16_t		main;
		} deploy;
		struct {
			uint16_t	state;
			uint16_t	reason;
		} state;
		struct {
			uint8_t		hour;
			uint8_t		minute;
			uint8_t		second;
			uint8_t		flags;
		} gps_time;
		int32_t		gps_latitude;
		int32_t		gps_longitude;
		struct {
			int16_t		altitude;
			uint16_t	unused;
		} gps_altitude;
		struct {
			uint16_t	svid;
			uint8_t		state;
			uint8_t		c_n;
			uint8_t		unused;
		} gps_sat;
		struct {
			uint16_t	d0;
			uint16_t	d1;
		} anon;
	} u;
};

/* Write a record to the eeprom log */
void
ao_log_data(struct ao_log_record *log);

/* Flush the log */
void
ao_log_flush(void);

/* Log dumping API:
 * ao_log_dump_first() - get first log record
 * ao_log_dump_next()  - get next log record
 */
extern __xdata struct ao_log_record ao_log_dump;

/* Retrieve first log record for the current flight */
uint8_t
ao_log_dump_first(void);

/* return next log record for the current flight */
uint8_t
ao_log_dump_next(void);

/* Logging thread main routine */
void
ao_log(void);

/* Start logging to eeprom */
void
ao_log_start(void);

/* Stop logging */
void
ao_log_stop(void);

/* Initialize the logging system */
void
ao_log_init(void);

/*
 * ao_flight.c
 */

enum ao_flight_state {
	ao_flight_startup = 0,
	ao_flight_idle = 1,
	ao_flight_pad = 2,
	ao_flight_boost = 3,
	ao_flight_fast = 4,
	ao_flight_coast = 5,
	ao_flight_drogue = 6,
	ao_flight_main = 7,
	ao_flight_landed = 8,
	ao_flight_invalid = 9
};

extern __xdata struct ao_adc		ao_flight_data;
extern __pdata enum ao_flight_state	ao_flight_state;
extern __pdata uint16_t			ao_flight_tick;
extern __pdata int16_t			ao_flight_accel;
extern __pdata int16_t			ao_flight_pres;
extern __pdata int32_t			ao_flight_vel;
extern __pdata int16_t			ao_ground_pres;
extern __pdata int16_t			ao_ground_accel;
extern __pdata int16_t			ao_min_pres;
extern __pdata uint16_t			ao_launch_time;

/* Flight thread */
void
ao_flight(void);

/* Initialize flight thread */
void
ao_flight_init(void);

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
 * ao_dbg.c
 *
 * debug another telemetrum board
 */

/* Send a byte to the dbg target */
void
ao_dbg_send_byte(uint8_t byte);

/* Receive a byte from the dbg target */
uint8_t
ao_dbg_recv_byte(void);

/* Start a bulk transfer to/from dbg target memory */
void
ao_dbg_start_transfer(uint16_t addr);

/* End a bulk transfer to/from dbg target memory */
void
ao_dbg_end_transfer(void);

/* Write a byte to dbg target memory */
void
ao_dbg_write_byte(uint8_t byte);

/* Read a byte from dbg target memory */
uint8_t
ao_dbg_read_byte(void);

/* Enable dbg mode, switching use of the pins */
void
ao_dbg_debug_mode(void);

/* Reset the dbg target */
void
ao_dbg_reset(void);

void
ao_dbg_init(void);

/*
 * ao_serial.c
 */

#if !AO_NO_SERIAL_ISR
void
ao_serial_rx1_isr(void) interrupt 3;

void
ao_serial_tx1_isr(void) interrupt 14;
#endif

char
ao_serial_getchar(void) __critical;

void
ao_serial_putchar(char c) __critical;

#define AO_SERIAL_SPEED_4800	0
#define AO_SERIAL_SPEED_57600	1

void
ao_serial_set_speed(uint8_t speed);

void
ao_serial_init(void);

/*
 * ao_gps.c
 */

#define AO_GPS_NUM_SAT_MASK	(0xf << 0)
#define AO_GPS_NUM_SAT_SHIFT	(0)

#define AO_GPS_VALID		(1 << 4)
#define AO_GPS_RUNNING		(1 << 5)

struct ao_gps_data {
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

#define SIRF_SAT_STATE_ACQUIRED			(1 << 0)
#define SIRF_SAT_STATE_CARRIER_PHASE_VALID	(1 << 1)
#define SIRF_SAT_BIT_SYNC_COMPLETE		(1 << 2)
#define SIRF_SAT_SUBFRAME_SYNC_COMPLETE		(1 << 3)
#define SIRF_SAT_CARRIER_PULLIN_COMPLETE	(1 << 4)
#define SIRF_SAT_CODE_LOCKED			(1 << 5)
#define SIRF_SAT_ACQUISITION_FAILED		(1 << 6)
#define SIRF_SAT_EPHEMERIS_AVAILABLE		(1 << 7)

struct ao_gps_sat_data {
	uint8_t		svid;
	uint8_t		state;
	uint8_t		c_n_1;
};

struct ao_gps_tracking_data {
	uint8_t			channels;
	struct ao_gps_sat_data	sats[12];
};

extern __xdata uint8_t ao_gps_mutex;
extern __xdata struct ao_gps_data ao_gps_data;
extern __xdata struct ao_gps_tracking_data ao_gps_tracking_data;

void
ao_gps(void);

void
ao_gps_print(__xdata struct ao_gps_data *gps_data);

void
ao_gps_tracking_print(__xdata struct ao_gps_tracking_data *gps_tracking_data);

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
 * ao_telemetry.c
 */

#define AO_MAX_CALLSIGN		8

struct ao_telemetry {
	uint8_t			addr;
	uint8_t			flight_state;
	int16_t			flight_accel;
	int16_t			ground_accel;
	int32_t			flight_vel;
	int16_t			flight_pres;
	int16_t			ground_pres;
	struct ao_adc		adc;
	struct ao_gps_data	gps;
	char			callsign[AO_MAX_CALLSIGN];
	struct ao_gps_tracking_data	gps_tracking;
};

/* Set delay between telemetry reports (0 to disable) */

#define AO_TELEMETRY_INTERVAL_PAD	AO_MS_TO_TICKS(1000)
#define AO_TELEMETRY_INTERVAL_FLIGHT	AO_MS_TO_TICKS(50)
#define AO_TELEMETRY_INTERVAL_RECOVER	AO_MS_TO_TICKS(1000)

void
ao_telemetry_set_interval(uint16_t interval);

void
ao_telemetry_init(void);

/*
 * ao_radio.c
 */

void
ao_radio_send(__xdata struct ao_telemetry *telemetry) __reentrant;

struct ao_radio_recv {
	struct ao_telemetry	telemetry;
	int8_t			rssi;
	uint8_t			status;
};

void
ao_radio_recv(__xdata struct ao_radio_recv *recv) __reentrant;

void
ao_radio_init(void);

/*
 * ao_monitor.c
 */

extern const char const * const ao_state_names[];

void
ao_monitor(void);

void
ao_set_monitor(uint8_t monitoring);

void
ao_monitor_init(uint8_t led, uint8_t monitoring) __reentrant;

/*
 * ao_stdio.c
 */

void
flush(void);

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

enum ao_igniter_status
ao_igniter_status(enum ao_igniter igniter);

void
ao_igniter_init(void);

/*
 * ao_config.c
 */

#define AO_CONFIG_MAJOR	1
#define AO_CONFIG_MINOR	0

struct ao_config {
	uint8_t		major;
	uint8_t		minor;
	uint16_t	main_deploy;
	int16_t		accel_zero_g;
	uint8_t		radio_channel;
	char		callsign[AO_MAX_CALLSIGN + 1];
};

extern __xdata struct ao_config ao_config;

void
ao_config_get(void);

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

extern const uint8_t ao_usb_descriptors [];
extern const uint16_t ao_serial_number;
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

/*
 * ao_packet.c
 *
 * Packet-based command interface
 */

#define AO_PACKET_MAX	32
#define AO_PACKET_WIN	256

#define AO_PACKET_FIN	(1 << 0)
#define AO_PACKET_SYN	(1 << 1)
#define AO_PACKET_RST	(1 << 2)
#define AO_PACKET_ACK	(1 << 3)

struct ao_packet {
	uint8_t		addr;
	uint8_t		flags;
	uint16_t	seq;
	uint16_t	ack;
	uint16_t	window;
	uint8_t		len;
	uint8_t		d[AO_PACKET_MAX];
};

uint8_t
ao_packet_connect(uint8_t dest);

uint8_t
ao_packet_accept(void);

int
ao_packet_send(uint8_t *data, int len);

int
ao_packet_recv(uint8_t *data, int len);

void
ao_packet_init(void);

#endif /* _AO_H_ */
