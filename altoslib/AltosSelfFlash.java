/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

public class AltosSelfFlash extends AltosProgrammer {
	File			file;
	FileInputStream		input;
	AltosHexfile		image;
	AltosLink		link;
	boolean			aborted;
	AltosFlashListener	listener;
	AltosRomconfig		rom_config;

	void action(String s, int percent) {
		if (listener != null && !aborted)
			listener.position(s, percent);
	}

	void action(int part, int total) {
		int percent = 100 * part / total;
		action(String.format("%d/%d (%d%%)",
				     part, total, percent),
		       percent);
	}

	byte[] read_memory(long addr, int len) throws InterruptedException, IOException {
		int b;
		byte[]	data = new byte[len];

		for (int offset = 0; offset < len; offset += 0x100) {
			link.printf("R %x\n", addr + offset);
			byte[]	reply = link.get_binary_reply(5000, 0x100);

			if (reply == null)
				throw new IOException("Read device memory timeout");
			for (b = 0; b < len; b++)
				data[b+offset] = reply[b];
		}
		return data;
	}

	void write_memory(long addr, byte[] data, int start, int len) {
		int b;
		link.printf("W %x\n", addr);
		link.flush_output();
		for (b = 0; b < len; b++)
			link.putchar(data[start + b]);
		for (; b < 0x100; b++)
			link.putchar((byte) 0xff);
	}

	void reboot() {
		link.printf("a\n");
		link.flush_output();
	}

	public void flash() {
		try {
			if (!check_rom_config())
				throw new IOException("Invalid rom config settings");

			/*
			 * Store desired config values into image
			 */
			rom_config.write(image);

			int remain = image.data.length;
			long flash_addr = image.address;
			int image_start = 0;

			action("start", 0);
			action(0, image.data.length);
			while (remain > 0 && !aborted) {
				int this_time = remain;
				if (this_time > 0x100)
					this_time = 0x100;

				if (link != null) {
					/* write the data */
					write_memory(flash_addr, image.data, image_start, this_time);

					byte[] check = read_memory(flash_addr, this_time);
					for (int i = 0; i < this_time; i++)
						if (check[i] != image.data[image_start + i])
							throw new IOException(String.format("Flash write failed at 0x%x (%02x != %02x)",
											    image.address + image_start + i,
											    check[i], image.data[image_start + i]));
				} else {
					Thread.sleep(100);
				}

				remain -= this_time;
				flash_addr += this_time;
				image_start += this_time;

				action(image.data.length - remain, image.data.length);
			}
			if (!aborted) {
				action("done", 100);
			}
			close();
		} catch (IOException ie) {
			action(ie.getMessage(), -1);
			abort();
		} catch (InterruptedException ie) {
			abort();
		}
	}

	public void close() {
		if (link != null) {
			reboot();
			try {
				link.close();
			} catch (InterruptedException ie) {
			}
			link = null;
		}
	}

	synchronized public void abort() {
		aborted = true;
		close();
	}

	private AltosHexfile get_rom() throws InterruptedException {
		try {
			int base = AltosRomconfig.fetch_base(image);
			int bounds = AltosRomconfig.fetch_bounds(image);
			byte[] data = read_memory(base, bounds - base);
			AltosHexfile hexfile = new AltosHexfile(data, base);
			hexfile.add_symbols(image);
			return hexfile;
		} catch (AltosNoSymbol none) {
			return null;
		} catch (IOException ie) {
			return null;
		}

	}

	public boolean check_rom_config() throws InterruptedException {
		if (link == null) {
			return true;
		}
		if (rom_config == null) {
			AltosHexfile hexfile = get_rom();
			if (hexfile != null)
				rom_config = new AltosRomconfig(hexfile);
		}
		return rom_config != null && rom_config.valid();
	}

	public void set_romconfig (AltosRomconfig romconfig) {
		rom_config = romconfig;
	}

	public AltosRomconfig romconfig() throws InterruptedException {
		if (!check_rom_config())
			return null;
		return rom_config;
	}

	public AltosSelfFlash(File file, AltosLink link, AltosFlashListener listener)
		throws IOException, FileNotFoundException, InterruptedException {
		this.file = file;
		this.link = link;
		this.listener = listener;
		input = new FileInputStream(file);
		image = new AltosHexfile(input);
	}
}