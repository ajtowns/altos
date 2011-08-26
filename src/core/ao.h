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
#include "ao_pins.h"

#define TRUE 1
#define FALSE 0

/* Convert a __data pointer into an __xdata pointer */
#define DATA_TO_XDATA(a)	((void __xdata *) ((uint8_t) (a) | 0xff00))

/* Stack runs from above the allocated __data space to 0xfe, which avoids
 * writing to 0xff as that triggers the stack overflow indicator
 */
#define AO_STACK_START	0x90
#define AO_STACK_END	0xfe
#define AO_STACK_SIZE	(AO_STACK_END - AO_STACK_START + 1)

/* An AltOS task */
struct ao_task {
	__xdata void *wchan;		/* current wait channel (NULL if running) */
	uint16_t alarm;			/* abort ao_sleep time */
	uint8_t	stack_count;		/* amount of saved stack */
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

/* Yield the processor to another task */
void
ao_yield(void) __naked;

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
ao_timer_isr(void) __interrupt 9;

/* Initialize the timer */
void
ao_timer_init(void);

/* Initialize the hardware clock. Must be called first */
void
ao_clock_init(void);

/*
 * One set of samples read from the A/D converter or telemetry
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

#ifndef HAS_ADC
#error Please define HAS_ADC
#endif

#if HAS_ADC

#if HAS_ACCEL
#ifndef HAS_ACCEL_REF
#error Please define HAS_ACCEL_REF
#endif
#else
#define HAS_ACCEL_REF 0
#endif

/*
 * ao_adc.c
 */

#define AO_ADC_RING	32
#define ao_adc_ring_next(n)	(((n) + 1) & (AO_ADC_RING - 1))
#define ao_adc_ring_prev(n)	(((n) - 1) & (AO_ADC_RING - 1))


/*
 * A/D data is stored in a ring, with the next sample to be written
 * at ao_adc_head
 */
extern volatile __xdata struct ao_adc	ao_adc_ring[AO_ADC_RING];
extern volatile __data uint8_t		ao_adc_head;
#if HAS_ACCEL_REF
extern volatile __xdata uint16_t	ao_accel_ref[AO_ADC_RING];
#endif

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

void
ao_adc_isr(void) __interrupt 1;

/* Initialize the A/D converter */
void
ao_adc_init(void);

#endif /* HAS_ADC */

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
 * ao_romconfig.c
 */

#define AO_ROMCONFIG_VERSION	2

extern __code __at (0x00a0) uint16_t ao_romconfig_version;
extern __code __at (0x00a2) uint16_t ao_romconfig_check;
extern __code __at (0x00a4) uint16_t ao_serial_number;
extern __code __at (0x00a6) uint32_t ao_radio_cal;

#ifndef HAS_USB
#error Please define HAS_USB
#endif

#if HAS_USB
extern __code __at (0x00aa) uint8_t ao_usb_descriptors [];
#endif

/*
 * ao_usb.c
 */

/* Put one character to the USB output queue */
void
ao_usb_putchar(char c);

/* Get one character from the USB input queue */
char
ao_usb_getchar(void);

/* Poll for a charcter on the USB input queue.
 * returns AO_READ_AGAIN if none are available
 */
char
ao_usb_pollchar(void);

/* Flush the USB output queue */
void
ao_usb_flush(void);

#if HAS_USB
/* USB interrupt handler */
void
ao_usb_isr(void) __interrupt 6;
#endif

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

void
ao_cmd_white(void);

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
ao_cmd_register(__code struct ao_cmds *cmds);

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
 * ao_dma.c
 */

/* Allocate a DMA channel. the 'done' parameter will be set when the
 * dma is finished and will be used to wakeup any waiters
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
ao_dma_isr(void) __interrupt 8;

/*
 * ao_mutex.c
 */

void
ao_mutex_get(__xdata uint8_t *ao_mutex) __reentrant;

void
ao_mutex_put(__xdata uint8_t *ao_mutex) __reentrant;

/*
 * Storage interface, provided by one of the eeprom or flash
 * drivers
 */

/* Total bytes of available storage */
extern __pdata uint32_t	ao_storage_total;

/* Block size - device is erased in these units. At least 256 bytes */
extern __pdata uint32_t	ao_storage_block;

/* Byte offset of config block. Will be ao_storage_block bytes long */
extern __pdata uint32_t	ao_storage_config;

/* Storage unit size - device reads and writes must be within blocks of this size. Usually 256 bytes. */
extern __pdata uint16_t ao_storage_unit;

#define AO_STORAGE_ERASE_LOG	(ao_storage_config + AO_CONFIG_MAX_SIZE)

/* Initialize above values. Can only be called once the OS is running */
void
ao_storage_setup(void) __reentrant;

/* Write data. Returns 0 on failure, 1 on success */
uint8_t
ao_storage_write(uint32_t pos, __xdata void *buf, uint16_t len) __reentrant;

/* Read data. Returns 0 on failure, 1 on success */
uint8_t
ao_storage_read(uint32_t pos, __xdata void *buf, uint16_t len) __reentrant;

/* Erase a block of storage. This always clears ao_storage_block bytes */
uint8_t
ao_storage_erase(uint32_t pos) __reentrant;

/* Flush any pending writes to stable storage */
void
ao_storage_flush(void) __reentrant;

/* Initialize the storage code */
void
ao_storage_init(void);

/*
 * Low-level functions wrapped by ao_storage.c
 */

/* Read data within a storage unit */
uint8_t
ao_storage_device_read(uint32_t pos, __xdata void *buf, uint16_t len) __reentrant;

/* Write data within a storage unit */
uint8_t
ao_storage_device_write(uint32_t pos, __xdata void *buf, uint16_t len) __reentrant;

/* Initialize low-level device bits */
void
ao_storage_device_init(void);

/* Print out information about flash chips */
void
ao_storage_device_info(void) __reentrant;

/*
 * ao_log.c
 */

/* We record flight numbers in the first record of
 * the log. Tasks may wait for this to be initialized
 * by sleeping on this variable.
 */
extern __xdata uint16_t ao_flight_number;

extern __pdata uint32_t ao_log_current_pos;
extern __pdata uint32_t ao_log_end_pos;
extern __pdata uint32_t ao_log_start_pos;
extern __xdata uint8_t	ao_log_running;
extern __pdata enum flight_state ao_log_state;

/* required functions from the underlying log system */

#define AO_LOG_FORMAT_UNKNOWN		0	/* unknown; altosui will have to guess */
#define AO_LOG_FORMAT_FULL		1	/* 8 byte typed log records */
#define AO_LOG_FORMAT_TINY		2	/* two byte state/baro records */
#define AO_LOG_FORMAT_TELEMETRY		3	/* 32 byte ao_telemetry records */
#define AO_LOG_FORMAT_TELESCIENCE	4	/* 32 byte typed telescience records */
#define AO_LOG_FORMAT_NONE		127	/* No log at all */

extern __code uint8_t ao_log_format;

/* Return the flight number from the given log slot, 0 if none */
uint16_t
ao_log_flight(uint8_t slot);

/* Flush the log */
void
ao_log_flush(void);

/* Logging thread main routine */
void
ao_log(void);

/* functions provided in ao_log.c */

/* Figure out the current flight number */
void
ao_log_scan(void) __reentrant;

/* Return the position of the start of the given log slot */
uint32_t
ao_log_pos(uint8_t slot);

/* Start logging to eeprom */
void
ao_log_start(void);

/* Stop logging */
void
ao_log_stop(void);

/* Initialize the logging system */
void
ao_log_init(void);

/* Write out the current flight number to the erase log */
void
ao_log_write_erase(uint8_t pos);

/* Returns true if there are any logs stored in eeprom */
uint8_t
ao_log_present(void);

/* Returns true if there is no more storage space available */
uint8_t
ao_log_full(void);

/*
 * ao_log_big.c
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
#define AO_LOG_GPS_DATE		'Y'

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
			uint8_t		unused;
			uint8_t		c_n;
		} gps_sat;
		struct {
			uint8_t		year;
			uint8_t		month;
			uint8_t		day;
			uint8_t		extra;
		} gps_date;
		struct {
			uint16_t	d0;
			uint16_t	d1;
		} anon;
	} u;
};

/* Write a record to the eeprom log */
uint8_t
ao_log_data(__xdata struct ao_log_record *log) __reentrant;

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

extern __pdata enum ao_flight_state	ao_flight_state;

extern __pdata uint16_t			ao_launch_time;
extern __pdata uint8_t			ao_flight_force_idle;

/* Flight thread */
void
ao_flight(void);

/* Initialize flight thread */
void
ao_flight_init(void);

/*
 * ao_flight_nano.c
 */

void
ao_flight_nano_init(void);

/*
 * ao_sample.c
 */

/*
 * Barometer calibration
 *
 * We directly sample the barometer. The specs say:
 *
 * Pressure range: 15-115 kPa
 * Voltage at 115kPa: 2.82
 * Output scale: 27mV/kPa
 *
 * If we want to detect launch with the barometer, we need
 * a large enough bump to not be fooled by noise. At typical
 * launch elevations (0-2000m), a 200Pa pressure change cooresponds
 * to about a 20m elevation change. This is 5.4mV, or about 3LSB.
 * As all of our calculations are done in 16 bits, we'll actually see a change
 * of 16 times this though
 *
 * 27 mV/kPa * 32767 / 3300 counts/mV = 268.1 counts/kPa
 */

/* Accelerometer calibration
 *
 * We're sampling the accelerometer through a resistor divider which
 * consists of 5k and 10k resistors. This multiplies the values by 2/3.
 * That goes into the cc1111 A/D converter, which is running at 11 bits
 * of precision with the bits in the MSB of the 16 bit value. Only positive
 * values are used, so values should range from 0-32752 for 0-3.3V. The
 * specs say we should see 40mV/g (uncalibrated), multiply by 2/3 for what
 * the A/D converter sees (26.67 mV/g). We should see 32752/3300 counts/mV,
 * for a final computation of:
 *
 * 26.67 mV/g * 32767/3300 counts/mV = 264.8 counts/g
 *
 * Zero g was measured at 16000 (we would expect 16384).
 * Note that this value is only require to tell if the
 * rocket is standing upright. Once that is determined,
 * the value of the accelerometer is averaged for 100 samples
 * to find the resting accelerometer value, which is used
 * for all further flight computations
 */

#define GRAVITY 9.80665

/*
 * Above this height, the baro sensor doesn't work
 */
#define AO_MAX_BARO_HEIGHT	12000

/*
 * Above this speed, baro measurements are unreliable
 */
#define AO_MAX_BARO_SPEED	200

#define ACCEL_NOSE_UP	(ao_accel_2g >> 2)

/*
 * Speed and acceleration are scaled by 16 to provide a bit more
 * resolution while still having reasonable range. Note that this
 * limits speed to 2047m/s (around mach 6) and acceleration to
 * 2047m/s² (over 200g)
 */

#define AO_M_TO_HEIGHT(m)	((int16_t) (m))
#define AO_MS_TO_SPEED(ms)	((int16_t) ((ms) * 16))
#define AO_MSS_TO_ACCEL(mss)	((int16_t) ((mss) * 16))

extern __pdata uint16_t	ao_sample_tick;		/* time of last data */
extern __pdata int16_t	ao_sample_pres;		/* most recent pressure sensor reading */
extern __pdata int16_t	ao_sample_alt;		/* MSL of ao_sample_pres */
extern __pdata int16_t	ao_sample_height;	/* AGL of ao_sample_pres */
extern __data uint8_t	ao_sample_adc;		/* Ring position of last processed sample */

#if HAS_ACCEL
extern __pdata int16_t	ao_sample_accel;	/* most recent accel sensor reading */
#endif

extern __pdata int16_t	ao_ground_pres;		/* startup pressure */
extern __pdata int16_t	ao_ground_height;	/* MSL of ao_ground_pres */

#if HAS_ACCEL
extern __pdata int16_t	ao_ground_accel;	/* startup acceleration */
extern __pdata int16_t 	ao_accel_2g;		/* factory accel calibration */
extern __pdata int32_t	ao_accel_scale;		/* sensor to m/s² conversion */
#endif

void ao_sample_init(void);

/* returns FALSE in preflight mode, TRUE in flight mode */
uint8_t ao_sample(void);

/*
 * ao_kalman.c
 */

#define to_fix16(x) ((int16_t) ((x) * 65536.0 + 0.5))
#define to_fix32(x) ((int32_t) ((x) * 65536.0 + 0.5))
#define from_fix(x)	((x) >> 16)

extern __pdata int16_t			ao_height;	/* meters */
extern __pdata int16_t			ao_speed;	/* m/s * 16 */
extern __pdata int16_t			ao_accel;	/* m/s² * 16 */
extern __pdata int16_t			ao_max_height;	/* max of ao_height */
extern __pdata int16_t			ao_avg_height;	/* running average of height */

extern __pdata int16_t			ao_error_h;
extern __pdata int16_t			ao_error_h_sq_avg;

#if HAS_ACCEL
extern __pdata int16_t			ao_error_a;
#endif

void ao_kalman(void);

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

#ifndef HAS_SERIAL_1
#error Please define HAS_SERIAL_1
#endif

#if HAS_SERIAL_1
#ifndef USE_SERIAL_STDIN
#error Please define USE_SERIAL_STDIN
#endif

void
ao_serial_rx1_isr(void) __interrupt 3;

void
ao_serial_tx1_isr(void) __interrupt 14;

char
ao_serial_getchar(void) __critical;

#if USE_SERIAL_STDIN
char
ao_serial_pollchar(void) __critical;

void
ao_serial_set_stdin(uint8_t stdin);
#endif

void
ao_serial_putchar(char c) __critical;

void
ao_serial_drain(void) __critical;

#define AO_SERIAL_SPEED_4800	0
#define AO_SERIAL_SPEED_9600	1
#define AO_SERIAL_SPEED_19200	2
#define AO_SERIAL_SPEED_57600	3

void
ao_serial_set_speed(uint8_t speed);

void
ao_serial_init(void);
#endif

/*
 * ao_spi.c
 */

extern __xdata uint8_t	ao_spi_mutex;

#define ao_spi_get_mask(reg,mask) do {\
	ao_mutex_get(&ao_spi_mutex); \
	(reg) &= ~(mask); \
	} while (0)

