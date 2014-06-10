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

#include "ao.h"
#include "ao_log.h"
#include <ao_config.h>
#if HAS_FLIGHT
#include <ao_sample.h>
#include <ao_data.h>
#endif
#if HAS_BEEP
#include <ao_beep.h>
#endif
#if HAS_TRACKER
#include <ao_tracker.h>
#endif

__xdata struct ao_config ao_config;
__pdata uint8_t ao_config_loaded;
__pdata uint8_t ao_config_dirty;
__xdata uint8_t ao_config_mutex;

#ifndef AO_CONFIG_DEFAULT_APRS_INTERVAL
#define AO_CONFIG_DEFAULT_APRS_INTERVAL	0
#endif
#define AO_CONFIG_DEFAULT_MAIN_DEPLOY	250
#define AO_CONFIG_DEFAULT_RADIO_CHANNEL	0
#define AO_CONFIG_DEFAULT_CALLSIGN	"N0CALL"
#define AO_CONFIG_DEFAULT_ACCEL_ZERO_G	16000
#define AO_CONFIG_DEFAULT_APOGEE_DELAY	0
#define AO_CONFIG_DEFAULT_IGNITE_MODE	AO_IGNITE_MODE_DUAL
#define AO_CONFIG_DEFAULT_PAD_ORIENTATION	AO_PAD_ORIENTATION_ANTENNA_UP
#define AO_CONFIG_DEFAULT_PYRO_TIME	AO_MS_TO_TICKS(50)
#if HAS_EEPROM
#ifndef USE_INTERNAL_FLASH
#error Please define USE_INTERNAL_FLASH
#endif
#endif
#ifndef AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX
#if USE_INTERNAL_FLASH
#define AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX	ao_storage_config
#else
#define AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX	((uint32_t) 192 * (uint32_t) 1024)
#endif
#endif
#ifndef AO_CONFIG_DEFAULT_RADIO_POWER
#define AO_CONFIG_DEFAULT_RADIO_POWER		0x60
#endif
#define AO_CONFIG_DEFAULT_RADIO_AMP		0

#if HAS_EEPROM
static void
_ao_config_put(void)
{
	ao_config_setup();
	ao_config_erase();
	ao_config_write(0, &ao_config, sizeof (ao_config));
#if HAS_FLIGHT
	ao_log_write_erase(0);
#endif
	ao_config_flush();
}

void
ao_config_put(void)
{
	ao_mutex_get(&ao_config_mutex);
	_ao_config_put();
	ao_mutex_put(&ao_config_mutex);
}
#endif

#if HAS_RADIO
void
ao_config_set_radio(void)
{
	ao_config.radio_setting = ao_freq_to_set(ao_config.frequency, ao_config.radio_cal);
}
#endif /* HAS_RADIO */

