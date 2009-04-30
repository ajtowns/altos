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
#define AO_CONFIG_DEFAULT_CALLSIGN	"KD7SGQ"
#define AO_CONFIG_DEFAULT_ACCEL_ZERO_G	16000

static void
_ao_config_put(void)
{
	ao_ee_write_config((uint8_t *) &ao_config, sizeof (ao_config));
}

static void
_ao_config_get(void)
{
	if (ao_config_loaded)
		return;
	ao_ee_read_config((uint8_t *) &ao_config, sizeof (ao_config));
	if (ao_config.major != AO_CONFIG_MAJOR) {
		ao_config.major = AO_CONFIG_MAJOR;
		ao_config.minor = AO_CONFIG_MINOR;
		ao_config.main_deploy = AO_CONFIG_DEFAULT_MAIN_DEPLOY;
		ao_config.radio_channel = AO_CONFIG_DEFAULT_RADIO_CHANNEL;
		ao_config.accel_zero_g = AO_CONFIG_DEFAULT_ACCEL_ZERO_G;
		memset(&ao_config.callsign, '\0', sizeof (ao_config.callsign));
		memcpy(&ao_config.callsign, AO_CONFIG_DEFAULT_CALLSIGN,
		       sizeof(AO_CONFIG_DEFAULT_CALLSIGN) - 1);
		ao_config_dirty = 1;
	}
	/* deal with minor version issues here, at 0 we haven't any */
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
}

void
ao_config_main_deploy_show(void) __reentrant
{
	printf("Main deploy set to %d meters (%d feet)\n",
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
ao_config_accel_zero_g_show(void) __reentrant
{
	printf("Accel zero g point set to %d\n",
	       ao_config.accel_zero_g);
}

#define ZERO_G_SAMPLES	1000

static int16_t
ao_config_accel_zero_g_auto(void) __reentrant
{
	uint16_t	i;
	int32_t		accel_total;
	uint8_t		cal_adc_ring;

	puts("Calibrating accelerometer..."); flush();
	i = ZERO_G_SAMPLES;
	accel_total = 0;
	cal_adc_ring = ao_adc_head;
	while (i) {
		ao_sleep(&ao_adc_ring);
		while (i && cal_adc_ring != ao_adc_head) {
			accel_total += (int32_t) ao_adc_ring[cal_adc_ring].accel;
			cal_adc_ring = ao_adc_ring_next(cal_adc_ring);
			i--;
		}
	}
	return (int16_t) (accel_total / ZERO_G_SAMPLES);
}
void
ao_config_accel_zero_g_set(void) __reentrant
{
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ao_cmd_lex_i == 0)
		ao_cmd_lex_i = ao_config_accel_zero_g_auto();
	ao_mutex_get(&ao_config_mutex);
	_ao_config_get();
	ao_config.accel_zero_g = ao_cmd_lex_i;
	ao_config_dirty = 1;
	ao_mutex_put(&ao_config_mutex);
	ao_config_accel_zero_g_show();
}

struct ao_config_var {
	char		cmd;
	void		(*set)(void) __reentrant;
	void		(*show)(void) __reentrant;
	const char	*help;
};

void
ao_config_help(void) __reentrant;

void
ao_config_show(void) __reentrant;

void
ao_config_write(void) __reentrant;

__code struct ao_config_var ao_config_vars[] = {
	{ 'm',	ao_config_main_deploy_set,	ao_config_main_deploy_show,
		"m <meters>  Set height above launch for main deploy (in meters)" },
	{ 'a',	ao_config_accel_zero_g_set,	ao_config_accel_zero_g_show,
		"a <value>   Set accelerometer zero g point (0 for auto)" },
	{ 'r',	ao_config_radio_channel_set,	ao_config_radio_channel_show,
		"r <channel> Set radio channel (freq = 434.550 + channel * .1)" },
	{ 'c',	ao_config_callsign_set,		ao_config_callsign_show,
		"c <call>    Set callsign broadcast in each packet (8 char max)" },
	{ 's',	ao_config_show,			ao_config_show,
		"s           Show current config values" },
	{ 'w',	ao_config_write,		ao_config_write,
		"w           Write current values to eeprom" },
	{ '?',	ao_config_help,			ao_config_help,
		"?           Show available config variables" },
	{ 0,	ao_config_main_deploy_set,	ao_config_main_deploy_show,
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

void
ao_config_help(void) __reentrant
{
	uint8_t cmd;
	for (cmd = 0; ao_config_vars[cmd].cmd != '\0'; cmd++)
		puts (ao_config_vars[cmd].help);
}

void
ao_config_show(void) __reentrant
{
	uint8_t cmd;
	for (cmd = 0; ao_config_vars[cmd].cmd != '\0'; cmd++)
		if (ao_config_vars[cmd].show != ao_config_vars[cmd].set)
			(*ao_config_vars[cmd].show)();
}

void
ao_config_write(void) __reentrant
{
	ao_mutex_get(&ao_config_mutex);
	if (ao_config_dirty) {
		_ao_config_put();
		ao_config_dirty = 0;
		printf("Saved\n");
	}
	ao_mutex_put(&ao_config_mutex);
}

__code struct ao_cmds ao_config_cmds[] = {
	{ 'c',	ao_config_set,	"c <var> <value>                    Set config variable (? for help, s to show)" },
	{ '\0', ao_config_set, NULL },
};

void
ao_config_init(void)
{
	ao_cmd_register(&ao_config_cmds[0]);
}
