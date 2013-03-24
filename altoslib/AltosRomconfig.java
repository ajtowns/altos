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

package org.altusmetrum.altoslib_1;

import java.io.*;

public class AltosRomconfig {
	public boolean	valid;
	public int	version;
	public int	check;
	public int	serial_number;
	public int	radio_calibration;

	static int get_int(byte[] bytes, int start, int len) {
		int	v = 0;
		int	o = 0;
		while (len > 0) {
			v = v | ((((int) bytes[start]) & 0xff) << o);
			start++;
			len--;
			o += 8;
		}
		return v;
	}

	static void put_int(int value, byte[] bytes, int start, int len) {
		while (len > 0) {
			bytes[start] = (byte) (value & 0xff);
			start++;
			len--;
			value >>= 8;
		}
	}

	static void put_string(String value, byte[] bytes, int start) {
		for (int i = 0; i < value.length(); i++)
			bytes[start + i] = (byte) value.charAt(i);
	}

	static final int AO_USB_DESC_STRING	= 3;

	static void put_usb_serial(int value, byte[] bytes, int start) {
		int offset = start + 0xa;
		int string_num = 0;

		while (offset < bytes.length && bytes[offset] != 0) {
			if (bytes[offset + 1] == AO_USB_DESC_STRING) {
				++string_num;
				if (string_num == 4)
					break;
			}
			offset += ((int) bytes[offset]) & 0xff;
		}
		if (offset >= bytes.length || bytes[offset] == 0)
			return;
		int len = ((((int) bytes[offset]) & 0xff) - 2) / 2;
		String fmt = String.format("%%0%dd", len);

		String s = String.format(fmt, value);
		if (s.length() != len) {
			System.out.printf("weird usb length issue %s isn't %d\n",
					  s, len);
			return;
		}
		for (int i = 0; i < len; i++) {
			bytes[offset + 2 + i*2] = (byte) s.charAt(i);
			bytes[offset + 2 + i*2+1] = 0;
		}
	}

	public AltosRomconfig(byte[] bytes, int offset) {
		version = get_int(bytes, offset + 0, 2);
		check = get_int(bytes, offset + 2, 2);
		if (check == (~version & 0xffff)) {
			switch (version) {
			case 2:
			case 1:
				serial_number = get_int(bytes, offset + 4, 2);
				radio_calibration = get_int(bytes, offset + 6, 4);
				valid = true;
				break;
			}
		}
	}

	public AltosRomconfig(AltosHexfile hexfile) {
		this(hexfile.data, 0xa0 - hexfile.address);
	}

	public void write(byte[] bytes, int offset) throws IOException {
		if (!valid)
			throw new IOException("rom configuration invalid");

		if (offset < 0 || bytes.length < offset + 10)
			throw new IOException("image cannot contain rom config");

		AltosRomconfig existing = new AltosRomconfig(bytes, offset);
		if (!existing.valid)
			throw new IOException("image does not contain existing rom config");

		switch (existing.version) {
		case 2:
			put_usb_serial(serial_number, bytes, offset);
		case 1:
			put_int(serial_number, bytes, offset + 4, 2);
			put_int(radio_calibration, bytes, offset + 6, 4);
			break;
		}
	}

	public void write (AltosHexfile hexfile) throws IOException {
		write(hexfile.data, 0xa0 - hexfile.address);
		AltosRomconfig check = new AltosRomconfig(hexfile);
		if (!check.valid())
			throw new IOException("writing new rom config failed\n");
	}

	public AltosRomconfig(int in_serial_number, int in_radio_calibration) {
		valid = true;
		version = 1;
		check = (~version & 0xffff);
		serial_number = in_serial_number;
		radio_calibration = in_radio_calibration;
	}

	public boolean valid() {
		return valid && serial_number != 0;
	}

	public AltosRomconfig() {
		valid = false;
	}
}
