/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_BEEP_H_
#define _AO_BEEP_H_

#ifndef HAS_BEEP_CONFIG
#if defined(USE_EEPROM_CONFIG) && USE_EEPROM_CONFIG || HAS_EEPROM
#define HAS_BEEP_CONFIG	1
#endif
#endif

/*
 * ao_beep.c
 */

/*
 * Various pre-defined beep frequencies
 *
 * frequency = 1/2 (24e6/32) / beep
 */

#define AO_BEEP_MID_DEFAULT	94	/* 3989Hz */

#if HAS_BEEP_CONFIG
#define AO_BEEP_MID	ao_config.mid_beep
#else
#define AO_BEEP_MID	AO_BEEP_MID_DEFAULT
#endif
#define AO_BEEP_LOW	AO_BEEP_MID * 150 / 94	/* 2500Hz */
#define AO_BEEP_HIGH	AO_BEEP_MID * 75 / 94	/* 5000Hz */

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

#endif /* _AO_BEEP_H_ */
