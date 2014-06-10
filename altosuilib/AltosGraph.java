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

package org.altusmetrum.altosuilib_2;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;

import org.jfree.ui.*;
import org.jfree.chart.*;
import org.jfree.chart.plot.*;
import org.jfree.chart.axis.*;
import org.jfree.chart.renderer.*;
import org.jfree.chart.renderer.xy.*;
import org.jfree.chart.labels.*;
import org.jfree.data.xy.*;
import org.jfree.data.*;

class AltosVoltage extends AltosUnits {

	public double value(double v, boolean imperial_units) {
		return v;
	}

	public double inverse(double v, boolean imperial_units) {
		return v;
	}

	public String show_units(boolean imperial_units) {
		return "V";
	}

	public String say_units(boolean imperial_units) {
		return "volts";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return width / 2;
	}
}

class AltosNsat extends AltosUnits {

	public double value(double v, boolean imperial_units) {
		return v;
	}

	public double inverse(double v, boolean imperial_units) {
		return v;
	}

	public String show_units(boolean imperial_units) {
		return "Sats";
	}

	public String say_units(boolean imperial_units) {
		return "Satellites";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return 0;
	}
}

class AltosPressure extends AltosUnits {

	public double value(double p, boolean imperial_units) {
		return p;
	}

	public double inverse(double p, boolean imperial_units) {
		return p;
	}

	public String show_units(boolean imperial_units) {
		return "Pa";
	}

	public String say_units(boolean imperial_units) {
		return "pascals";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return 0;
	}
}

class AltosDbm extends AltosUnits {

	public double value(double d, boolean imperial_units) {
		return d;
	}

	public double inverse(double d, boolean imperial_units) {
		return d;
	}

	public String show_units(boolean imperial_units) {
		return "dBm";
	}

	public String say_units(boolean imperial_units) {
		return "D B M";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return 0;
	}
}

class AltosGyroUnits extends AltosUnits {

	public double value(double p, boolean imperial_units) {
		return p;
	}

	public double inverse(double p, boolean imperial_units) {
		return p;
	}

	public String show_units(boolean imperial_units) {
		return "°/sec";
	}

	public String say_units(boolean imperial_units) {
		return "degrees per second";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return 1;
	}
}

class AltosMagUnits extends AltosUnits {

	public double value(double p, boolean imperial_units) {
		return p;
	}

	public double inverse(double p, boolean imperial_units) {
		return p;
	}

	public String show_units(boolean imperial_units) {
		return "Ga";
	}

	public String say_units(boolean imperial_units) {
		return "gauss";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return 2;
	}
}

public class AltosGraph extends AltosUIGraph {

	static final private Color height_color = new Color(194,31,31);
	static final private Color gps_height_color = new Color(150,31,31);
	static final private Color pressure_color = new Color (225,31,31);
	static final private Color range_color = new Color(100, 31, 31);
	static final private Color distance_color = new Color(100, 31, 194);
	static final private Color speed_color = new Color(31,194,31);
	static final private Color accel_color = new Color(31,31,194);
	static final private Color voltage_color = new Color(194, 194, 31);
	static final private Color battery_voltage_color = new Color(194, 194, 31);
	static final private Color drogue_voltage_color = new Color(150, 150, 31);
	static final private Color main_voltage_color = new Color(100, 100, 31);
	static final private Color gps_nsat_color = new Color (194, 31, 194);
	static final private Color gps_nsat_solution_color = new Color (194, 31, 194);
	static final private Color gps_nsat_view_color = new Color (150, 31, 150);
	static final private Color gps_course_color = new Color (100, 31, 112);
	static final private Color gps_ground_speed_color = new Color (31, 112, 100);
	static final private Color gps_climb_rate_color = new Color (31, 31, 112);
	static final private Color temperature_color = new Color (31, 194, 194);
	static final private Color dbm_color = new Color(31, 100, 100);
	static final private Color state_color = new Color(0,0,0);
	static final private Color accel_x_color = new Color(255, 0, 0);
	static final private Color accel_y_color = new Color(0, 255, 0);
	static final private Color accel_z_color = new Color(0, 0, 255);
	static final private Color gyro_x_color = new Color(192, 0, 0);
	static final private Color gyro_y_color = new Color(0, 192, 0);
	static final private Color gyro_z_color = new Color(0, 0, 192);
	static final private Color mag_x_color = new Color(128, 0, 0);
	static final private Color mag_y_color = new Color(0, 128, 0);
	static final private Color mag_z_color = new Color(0, 0, 128);
	static final private Color orient_color = new Color(31, 31, 31);

