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

package org.altusmetrum.altoslib_1;
import java.text.*;

public abstract class AltosTelemetryRecord {

	long	received_time;
	abstract public AltosRecord update_state(AltosRecord previous);

	static boolean cksum(int[] bytes) {
		int	sum = 0x5a;
		for (int i = 1; i < bytes.length - 1; i++)
			sum += bytes[i];
		sum &= 0xff;
		return sum == bytes[bytes.length - 1];
	}

	final static int PKT_APPEND_STATUS_1_CRC_OK		= (1 << 7);
	final static int PKT_APPEND_STATUS_1_LQI_MASK		= (0x7f);
	final static int PKT_APPEND_STATUS_1_LQI_SHIFT		= 0;

	final static int packet_type_TM_sensor = 0x01;
	final static int packet_type_Tm_sensor = 0x02;
	final static int packet_type_Tn_sensor = 0x03;
	final static int packet_type_configuration = 0x04;
	final static int packet_type_location = 0x05;
	final static int packet_type_satellite = 0x06;
	final static int packet_type_companion = 0x07;
	final static int packet_type_MM_sensor = 0x08;
	final static int packet_type_MM_data = 0x09;
	
	static AltosTelemetryRecord parse_hex(String hex)  throws ParseException, AltosCRCException {
		AltosTelemetryRecord	r;

		int[] bytes;
		try {
			bytes = AltosLib.hexbytes(hex);
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

		int	rssi = AltosLib.int8(bytes, bytes.length - 3) / 2 - 74;
		int	status = AltosLib.uint8(bytes, bytes.length - 2);

		if ((status & PKT_APPEND_STATUS_1_CRC_OK) == 0)
			throw new AltosCRCException(rssi);

		/* length, data ..., rssi, status, checksum -- 4 bytes extra */
		switch (bytes.length) {
		case AltosLib.ao_telemetry_standard_len + 4:
			int	type = AltosLib.uint8(bytes, 4 + 1);
			switch (type) {
			case packet_type_TM_sensor:
			case packet_type_Tm_sensor:
			case packet_type_Tn_sensor:
				r = new AltosTelemetryRecordSensor(bytes, rssi);
				break;
			case packet_type_configuration:
				r = new AltosTelemetryRecordConfiguration(bytes, rssi);
				break;
			case packet_type_location:
				r = new AltosTelemetryRecordLocation(bytes, rssi);
				break;
			case packet_type_satellite:
				r = new AltosTelemetryRecordSatellite(bytes, rssi);
				break;
			case packet_type_companion:
				r = new AltosTelemetryRecordCompanion(bytes, rssi);
				break;
			case packet_type_MM_sensor:
				r = new AltosTelemetryRecordMegaSensor(bytes, rssi);
				break;
			case packet_type_MM_data:
				r = new AltosTelemetryRecordMegaData(bytes, rssi);
				break;
			default:
				r = new AltosTelemetryRecordRaw(bytes, rssi);
				break;
			}
			break;
		case AltosLib.ao_telemetry_0_9_len + 4:
			r = new AltosTelemetryRecordLegacy(bytes, rssi, status);
			break;
		case AltosLib.ao_telemetry_0_8_len + 4:
			r = new AltosTelemetryRecordLegacy(bytes, rssi, status);
			break;
		default:
			throw new ParseException(String.format("Invalid packet length %d", bytes.length), 0);
		}
		r.received_time = System.currentTimeMillis();
		return r;
	}

	public static AltosTelemetryRecord parse(String line) throws ParseException, AltosCRCException {
		AltosTelemetryRecord	r;

		String[] word = line.split("\\s+");
		int i =0;
		if (word[i].equals("CRC") && word[i+1].equals("INVALID")) {
			i += 2;
			AltosParse.word(word[i++], "RSSI");
			throw new AltosCRCException(AltosParse.parse_int(word[i++]));
		}

		if (word[i].equals("TELEM"))
			r = parse_hex(word[i+1]);
		else
			r = new AltosTelemetryRecordLegacy(line);
		return r;
	}
}
