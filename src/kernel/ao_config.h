/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_CONFIG_H_
#define _AO_CONFIG_H_

#include <ao_pyro.h>

#ifndef USE_STORAGE_CONFIG
#define USE_STORAGE_CONFIG 1
#endif

#ifndef USE_EEPROM_CONFIG
#define USE_EEPROM_CONFIG 0
#endif

#if USE_STORAGE_CONFIG

#include <ao_storage.h>

#define ao_config_setup() 		ao_storage_setup()
#define ao_config_erase()		ao_storage_erase(ao_storage_config)
#define ao_config_write(pos,bytes, len)	ao_storage_write(ao_storage_config+(pos), bytes, len)
#define ao_config_read(pos,bytes, len)	ao_storage_read(ao_storage_config+(pos), bytes, len)
#define ao_config_flush()		ao_storage_flush()

#endif

#if USE_EEPROM_CONFIG

#include <ao_eeprom.h>

#define ao_config_setup()
#define ao_config_erase()
#define ao_config_write(pos,bytes, len)	ao_eeprom_write(pos, bytes, len)
#define ao_config_read(pos,bytes, len)	ao_eeprom_read(pos, bytes, len)
#define ao_config_flush()

#endif

#define AO_CONFIG_MAJOR	1
#define AO_CONFIG_MINOR	18

#define AO_AES_LEN 16

extern __xdata uint8_t ao_config_aes_seq;

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
#if AO_PYRO_NUM
	struct ao_pyro	pyro[AO_PYRO_NUM];	/* minor version 12 */
#endif
	uint16_t	aprs_interval;		/* minor version 13 */
#if HAS_RADIO_POWER
	uint8_t		radio_power;		/* minor version 14 */
#endif
#if HAS_RADIO_AMP
	uint8_t		radio_amp;		/* minor version 14 */
#endif
#if HAS_GYRO
	int16_t		accel_zero_along;	/* minor version 15 */
	int16_t		accel_zero_across;	/* minor version 15 */
	int16_t		accel_zero_through;	/* minor version 15 */
#endif
#if HAS_BEEP
	uint8_t		mid_beep;		/* minor version 16 */
#endif
#if HAS_TRACKER
	uint16_t	tracker_motion;		/* minor version 17 */
	uint8_t		tracker_interval;	/* minor version 17 */
#endif
#if AO_PYRO_NUM
	uint16_t	pyro_time;		/* minor version 18 */
#endif
};

#define AO_IGNITE_MODE_DUAL		0
#define AO_IGNITE_MODE_APOGEE		1
#define AO_IGNITE_MODE_MAIN		2

#define AO_RADIO_ENABLE_CORE		1
#define AO_RADIO_DISABLE_TELEMETRY	2
#define AO_RADIO_DISABLE_RDF		4

#define AO_PAD_ORIENTATION_ANTENNA_UP	0
#define AO_PAD_ORIENTATION_ANTENNA_DOWN	1

#ifndef AO_CONFIG_MAX_SIZE
#define AO_CONFIG_MAX_SIZE	128
#endif

/* Make sure AO_CONFIG_MAX_SIZE is big enough */
typedef uint8_t	config_check_space[(int) (AO_CONFIG_MAX_SIZE - sizeof (struct ao_config))];

extern __xdata struct ao_config ao_config;
extern __pdata uint8_t ao_config_loaded;

void
_ao_config_edit_start(void);

void
_ao_config_edit_finish(void);

void
ao_config_get(void);

void
ao_config_put(void);

void
ao_config_set_radio(void);

void
ao_config_init(void);

#endif /* _AO_CONFIG_H_ */
