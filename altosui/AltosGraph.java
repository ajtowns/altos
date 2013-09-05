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

package altosui;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_2.*;
import org.altusmetrum.altosuilib_1.*;

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

	public double value(double v) {
		return v;
	}

	public String show_units() {
		return "V";
	}

	public String say_units() {
		return "volts";
	}

	public int show_fraction(int width) {
		return width / 2;
	}
}

class AltosNsat extends AltosUnits {

	public double value(double v) {
		return v;
	}

	public String show_units() {
		return "Sats";
	}

	public String say_units() {
		return "Satellites";
	}

	public int show_fraction(int width) {
		return 0;
	}
}

class AltosPressure extends AltosUnits {

	public double value(double p) {
		return p;
	}

	public String show_units() {
		return "Pa";
	}

	public String say_units() {
		return "pascals";
	}

	public int show_fraction(int width) {
		return 0;
	}
}

class AltosDbm extends AltosUnits {

	public double value(double d) {
		return d;
	}

	public String show_units() {
		return "dBm";
	}

	public String say_units() {
		return "D B M";
	}

	public int show_fraction(int width) {
		return 0;
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
	static final private Color temperature_color = new Color (31, 194, 194);
	static final private Color dbm_color = new Color(31, 100, 100);
	static final private Color state_color = new Color(0,0,0);

	static AltosVoltage voltage_units = new AltosVoltage();
	static AltosPressure pressure_units = new AltosPressure();
	static AltosNsat nsat_units = new AltosNsat();
	static AltosDbm dbm_units = new AltosDbm();

	AltosUIAxis	height_axis, speed_axis, accel_axis, voltage_axis, temperature_axis, nsat_axis, dbm_axis;
	AltosUIAxis	distance_axis, pressure_axis;

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

		addMarker("State", AltosGraphDataPoint.data_state, state_color);
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
		if (stats.has_gps) {
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
				  false,
				  distance_axis);
			addSeries("GPS Height",
				  AltosGraphDataPoint.data_gps_height,
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
		}
		if (stats.has_rssi)
			addSeries("Received Signal Strength",
				  AltosGraphDataPoint.data_rssi,
				  dbm_units,
				  dbm_color,
				  false,
				  dbm_axis);
		if (stats.has_other_adc) {
			addSeries("Temperature",
				  AltosGraphDataPoint.data_temperature,
				  AltosConvert.temperature,
				  temperature_color,
				  false,
				  temperature_axis);
			addSeries("Battery Voltage",
				  AltosGraphDataPoint.data_battery_voltage,
				  voltage_units,
				  battery_voltage_color,
				  false,
				  voltage_axis);
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

		setDataSet(dataSet);
	}
}