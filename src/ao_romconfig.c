/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

__code __at (0x00a0) uint16_t ao_romconfig_version = AO_ROMCONFIG_VERSION;
__code __at (0x00a2) uint16_t ao_romconfig_check = ~AO_ROMCONFIG_VERSION;
__code __at (0x00a4) uint16_t ao_serial_number = 0;
/*
 * For 434.550MHz, the frequency value is:
 *
 * 434.550e6 / (24e6 / 2**16) = 1186611.2
 *
 * This value is stored in a const variable so that
 * ao-load can change it during programming for
 * devices that have no eeprom for config data.
 */
__code __at (0x00a6) uint32_t ao_radio_cal = 1186611;
