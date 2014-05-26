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

package org.altusmetrum.altoslib_4;

import java.util.*;
import java.text.*;
import java.util.concurrent.*;

public class AltosPyro {
	public static final int pyro_none			= 0x00000000;

	public static final int pyro_accel_less			= 0x00000001;
	public static final int pyro_accel_greater		= 0x00000002;
	public static final String pyro_accel_less_string	= "a<";
	public static final String pyro_accel_greater_string	= "a>";
	public static final String pyro_accel_less_name		= "Acceleration less than";
	public static final String pyro_accel_greater_name	= "Acceleration greater than";
	public static final double pyro_accel_scale		= 16.0;

	public static final int pyro_speed_less			= 0x00000004;
	public static final int pyro_speed_greater		= 0x00000008;
	public static final String pyro_speed_less_string	= "s<";
	public static final String pyro_speed_greater_string	= "s>";
	public static final String pyro_speed_less_name		= "Speed less than";
	public static final String pyro_speed_greater_name	= "Speed greater than";
	public static final double pyro_speed_scale		= 16.0;

	public static final int pyro_height_less		= 0x00000010;
	public static final int pyro_height_greater		= 0x00000020;
	public static final String pyro_height_less_string	= "h<";
	public static final String pyro_height_greater_string	= "h>";
	public static final String pyro_height_less_name	= "Height less than";
	public static final String pyro_height_greater_name	= "Height greater than";
	public static final double pyro_height_scale		= 1.0;

	public static final int pyro_orient_less		= 0x00000040;
	public static final int pyro_orient_greater		= 0x00000080;
	public static final String pyro_orient_less_string	= "o<";
	public static final String pyro_orient_greater_string	= "o>";
	public static final String pyro_orient_less_name	= "Angle from vertical less than (degrees)";
	public static final String pyro_orient_greater_name	= "Angle from vertical greater than (degrees)";
	public static final double pyro_orient_scale		= 1.0;

	public static final int pyro_time_less			= 0x00000100;
	public static final int pyro_time_greater		= 0x00000200;
	public static final String pyro_time_less_string	= "t<";
	public static final String pyro_time_greater_string	= "t>";
	public static final String pyro_time_less_name		= "Time since boost less than (s)";
	public static final String pyro_time_greater_name	= "Time since boost greater than (s)";
	public static final double pyro_time_scale		= 100.0;

	public static final int pyro_ascending			= 0x00000400;
	public static final int pyro_descending			= 0x00000800;
	public static final String pyro_ascending_string	= "A";
	public static final String pyro_descending_string	= "D";
	public static final String pyro_ascending_name		= "Ascending";
	public static final String pyro_descending_name		= "Descending";

	public static final int pyro_after_motor		= 0x00001000;
	public static final String pyro_after_motor_string	= "m";
	public static final String pyro_after_motor_name	= "After motor number";
	public static final double pyro_after_motor_scale	= 1.0;

	public static final int pyro_delay			= 0x00002000;
	public static final String pyro_delay_string		= "d";
	public static final String pyro_delay_name		= "Delay after other conditions (s)";
	public static final double pyro_delay_scale		= 100.0;

	public static final int pyro_state_less			= 0x00004000;
	public static final int pyro_state_greater_or_equal  	= 0x00008000;
	public static final String pyro_state_less_string	= "f<";
	public static final String pyro_state_greater_or_equal_string	= "f>=";
	public static final String pyro_state_less_name		= "Flight state before";
	public static final String pyro_state_greater_or_equal_name	= "Flight state after";
	public static final double pyro_state_scale		= 1.0;

	public static final int	pyro_all			= 0x0000ffff;

	public static final int pyro_no_value			= (pyro_ascending |
								   pyro_descending);

	public static final int pyro_state_value		= pyro_state_less | pyro_state_greater_or_equal;

	private static HashMap<String,Integer> string_to_pyro = new HashMap<String,Integer>();

	private static HashMap<Integer,String> pyro_to_string = new HashMap<Integer,String>();

