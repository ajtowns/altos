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

#include "cc.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

static void
cook_timed(struct cc_timedata *td, struct cc_perioddata *pd,
	   double start_time, double stop_time,
	   double omega_pass, double omega_stop, double error)
{
	struct cc_perioddata	*unfiltered, *filtered;

	unfiltered = cc_period_make(td, start_time, stop_time);
	filtered = cc_period_low_pass (unfiltered, omega_pass, omega_stop, error);
	*pd = *filtered;
	free (filtered);
	free (unfiltered->data);
	free (unfiltered);
}

static double
barometer_to_altitude(double b, double pad_alt)
{
	return cc_barometer_to_altitude(b) - pad_alt;
}

struct cc_flightcooked *
cc_flight_cook(struct cc_flightraw *raw)
{
	struct cc_flightcooked *cooked;
	double			flight_start;
	double			flight_stop;
	int			start_set = 0;
	int			stop_set = 0;
	int			i;
	struct cc_timedata	*accel;
	struct cc_timedata	*accel_speed;
	struct cc_timedata	*accel_pos;
	struct cc_timedata	*pres;
	struct cc_perioddata	*pres_speed;
	struct cc_perioddata	*pres_accel;

	if (raw->accel.num == 0)
		return NULL;

	cooked = calloc (1, sizeof (struct cc_flightcooked));

	/*
	 * Find flight start and stop times by looking at
	 * state transitions. The stop time is set to the time
	 * of landing, which may be long after it landed (due to radio
	 * issues). Refine this value by looking through the sensor data
	 */
	for (i = 0; i < raw->state.num; i++) {
		if (!start_set && raw->state.data[i].value > ao_flight_pad) {
			flight_start = raw->state.data[i].time - 10;
			start_set = 1;
		}
		if (!stop_set && raw->state.data[i].value > ao_flight_main) {
			flight_stop = raw->state.data[i].time;
			stop_set = 1;
		}
	}

	if (!start_set || flight_start < raw->accel.data[0].time)
		flight_start = raw->accel.data[0].time;
	if (stop_set) {
		for (i = 0; i < raw->accel.num - 1; i++) {
			if (raw->accel.data[i+1].time >= flight_stop) {
				flight_stop = raw->accel.data[i].time;
				break;
			}
		}
	} else {
		flight_stop = raw->accel.data[raw->accel.num-1].time;
	}
	cooked->flight_start = flight_start;
	cooked->flight_stop = flight_stop;

	/* Integrate the accelerometer data to get speed and position */
	accel = cc_timedata_convert(&raw->accel, cc_accelerometer_to_acceleration, raw->ground_accel);
	cooked->accel = *accel;
	free(accel);
	accel_speed = cc_timedata_integrate(&cooked->accel, flight_start - 10, flight_stop);
	accel_pos = cc_timedata_integrate(accel_speed, flight_start - 10, flight_stop);

#define ACCEL_OMEGA_PASS	(2 * M_PI * 20 / 100)
#define ACCEL_OMEGA_STOP	(2 * M_PI * 30 / 100)
#define BARO_OMEGA_PASS		(2 * M_PI * .5 / 100)
#define BARO_OMEGA_STOP		(2 * M_PI * 1 / 100)
#define FILTER_ERROR		(1e-8)

	cook_timed(&cooked->accel, &cooked->accel_accel,
		   flight_start, flight_stop,
		   ACCEL_OMEGA_PASS, ACCEL_OMEGA_STOP, FILTER_ERROR);
	cook_timed(accel_speed, &cooked->accel_speed,
		   flight_start, flight_stop,
		   ACCEL_OMEGA_PASS, ACCEL_OMEGA_STOP, FILTER_ERROR);
	free(accel_speed->data); free(accel_speed);
	cook_timed(accel_pos, &cooked->accel_pos,
		   flight_start, flight_stop,
		   ACCEL_OMEGA_PASS, ACCEL_OMEGA_STOP, FILTER_ERROR);
	free(accel_pos->data); free(accel_pos);

	/* Filter the pressure data */
	pres = cc_timedata_convert(&raw->pres, barometer_to_altitude,
				   cc_barometer_to_altitude(raw->ground_pres));
	cooked->pres = *pres;
	free(pres);
	cook_timed(&cooked->pres, &cooked->pres_pos,
		   flight_start, flight_stop,
		   BARO_OMEGA_PASS, BARO_OMEGA_STOP, FILTER_ERROR);
	/* differentiate twice to get to acceleration */
	pres_speed = cc_perioddata_differentiate(&cooked->pres_pos);
	pres_accel = cc_perioddata_differentiate(pres_speed);

	cooked->pres_speed = *pres_speed;
	free(pres_speed);
	cooked->pres_accel = *pres_accel;
	free(pres_accel);

	/* copy state */
	cooked->state.num = raw->state.num;
	cooked->state.size = raw->state.num;
	cooked->state.data = calloc(cooked->state.num, sizeof (struct cc_timedataelt));
	memcpy(cooked->state.data, raw->state.data, cooked->state.num * sizeof (struct cc_timedataelt));
	cooked->state.time_offset = raw->state.time_offset;
	return cooked;
}

#define if_free(x)	((x) ? free(x) : (void) 0)

void
cc_flightcooked_free(struct cc_flightcooked *cooked)
{
	if_free(cooked->accel_accel.data);
	if_free(cooked->accel_speed.data);
	if_free(cooked->accel_pos.data);
	if_free(cooked->pres_pos.data);
	if_free(cooked->pres_speed.data);
	if_free(cooked->pres_accel.data);
	if_free(cooked->gps_lat.data);
	if_free(cooked->gps_lon.data);
	if_free(cooked->gps_alt.data);
	if_free(cooked->state.data);
	if_free(cooked->accel.data);
	if_free(cooked->pres.data);
	free(cooked);
}