static void
_ao_config_get(void)
{
	uint8_t	minor;

	if (ao_config_loaded)
		return;
#if HAS_EEPROM
	/* Yes, I know ao_storage_read calls ao_storage_setup,
	 * but ao_storage_setup *also* sets ao_storage_config, which we
	 * need before calling ao_storage_read here
	 */
	ao_config_setup();
	ao_config_read(0, &ao_config, sizeof (ao_config));
#endif
	if (ao_config.major != AO_CONFIG_MAJOR) {
		ao_config.major = AO_CONFIG_MAJOR;
		ao_config.minor = 0;

		/* Version 0 stuff */
		ao_config.main_deploy = AO_CONFIG_DEFAULT_MAIN_DEPLOY;
		ao_xmemset(&ao_config.callsign, '\0', sizeof (ao_config.callsign));
		ao_xmemcpy(&ao_config.callsign, CODE_TO_XDATA(AO_CONFIG_DEFAULT_CALLSIGN),
		       sizeof(AO_CONFIG_DEFAULT_CALLSIGN) - 1);
		ao_config._legacy_radio_channel = 0;
	}
	minor = ao_config.minor;
	if (minor != AO_CONFIG_MINOR) {
		/* Fixups for minor version 1 */
		if (minor < 1)
			ao_config.apogee_delay = AO_CONFIG_DEFAULT_APOGEE_DELAY;
		/* Fixups for minor version 2 */
		if (minor < 2) {
			ao_config.accel_plus_g = 0;
			ao_config.accel_minus_g = 0;
		}
		/* Fixups for minor version 3 */
#if HAS_RADIO
		if (minor < 3)
			ao_config.radio_cal = ao_radio_cal;
#endif
		/* Fixups for minor version 4 */
#if HAS_LOG
		if (minor < 4)
			ao_config.flight_log_max = AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX;
#endif
		/* Fixupes for minor version 5 */
		if (minor < 5)
			ao_config.ignite_mode = AO_CONFIG_DEFAULT_IGNITE_MODE;
		if (minor < 6)
			ao_config.pad_orientation = AO_CONFIG_DEFAULT_PAD_ORIENTATION;
		if (minor < 8)
			ao_config.radio_enable = AO_RADIO_ENABLE_CORE;
		if (minor < 9)
			ao_xmemset(&ao_config.aes_key, '\0', AO_AES_LEN);
		if (minor < 10)
			ao_config.frequency = 434550 + ao_config._legacy_radio_channel * 100;
		if (minor < 11)
			ao_config.apogee_lockout = 0;
#if AO_PYRO_NUM
		if (minor < 12)
			memset(&ao_config.pyro, '\0', sizeof (ao_config.pyro));
#endif
		if (minor < 13)
			ao_config.aprs_interval = AO_CONFIG_DEFAULT_APRS_INTERVAL;
#if HAS_RADIO_POWER
		if (minor < 14)
			ao_config.radio_power = AO_CONFIG_DEFAULT_RADIO_POWER;
		#endif
#if HAS_RADIO_AMP
		if (minor  < 14)
			ao_config.radio_amp = AO_CONFIG_DEFAULT_RADIO_AMP;
#endif
#if HAS_GYRO
		if (minor < 15) {
			ao_config.accel_zero_along = 0;
			ao_config.accel_zero_across = 0;
			ao_config.accel_zero_through = 0;

			/* Reset the main accel offsets to force
			 * re-calibration
			 */
			ao_config.accel_plus_g = 0;
			ao_config.accel_minus_g = 0;
		}
#endif
#if HAS_BEEP_CONFIG
		if (minor < 16)
			ao_config.mid_beep = AO_BEEP_MID_DEFAULT;
#endif
#if HAS_TRACKER
		if (minor < 17) {
			ao_config.tracker_motion = AO_TRACKER_MOTION_DEFAULT;
			ao_config.tracker_interval = AO_TRACKER_INTERVAL_DEFAULT;
		}
#endif
#if AO_PYRO_NUM
		if (minor < 18)
			ao_config.pyro_time = AO_CONFIG_DEFAULT_PYRO_TIME;
#endif
		ao_config.minor = AO_CONFIG_MINOR;
		ao_config_dirty = 1;
	}
#if HAS_RADIO
#if HAS_FORCE_FREQ
	if (ao_force_freq) {
		ao_config.frequency = 434550;
		ao_config.radio_cal = ao_radio_cal;
		ao_xmemcpy(&ao_config.callsign, CODE_TO_XDATA(AO_CONFIG_DEFAULT_CALLSIGN),
		       sizeof(AO_CONFIG_DEFAULT_CALLSIGN) - 1);
	}
#endif
	ao_config_set_radio();
#endif
	ao_config_loaded = 1;
}

void
_ao_config_edit_start(void)
{
	ao_mutex_get(&ao_config_mutex);
	_ao_config_get();
}

void
_ao_config_edit_finish(void)
{
	ao_config_dirty = 1;
	ao_mutex_put(&ao_config_mutex);
}

void
ao_config_get(void)
{
	_ao_config_edit_start();
	ao_mutex_put(&ao_config_mutex);
}

void
ao_config_callsign_show(void)
{
	printf ("Callsign: \"%s\"\n", ao_config.callsign);
}

void
ao_config_callsign_set(void) __reentrant
{
	uint8_t	c;
	static __xdata char callsign[AO_MAX_CALLSIGN + 1];

	ao_xmemset(callsign, '\0', sizeof callsign);
	ao_cmd_white();
	c = 0;
	while (ao_cmd_lex_c != '\n') {
		if (c < AO_MAX_CALLSIGN)
			callsign[c++] = ao_cmd_lex_c;
		else
			ao_cmd_status = ao_cmd_lex_error;
		ao_cmd_lex();
	}
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_xmemcpy(&ao_config.callsign, &callsign,
	       AO_MAX_CALLSIGN + 1);
	_ao_config_edit_finish();
}

