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
#include <ao_pins.h>
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

#ifndef HAS_TASK
#define HAS_TASK	1
#endif

#ifndef AO_PORT_TYPE
#define AO_PORT_TYPE uint8_t
#endif

typedef AO_PORT_TYPE ao_port_t;

#if HAS_TASK
#include <ao_task.h>
#else
#include <ao_notask.h>
#endif

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
#define AO_PANIC_CRASH		14	/* Processor crashed */
#define AO_PANIC_BUFIO		15	/* Mis-using bufio API */
#define AO_PANIC_EXTI		16	/* Mis-using exti API */
#define AO_PANIC_FAST_TIMER	17	/* Mis-using fast timer API */
#define AO_PANIC_SELF_TEST_CC1120	0x40 | 1	/* Self test failure */
#define AO_PANIC_SELF_TEST_HMC5883	0x40 | 2	/* Self test failure */
#define AO_PANIC_SELF_TEST_MPU6000	0x40 | 3	/* Self test failure */
#define AO_PANIC_SELF_TEST_MS5607	0x40 | 4	/* Self test failure */

/* Stop the operating system, beeping and blinking the reason */
void
ao_panic(uint8_t reason);

/*
 * ao_timer.c
 */

#ifndef AO_TICK_TYPE
#define AO_TICK_TYPE	uint16_t
#define AO_TICK_SIGNED	int16_t
#endif

extern volatile __data AO_TICK_TYPE ao_tick_count;

/* Our timer runs at 100Hz */
#ifndef AO_HERTZ
#define AO_HERTZ		100
#endif
#define AO_MS_TO_TICKS(ms)	((ms) / (1000 / AO_HERTZ))
#define AO_SEC_TO_TICKS(s)	((s) * AO_HERTZ)

/* Returns the current time in ticks */
AO_TICK_TYPE
ao_time(void);

/* Suspend the current task until ticks time has passed */
void
ao_delay(uint16_t ticks);

/* Set the ADC interval */
void
ao_timer_set_adc_interval(uint8_t interval);

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

#ifndef ao_mutex_get
void
ao_mutex_get(__xdata uint8_t *ao_mutex) __reentrant;

void
ao_mutex_put(__xdata uint8_t *ao_mutex) __reentrant;
#endif

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
ao_put_string(__code char *s);

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
ao_cmd_decimal(void) __reentrant;

/* Read a single hex nibble off stdin. */
uint8_t
ao_getnibble(void);

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

#define AO_RDF_INTERVAL_TICKS	AO_SEC_TO_TICKS(5)
#define AO_RDF_LENGTH_MS	500
#define AO_RDF_CONTINUITY_MS	32
#define AO_RDF_CONTINUITY_PAUSE	96
#define AO_RDF_CONTINUITY_TOTAL	((AO_RDF_CONTINUITY_PAUSE + AO_RDF_CONTINUITY_MS) * 3 + AO_RDF_CONTINUITY_PAUSE)

/* This assumes that we're generating a 1kHz tone, which
 * modulates the carrier at 2kbps, or 250kBps
 */
#define AO_MS_TO_RDF_LEN(ms) ((ms) / 4)

#define AO_RADIO_RDF_LEN	AO_MS_TO_RDF_LEN(AO_RDF_LENGTH_MS)
#define AO_RADIO_CONT_TONE_LEN	AO_MS_TO_RDF_LEN(AO_RDF_CONTINUITY_MS)
#define AO_RADIO_CONT_PAUSE_LEN	AO_MS_TO_RDF_LEN(AO_RDF_CONTINUITY_PAUSE)
#define AO_RADIO_CONT_TOTAL_LEN	AO_MS_TO_RDF_LEN(AO_RDF_CONTINUITY_TOTAL)

/* returns a value 0-3 to indicate igniter continuity */
uint8_t
ao_report_igniter(void);

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

#include <ao_data.h>

alt_t
ao_pa_to_altitude(int32_t pa);

int32_t
ao_altitude_to_pa(alt_t alt);

#if HAS_DBG
#include <ao_dbg.h>
#endif

#if HAS_SERIAL_0 || HAS_SERIAL_1 || HAS_SERIAL_2 || HAS_SERIAL_3
#include <ao_serial.h>
#endif

/*
 * ao_convert_volt.c
 *
 * Convert ADC readings to decivolts
 */

int16_t
ao_battery_decivolt(int16_t adc);

int16_t
ao_ignite_decivolt(int16_t adc);

/*
 * ao_spi_slave.c
 */

uint8_t
ao_spi_slave_recv(void *buf, uint16_t len);

void
ao_spi_slave_send(void *buf, uint16_t len);

void
ao_spi_slave_init(void);

/* This must be defined by the product; it will get called when chip
 * select goes low, at which point it should use ao_spi_read and
 * ao_spi_write to deal with the request
 */

void
ao_spi_slave(void);

#include <ao_telemetry.h>
/*
 * ao_gps.c
 */

#define AO_GPS_NUM_SAT_MASK	(0xf << 0)
#define AO_GPS_NUM_SAT_SHIFT	(0)

