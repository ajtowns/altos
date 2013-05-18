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

#ifndef _AO_ADC_H_
#define _AO_ADC_H_

#include <ao_data.h>

/* Trigger a conversion sequence (called from the timer interrupt) */
void
ao_adc_poll(void);

/* Suspend the current task until another A/D sample is converted */
void
ao_adc_sleep(void);

/* Initialize the A/D converter */
void
ao_adc_init(void);

#endif /* _AO_ADC_H_ */
