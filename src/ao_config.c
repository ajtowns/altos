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

__xdata struct ao_config ao_config;
__xdata uint8_t ao_config_loaded;
__xdata uint8_t ao_config_dirty;
__xdata uint8_t ao_config_mutex;

#define AO_CONFIG_DEFAULT_MAIN_DEPLOY	250
#define AO_CONFIG_DEFAULT_RADIO_CHANNEL	0
#define AO_CONFIG_DEFAULT_CALLSIGN	"N0CALL"
#define AO_CONFIG_DEFAULT_ACCEL_ZERO_G	16000
#define AO_CONFIG_DEFAULT_APOGEE_DELAY	0
#define AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX	((uint32_t) 192 * (uint32_t) 1024)

#if HAS_EEPROM
static void
_ao_config_put(void)
{
	ao_storage_setup();
	ao_storage_erase(ao_storage_config);
	ao_storage_write(ao_storage_config, &ao_config, sizeof (ao_config));
	ao_log_write_erase(0);
	ao_storage_flush();
}

void
ao_config_put(void)
{
	ao_mutex_get(&ao_config_mutex);
	_ao_config_put();
	ao_mutex_put(&ao_config_mutex);
}
#endif

static void
_ao_config_get(void)
{
	if (ao_config_loaded)
		return;
#if HAS_EEPROM
	ao_storage_setup();
	ao_storage_read(ao_storage_config, &ao_config, sizeof (ao_config));
#endif
	if (ao_config.major != AO_CONFIG_MAJOR) {
		ao_config.major = AO_CONFIG_MAJOR;
		ao_config.minor = AO_CONFIG_MINOR;
		ao_config.main_deploy = AO_CONFIG_DEFAULT_MAIN_DEPLOY;
		ao_config.radio_channel = AO_CONFIG_DEFAULT_RADIO_CHANNEL;
		ao_config.accel_plus_g = 0;
		ao_config.accel_minus_g = 0;
		memset(&ao_config.callsign, '\0', sizeof (ao_config.callsign));
		memcpy(&ao_config.callsign, AO_CONFIG_DEFAULT_CALLSIGN,
		       sizeof(AO_CONFIG_DEFAULT_CALLSIGN) - 1);
		ao_config.apogee_delay = AO_CONFIG_DEFAULT_APOGEE_DELAY;
		ao_config.radio_cal = ao_radio_cal;
		ao_config_dirty = 1;
	}
	if (ao_config.minor < AO_CONFIG_MINOR) {
		/* Fixups for minor version 1 */
		if (ao_config.minor < 1)
			ao_config.apogee_delay = AO_CONFIG_DEFAULT_APOGEE_DELAY;
		/* Fixups for minor version 2 */
		if (ao_config.minor < 2) {
			ao_config.accel_plus_g = 0;
			ao_config.accel_minus_g = 0;
		}
		/* Fixups for minor version 3 */
		if (ao_config.minor < 3)
			ao_config.radio_cal = ao_radio_cal;
		/* Fixups for minor version 4 */
		if (ao_config.minor < 4)
			ao_config.flight_log_max = AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX;
		ao_config.minor = AO_CONFIG_MINOR;
		ao_config_dirty = 1;
	}
	ao_config_loaded = 1;
}

void
ao_config_get(void)
{
	ao_mutex_get(&ao_config_mutex);
	_ao_config_get();
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
	char callsign[AO_MAX_CALLSIGN + 1];

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
	ao_mutex_get(&ao_config_mutex);
	_ao_config_get();
	while (c < AO_MAX_CALLSIGN + 1)
		callsign[c++] = '\0';
	memcpy(&ao_config.callsign, &callsign,
	       AO_MAX_CALLSIGN + 1);
	ao_config_dirty = 1;
	ao_mutex_put(&ao_config_mutex);
	ao_config_callsign_show();
}

void
ao_config_radio_channel_show(void) __reentrant
{
	uint32_t	freq = 434550L + ao_config.radio_channel * 100L;
	uint16_t	mhz = freq / 1000L;
	uint16_t	khz = freq % 1000L;

	printf("Radio channel: %d (%d.%03dMHz)\n",
	       ao_config.radio_channel, mhz, khz);
}

