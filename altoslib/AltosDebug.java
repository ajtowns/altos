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

public class AltosDebug {

	public static final byte WR_CONFIG =		0x1d;
	public static final byte RD_CONFIG =		0x24;
	public static final byte CONFIG_TIMERS_OFF =		(1 << 3);
	public static final byte CONFIG_DMA_PAUSE =		(1 << 2);
	public static final byte CONFIG_TIMER_SUSPEND =		(1 << 1);
	public static final byte SET_FLASH_INFO_PAGE =		(1 << 0);

	public static final byte GET_PC	=		0x28;
	public static final byte READ_STATUS =		0x34;
	public static final byte STATUS_CHIP_ERASE_DONE =	(byte) (1 << 7);
	public static final byte STATUS_PCON_IDLE =		(1 << 6);
	public static final byte STATUS_CPU_HALTED =		(1 << 5);
	public static final byte STATUS_POWER_MODE_0 =		(1 << 4);
	public static final byte STATUS_HALT_STATUS =		(1 << 3);
	public static final byte STATUS_DEBUG_LOCKED =		(1 << 2);
	public static final byte STATUS_OSCILLATOR_STABLE =	(1 << 1);
	public static final byte STATUS_STACK_OVERFLOW =	(1 << 0);

	public static final byte SET_HW_BRKPNT =	0x3b;
	public static byte       HW_BRKPNT_N(byte n)	{ return (byte) ((n) << 3); }
	public static final byte HW_BRKPNT_N_MASK =		(0x3 << 3);
	public static final byte HW_BRKPNT_ENABLE =		(1 << 2);

	public static final byte HALT =			0x44;
	public static final byte RESUME	=		0x4c;
	public static       byte DEBUG_INSTR(byte n)	{ return (byte) (0x54|(n)); }
	public static final byte STEP_INSTR =		0x5c;
	public static        byte STEP_REPLACE(byte n)	{ return  (byte) (0x64|(n)); }
	public static final byte GET_CHIP_ID =		0x68;


	AltosLink	link;

	boolean	debug_mode;

	void ensure_debug_mode() throws InterruptedException {
		if (!debug_mode) {
			link.printf("D\n");
			link.flush_input();
			debug_mode = true;
		}
	}

	void dump_memory(String header, int address, byte[] bytes, int start, int len) {
		System.out.printf("%s\n", header);
		for (int j = 0; j < len; j++) {
			if ((j & 15) == 0) {
				if (j != 0)
					System.out.printf("\n");
				System.out.printf ("%04x:", address + j);
			}
			System.out.printf(" %02x", bytes[start + j]);
		}
		System.out.printf("\n");
	}

	public void close() {
		try {
			link.close();
		} catch (InterruptedException ie) {
		}
	}

	/*
	 * Write target memory
	 */
	public void write_memory(int address, byte[] bytes, int start, int len) throws InterruptedException {
		ensure_debug_mode();
//		dump_memory("write_memory", address, bytes, start, len);
		link.printf("O %x %x\n", len, address);
		for (int i = 0; i < len; i++)
			link.printf("%02x", bytes[start + i]);
	}

	public void write_memory(int address, byte[] bytes) throws InterruptedException {
		write_memory(address, bytes, 0, bytes.length);
	}

	/*
	 * Read target memory
	 */
	public byte[] read_memory(int address, int length)
		throws IOException, InterruptedException {
		byte[]	data = new byte[length];

		link.flush_input();
		ensure_debug_mode();
		link.printf("I %x %x\n", length, address);
		int i = 0;
		int start = 0;
		while (i < length) {
			String	line = link.get_reply().trim();
			if (!AltosLib.ishex(line) || line.length() % 2 != 0)
				throw new IOException(
					String.format
					("Invalid reply \"%s\"", line));
			int this_time = line.length() / 2;
			for (int j = 0; j < this_time; j++)
				data[start + j] = (byte) ((AltosLib.fromhex(line.charAt(j*2)) << 4) +
						  AltosLib.fromhex(line.charAt(j*2+1)));
			start += this_time;
			i += this_time;
		}
//		dump_memory("read_memory", address, data, 0, length);

		return data;
	}

