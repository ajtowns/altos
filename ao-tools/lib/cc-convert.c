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
#include <math.h>

/*
 * Pressure Sensor Model, version 1.1
 *
 * written by Holly Grimes
 *
 * Uses the International Standard Atmosphere as described in
 *   "A Quick Derivation relating altitude to air pressure" (version 1.03)
 *    from the Portland State Aerospace Society, except that the atmosphere
 *    is divided into layers with each layer having a different lapse rate.
 *
 * Lapse rate data for each layer was obtained from Wikipedia on Sept. 1, 2007
 *    at site <http://en.wikipedia.org/wiki/International_Standard_Atmosphere
 *
 * Height measurements use the local tangent plane.  The postive z-direction is up.
 *
 * All measurements are given in SI units (Kelvin, Pascal, meter, meters/second^2).
 *   The lapse rate is given in Kelvin/meter, the gas constant for air is given
 *   in Joules/(kilogram-Kelvin).
 */

#define GRAVITATIONAL_ACCELERATION -9.80665
#define AIR_GAS_CONSTANT	287.053
#define NUMBER_OF_LAYERS	7
#define MAXIMUM_ALTITUDE	84852.0
#define MINIMUM_PRESSURE	0.3734
#define LAYER0_BASE_TEMPERATURE	288.15
#define LAYER0_BASE_PRESSURE	101325

/* lapse rate and base altitude for each layer in the atmosphere */
static const double lapse_rate[NUMBER_OF_LAYERS] = {
	-0.0065, 0.0, 0.001, 0.0028, 0.0, -0.0028, -0.002
};

static const int base_altitude[NUMBER_OF_LAYERS] = {
	0, 11000, 20000, 32000, 47000, 51000, 71000
};

/* outputs atmospheric pressure associated with the given altitude. altitudes
   are measured with respect to the mean sea level */
double
cc_altitude_to_pressure(double altitude)
{

   double base_temperature = LAYER0_BASE_TEMPERATURE;
   double base_pressure = LAYER0_BASE_PRESSURE;

   double pressure;
   double base; /* base for function to determine pressure */
   double exponent; /* exponent for function to determine pressure */
   int layer_number; /* identifies layer in the atmosphere */
   int delta_z; /* difference between two altitudes */

   if (altitude > MAXIMUM_ALTITUDE) /* FIX ME: use sensor data to improve model */
      return 0;

   /* calculate the base temperature and pressure for the atmospheric layer
      associated with the inputted altitude */
   for(layer_number = 0; layer_number < NUMBER_OF_LAYERS - 1 && altitude > base_altitude[layer_number + 1]; layer_number++) {
      delta_z = base_altitude[layer_number + 1] - base_altitude[layer_number];
      if (lapse_rate[layer_number] == 0.0) {
         exponent = GRAVITATIONAL_ACCELERATION * delta_z
              / AIR_GAS_CONSTANT / base_temperature;
         base_pressure *= exp(exponent);
      }
      else {
         base = (lapse_rate[layer_number] * delta_z / base_temperature) + 1.0;
         exponent = GRAVITATIONAL_ACCELERATION /
              (AIR_GAS_CONSTANT * lapse_rate[layer_number]);
         base_pressure *= pow(base, exponent);
      }
      base_temperature += delta_z * lapse_rate[layer_number];
   }

   /* calculate the pressure at the inputted altitude */
   delta_z = altitude - base_altitude[layer_number];
   if (lapse_rate[layer_number] == 0.0) {
      exponent = GRAVITATIONAL_ACCELERATION * delta_z
           / AIR_GAS_CONSTANT / base_temperature;
      pressure = base_pressure * exp(exponent);
   }
   else {
      base = (lapse_rate[layer_number] * delta_z / base_temperature) + 1.0;
      exponent = GRAVITATIONAL_ACCELERATION /
           (AIR_GAS_CONSTANT * lapse_rate[layer_number]);
      pressure = base_pressure * pow(base, exponent);
   }

   return pressure;
}


/* outputs the altitude associated with the given pressure. the altitude
   returned is measured with respect to the mean sea level */