void
ao_config_radio_channel_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_mutex_get(&ao_config_mutex);
	_ao_config_get();
	ao_config.radio_channel = ao_cmd_lex_i;
	ao_config_dirty = 1;
	ao_mutex_put(&ao_config_mutex);
	ao_config_radio_channel_show();
	ao_radio_recv_abort();
}

#if HAS_ADC

void
ao_config_main_deploy_show(void) __reentrant
{
	printf("Main deploy: %d meters (%d feet)\n",
	       ao_config.main_deploy,
	       (int16_t) ((int32_t) ao_config.main_deploy * 328 / 100));
}

void
ao_config_main_deploy_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_mutex_get(&ao_config_mutex);
	_ao_config_get();
	ao_config.main_deploy = ao_cmd_lex_i;
	ao_config_dirty = 1;
	ao_mutex_put(&ao_config_mutex);
	ao_config_main_deploy_show();
}

void
ao_config_accel_calibrate_show(void) __reentrant
{
	printf("Accel cal +1g: %d -1g: %d\n",
	       ao_config.accel_plus_g, ao_config.accel_minus_g);
}

#define ACCEL_CALIBRATE_SAMPLES	1024
#define ACCEL_CALIBRATE_SHIFT	10

static int16_t
ao_config_accel_calibrate_auto(char *orientation) __reentrant
{
	uint16_t	i;
	int32_t		accel_total;
	uint8_t		cal_adc_ring;

	printf("Orient %s and press a key...", orientation);
	flush();
	(void) getchar();
	puts("\r\n"); flush();
	puts("Calibrating..."); flush();
	i = ACCEL_CALIBRATE_SAMPLES;
	accel_total = 0;
	cal_adc_ring = ao_flight_adc;
	while (i) {
		ao_sleep(DATA_TO_XDATA(&ao_flight_adc));
		while (i && cal_adc_ring != ao_flight_adc) {
			accel_total += (int32_t) ao_adc_ring[cal_adc_ring].accel;
			cal_adc_ring = ao_adc_ring_next(cal_adc_ring);
			i--;
		}
	}
	return accel_total >> ACCEL_CALIBRATE_SHIFT;
}

void
ao_config_accel_calibrate_set(void) __reentrant
{
	int16_t	up, down;
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ao_cmd_lex_i == 0) {
		up = ao_config_accel_calibrate_auto("antenna up");
		down = ao_config_accel_calibrate_auto("antenna down");
	} else {
		up = ao_cmd_lex_i;
		ao_cmd_decimal();
		if (ao_cmd_status != ao_cmd_success)
			return;
		down = ao_cmd_lex_i;
	}
	if (up >= down) {
		printf("Invalid accel calibration: antenna up (%d) should be less than antenna down (%d)\n",
		       up, down);
		return;
	}
	ao_mutex_get(&ao_config_mutex);
	_ao_config_get();
	ao_config.accel_plus_g = up;
	ao_config.accel_minus_g = down;
	ao_config_dirty = 1;
	ao_mutex_put(&ao_config_mutex);
	ao_config_accel_calibrate_show();
}

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
	ao_mutex_get(&ao_config_mutex);
	_ao_config_get();
	ao_config.apogee_delay = ao_cmd_lex_i;
	ao_config_dirty = 1;
	ao_mutex_put(&ao_config_mutex);
	ao_config_apogee_delay_show();
}

#endif /* HAS_ADC */

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
	ao_mutex_get(&ao_config_mutex);
	_ao_config_get();
	ao_config.radio_cal = ao_cmd_lex_u32;
	ao_config_dirty = 1;
	ao_mutex_put(&ao_config_mutex);
	ao_config_radio_cal_show();
}

#if HAS_EEPROM
void
ao_config_log_show(void) __reentrant
{
	printf("Max flight log: %d kB\n", (int16_t) (ao_config.flight_log_max >> 10));
}