	private static HashMap<Integer,String> pyro_to_name = new HashMap<Integer,String>();

	private static HashMap<Integer,AltosUnits> pyro_to_units = new HashMap<Integer,AltosUnits>();

	private static HashMap<Integer,Double> pyro_to_scale = new HashMap<Integer,Double>();

	private static void insert_map(int flag, String string, String name, AltosUnits units, double scale) {
		string_to_pyro.put(string, flag);
		pyro_to_string.put(flag, string);
		pyro_to_name.put(flag, name);
		if (units != null)
			pyro_to_units.put(flag, units);
		pyro_to_scale.put(flag, scale);
	}

	public static int string_to_pyro(String name) {
		if (string_to_pyro.containsKey(name))
			return string_to_pyro.get(name);
		return pyro_none;
	}

	public static String pyro_to_string(int flag) {
		if (pyro_to_string.containsKey(flag))
			return pyro_to_string.get(flag);
		return null;
	}

	public static String pyro_to_name(int flag) {
		String		name;
		AltosUnits	units = null;
		if (!pyro_to_name.containsKey(flag))
			return null;

		name = pyro_to_name.get(flag);
		if (pyro_to_units.containsKey(flag))
			units = pyro_to_units.get(flag);
		if (units == null)
			return name;
		return String.format ("%s (%s)", name, units.show_units());
	}

	public static AltosUnits pyro_to_units(int flag) {
		if (pyro_to_units.containsKey(flag))
			return pyro_to_units.get(flag);
		return null;
	}

	public static double pyro_to_scale(int flag) {
		if (pyro_to_scale.containsKey(flag))
			return pyro_to_scale.get(flag);
		return 1.0;
	}

	private static void initialize_maps() {
		insert_map(pyro_accel_less, pyro_accel_less_string, pyro_accel_less_name, AltosConvert.accel, pyro_accel_scale);
		insert_map(pyro_accel_greater, pyro_accel_greater_string, pyro_accel_greater_name, AltosConvert.accel, pyro_accel_scale);

		insert_map(pyro_speed_less, pyro_speed_less_string, pyro_speed_less_name, AltosConvert.speed, pyro_speed_scale);
		insert_map(pyro_speed_greater, pyro_speed_greater_string, pyro_speed_greater_name, AltosConvert.speed, pyro_speed_scale);

		insert_map(pyro_height_less, pyro_height_less_string, pyro_height_less_name, AltosConvert.height, pyro_height_scale);
		insert_map(pyro_height_greater, pyro_height_greater_string, pyro_height_greater_name, AltosConvert.height, pyro_height_scale);

		insert_map(pyro_orient_less, pyro_orient_less_string, pyro_orient_less_name, null, pyro_orient_scale);
		insert_map(pyro_orient_greater, pyro_orient_greater_string, pyro_orient_greater_name, null, pyro_orient_scale);

		insert_map(pyro_time_less, pyro_time_less_string, pyro_time_less_name, null, pyro_time_scale);
		insert_map(pyro_time_greater, pyro_time_greater_string, pyro_time_greater_name, null, pyro_time_scale);

		insert_map(pyro_ascending, pyro_ascending_string, pyro_ascending_name, null, 1.0);
		insert_map(pyro_descending, pyro_descending_string, pyro_descending_name, null, 1.0);

		insert_map(pyro_after_motor, pyro_after_motor_string, pyro_after_motor_name, null, 1.0);
		insert_map(pyro_delay, pyro_delay_string, pyro_delay_name, null, pyro_delay_scale);

		insert_map(pyro_state_less, pyro_state_less_string, pyro_state_less_name, null, 1.0);
		insert_map(pyro_state_greater_or_equal, pyro_state_greater_or_equal_string, pyro_state_greater_or_equal_name, null, 1.0);
	}

	{
		initialize_maps();
	}

