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

package org.altusmetrum.micropeak;

import java.lang.*;
import java.io.*;
import java.util.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

class MicroIterator implements Iterator<MicroDataPoint> {
	int		i;
	MicroData	data;

	public boolean hasNext() {
		return i < data.pressures.length;
	}

	public MicroDataPoint next() {
		return new MicroDataPoint(data, i++);
	}

	public MicroIterator (MicroData data) {
		this.data = data;
		i = 0;
	}

	public void remove() {
	}
}

class MicroIterable implements Iterable<MicroDataPoint> {

	MicroData	data;

	public Iterator<MicroDataPoint> iterator() {
		return new MicroIterator(data);
	}

	public MicroIterable(MicroData data) {
		this.data = data;
	}
}

class MicroUIIterator implements Iterator<AltosUIDataPoint> {
	int		i;
	MicroData	data;

	public boolean hasNext() {
		return i < data.pressures.length;
	}

	public AltosUIDataPoint next() {
		return new MicroDataPoint(data, i++);
	}

	public MicroUIIterator (MicroData data) {
		this.data = data;
		i = 0;
	}

	public void remove() {
	}
}

class MicroUIIterable implements Iterable<AltosUIDataPoint> {
	MicroData	data;

	public Iterator<AltosUIDataPoint> iterator() {
		return new MicroUIIterator(data);
	}

	public MicroUIIterable(MicroData data) {
		this.data = data;
	}
}

public class MicroData implements AltosUIDataSet {
	public int		ground_pressure;
	public int		min_pressure;
	public int[]		pressures;
	private double		time_step;
	private double		ground_altitude;
	private ArrayList<Integer>	bytes;
	String			name;
	MicroStats		stats;
	
	public class FileEndedException extends Exception {
	}

	public class NonHexcharException extends Exception {
	}

	public class InvalidCrcException extends Exception {
	}

	private int getc(InputStream f) throws IOException, FileEndedException {
		int	c = f.read();

		if (c == -1)
			throw new FileEndedException();
		bytes.add(c);
		return c;
	}

	private int get_nonwhite(InputStream f) throws IOException, FileEndedException {
		int	c;

		for (;;) {
			c = getc(f);
			if (!Character.isWhitespace(c))
				return c;
		}
	}

	private int get_hexc(InputStream f) throws IOException, FileEndedException, NonHexcharException {
		int	c = get_nonwhite(f);

		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		throw new NonHexcharException();
	}

	private static final int POLY = 0x8408;

	private int log_crc(int crc, int b) {
		int	i;

		for (i = 0; i < 8; i++) {
			if (((crc & 0x0001) ^ (b & 0x0001)) != 0)
				crc = (crc >> 1) ^ POLY;
			else
				crc = crc >> 1;
			b >>= 1;
		}
		return crc & 0xffff;
	}

	int	file_crc;

	private int get_hex(InputStream f) throws IOException, FileEndedException, NonHexcharException {
		int	a = get_hexc(f);
		int	b = get_hexc(f);

		int h = (a << 4) + b;

		file_crc = log_crc(file_crc, h);
		return h;
	}

	private boolean find_header(InputStream f) throws IOException, FileEndedException {
		for (;;) {
			if (get_nonwhite(f) == 'M' && get_nonwhite(f) == 'P')
				return true;
		}
	} 

	private int get_32(InputStream f)  throws IOException, FileEndedException, NonHexcharException {
		int	v = 0;
		for (int i = 0; i < 4; i++) {
			v += get_hex(f) << (i * 8);
		}
		return v;
	}

	private int get_16(InputStream f) throws IOException, FileEndedException, NonHexcharException {
		int	v = 0;
		for (int i = 0; i < 2; i++) {
			v += get_hex(f) << (i * 8);
		}
		return v;
	}

	private int swap16(int i) {
		return ((i << 8) & 0xff00) | ((i >> 8) & 0xff);
	}

	public boolean	crc_valid;

	int mix_in (int high, int low) {
		return  high - (high & 0xffff) + low;
	}

	boolean closer (int target, int a, int b) {
		return Math.abs (target - a) < Math.abs(target - b);
	}

	public double altitude(int i) {
		return AltosConvert.pressure_to_altitude(pressures[i]);
	}

	public String name() {
		return name;
	}

	public Iterable<AltosUIDataPoint> dataPoints() {
		return new MicroUIIterable(this);
	}

	public Iterable<MicroDataPoint> points() {
		return new MicroIterable(this);
	}

