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
import java.util.LinkedList;
import java.util.Arrays;

class HexFileInputStream extends PushbackInputStream {
	public int line;

	public HexFileInputStream(FileInputStream o) {
		super(new BufferedInputStream(o));
		line = 1;
	}

	public int read() throws IOException {
		int	c = super.read();
		if (c == '\n')
			line++;
		return c;
	}

	public void unread(int c) throws IOException {
		if (c == '\n')
			line--;
		if (c != -1)
			super.unread(c);
	}
}

class HexRecord implements Comparable<Object> {
	public int	address;
	public int	type;
	public byte	checksum;
	public byte[]	data;

	static final int NORMAL = 0;
	static final int EOF = 1;
	static final int EXTENDED_ADDRESS = 2;

	enum read_state {
		marker,
		length,
		address,
		type,
		data,
		checksum,
		newline,
		white,
		done,
	}

	boolean ishex(int c) {
		if ('0' <= c && c <= '9')
			return true;
		if ('a' <= c && c <= 'f')
			return true;
		if ('A' <= c && c <= 'F')
			return true;
		return false;
	}

	boolean isspace(int c) {
		switch (c) {
		case ' ':
		case '\t':
			return true;
		}
		return false;
	}

	int fromhex(int c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	public byte checksum() {
		byte	got = 0;

		got += data.length;
		got += (address >> 8) & 0xff;
		got += (address     ) & 0xff;
		got += type;
		for (int i = 0; i < data.length; i++)
			got += data[i];
		return (byte) (-got);
	}

	public int compareTo(Object other) {
		HexRecord	o = (HexRecord) other;
		return address - o.address;
	}

	public String toString() {
		return String.format("%04x: %02x (%d)", address, type, data.length);
	}

	public HexRecord(HexFileInputStream input) throws IOException {
		read_state	state = read_state.marker;
		int		nhexbytes = 0;
		int		hex = 0;
		int		ndata = 0;
		byte		got_checksum;

		while (state != read_state.done) {
			int c = input.read();
			if (c < 0 && state != read_state.white)
				throw new IOException(String.format("%d: Unexpected EOF", input.line));
			if (c == ' ')
				continue;
			switch (state) {
			case marker:
				if (c != ':')
					throw new IOException("Missing ':'");
				state = read_state.length;
				nhexbytes = 2;
				hex = 0;
				break;
			case length:
			case address:
			case type:
			case data:
			case checksum:
				if(!ishex(c))
					throw new IOException(String.format("Non-hex char '%c'", c));
				hex = hex << 4 | fromhex(c);
				--nhexbytes;
				if (nhexbytes != 0)
					break;

				switch (state) {
				case length:
					data = new byte[hex];
					state = read_state.address;
					nhexbytes = 4;
					break;
				case address:
					address = hex;
					state = read_state.type;
					nhexbytes = 2;
					break;
				case type:
					type = hex;
					if (data.length > 0)
						state = read_state.data;
					else
						state = read_state.checksum;
					nhexbytes = 2;
					ndata = 0;
					break;
				case data:
					data[ndata] = (byte) hex;
					ndata++;
					nhexbytes = 2;
					if (ndata == data.length)
						state = read_state.checksum;
					break;
				case checksum:
					checksum = (byte) hex;
					state = read_state.newline;
					break;
				default:
					break;
				}
				hex = 0;
				break;
			case newline:
				if (c != '\n' && c != '\r')
					throw new IOException("Missing newline");
				state = read_state.white;
				break;
			case white:
				if (!isspace(c)) {
					input.unread(c);
					state = read_state.done;
				}
				break;
			case done:
				break;
			}
		}
		got_checksum = checksum();
		if (got_checksum != checksum)
			throw new IOException(String.format("Invalid checksum (read 0x%02x computed 0x%02x)\n",
							    checksum, got_checksum));
	}
}

public class AltosHexfile {
	public int	address;
	public byte[]	data;

	public byte get_byte(int a) {
		return data[a - address];
	}

	public AltosHexfile(FileInputStream file) throws IOException {
		HexFileInputStream	input = new HexFileInputStream(file);
		LinkedList<HexRecord>	record_list = new LinkedList<HexRecord>();
		boolean			done = false;

		while (!done) {
			HexRecord	record = new HexRecord(input);

			if (record.type == HexRecord.EOF)
				done = true;
			else
				record_list.add(record);
		}

		long	extended_addr = 0;
		long	base = 0xffffffff;
		long	bound = 0x00000000;
		for (HexRecord record : record_list) {
			switch (record.type) {
			case 0:
				long addr = extended_addr + record.address;
				long r_bound = addr + record.data.length;
				if (addr < base)
					base = addr;
				if (r_bound > bound)
					bound = r_bound;
				break;
			case 1:
				break;
			case 2:
				if (record.data.length != 2)
					throw new IOException("invalid extended segment address record");
				extended_addr = ((record.data[0] << 8) + (record.data[1])) << 4;
				break;
			case 4:
				if (record.data.length != 2)
					throw new IOException("invalid extended segment address record");
				extended_addr = ((record.data[0] << 8) + (record.data[1])) << 16;
				break;
			default:
				throw new IOException ("invalid hex record type");
			}
		}

		if (base >= bound)
			throw new IOException("invalid hex file");

		if (bound - base > 4 * 1024 * 1024)
			throw new IOException("hex file too large");

		data = new byte[(int) (bound - base)];
		address = (int) base;
		Arrays.fill(data, (byte) 0xff);

		/* Paint the records into the new array */
		for (HexRecord record : record_list) {
			switch (record.type) {
			case 0:
				long addr = extended_addr + record.address;
				long r_bound = addr + record.data.length;
				for (int j = 0; j < record.data.length; j++)
					data[(int) (addr - base) + j] = record.data[j];
				break;
			case 1:
				break;
			case 2:
				if (record.data.length != 2)
					throw new IOException("invalid extended segment address record");
				extended_addr = ((record.data[0] << 8) + (record.data[1])) << 4;
				break;
			case 4:
				if (record.data.length != 2)
					throw new IOException("invalid extended segment address record");
				extended_addr = ((record.data[0] << 8) + (record.data[1])) << 16;
				break;
			default:
				throw new IOException ("invalid hex record type");
			}
		}
	}
}