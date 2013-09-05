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

package org.altusmetrum.altoslib_2;

import java.io.*;

public class AltosSelfFlash {
	File			file;
	FileInputStream		input;
	AltosHexfile		image;
	AltosLink		link;
	boolean			aborted;
	AltosFlashListener	listener;
	byte[]			read_block, write_block;

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

	void read_block(long addr) {
		link.printf("R %x\n", addr);
		
	}

	void read_memory(long addr, int len) {
	}
		
	void write_memory(long addr, byte[] data, int start, int len) {
		
	}

	void reboot() {
	}

	public void flash() {
		try {
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
				if (link != null) {
					reboot();
				}
			}
			if (link != null)
				link.close();
		} catch (IOException ie) {
			action(ie.getMessage(), -1);
			abort();
		} catch (InterruptedException ie) {
			abort();
		}
	}

	public void close() {
		if (link != null)
			link.close();
	}

	synchronized public void abort() {
		aborted = true;
		close();
	}

	public boolean check_rom_config() {
		if (link == null)
			return true;
		if (rom_config == null)
			rom_config = debug.romconfig();
		return rom_config != null && rom_config.valid();
	}

	public void set_romconfig (AltosRomconfig romconfig) {
		rom_config = romconfig;
	}

	public AltosRomconfig romconfig() {
		if (!check_rom_config())
			return null;
		return rom_config;
	}

	public AltosFlash(File file, AltosLink link, AltosFlashListener listener)
		throws IOException, FileNotFoundException, InterruptedException {
		this.file = file;
		this.link = link;
		this.listener = listener;
		this.read_block = new byte[256];
		this.write_block = new byte[256];
		input = new FileInputStream(file);
		image = new AltosHexfile(input);
		if (link != null) {
			debug.close();
			throw new IOException("Debug port not connected");
		}
	}
}