#if HAS_RADIO

void
ao_config_frequency_show(void) __reentrant
{
	printf("Frequency: %ld\n",
	       ao_config.frequency);
}

void
ao_config_frequency_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.frequency = ao_cmd_lex_u32;
	ao_config_set_radio();
	_ao_config_edit_finish();
#if HAS_RADIO_RECV
	ao_radio_recv_abort();
#endif
}
#endif

#if HAS_FLIGHT

void
ao_config_main_deploy_show(void) __reentrant
{
	printf("Main deploy: %d meters\n",
	       ao_config.main_deploy);
}

void
ao_config_main_deploy_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.main_deploy = ao_cmd_lex_i;
	_ao_config_edit_finish();
}

#if HAS_ACCEL
void
ao_config_accel_calibrate_show(void) __reentrant
{
	printf("Accel cal +1g: %d -1g: %d\n",
	       ao_config.accel_plus_g, ao_config.accel_minus_g);
#if HAS_GYRO
	printf ("IMU cal along %d across %d through %d\n",
		ao_config.accel_zero_along,
		ao_config.accel_zero_across,
		ao_config.accel_zero_through);
#endif
}

#define ACCEL_CALIBRATE_SAMPLES	1024
#define ACCEL_CALIBRATE_SHIFT	10

#if HAS_GYRO
static int16_t accel_cal_along;
static int16_t accel_cal_across;
static int16_t accel_cal_through;
#endif

static int16_t
ao_config_accel_calibrate_auto(char *orientation) __reentrant
{
	uint16_t	i;
	int32_t		accel_total;
	uint8_t		cal_data_ring;
#if HAS_GYRO
	int32_t		accel_along_total = 0;
	int32_t		accel_across_total = 0;
	int32_t		accel_through_total = 0;
#endif

	printf("Orient antenna %s and press a key...", orientation);
	flush();
	(void) getchar();
	puts("\r\n"); flush();
	puts("Calibrating..."); flush();
	i = ACCEL_CALIBRATE_SAMPLES;
	accel_total = 0;
	cal_data_ring = ao_sample_data;
	while (i) {
		ao_sleep(DATA_TO_XDATA(&ao_sample_data));
		while (i && cal_data_ring != ao_sample_data) {
			accel_total += (int32_t) ao_data_accel(&ao_data_ring[cal_data_ring]);
#if HAS_GYRO
			accel_along_total += (int32_t) ao_data_along(&ao_data_ring[cal_data_ring]);
			accel_across_total += (int32_t) ao_data_across(&ao_data_ring[cal_data_ring]);
			accel_through_total += (int32_t) ao_data_through(&ao_data_ring[cal_data_ring]);
#endif
			cal_data_ring = ao_data_ring_next(cal_data_ring);
			i--;
		}
	}
#if HAS_GYRO
	accel_cal_along = accel_along_total >> ACCEL_CALIBRATE_SHIFT;
	accel_cal_across = accel_across_total >> ACCEL_CALIBRATE_SHIFT;
	accel_cal_through = accel_through_total >> ACCEL_CALIBRATE_SHIFT;
#endif
	return accel_total >> ACCEL_CALIBRATE_SHIFT;
}

void
ao_config_accel_calibrate_set(void) __reentrant
{
	int16_t	up, down;
#if HAS_GYRO
	int16_t	accel_along_up = 0, accel_along_down = 0;
	int16_t	accel_across_up = 0, accel_across_down = 0;
	int16_t	accel_through_up = 0, accel_through_down = 0;
#endif

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ao_cmd_lex_i == 0) {
		up = ao_config_accel_calibrate_auto("up");
#if HAS_GYRO
		accel_along_up = accel_cal_along;
		accel_across_up = accel_cal_across;
		accel_through_up = accel_cal_through;
#endif
		down = ao_config_accel_calibrate_auto("down");
#if HAS_GYRO
		accel_along_down = accel_cal_along;
		accel_across_down = accel_cal_across;
		accel_through_down = accel_cal_through;
#endif
	} else {
		up = ao_cmd_lex_i;
		ao_cmd_decimal();
		if (ao_cmd_status != ao_cmd_success)
			return;
		down = ao_cmd_lex_i;
	}
	if (up >= down) {
		printf("Invalid accel: up (%d) down (%d)\n",
		       up, down);
		return;
	}
	_ao_config_edit_start();
	ao_config.accel_plus_g = up;
	ao_config.accel_minus_g = down;