	static AltosVoltage voltage_units = new AltosVoltage();
	static AltosPressure pressure_units = new AltosPressure();
	static AltosNsat nsat_units = new AltosNsat();
	static AltosDbm dbm_units = new AltosDbm();
	static AltosGyroUnits gyro_units = new AltosGyroUnits();
	static AltosOrient orient_units = new AltosOrient();
	static AltosMagUnits mag_units = new AltosMagUnits();

	AltosUIAxis	height_axis, speed_axis, accel_axis, voltage_axis, temperature_axis, nsat_axis, dbm_axis;
	AltosUIAxis	distance_axis, pressure_axis;
	AltosUIAxis	gyro_axis, orient_axis, mag_axis;
	AltosUIAxis	course_axis;

	public AltosGraph(AltosUIEnable enable, AltosFlightStats stats, AltosGraphDataSet dataSet) {
		super(enable);

		height_axis = newAxis("Height", AltosConvert.height, height_color);
		pressure_axis = newAxis("Pressure", pressure_units, pressure_color, 0);
		speed_axis = newAxis("Speed", AltosConvert.speed, speed_color);
		accel_axis = newAxis("Acceleration", AltosConvert.accel, accel_color);
		voltage_axis = newAxis("Voltage", voltage_units, voltage_color);
		temperature_axis = newAxis("Temperature", AltosConvert.temperature, temperature_color, 0);
		nsat_axis = newAxis("Satellites", nsat_units, gps_nsat_color,
				    AltosUIAxis.axis_include_zero | AltosUIAxis.axis_integer);
		dbm_axis = newAxis("Signal Strength", dbm_units, dbm_color, 0);
		distance_axis = newAxis("Distance", AltosConvert.distance, range_color);

		gyro_axis = newAxis("Rotation Rate", gyro_units, gyro_z_color, 0);
		orient_axis = newAxis("Tilt Angle", orient_units, orient_color, 0);
		mag_axis = newAxis("Magnetic Field", mag_units, mag_x_color, 0);
		course_axis = newAxis("Course", orient_units, gps_course_color, 0);

		addMarker("State", AltosGraphDataPoint.data_state, state_color);

		if (stats.has_flight_data) {
			addSeries("Height",
				  AltosGraphDataPoint.data_height,
				  AltosConvert.height,
				  height_color,
				  true,
				  height_axis);
			addSeries("Pressure",
				  AltosGraphDataPoint.data_pressure,
				  pressure_units,
				  pressure_color,
				  false,
				  pressure_axis);
			addSeries("Speed",
				  AltosGraphDataPoint.data_speed,
				  AltosConvert.speed,
				  speed_color,
				  true,
				  speed_axis);
			addSeries("Acceleration",
				  AltosGraphDataPoint.data_accel,
				  AltosConvert.accel,
				  accel_color,
				  true,
				  accel_axis);
		}
		if (stats.has_gps) {
			boolean	enable_gps = false;

			if (!stats.has_flight_data)
				enable_gps = true;

			addSeries("Range",
				  AltosGraphDataPoint.data_range,
				  AltosConvert.distance,
				  range_color,
				  false,
				  distance_axis);
			addSeries("Distance",
				  AltosGraphDataPoint.data_distance,
				  AltosConvert.distance,
				  distance_color,
				  enable_gps,
				  distance_axis);
			addSeries("GPS Height",
				  AltosGraphDataPoint.data_gps_height,
				  AltosConvert.height,
				  gps_height_color,
				  enable_gps,
				  height_axis);
			addSeries("GPS Altitude",
				  AltosGraphDataPoint.data_gps_altitude,
				  AltosConvert.height,
				  gps_height_color,
				  false,
				  height_axis);
			addSeries("GPS Satellites in Solution",
				  AltosGraphDataPoint.data_gps_nsat_solution,
				  nsat_units,
				  gps_nsat_solution_color,
				  false,
				  nsat_axis);
			addSeries("GPS Satellites in View",
				  AltosGraphDataPoint.data_gps_nsat_view,
				  nsat_units,
				  gps_nsat_view_color,
				  false,
				  nsat_axis);
			addSeries("GPS Course",
				  AltosGraphDataPoint.data_gps_course,
				  orient_units,
				  gps_course_color,
				  false,
				  course_axis);
			addSeries("GPS Ground Speed",
				  AltosGraphDataPoint.data_gps_ground_speed,
				  AltosConvert.speed,
				  gps_ground_speed_color,
				  enable_gps,
				  speed_axis);
			addSeries("GPS Climb Rate",
				  AltosGraphDataPoint.data_gps_climb_rate,
				  AltosConvert.speed,
				  gps_climb_rate_color,
				  enable_gps,
				  speed_axis);
		}
		if (stats.has_rssi)
			addSeries("Received Signal Strength",
				  AltosGraphDataPoint.data_rssi,
				  dbm_units,
				  dbm_color,
				  false,
				  dbm_axis);

		if (stats.has_battery)
			addSeries("Battery Voltage",
				  AltosGraphDataPoint.data_battery_voltage,
				  voltage_units,
				  battery_voltage_color,
				  false,
				  voltage_axis);

		if (stats.has_flight_adc) {
			addSeries("Temperature",
				  AltosGraphDataPoint.data_temperature,
				  AltosConvert.temperature,
				  temperature_color,
				  false,
				  temperature_axis);
			addSeries("Drogue Voltage",
				  AltosGraphDataPoint.data_drogue_voltage,
				  voltage_units,
				  drogue_voltage_color,
				  false,
				  voltage_axis);
			addSeries("Main Voltage",
				  AltosGraphDataPoint.data_main_voltage,
				  voltage_units,
				  main_voltage_color,
				  false,
				  voltage_axis);
		}

		if (stats.has_imu) {
			addSeries("Acceleration X",
				  AltosGraphDataPoint.data_accel_x,
				  AltosConvert.accel,
				  accel_x_color,
				  false,
				  accel_axis);
			addSeries("Acceleration Y",
				  AltosGraphDataPoint.data_accel_y,
				  AltosConvert.accel,
				  accel_y_color,
				  false,
				  accel_axis);
			addSeries("Acceleration Z",
				  AltosGraphDataPoint.data_accel_z,
				  AltosConvert.accel,
				  accel_z_color,
				  false,
				  accel_axis);
			addSeries("Rotation Rate X",
				  AltosGraphDataPoint.data_gyro_x,
				  gyro_units,
				  gyro_x_color,
				  false,
				  gyro_axis);
			addSeries("Rotation Rate Y",
				  AltosGraphDataPoint.data_gyro_y,
				  gyro_units,
				  gyro_y_color,
				  false,
				  gyro_axis);
			addSeries("Rotation Rate Z",
				  AltosGraphDataPoint.data_gyro_z,
				  gyro_units,
				  gyro_z_color,
				  false,
				  gyro_axis);
		}
		if (stats.has_mag) {
			addSeries("Magnetometer X",
				  AltosGraphDataPoint.data_mag_x,
				  mag_units,
				  mag_x_color,
				  false,
				  mag_axis);
			addSeries("Magnetometer Y",
				  AltosGraphDataPoint.data_mag_y,
				  mag_units,
				  mag_y_color,
				  false,
				  mag_axis);
			addSeries("Magnetometer Z",
				  AltosGraphDataPoint.data_mag_z,
				  mag_units,
				  mag_z_color,
				  false,
				  mag_axis);
		}
		if (stats.has_orient)
			addSeries("Tilt Angle",
				  AltosGraphDataPoint.data_orient,
				  orient_units,
				  orient_color,
				  false,
				  orient_axis);
		if (stats.num_ignitor > 0) {
			for (int i = 0; i < stats.num_ignitor; i++)
				addSeries(AltosLib.ignitor_name(i),
					  AltosGraphDataPoint.data_ignitor_0 + i,
					  voltage_units,
					  main_voltage_color,
					  false,
					  voltage_axis);
			for (int i = 0; i < stats.num_ignitor; i++)
				addMarker(AltosLib.ignitor_name(i), AltosGraphDataPoint.data_ignitor_fired_0 + i, state_color);
		}

		setDataSet(dataSet);
	}
}
