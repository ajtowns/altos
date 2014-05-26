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

package org.altusmetrum.altoslib_4;

import java.io.*;

public class AltosKML implements AltosWriter {

	File			name;
	PrintStream		out;
	int			flight_state = -1;
	AltosState		prev = null;
	double			gps_start_altitude;

	static final String[] kml_state_colors = {
		"FF000000",
		"FF000000",
		"FF000000",
		"FF0000FF",
		"FF4080FF",
		"FF00FFFF",
		"FFFF0000",
		"FF00FF00",
		"FF000000",
		"FFFFFFFF"
	};

	static final String kml_header_start =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
		"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n" +
		"<Document>\n" +
		"  <name>AO Flight#%d S/N: %03d</name>\n" +
		"  <description>\n";
	static final String kml_header_end =
		"  </description>\n" +
		"  <open>0</open>\n";

	static final String kml_style_start =
		"  <Style id=\"ao-flightstate-%s\">\n" +
		"    <LineStyle><color>%s</color><width>4</width></LineStyle>\n" +
		"    <BalloonStyle>\n" +
		"      <text>\n";

	static final String kml_style_end =
		"      </text>\n" +
		"    </BalloonStyle>\n" +
		"  </Style>\n";

	static final String kml_placemark_start =
		"  <Placemark>\n" +
		"    <name>%s</name>\n" +
		"    <styleUrl>#ao-flightstate-%s</styleUrl>\n" +
		"    <LineString>\n" +
		"      <tessellate>1</tessellate>\n" +
		"      <altitudeMode>absolute</altitudeMode>\n" +
		"      <coordinates>\n";

	static final String kml_coord_fmt =
	"        %.7f,%.7f,%.7f <!-- alt %12.7f time %12.7f sats %d -->\n";

	static final String kml_placemark_end =
		"      </coordinates>\n" +
		"    </LineString>\n" +
		"  </Placemark>\n";

	static final String kml_footer =
		"</Document>\n" +
		"</kml>\n";

	void start (AltosState record) {
		out.printf(kml_header_start, record.flight, record.serial);
		out.printf("Date:   %04d-%02d-%02d\n",
			   record.gps.year, record.gps.month, record.gps.day);
		out.printf("Time:     %2d:%02d:%02d\n",
			   record.gps.hour, record.gps.minute, record.gps.second);
		out.printf("%s", kml_header_end);
	}

	boolean	started = false;

	void state_start(AltosState state) {
		String	state_name = AltosLib.state_name(state.state);
		out.printf(kml_style_start, state_name, kml_state_colors[state.state]);
		out.printf("\tState: %s\n", state_name);
		out.printf("%s", kml_style_end);
		out.printf(kml_placemark_start, state_name, state_name);
	}

	void state_end(AltosState state) {
		out.printf("%s", kml_placemark_end);
	}

	void coord(AltosState state) {
		AltosGPS	gps = state.gps;
		double		altitude;

		if (state.height() != AltosLib.MISSING)
			altitude = state.height() + gps_start_altitude;
		else
			altitude = gps.alt;
		out.printf(kml_coord_fmt,
			   gps.lon, gps.lat,
			   altitude, (double) gps.alt,
			   state.time, gps.nsat);
	}

	void end() {
		out.printf("%s", kml_footer);
	}

	public void close() {
		if (prev != null) {
			state_end(prev);
			end();
			prev = null;
		}
	}

	public void write(AltosState state) {
		AltosGPS	gps = state.gps;

		if (gps == null)
			return;

		if (gps.lat == AltosLib.MISSING)
			return;
		if (gps.lon == AltosLib.MISSING)
			return;
		if (!started) {
			start(state);
			started = true;
			gps_start_altitude = gps.alt;
		}
		if (prev != null && prev.gps_sequence == state.gps_sequence)
			return;
		if (state.state != flight_state) {
			flight_state = state.state;
			if (prev != null) {
				coord(state);
				state_end(prev);
			}
			state_start(state);
		}
		coord(state);
		prev = state;
	}

	public void write(AltosStateIterable states) {
		for (AltosState state : states) {
			if ((state.set & AltosState.set_gps) != 0)
				write(state);
		}
	}

	public AltosKML(File in_name) throws FileNotFoundException {
		name = in_name;
		out = new PrintStream(name);
	}

	public AltosKML(String in_string) throws FileNotFoundException {
		this(new File(in_string));
	}
}