#if HAS_GYRO
	if (ao_cmd_lex_i == 0) {
		ao_config.accel_zero_along = (accel_along_up + accel_along_down) / 2;
		ao_config.accel_zero_across = (accel_across_up + accel_across_down) / 2;
		ao_config.accel_zero_through = (accel_through_up + accel_through_down) / 2;
	}
#endif
	_ao_config_edit_finish();
}
#endif /* HAS_ACCEL */

void
ao_config_apogee_delay_show(void) __reentrant
{
	printf("Apogee delay: %d seconds\n",
	       ao_config.apogee_delay);
}

void
ao_config_apogee_delay_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.apogee_delay = ao_cmd_lex_i;
	_ao_config_edit_finish();
}

void
ao_config_apogee_lockout_show(void) __reentrant
{
	printf ("Apogee lockout: %d seconds\n",
		ao_config.apogee_lockout);
}

void
ao_config_apogee_lockout_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.apogee_lockout = ao_cmd_lex_i;
	_ao_config_edit_finish();
}

#endif /* HAS_FLIGHT */

#if HAS_RADIO
void
ao_config_radio_cal_show(void) __reentrant
{
	printf("Radio cal: %ld\n", ao_config.radio_cal);
}

void
ao_config_radio_cal_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.radio_cal = ao_cmd_lex_u32;
	ao_config_set_radio();
	_ao_config_edit_finish();
}
#endif

#if HAS_LOG
void
ao_config_log_show(void) __reentrant
{
	printf("Max flight log: %d kB\n", (int16_t) (ao_config.flight_log_max >> 10));
}

void
ao_config_log_set(void) __reentrant
{
	uint16_t	block = (uint16_t) (ao_storage_block >> 10);
	uint16_t	log_max = (uint16_t) (ao_storage_log_max >> 10);

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ao_log_present())
		printf("Storage must be empty before changing log size\n");
	else if (block > 1024 && (ao_cmd_lex_i & (block - 1)))
		printf("Flight log size must be multiple of %d kB\n", block);
	else if (ao_cmd_lex_i > log_max)
		printf("Flight log max %d kB\n", log_max);
	else {
		_ao_config_edit_start();
		ao_config.flight_log_max = (uint32_t) ao_cmd_lex_i << 10;
		_ao_config_edit_finish();
	}
}
#endif /* HAS_LOG */

#if HAS_IGNITE
void
ao_config_ignite_mode_show(void) __reentrant
{
	printf("Ignite mode: %d\n", ao_config.ignite_mode);
}

void
ao_config_ignite_mode_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.ignite_mode = ao_cmd_lex_i;
	_ao_config_edit_finish();
}
#endif

#if HAS_ACCEL
void
ao_config_pad_orientation_show(void) __reentrant
{
	printf("Pad orientation: %d\n", ao_config.pad_orientation);
}

#ifndef AO_ACCEL_INVERT
#define AO_ACCEL_INVERT	0x7fff
#endif

void
ao_config_pad_orientation_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_cmd_lex_i &= 1;
	if (ao_config.pad_orientation != ao_cmd_lex_i) {
		int16_t t;
		t = ao_config.accel_plus_g;
		ao_config.accel_plus_g = AO_ACCEL_INVERT - ao_config.accel_minus_g;
		ao_config.accel_minus_g = AO_ACCEL_INVERT - t;
	}
	ao_config.pad_orientation = ao_cmd_lex_i;
	_ao_config_edit_finish();
}
#endif

#if HAS_RADIO
void
ao_config_radio_enable_show(void) __reentrant
{
	printf("Radio enable: %d\n", ao_config.radio_enable);
}

void
ao_config_radio_enable_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.radio_enable = ao_cmd_lex_i;
	_ao_config_edit_finish();
}
#endif /* HAS_RADIO */

#if HAS_AES

__xdata uint8_t	ao_config_aes_seq = 1;

