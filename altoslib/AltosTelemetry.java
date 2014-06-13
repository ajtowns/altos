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

import java.text.*;

/*
 * Telemetry data contents
 */

public abstract class AltosTelemetry implements AltosStateUpdate {

	/* All telemetry packets have these fields */
	public int	tick;
	public int	serial;
	public int	rssi;
	public int	status;

	/* Mark when we received the packet */
	long		received_time;

	static boolean cksum(int[] bytes) {
		int	sum = 0x5a;
		for (int i = 1; i < bytes.length - 1; i++)
			sum += bytes[i];
		sum &= 0xff;
		return sum == bytes[bytes.length - 1];
	}

	public void update_state(AltosState state) {
		state.set_serial(serial);
		if (state.state == AltosLib.ao_flight_invalid)
			state.set_state(AltosLib.ao_flight_startup);
		state.set_tick(tick);
		state.set_rssi(rssi, status);
		state.set_received_time(received_time);
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
	final static int packet_type_mega_sensor = 0x08;
	final static int packet_type_mega_data = 0x09;
	final static int packet_type_metrum_sensor = 0x0a;
	final static int packet_type_metrum_data = 0x0b;
	final static int packet_type_mini = 0x10;

	static AltosTelemetry parse_hex(String hex)  throws ParseException, AltosCRCException {
		AltosTelemetry	telem = null;

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
			telem = AltosTelemetryStandard.parse_hex(bytes);
			break;
		case AltosLib.ao_telemetry_0_9_len + 4:
			telem = new AltosTelemetryLegacy(bytes);
			break;
		case AltosLib.ao_telemetry_0_8_len + 4:
			telem = new AltosTelemetryLegacy(bytes);
			break;
		default:
			throw new ParseException(String.format("Invalid packet length %d", bytes.length), 0);
		}
		if (telem != null) {
			telem.received_time = System.currentTimeMillis();
			telem.rssi = rssi;
			telem.status = status;
		}
		return telem;
	}

	public static AltosTelemetry parse(String line) throws ParseException, AltosCRCException {
		String[] word = line.split("\\s+");
		int i =0;

		if (word[i].equals("CRC") && word[i+1].equals("INVALID")) {
			i += 2;
			AltosParse.word(word[i++], "RSSI");
			throw new AltosCRCException(AltosParse.parse_int(word[i++]));
		}

		AltosTelemetry telem;

		if (word[i].equals("TELEM")) {
			telem = parse_hex(word[i+1]);
		} else {
			telem = new AltosTelemetryLegacy(line);
		}
		return telem;
	}
}
