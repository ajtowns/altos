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

package org.altusmetrum.altoslib_4;

public abstract class AltosTelemetryStandard extends AltosTelemetry {
	int[]	bytes;
	int	type;

	public int int8(int off) {
		return AltosLib.int8(bytes, off + 1);
	}

	public int uint8(int off) {
		return AltosLib.uint8(bytes, off + 1);
	}

	public int int16(int off) {
		return AltosLib.int16(bytes, off + 1);
	}

	public int uint16(int off) {
		return AltosLib.uint16(bytes, off + 1);
	}

	public int uint32(int off) {
		return AltosLib.uint32(bytes, off + 1);
	}

	public int int32(int off) {
		return AltosLib.int32(bytes, off + 1);
	}

	public String string(int off, int l) {
		return AltosLib.string(bytes, off + 1, l);
	}

	public static AltosTelemetry parse_hex(int[] bytes) {
		int	type = AltosLib.uint8(bytes, 4 + 1);

		AltosTelemetry	telem;
		switch (type) {
		case packet_type_TM_sensor:
		case packet_type_Tm_sensor:
		case packet_type_Tn_sensor:
			telem = new AltosTelemetrySensor(bytes);
			break;
		case packet_type_configuration:
			telem = new AltosTelemetryConfiguration(bytes);
			break;
		case packet_type_location:
			telem = new AltosTelemetryLocation(bytes);
			break;
		case packet_type_satellite:
			telem = new AltosTelemetrySatellite(bytes);
			break;
/*
		case packet_type_companion:
			telem = new AltosTelemetryCompanion(bytes);
			break;
*/
		case packet_type_mega_sensor:
			telem = new AltosTelemetryMegaSensor(bytes);
			break;
		case packet_type_mega_data:
			telem = new AltosTelemetryMegaData(bytes);
			break;
		case packet_type_metrum_sensor:
			telem = new AltosTelemetryMetrumSensor(bytes);
			break;
		case packet_type_metrum_data:
			telem = new AltosTelemetryMetrumData(bytes);
			break;
		case packet_type_mini:
			telem = new AltosTelemetryMini(bytes);
			break;
		default:
			telem = new AltosTelemetryRaw(bytes);
			break;
		}
		return telem;
	}

	public AltosTelemetryStandard(int[] bytes) {
		this.bytes = bytes;

		serial = uint16(0);
		tick   = uint16(2);
		type   = uint8(4);
	}

	public void update_state(AltosState state) {
		super.update_state(state);
	}
}