void
ao_config_key_show(void) __reentrant
{
	uint8_t	i;
	printf("AES key: ");
	for (i = 0; i < AO_AES_LEN; i++)
		printf ("%02x", ao_config.aes_key[i]);
	printf("\n");
}

void
ao_config_key_set(void) __reentrant
{
	uint8_t i;

	_ao_config_edit_start();
	for (i = 0; i < AO_AES_LEN; i++) {
		ao_cmd_hexbyte();
		if (ao_cmd_status != ao_cmd_success)
			break;
		ao_config.aes_key[i] = ao_cmd_lex_i;
	}
	++ao_config_aes_seq;
	_ao_config_edit_finish();
}
#endif

#if HAS_APRS

void
ao_config_aprs_show(void)
{
	printf ("APRS interval: %d\n", ao_config.aprs_interval);
}

void
ao_config_aprs_set(void)
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.aprs_interval = ao_cmd_lex_i;
	_ao_config_edit_finish();
}

#endif /* HAS_APRS */

#if HAS_RADIO_AMP

void
ao_config_radio_amp_show(void)
{
	printf ("Radio amp setting: %d\n", ao_config.radio_amp);
}

void
ao_config_radio_amp_set(void)
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.radio_amp = ao_cmd_lex_i;
	_ao_config_edit_finish();
}

#endif

#if HAS_RADIO_POWER

void
ao_config_radio_power_show(void)
{
	printf ("Radio power setting: %d\n", ao_config.radio_power);
}

void
ao_config_radio_power_set(void)
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.radio_power = ao_cmd_lex_i;
	_ao_config_edit_finish();
}

#endif

#if HAS_BEEP_CONFIG
void
ao_config_beep_show(void)
{
	printf ("Beeper setting: %d\n", ao_config.mid_beep);
}

void
ao_config_beep_set(void)
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.mid_beep = ao_cmd_lex_i;
	_ao_config_edit_finish();
}
#endif

#if HAS_TRACKER
void
ao_config_tracker_show(void)
{
	printf ("Tracker setting: %d %d\n",
		ao_config.tracker_motion,
		ao_config.tracker_interval);
}

void
ao_config_tracker_set(void)
{
	uint16_t	m, i;
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	m = ao_cmd_lex_i;
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	i = ao_cmd_lex_i;
	_ao_config_edit_start();
	ao_config.tracker_motion = m;
	ao_config.tracker_interval = i;
	_ao_config_edit_finish();
}
#endif /* HAS_TRACKER */

#if AO_PYRO_NUM
void
ao_config_pyro_time_show(void)
{
	printf ("Pyro time: %d\n", ao_config.pyro_time);
}

void
ao_config_pyro_time_set(void)
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	_ao_config_edit_start();
	ao_config.pyro_time = ao_cmd_lex_i;
	_ao_config_edit_finish();
}
#endif

struct ao_config_var {
	__code char	*str;
	void		(*set)(void) __reentrant;
	void		(*show)(void) __reentrant;
};

static void
ao_config_help(void) __reentrant;

static void
ao_config_show(void) __reentrant;

#if HAS_EEPROM
static void
ao_config_save(void) __reentrant;
#endif