void
ao_config_log_set(void) __reentrant
{
	uint16_t	block = (uint16_t) (ao_storage_block >> 10);
	uint16_t	config = (uint16_t) (ao_storage_config >> 10);

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ao_log_present())
		printf("Storage must be empty before changing log size\n");
	else if (block > 1024 && (ao_cmd_lex_i & (block - 1)))
		printf("Flight log size must be multiple of %d kB\n", block);
	else if (ao_cmd_lex_i > config)
		printf("Flight log max %d kB\n", config);
	else {
		ao_mutex_get(&ao_config_mutex);
		_ao_config_get();
		ao_config.flight_log_max = (uint32_t) ao_cmd_lex_i << 10;
		ao_config_dirty = 1;
		ao_mutex_put(&ao_config_mutex);
		ao_config_log_show();
	}
}
#endif /* HAS_EEPROM */

struct ao_config_var {
	char		cmd;
	void		(*set)(void) __reentrant;
	void		(*show)(void) __reentrant;
	const char	*help;
};

static void
ao_config_help(void) __reentrant;

static void
ao_config_show(void) __reentrant;

static void
ao_config_write(void) __reentrant;

__code struct ao_config_var ao_config_vars[] = {
#if HAS_ADC
	{ 'm',	ao_config_main_deploy_set,	ao_config_main_deploy_show,
		"m <meters>  Set height above launch for main deploy (in meters)" },
	{ 'd',	ao_config_apogee_delay_set,	ao_config_apogee_delay_show,
	        "d <delay>   Set apogee igniter delay (in seconds)" },
#endif /* HAS_ADC */
	{ 'r',	ao_config_radio_channel_set,	ao_config_radio_channel_show,
		"r <channel> Set radio channel (freq = 434.550 + channel * .1)" },
	{ 'c',	ao_config_callsign_set,		ao_config_callsign_show,
		"c <call>    Set callsign broadcast in each packet (8 char max)" },
#if HAS_ADC
	{ 'a',	ao_config_accel_calibrate_set,	ao_config_accel_calibrate_show,
		"a <+g> <-g> Set accelerometer calibration (0 for auto)" },
#endif /* HAS_ADC */
	{ 'f',  ao_config_radio_cal_set,  	ao_config_radio_cal_show,
		"f <cal>     Set radio calibration value (cal = rf/(xtal/2^16))" },
#if HAS_EEPROM
	{ 'l',  ao_config_log_set,		ao_config_log_show,
		"l <size>    Set flight log size in kB" },
#endif
	{ 's',	ao_config_show,			ao_config_show,
		"s           Show current config values" },
#if HAS_EEPROM
	{ 'w',	ao_config_write,		ao_config_write,
		"w           Write current values to eeprom" },
#endif
	{ '?',	ao_config_help,			ao_config_help,
		"?           Show available config variables" },
	{ 0,	ao_config_help,	ao_config_help,
		NULL },
};

void
ao_config_set(void)
{
	char	c;
	uint8_t cmd;
	void (*__xdata func)(void) __reentrant;

	ao_cmd_white();
	c = ao_cmd_lex_c;
	ao_cmd_lex();
	func = 0;
	for (cmd = 0; ao_config_vars[cmd].cmd != '\0'; cmd++)
		if (ao_config_vars[cmd].cmd == c) {
			func = ao_config_vars[cmd].set;
			break;
		}
	if (func)
		(*func)();
	else
		ao_cmd_status = ao_cmd_syntax_error;
}

static void
ao_config_help(void) __reentrant
{
	uint8_t cmd;
	for (cmd = 0; ao_config_vars[cmd].cmd != '\0'; cmd++)
		puts (ao_config_vars[cmd].help);
}

static void
ao_config_show(void) __reentrant
{
	uint8_t cmd;
	printf("Config version: %d.%d\n",
	       ao_config.major, ao_config.minor);
	for (cmd = 0; ao_config_vars[cmd].cmd != '\0'; cmd++)
		if (ao_config_vars[cmd].show != ao_config_vars[cmd].set)
			(*ao_config_vars[cmd].show)();
}

#if HAS_EEPROM
static void
ao_config_write(void) __reentrant
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
	{ 'c',	ao_config_set,	"c <var> <value>                    Set config variable (? for help, s to show)" },
	{ '\0', ao_config_set, NULL },
};

void
ao_config_init(void)
{
	ao_cmd_register(&ao_config_cmds[0]);
}