#define ao_spi_put_mask(reg,mask) do { \
	(reg) |= (mask); \
	ao_mutex_put(&ao_spi_mutex); \
	} while (0)

#define ao_spi_get_bit(bit) do {\
	ao_mutex_get(&ao_spi_mutex); \
	(bit) = 0; \
	} while (0)

#define ao_spi_put_bit(bit) do { \
	(bit) = 1; \
	ao_mutex_put(&ao_spi_mutex); \
	} while (0)

/*
 * The SPI mutex must be held to call either of these
 * functions -- this mutex covers the entire SPI operation,
 * from chip select low to chip select high
 */

void
ao_spi_send(void __xdata *block, uint16_t len) __reentrant;

void
ao_spi_recv(void __xdata *block, uint16_t len) __reentrant;

void
ao_spi_init(void);

/*
 * ao_telemetry.c
 */
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
	
union ao_telemetry_all {
	struct ao_telemetry_generic		generic;
	struct ao_telemetry_sensor		sensor;
	struct ao_telemetry_configuration	configuration;
	struct ao_telemetry_location		location;
	struct ao_telemetry_satellite		satellite;
	struct ao_telemetry_companion		companion;
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

/*
 * ao_radio_recv tacks on rssi and status bytes
 */

struct ao_telemetry_raw_recv {
	uint8_t			packet[AO_MAX_TELEMETRY + 2];
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

/* Set delay between telemetry reports (0 to disable) */

#define AO_TELEMETRY_INTERVAL_PAD	AO_MS_TO_TICKS(1000)
#define AO_TELEMETRY_INTERVAL_FLIGHT	AO_MS_TO_TICKS(100)
#define AO_TELEMETRY_INTERVAL_RECOVER	AO_MS_TO_TICKS(1000)

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
ao_radio_general_isr(void) __interrupt 16;

void
ao_radio_get(uint8_t len);

#define ao_radio_put() ao_mutex_put(&ao_radio_mutex)

void
ao_radio_set_packet(void);

void
ao_radio_send(__xdata void *data, uint8_t size) __reentrant;

uint8_t
ao_radio_recv(__xdata void *data, uint8_t size) __reentrant;

void
ao_radio_recv_abort(void);

void
ao_radio_rdf(int ms);

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

void
ao_monitor(void);

#define AO_MONITORING_OFF	0
#define AO_MONITORING_ORIG	1
#define AO_MONITORING_TINY	2

void
ao_set_monitor(uint8_t monitoring);

void
ao_monitor_init(uint8_t led, uint8_t monitoring) __reentrant;

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

enum ao_igniter_status
ao_igniter_status(enum ao_igniter igniter);

void
ao_ignite_set_pins(void);

void
ao_igniter_init(void);

/*
 * ao_config.c
 */

#define AO_CONFIG_MAJOR	1
#define AO_CONFIG_MINOR	8

struct ao_config {
	uint8_t		major;
	uint8_t		minor;
	uint16_t	main_deploy;
	int16_t		accel_plus_g;		/* changed for minor version 2 */
	uint8_t		radio_channel;
	char		callsign[AO_MAX_CALLSIGN + 1];
	uint8_t		apogee_delay;		/* minor version 1 */
	int16_t		accel_minus_g;		/* minor version 2 */
	uint32_t	radio_cal;		/* minor version 3 */
	uint32_t	flight_log_max;		/* minor version 4 */
	uint8_t		ignite_mode;		/* minor version 5 */
	uint8_t		pad_orientation;	/* minor version 6 */
	uint32_t	radio_setting;		/* minor version 7 */
	uint8_t		radio_enable;		/* minor version 8 */
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

/*
 * ao_packet.c
 *
 * Packet-based command interface
 */

#define AO_PACKET_MAX		64
#define AO_PACKET_SYN		(uint8_t) 0xff

struct ao_packet {
	uint8_t		addr;
	uint8_t		len;
	uint8_t		seq;
	uint8_t		ack;
	uint8_t		d[AO_PACKET_MAX];
	uint8_t		callsign[AO_MAX_CALLSIGN];
};

struct ao_packet_recv {
	struct ao_packet	packet;
	int8_t			rssi;
	uint8_t			status;
};

extern __xdata struct ao_packet_recv ao_rx_packet;
extern __xdata struct ao_packet ao_tx_packet;
extern __xdata struct ao_task	ao_packet_task;
extern __xdata uint8_t ao_packet_enable;
extern __xdata uint8_t ao_packet_master_sleeping;
extern __pdata uint8_t ao_packet_rx_len, ao_packet_rx_used, ao_packet_tx_used;

void
ao_packet_send(void);

uint8_t
ao_packet_recv(void);

void
ao_packet_flush(void);

void
ao_packet_putchar(char c) __reentrant;

char
ao_packet_pollchar(void) __critical;

/* ao_packet_master.c */

void
ao_packet_master_init(void);

/* ao_packet_slave.c */

void
ao_packet_slave_start(void);

void
ao_packet_slave_stop(void);

void
ao_packet_slave_init(uint8_t enable);

/* ao_btm.c */

/* If bt_link is on P2, this interrupt is shared by USB, so the USB
 * code calls this function. Otherwise, it's a regular ISR.
 */

void
ao_btm_isr(void)
#if BT_LINK_ON_P1
	__interrupt 15
#endif
	;

void
ao_btm_init(void);

/* ao_companion.c */

#define AO_COMPANION_SETUP		1
#define AO_COMPANION_FETCH		2
#define AO_COMPANION_NOTIFY		3

struct ao_companion_command {
	uint8_t		command;
	uint8_t		flight_state;
	uint16_t	tick;
	uint16_t	serial;
	uint16_t	flight;
};

struct ao_companion_setup {
	uint16_t	board_id;
	uint16_t	board_id_inverse;
	uint8_t		update_period;
	uint8_t		channels;
};

extern __pdata uint8_t				ao_companion_running;
extern __xdata struct ao_companion_setup	ao_companion_setup;
extern __xdata uint8_t				ao_companion_mutex;
extern __xdata uint16_t				ao_companion_data[AO_COMPANION_MAX_CHANNELS];

void
ao_companion_init(void);

#endif /* _AO_H_ */
