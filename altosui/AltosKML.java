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

package altosui;

import java.lang.*;
import java.io.*;
import java.text.*;
import java.util.*;
import org.altusmetrum.AltosLib.*;

public class AltosKML implements AltosWriter {

	File			name;
	PrintStream		out;
	int			state = -1;
	AltosRecord		prev = null;

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

	void start (AltosRecord record) {
		out.printf(kml_header_start, record.flight, record.serial);
		out.printf("Date:   %04d-%02d-%02d\n",
			   record.gps.year, record.gps.month, record.gps.day);
		out.printf("Time:     %2d:%02d:%02d\n",
			   record.gps.hour, record.gps.minute, record.gps.second);
		out.printf("%s", kml_header_end);
	}

	boolean	started = false;

	void state_start(AltosRecord record) {
		String	state_name = Altos.state_name(record.state);
		out.printf(kml_style_start, state_name, kml_state_colors[record.state]);
		out.printf("\tState: %s\n", state_name);
		out.printf("%s", kml_style_end);
		out.printf(kml_placemark_start, state_name, state_name);
	}

	void state_end(AltosRecord record) {
		out.printf("%s", kml_placemark_end);
	}

	void coord(AltosRecord record) {
		AltosGPS	gps = record.gps;
		out.printf(kml_coord_fmt,
			   gps.lon, gps.lat,
			   record.filtered_altitude(), (double) gps.alt,
			   record.time, gps.nsat);
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

	public void write(AltosRecord record) {
		AltosGPS	gps = record.gps;

		if (gps == null)
			return;

		if ((record.seen & (AltosRecord.seen_flight)) == 0)
			return;
		if ((record.seen & (AltosRecord.seen_gps_lat)) == 0)
			return;
		if ((record.seen & (AltosRecord.seen_gps_lon)) == 0)
			return;
		if (!started) {
			start(record);
			started = true;
		}
		if (prev != null &&
		    prev.gps.second == record.gps.second &&
		    prev.gps.minute == record.gps.minute &&
		    prev.gps.hour == record.gps.hour)
			return;
		if (record.state != state) {
			state = record.state;
			if (prev != null) {
				coord(record);
				state_end(prev);
			}
			state_start(record);
		}
		coord(record);
		prev = record;
	}

	public void write(AltosRecordIterable iterable) {
		for (AltosRecord record : iterable)
			write(record);
	}

	public AltosKML(File in_name) throws FileNotFoundException {
		name = in_name;
		out = new PrintStream(name);
	}

	public AltosKML(String in_string) throws FileNotFoundException {
		this(new File(in_string));
	}
}