	public int	channel;
	public int	flags;
	public int	accel_less, accel_greater;
	public int	speed_less, speed_greater;
	public int	height_less, height_greater;
	public int	orient_less, orient_greater;
	public int	time_less, time_greater;
	public int	delay;
	public int	state_less, state_greater_or_equal;
	public int	motor;

	public AltosPyro(int in_channel) {
		channel = in_channel;
		flags = 0;
	}

	private boolean set_ivalue(int flag, int value) {
		switch (flag) {
		case pyro_accel_less:			accel_less = value; break;
		case pyro_accel_greater:		accel_greater = value; break;
		case pyro_speed_less:			speed_less = value; break;
		case pyro_speed_greater:		speed_greater = value; break;
		case pyro_height_less:			height_less = value; break;
		case pyro_height_greater:		height_greater = value; break;
		case pyro_orient_less:			orient_less = value; break;
		case pyro_orient_greater:		orient_greater = value; break;
		case pyro_time_less:			time_less = value; break;
		case pyro_time_greater:			time_greater = value; break;
		case pyro_after_motor:			motor = value; break;
		case pyro_delay:			delay = value; break;
		case pyro_state_less:			state_less = value; break;
		case pyro_state_greater_or_equal:	state_greater_or_equal = value; break;
		default:
			return false;
		}
		return true;
	}

	public boolean set_value(int flag, double dvalue) {
		return set_ivalue(flag, (int) (dvalue * pyro_to_scale(flag)));
	}

	private int get_ivalue (int flag) {
		int	value;

		switch (flag) {
		case pyro_accel_less:			value = accel_less; break;
		case pyro_accel_greater:		value = accel_greater; break;
		case pyro_speed_less:			value = speed_less; break;
		case pyro_speed_greater:		value = speed_greater; break;
		case pyro_height_less:			value = height_less; break;
		case pyro_height_greater:		value = height_greater; break;
		case pyro_orient_less:			value = orient_less; break;
		case pyro_orient_greater:		value = orient_greater; break;
		case pyro_time_less:			value = time_less; break;
		case pyro_time_greater:			value = time_greater; break;
		case pyro_after_motor:			value = motor; break;
		case pyro_delay:			value = delay; break;
		case pyro_state_less:			value = state_less; break;
		case pyro_state_greater_or_equal:	value = state_greater_or_equal; break;
		default:				value = 0; break;
		}
		return value;
	}

	public double get_value(int flag) {
		return get_ivalue(flag) / pyro_to_scale(flag);
	}

	public AltosPyro(int in_channel, String line) throws ParseException {
		String[] tokens = line.split("\\s+");

		channel = in_channel;
		flags = 0;

		int i = 0;
		if (tokens[i].equals("Pyro"))
			i += 2;

		for (; i < tokens.length; i++) {

			if (tokens[i].equals("<disabled>"))
				break;

			int	flag = string_to_pyro(tokens[i]);
			if (flag == pyro_none)
				throw new ParseException(String.format("Invalid pyro token \"%s\"",
								       tokens[i]), i);
			flags |= flag;

			if ((flag & pyro_no_value) == 0) {
				int	value = 0;
				++i;
				try {
					value = AltosLib.fromdec(tokens[i]);
				} catch (NumberFormatException n) {
					throw new ParseException(String.format("Invalid pyro value \"%s\"",
									       tokens[i]), i);
				}
				if (!set_ivalue(flag, value))
					throw new ParseException(String.format("Internal parser error \"%s\" \"%s\"",
									       tokens[i-1], tokens[i]), i-1);
			}
		}
	}

	public String toString() {
		String	ret = String.format("%d", channel);

		for (int flag = 1; flag <= flags; flag <<= 1) {
			if ((flags & flag) != 0) {
				String	add;
				if ((flag & pyro_no_value) == 0) {
					add = String.format(" %s %d",
							    pyro_to_string.get(flag),
							    get_ivalue(flag));
				} else {
					add = String.format(" %s",
							    pyro_to_string.get(flag));
				}
				ret = ret.concat(add);
			}
		}
		return ret;
	}
}
