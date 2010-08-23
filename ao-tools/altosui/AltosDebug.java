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
import java.util.concurrent.LinkedBlockingQueue;
import java.util.LinkedList;
import java.util.Iterator;
import altosui.AltosSerial;
import altosui.AltosRomconfig;

public class AltosDebug extends AltosSerial {

	static final byte WR_CONFIG =		0x1d;
	static final byte RD_CONFIG =		0x24;
	static final byte CONFIG_TIMERS_OFF =		(1 << 3);
	static final byte CONFIG_DMA_PAUSE =		(1 << 2);
	static final byte CONFIG_TIMER_SUSPEND =		(1 << 1);
	static final byte SET_FLASH_INFO_PAGE =		(1 << 0);

	static final byte GET_PC	=		0x28;
	static final byte READ_STATUS =		0x34;
	static final byte STATUS_CHIP_ERASE_DONE =	(byte) (1 << 7);
	static final byte STATUS_PCON_IDLE =		(1 << 6);
	static final byte STATUS_CPU_HALTED =		(1 << 5);
	static final byte STATUS_POWER_MODE_0 =		(1 << 4);
	static final byte STATUS_HALT_STATUS =		(1 << 3);
	static final byte STATUS_DEBUG_LOCKED =		(1 << 2);
	static final byte STATUS_OSCILLATOR_STABLE =	(1 << 1);
	static final byte STATUS_STACK_OVERFLOW =	(1 << 0);

	static final byte SET_HW_BRKPNT =	0x3b;
	static byte       HW_BRKPNT_N(byte n)	{ return (byte) ((n) << 3); }
	static final byte HW_BRKPNT_N_MASK =		(0x3 << 3);
	static final byte HW_BRKPNT_ENABLE =		(1 << 2);

	static final byte HALT =			0x44;
	static final byte RESUME	=		0x4c;
	static       byte DEBUG_INSTR(byte n)	{ return (byte) (0x54|(n)); }
	static final byte STEP_INSTR =		0x5c;
	static        byte STEP_REPLACE(byte n)	{ return  (byte) (0x64|(n)); }
	static final byte GET_CHIP_ID =		0x68;


	static boolean ishex(int c) {
		if ('0' <= c && c <= '9')
			return true;
		if ('a' <= c && c <= 'f')
			return true;
		if ('A' <= c && c <= 'F')
			return true;
		return false;
	}

	static boolean ishex(String s) {
		for (int i = 0; i < s.length(); i++)
			if (!ishex(s.charAt(i)))
				return false;
		return true;
	}
	static boolean isspace(int c) {
		switch (c) {
		case ' ':
		case '\t':
			return true;
		}
		return false;
	}

	static int fromhex(int c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	boolean	debug_mode;

	void ensure_debug_mode() {
		if (!debug_mode) {
			printf("D\n");
			debug_mode = true;
		}
	}

	/*
	 * Write target memory
	 */
	public void write_memory(int address, byte[] bytes) {
		ensure_debug_mode();
		printf("O %x %x\n", bytes.length, address);
		for (int i = 0; i < bytes.length; i++)
			printf("%02x", bytes[i]);
	}

	/*
	 * Read target memory
	 */
	public byte[] read_memory(int address, int length)
		throws IOException, InterruptedException {
		byte[]	data = new byte[length];

		flush_reply();
		ensure_debug_mode();
		printf("I %x %x\n", length, address);
		int i = 0;
		while (i < length) {
			String	line = get_reply().trim();
			if (!ishex(line) || line.length() % 2 != 0)
				throw new IOException(
					String.format
					("Invalid reply \"%s\"", line));
			int this_time = line.length() / 2;
			for (int j = 0; j < this_time; j++)
				data[j] = (byte) ((fromhex(line.charAt(j*2)) << 4) +
						  fromhex(line.charAt(j*2+1)));
			i += this_time;
		}

		return data;
	}

	/*
	 * Write raw bytes to the debug link using the 'P' command
	 */
	public void write_bytes(byte[] bytes) throws IOException {
		int i = 0;
		ensure_debug_mode();
		while (i < bytes.length) {
			int this_time = bytes.length - i;
			if (this_time > 8)
				this_time = 0;
			printf("P");
			for (int j = 0; j < this_time; j++)
				printf(" %02x", bytes[i+j]);
			printf("\n");
			i += this_time;
		}
	}

	public void write_byte(byte b) throws IOException {
		byte[] bytes = { b };
		write_bytes(bytes);
	}

	/*
	 * Read raw bytes from the debug link using the 'G' command
	 */
	public byte[] read_bytes(int length)
		throws IOException, InterruptedException {

		flush_reply();
		ensure_debug_mode();
		printf("G %x\n", length);
		int i = 0;
		byte[] data = new byte[length];
		while (i < length) {
			String line = get_reply().trim();
			String tokens[] = line.split("\\s+");
			for (int j = 0; j < tokens.length; j++) {
				if (!ishex(tokens[j]) ||
				    tokens[j].length() != 2)
					throw new IOException(
						String.format
						("Invalid read_bytes reply \"%s\"", line));
				try {
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

	public byte read_status() throws IOException, InterruptedException {
		write_byte(READ_STATUS);
		return read_bytes(2)[0];
	}

	public boolean check_connection() throws IOException, InterruptedException {
		byte reply = read_status();
		System.out.printf("status %x\n", reply);
		if ((reply & STATUS_CHIP_ERASE_DONE) == 0)
			return false;
		if ((reply & STATUS_PCON_IDLE) != 0)
			return false;
		if ((reply & STATUS_POWER_MODE_0) == 0)
			return false;
		return true;
	}

	public AltosRomconfig romconfig() {
		try {
			byte[] bytes = read_memory(0xa0, 10);
			return new AltosRomconfig(bytes, 0);
		} catch (IOException ie) {
		} catch (InterruptedException ie) {
		}
		return new AltosRomconfig();
	}

	/*
	 * Reset target
	 */
	public void reset() {
		printf ("R\n");
	}
}