	int fact(int n) {
		if (n == 0)
			return 1;
		return n * fact(n-1);
	}

	int choose(int n, int k) {
		return fact(n) / (fact(k) * fact(n-k));
	}


	public double avg_altitude(int center, int dist) {
		int	start = center - dist;
		int	stop = center + dist;

		if (start < 0)
			start = 0;
		if (stop >= pressures.length)
			stop = pressures.length - 1;

		double	sum = 0;
		double	div = 0;

		int	n = dist * 2;

		for (int i = start; i <= stop; i++) {
			int	k = i - (center - dist);
			int	c = choose (n, k);

			sum += c * pressures[i];
			div += c;
		}

		double pres = sum / div;

		double alt = AltosConvert.pressure_to_altitude(pres);
		return alt;
	}

	public double pressure(int i) {
		return pressures[i];
	}

	public double height(int i) {
		return altitude(i) - ground_altitude;
	}

	public double apogee_pressure() {
		return min_pressure;
	}

	public double apogee_altitude() {
		return AltosConvert.pressure_to_altitude(apogee_pressure());
	}

	public double apogee_height() {
		return apogee_altitude() - ground_altitude;
	}

	static final int speed_avg = 3;
	static final int accel_avg = 5;

	private double avg_speed(int center, int dist) {
		if (center == 0)
			return 0;

		double ai = avg_altitude(center, dist);
		double aj = avg_altitude(center - 1, dist);
		double s = (ai - aj) / time_step;

		return s;
	}

	public double speed(int i) {
		return avg_speed(i, speed_avg);
	}

	public double acceleration(int i) {
		if (i == 0)
			return 0;
		return (avg_speed(i, accel_avg) - avg_speed(i-1, accel_avg)) / time_step;
	}

	public double time(int i) {
		return i * time_step;
	}

	public void save (OutputStream f) throws IOException {
		for (int c : bytes)
			f.write(c);
		f.write('\n');
	}

	public void export (Writer f) throws IOException {
		PrintWriter	pw = new PrintWriter(f);
		pw.printf("  Time, Press(Pa), Height(m), Height(f), Speed(m/s), Speed(mph), Speed(mach), Accel(m/s²), Accel(ft/s²),  Accel(g)\n");
		for (MicroDataPoint point : points()) {
			pw.printf("%6.3f,%10.0f,%10.1f,%10.1f,%11.2f,%11.2f,%12.4f,%12.2f,%13.2f,%10.4f\n",
				  point.time,
				  point.pressure,
				  point.height,
				  AltosConvert.meters_to_feet(point.height),
				  point.speed,
				  AltosConvert.meters_to_mph(point.speed),
				  AltosConvert.meters_to_mach(point.speed),
				  point.accel,
				  AltosConvert.meters_to_feet(point.accel),
				  AltosConvert.meters_to_g(point.accel));
		}
	}

	public void set_name(String name) {
		this.name = name;
	}

	public MicroData (InputStream f, String name) throws IOException, InterruptedException, NonHexcharException, FileEndedException {
		this.name = name;
		bytes = new ArrayList<Integer>();
		if (!find_header(f))
			throw new IOException("No MicroPeak data header found");
		try {
			file_crc = 0xffff;
			ground_pressure = get_32(f);
			min_pressure = get_32(f);
			int nsamples = get_16(f);
			pressures = new int[nsamples + 1];

			ground_altitude = AltosConvert.pressure_to_altitude(ground_pressure);
			int cur = ground_pressure;
			pressures[0] = cur;
			for (int i = 0; i < nsamples; i++) {
				int	k = get_16(f);
				int	same = mix_in(cur, k);
				int	up = mix_in(cur + 0x10000, k);
				int	down = mix_in(cur - 0x10000, k);

				if (closer (cur, same, up)) {
					if (closer (cur, same, down))
						cur = same;
					else
						cur = down;
				} else {
					if (closer (cur, up, down))
						cur = up;
					else
						cur = down;
				}
				
				pressures[i+1] = cur;
			}

			int current_crc = swap16(~file_crc & 0xffff);
			int crc = get_16(f);

			crc_valid = crc == current_crc;

			time_step = 0.192;
			stats = new MicroStats(this);
		} catch (FileEndedException fe) {
			throw new IOException("File Ended Unexpectedly");
		}
	}

	public MicroData() {
		ground_pressure = 101000;
		min_pressure = 101000;
		pressures = new int[1];
		pressures[0] = 101000;
	}
	
}
