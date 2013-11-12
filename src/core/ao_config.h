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

#ifndef USE_STORAGE_CONFIG
#define USE_STORAGE_CONFIG 1
#endif

#ifndef USE_EEPROM_CONFIG
#define USE_EEPROM_CONFIG 0
#endif

#if USE_STORAGE_CONFIG

#include <ao_storage.h>

#define ao_config_setup() 		ao_storage_setup()

#define ao_config_write(bytes, len)	do {				\
		ao_storage_erase(ao_storage_config);			\
		ao_storage_write(ao_storage_config, bytes, len);	\
	} while (0)

#define ao_config_read(bytes, len)	ao_storage_read(ao_storage_config, bytes, len)

#define ao_config_flush()		ao_storage_flush()

#endif

#if USE_EEPROM_CONFIG

#include <ao_eeprom.h>

#define ao_config_setup()
#define ao_config_write(bytes, len)	ao_eeprom_write(0, bytes, len)
#define ao_config_read(bytes, len)	ao_eeprom_read(0, bytes, len)
#define ao_config_flush()

#endif

#endif /* _AO_CONFIG_H_ */