	/*
	 * Write raw bytes to the debug link using the 'P' command
	 */
	public void write_bytes(byte[] bytes) throws IOException, InterruptedException {
		int i = 0;
		ensure_debug_mode();
		while (i < bytes.length) {
			int this_time = bytes.length - i;
			if (this_time > 8)
				this_time = 0;
			link.printf("P");
			for (int j = 0; j < this_time; j++)
				link.printf(" %02x", bytes[i+j]);
			link.printf("\n");
			i += this_time;
		}
	}

	public void write_byte(byte b) throws IOException, InterruptedException {
		byte[] bytes = { b };
		write_bytes(bytes);
	}

	/*
	 * Read raw bytes from the debug link using the 'G' command
	 */
	public byte[] read_bytes(int length)
		throws IOException, InterruptedException {

		link.flush_input();
		ensure_debug_mode();
		link.printf("G %x\n", length);
		int i = 0;
		byte[] data = new byte[length];
		while (i < length) {
			String line = link.get_reply();

			if (line == null)
				throw new IOException("Timeout in read_bytes");
			line = line.trim();
			String tokens[] = line.split("\\s+");
			for (int j = 0; j < tokens.length; j++) {
				if (!AltosLib.ishex(tokens[j]) ||
				    tokens[j].length() != 2)
					throw new IOException(
						String.format
						("Invalid read_bytes reply \"%s\"", line));
				try {
					if (i + j >= length)
						throw new IOException(
							String.format
							("Invalid read_bytes reply \"%s\"", line));
					else
						data[i + j] = (byte) Integer.parseInt(tokens[j], 16);
				} catch (NumberFormatException ne) {
					throw new IOException(
						String.format
						("Invalid read_bytes reply \"%s\"", line));
				}
			}
			i += tokens.length;
		}
		return data;
	}

	public byte read_byte() throws IOException, InterruptedException {
		return read_bytes(1)[0];
	}

	public byte debug_instr(byte[] instruction) throws IOException, InterruptedException {
		byte[] command = new byte[1 + instruction.length];
		command[0] = DEBUG_INSTR((byte) instruction.length);
		for (int i = 0; i < instruction.length; i++)
			command[i+1] = instruction[i];
		write_bytes(command);
		return read_byte();
	}

	public byte resume() throws IOException, InterruptedException {
		write_byte(RESUME);
		return read_byte();
	}

	public int read_uint16() throws IOException, InterruptedException {
		byte[] d = read_bytes(2);
		return ((int) (d[0] & 0xff) << 8) | (d[1] & 0xff);
	}

	public int read_uint8()  throws IOException, InterruptedException {
		byte[] d = read_bytes(1);
		return (int) (d[0] & 0xff);
	}

	public int get_chip_id() throws IOException, InterruptedException {
		write_byte(GET_CHIP_ID);
		return read_uint16();
	}

	public int get_pc() throws IOException, InterruptedException {
		write_byte(GET_PC);
		return read_uint16();
	}

	public byte read_status() throws IOException, InterruptedException {
		write_byte(READ_STATUS);
		return read_byte();
	}

	static final byte LJMP			= 0x02;

	public void set_pc(int pc) throws IOException, InterruptedException {
		byte high = (byte) (pc >> 8);
		byte low = (byte) pc;
		byte[] jump_mem = { LJMP, high, low };
		debug_instr(jump_mem);
	}

	public boolean check_connection() throws IOException, InterruptedException {
		byte reply = read_status();
		if ((reply & STATUS_CHIP_ERASE_DONE) == 0)
			return false;
		if ((reply & STATUS_PCON_IDLE) != 0)
			return false;
		if ((reply & STATUS_POWER_MODE_0) == 0)
			return false;
		return true;
	}

	public AltosRomconfig romconfig() throws InterruptedException {
		try {
			byte[] bytes = read_memory(0xa0, 10);
			AltosHexfile hexfile = new AltosHexfile (bytes, 0xa0);
			return new AltosRomconfig(hexfile);
		} catch (IOException ie) {
		}
		return new AltosRomconfig();
	}

	/*
	 * Reset target
	 */
	public void reset() {
		link.printf ("R\n");
	}

	public AltosDebug (AltosLink link) {
		this.link = link;
	}
}