double
cc_pressure_to_altitude(double pressure)
{

   double next_base_temperature = LAYER0_BASE_TEMPERATURE;
   double next_base_pressure = LAYER0_BASE_PRESSURE;

   double altitude;
   double base_pressure;
   double base_temperature;
   double base; /* base for function to determine base pressure of next layer */
   double exponent; /* exponent for function to determine base pressure
                             of next layer */
   double coefficient;
   int layer_number; /* identifies layer in the atmosphere */
   int delta_z; /* difference between two altitudes */

   if (pressure < 0)  /* illegal pressure */
      return -1;
   if (pressure < MINIMUM_PRESSURE) /* FIX ME: use sensor data to improve model */
      return MAXIMUM_ALTITUDE;

   /* calculate the base temperature and pressure for the atmospheric layer
      associated with the inputted pressure. */
   layer_number = -1;
   do {
      layer_number++;
      base_pressure = next_base_pressure;
      base_temperature = next_base_temperature;
      delta_z = base_altitude[layer_number + 1] - base_altitude[layer_number];
      if (lapse_rate[layer_number] == 0.0) {
         exponent = GRAVITATIONAL_ACCELERATION * delta_z
              / AIR_GAS_CONSTANT / base_temperature;
         next_base_pressure *= exp(exponent);
      }
      else {
         base = (lapse_rate[layer_number] * delta_z / base_temperature) + 1.0;
         exponent = GRAVITATIONAL_ACCELERATION /
              (AIR_GAS_CONSTANT * lapse_rate[layer_number]);
         next_base_pressure *= pow(base, exponent);
      }
      next_base_temperature += delta_z * lapse_rate[layer_number];
   }
   while(layer_number < NUMBER_OF_LAYERS - 1 && pressure < next_base_pressure);

   /* calculate the altitude associated with the inputted pressure */
   if (lapse_rate[layer_number] == 0.0) {
      coefficient = (AIR_GAS_CONSTANT / GRAVITATIONAL_ACCELERATION)
                                                    * base_temperature;
      altitude = base_altitude[layer_number]
                    + coefficient * log(pressure / base_pressure);
   }
   else {
      base = pressure / base_pressure;
      exponent = AIR_GAS_CONSTANT * lapse_rate[layer_number]
                                       / GRAVITATIONAL_ACCELERATION;
      coefficient = base_temperature / lapse_rate[layer_number];
      altitude = base_altitude[layer_number]
                      + coefficient * (pow(base, exponent) - 1);
   }

   return altitude;
}

/*
 * Values for our MP3H6115A pressure sensor
 *
 * From the data sheet:
 *
 * Pressure range: 15-115 kPa
 * Voltage at 115kPa: 2.82
 * Output scale: 27mV/kPa
 *
 *
 * 27 mV/kPa * 2047 / 3300 counts/mV = 16.75 counts/kPa
 * 2.82V * 2047 / 3.3 counts/V = 1749 counts/115 kPa
 */

static const double counts_per_kPa = 27 * 2047 / 3300;
static const double counts_at_101_3kPa = 1674.0;

double
cc_barometer_to_pressure(double count)
{
	return ((count / 16.0) / 2047.0 + 0.095) / 0.009 * 1000.0;
}

double
cc_barometer_to_altitude(double baro)
{
	double Pa = cc_barometer_to_pressure(baro);
	return cc_pressure_to_altitude(Pa);
}

static const double count_per_mss = 27.0;

double
cc_accelerometer_to_acceleration(double accel, double ground_accel)
{
	return (ground_accel - accel) / count_per_mss;
}

/* Value for the CC1111 built-in temperature sensor
 * Output voltage at 0°C = 0.755V
 * Coefficient = 0.00247V/°C
 * Reference voltage = 1.25V
 *
 * temp = ((value / 32767) * 1.25 - 0.755) / 0.00247
 *      = (value - 19791.268) / 32768 * 1.25 / 0.00247
 */

double
cc_thermometer_to_temperature(double thermo)
{
	return (thermo - 19791.268) / 32728.0 * 1.25 / 0.00247;
}

double
cc_battery_to_voltage(double battery)
{
	return battery / 32767.0 * 5.0;
}

double
cc_ignitor_to_voltage(double ignite)
{
	return ignite / 32767 * 15.0;
}

static inline double sqr(double a) { return a * a; }

void
cc_great_circle (double start_lat, double start_lon,
		 double end_lat, double end_lon,
		 double *dist, double *bearing)
{
	const double rad = M_PI / 180;
	const double earth_radius = 6371.2 * 1000;	/* in meters */
	double lat1 = rad * start_lat;
	double lon1 = rad * -start_lon;
	double lat2 = rad * end_lat;
	double lon2 = rad * -end_lon;

//	double d_lat = lat2 - lat1;
	double d_lon = lon2 - lon1;

	/* From http://en.wikipedia.org/wiki/Great-circle_distance */
	double vdn = sqrt(sqr(cos(lat2) * sin(d_lon)) +
			  sqr(cos(lat1) * sin(lat2) -
			      sin(lat1) * cos(lat2) * cos(d_lon)));
	double vdd = sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(d_lon);
	double d = atan2(vdn,vdd);
	double course;

	if (cos(lat1) < 1e-20) {
		if (lat1 > 0)
			course = M_PI;
		else
			course = -M_PI;
	} else {
		if (d < 1e-10)
			course = 0;
		else
			course = acos((sin(lat2)-sin(lat1)*cos(d)) /
				      (sin(d)*cos(lat1)));
		if (sin(lon2-lon1) > 0)
			course = 2 * M_PI-course;
	}
	*dist = d * earth_radius;
	*bearing = course * 180/M_PI;
}