__code struct ao_config_var ao_config_vars[] = {
#if HAS_FLIGHT
	{ "m <meters>\0Main deploy (m)",
	  ao_config_main_deploy_set,	ao_config_main_deploy_show, },
	{ "d <delay>\0Apogee delay (s)",
	  ao_config_apogee_delay_set,	ao_config_apogee_delay_show },
	{ "L <seconds>\0Apogee detect lockout (s)",
	  ao_config_apogee_lockout_set, ao_config_apogee_lockout_show, },
#endif /* HAS_FLIGHT */
#if HAS_RADIO
	{ "F <freq>\0Frequency (kHz)",
	  ao_config_frequency_set, ao_config_frequency_show },
	{ "c <call>\0Callsign (8 char max)",
	  ao_config_callsign_set,	ao_config_callsign_show },
	{ "e <0 disable, 1 enable>\0Enable telemetry and RDF",
	  ao_config_radio_enable_set, ao_config_radio_enable_show },
	{ "f <cal>\0Radio calib (cal = rf/(xtal/2^16))",
	  ao_config_radio_cal_set,  	ao_config_radio_cal_show },
#if HAS_RADIO_POWER
	{ "p <setting>\0Radio power setting (0-255)",
	  ao_config_radio_power_set,	ao_config_radio_power_show },
#endif
#if HAS_RADIO_AMP
	{ "d <setting>\0Radio amplifier setting (0-3)",
	  ao_config_radio_amp_set,	ao_config_radio_amp_show },
#endif
#endif /* HAS_RADIO */
#if HAS_ACCEL
	{ "a <+g> <-g>\0Accel calib (0 for auto)",
	  ao_config_accel_calibrate_set,ao_config_accel_calibrate_show },
	{ "o <0 antenna up, 1 antenna down>\0Pad orientation",
	  ao_config_pad_orientation_set,ao_config_pad_orientation_show },
#endif /* HAS_ACCEL */
#if HAS_LOG
	{ "l <size>\0Flight log size (kB)",
	  ao_config_log_set,		ao_config_log_show },
#endif
#if HAS_IGNITE
	{ "i <0 dual, 1 apogee, 2 main>\0Igniter mode",
	  ao_config_ignite_mode_set,	ao_config_ignite_mode_show },
#endif
#if HAS_AES
	{ "k <32 hex digits>\0AES encryption key",
	  ao_config_key_set, ao_config_key_show },
#endif
#if AO_PYRO_NUM
	{ "P <n,?>\0Pyro channels",
	  ao_pyro_set, ao_pyro_show },
	{ "I <ticks>\0Pyro firing time",
	  ao_config_pyro_time_set, ao_config_pyro_time_show },
#endif
#if HAS_APRS
	{ "A <secs>\0APRS packet interval (0 disable)",
	  ao_config_aprs_set, ao_config_aprs_show },
#endif
#if HAS_BEEP_CONFIG
	{ "b <val>\0Beeper tone (freq = 1/2 (24e6/32) / beep",
	  ao_config_beep_set, ao_config_beep_show },
#endif
#if HAS_TRACKER
	{ "t <motion> <interval>\0Tracker configuration",
	  ao_config_tracker_set, ao_config_tracker_show },
#endif
	{ "s\0Show",
	  ao_config_show,		0 },
#if HAS_EEPROM
	{ "w\0Write to eeprom",
	  ao_config_save,		0 },
#endif
	{ "?\0Help",
	  ao_config_help,		0 },
	{ 0, 0, 0 }
};

void
ao_config_set(void)
{
	char	c;
	uint8_t cmd;

	ao_cmd_white();
	c = ao_cmd_lex_c;
	ao_cmd_lex();
	for (cmd = 0; ao_config_vars[cmd].str != NULL; cmd++)
		if (ao_config_vars[cmd].str[0] == c) {
			(*ao_config_vars[cmd].set)();
			return;
		}
	ao_cmd_status = ao_cmd_syntax_error;
}

static void
ao_config_help(void) __reentrant
{
	uint8_t cmd;
	for (cmd = 0; ao_config_vars[cmd].str != NULL; cmd++)
		printf("%-20s %s\n",
		       ao_config_vars[cmd].str,
		       ao_config_vars[cmd].str+1+
		       strlen(ao_config_vars[cmd].str));
}

static void
ao_config_show(void) __reentrant
{
	uint8_t cmd;
	ao_config_get();
	printf("Config version: %d.%d\n",
	       ao_config.major, ao_config.minor);
	for (cmd = 0; ao_config_vars[cmd].str != NULL; cmd++)
		if (ao_config_vars[cmd].show)
			(*ao_config_vars[cmd].show)();
#if HAS_MS5607
	ao_ms5607_info();
#endif
}

#if HAS_EEPROM
static void
ao_config_save(void) __reentrant
{
	uint8_t saved = 0;
	ao_mutex_get(&ao_config_mutex);
	if (ao_config_dirty) {
		_ao_config_put();
		ao_config_dirty = 0;
		saved = 1;
	}
	ao_mutex_put(&ao_config_mutex);
	if (saved)
		puts("Saved");
	else
		puts("Nothing to save");
}
#endif

__code struct ao_cmds ao_config_cmds[] = {
	{ ao_config_set,	"c <var> <value>\0Set config (? for help, s to show)" },
	{ 0, NULL },
};

void
ao_config_init(void)
{
	ao_cmd_register(&ao_config_cmds[0]);
}
