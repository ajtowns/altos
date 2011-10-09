/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
import java.text.*;
import java.util.HashMap;

public class AltosTelemetryRecordRaw implements AltosTelemetryRecord {
	int[]	bytes;
	int	serial;
	int	tick;
	int	type;

	final static int packet_type_TM_sensor = 0x01;
	final static int packet_type_Tm_sensor = 0x02;
	final static int packet_type_Tn_sensor = 0x03;
	final static int packet_type_configuration = 0x04;
	final static int packet_type_location = 0x05;
	final static int packet_type_satellite = 0x06;
	final static int packet_type_companion = 0x07;
	
	final static int PKT_APPEND_STATUS_1_CRC_OK		= (1 << 7);
	final static int PKT_APPEND_STATUS_1_LQI_MASK		= (0x7f);
	final static int PKT_APPEND_STATUS_1_LQI_SHIFT		= 0;

	static boolean cksum(int[] bytes) {
		int	sum = 0x5a;
		for (int i = 1; i < bytes.length - 1; i++)
			sum += bytes[i];
		sum &= 0xff;
		return sum == bytes[bytes.length - 1];
	}

	public static AltosTelemetryRecord parse (String hex) throws ParseException, AltosCRCException {
		AltosTelemetryRecord	r;

		int[] bytes;
		try {
			bytes = Altos.hexbytes(hex);
		} catch (NumberFormatException ne) {
			throw new ParseException(ne.getMessage(), 0);
		}

		/* one for length, one for checksum */
		if (bytes[0] != bytes.length - 2)
			throw new ParseException(String.format("invalid length %d != %d\n",
							       bytes[0],
							       bytes.length - 2), 0);
		if (!cksum(bytes))
			throw new ParseException(String.format("invalid line \"%s\"", hex), 0);

		int	rssi = Altos.int8(bytes, bytes.length - 3) / 2 - 74;
		int	status = Altos.uint8(bytes, bytes.length - 2);

		if ((status & PKT_APPEND_STATUS_1_CRC_OK) == 0)
			throw new AltosCRCException(rssi);

		/* length, data ..., rssi, status, checksum -- 4 bytes extra */
		switch (bytes.length) {
		case Altos.ao_telemetry_standard_len + 4:
			int	type = Altos.uint8(bytes, 4 + 1);
			switch (type) {
			case packet_type_TM_sensor:
			case packet_type_Tm_sensor:
			case packet_type_Tn_sensor:
				r = new AltosTelemetryRecordSensor(bytes, rssi);
				break;
			case packet_type_configuration:
				r = new AltosTelemetryRecordConfiguration(bytes);
				break;
			case packet_type_location:
				r = new AltosTelemetryRecordLocation(bytes);
				break;
			case packet_type_satellite:
				r = new AltosTelemetryRecordSatellite(bytes);
				break;
			case packet_type_companion:
				r = new AltosTelemetryRecordCompanion(bytes);
				break;
			default:
				r = new AltosTelemetryRecordRaw(bytes);
				break;
			}
			break;
		case Altos.ao_telemetry_0_9_len + 4:
			r = new AltosTelemetryRecordLegacy(bytes, rssi, status);
			break;
		case Altos.ao_telemetry_0_8_len + 4:
			r = new AltosTelemetryRecordLegacy(bytes, rssi, status);
			break;
		default:
			throw new ParseException(String.format("Invalid packet length %d", bytes.length), 0);
		}
		return r;
	}

	public int int8(int off) {
		return Altos.int8(bytes, off + 1);
	}

	public int uint8(int off) {
		return Altos.uint8(bytes, off + 1);
	}

	public int int16(int off) {
		return Altos.int16(bytes, off + 1);
	}

	public int uint16(int off) {
		return Altos.uint16(bytes, off + 1);
	}

	public int uint32(int off) {
		return Altos.uint32(bytes, off + 1);
	}

	public String string(int off, int l) {
		return Altos.string(bytes, off + 1, l);
	}

	public AltosTelemetryRecordRaw(int[] in_bytes) {
		bytes = in_bytes;
		serial = uint16(0);
		tick   = uint16(2);
		type   = uint8(4);
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	next;
		if (previous != null)
			next = new AltosRecord(previous);
		else
			next = new AltosRecord();
		next.serial = serial;
		next.tick = tick;
		return next;
	}
}