#define AO_GPS_VALID		(1 << 4)
#define AO_GPS_RUNNING		(1 << 5)
#define AO_GPS_DATE_VALID	(1 << 6)
#define AO_GPS_COURSE_VALID	(1 << 7)

#define AO_GPS_NEW_DATA		1
#define AO_GPS_NEW_TRACKING	2

extern __xdata uint8_t ao_gps_new;
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
ao_gps_set_rate(uint8_t rate);

void
ao_gps(void);

void
ao_gps_print(__xdata struct ao_gps_orig *gps_data);

void
ao_gps_tracking_print(__xdata struct ao_gps_tracking_orig *gps_tracking_data);

void
ao_gps_show(void) __reentrant;

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
 * ao_gps_report_mega.c
 */

void
ao_gps_report_mega(void);

void
ao_gps_report_mega_init(void);

/*
 * ao_telemetry_orig.c
 */

#if LEGACY_MONITOR
struct ao_adc_orig {
	uint16_t	tick;		/* tick when the sample was read */
	int16_t		accel;		/* accelerometer */
	int16_t		pres;		/* pressure sensor */
	int16_t		temp;		/* temperature sensor */
	int16_t		v_batt;		/* battery voltage */
	int16_t		sense_d;	/* drogue continuity sense */
	int16_t		sense_m;	/* main continuity sense */
};

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
	struct ao_adc_orig	adc;
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

#endif /* LEGACY_MONITOR */

/* Unfortunately, we've exposed the CC1111 rssi units as the 'usual' method
 * for reporting RSSI. So, now we use these values everywhere
 */
#define AO_RSSI_FROM_RADIO(radio)	((int16_t) ((int8_t) (radio) >> 1) - 74)
#define AO_RADIO_FROM_RSSI(rssi)	(((int8_t) (rssi) + 74) << 1)

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

extern __xdata int8_t	ao_radio_rssi;

#ifdef PKT_APPEND_STATUS_1_CRC_OK
#define AO_RADIO_STATUS_CRC_OK	PKT_APPEND_STATUS_1_CRC_OK
#else
#include <ao_fec.h>
#define AO_RADIO_STATUS_CRC_OK	AO_FEC_DECODE_CRC_OK
#endif

#ifndef HAS_RADIO_RECV
#define HAS_RADIO_RECV HAS_RADIO
#endif
#ifndef HAS_RADIO_XMIT
#define HAS_RADIO_XMIT HAS_RADIO
#endif

void
ao_radio_general_isr(void) ao_arch_interrupt(16);

#if HAS_RADIO_XMIT
void
ao_radio_send(const __xdata void *d, uint8_t size) __reentrant;
#endif

#if HAS_RADIO_RECV
uint8_t
ao_radio_recv(__xdata void *d, uint8_t size, uint8_t timeout) __reentrant;

void
ao_radio_recv_abort(void);
#endif

void
ao_radio_test(uint8_t on);

typedef int16_t (*ao_radio_fill_func)(uint8_t *buffer, int16_t len);

void
ao_radio_send_aprs(ao_radio_fill_func fill);

/*
 * ao_radio_pa
 */

#if HAS_RADIO_AMP
void
ao_radio_pa_on(void);

void
ao_radio_pa_off(void);

void
ao_radio_pa_init(void);
#else
#define ao_radio_pa_on()
#define ao_radio_pa_off()
#define ao_radio_pa_init()
#endif

/*
 * Compute the packet length as follows:
 *
 * 2000 bps (for a 1kHz tone)
 * so, for 'ms' milliseconds, we need
 * 2 * ms bits, or ms / 4 bytes
 */

void
ao_radio_rdf(void);

void
ao_radio_continuity(uint8_t c);

void
ao_radio_rdf_abort(void);

void
ao_radio_init(void);

/*
 * ao_monitor.c
 */

#if HAS_MONITOR

extern const char const * const ao_state_names[];

#define AO_MONITOR_RING	8

union ao_monitor {
	struct ao_telemetry_raw_recv	raw;
	struct ao_telemetry_all_recv	all;
#if LEGACY_MONITOR
	struct ao_telemetry_orig_recv	orig;
	struct ao_telemetry_tiny_recv	tiny;
#endif
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

#endif

/*
 * ao_stdio.c
 */

#define AO_READ_AGAIN	(-1)

struct ao_stdio {
	int	(*_pollchar)(void);	/* Called with interrupts blocked */
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
ao_add_stdio(int (*pollchar)(void),
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

extern __code char * __code ao_igniter_status_names[];

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
#include <ao_config.h>

#if AO_PYRO_NUM
#include <ao_pyro.h>
#endif

#if HAS_FORCE_FREQ
/*
 * Set this to force the frequency to 434.550MHz
 */
extern __xdata uint8_t ao_force_freq;
#endif

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

/*
 * ao_ms5607.c
 */

void ao_ms5607_init(void);

#include <ao_arch_funcs.h>

#endif /* _AO_H_ */
