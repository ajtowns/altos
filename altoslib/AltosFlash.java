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

public class AltosFlash extends AltosProgrammer {
	File			file;
	FileInputStream		input;
	AltosHexfile		image;
	AltosLink		link;
	AltosDebug		debug;
	AltosRomconfig		rom_config;
	boolean			aborted;
	AltosFlashListener	listener;

	static final byte MOV_direct_data	= (byte) 0x75;
	static final byte MOV_DPTR_data16	= (byte) 0x90;
	static final byte MOV_A_data		= (byte) 0x74;
	static final byte MOVX_atDPTR_A		= (byte) 0xf0;
	static final byte MOVX_A_atDPTR	        = (byte) 0xe0;
	static final byte INC_DPTR		= (byte) 0xa3;
	static final byte TRAP			= (byte) 0xa5;

	static final byte JB			= (byte) 0x20;

	static final byte MOV_A_direct		= (byte) 0xe5;
	static final byte MOV_direct1_direct2	= (byte) 0x85;
	static final byte MOV_direct_A		= (byte) 0xf5;
	static final byte MOV_R0_data		= (byte) (0x78 | 0);
	static final byte MOV_R1_data		= (byte) (0x78 | 1);
	static final byte MOV_R2_data		= (byte) (0x78 | 2);
	static final byte MOV_R3_data		= (byte) (0x78 | 3);
	static final byte MOV_R4_data		= (byte) (0x78 | 4);
	static final byte MOV_R5_data		= (byte) (0x78 | 5);
	static final byte MOV_R6_data		= (byte) (0x78 | 6);
	static final byte MOV_R7_data		= (byte) (0x78 | 7);
	static final byte DJNZ_R0_rel		= (byte) (0xd8 | 0);
	static final byte DJNZ_R1_rel		= (byte) (0xd8 | 1);
	static final byte DJNZ_R2_rel		= (byte) (0xd8 | 2);
	static final byte DJNZ_R3_rel		= (byte) (0xd8 | 3);
	static final byte DJNZ_R4_rel		= (byte) (0xd8 | 4);
	static final byte DJNZ_R5_rel		= (byte) (0xd8 | 5);
	static final byte DJNZ_R6_rel		= (byte) (0xd8 | 6);
	static final byte DJNZ_R7_rel		= (byte) (0xd8 | 7);

	static final byte P1DIR			= (byte) 0xFE;
	static final byte P1			= (byte) 0x90;

	/* flash controller */
	static final byte FWT			= (byte) 0xAB;
	static final byte FADDRL		= (byte) 0xAC;
	static final byte FADDRH		= (byte) 0xAD;
	static final byte FCTL			= (byte) 0xAE;
	static final byte FCTL_BUSY		= (byte) 0x80;
	static final byte FCTL_BUSY_BIT		= (byte) 7;
	static final byte FCTL_SWBSY		= (byte) 0x40;
	static final byte FCTL_SWBSY_BIT	= (byte) 6;
	static final byte FCTL_CONTRD		= (byte) 0x10;
	static final byte FCTL_WRITE		= (byte) 0x02;
	static final byte FCTL_ERASE		= (byte) 0x01;
	static final byte FWDATA		= (byte) 0xAF;

	static final byte ACC			= (byte) 0xE0;

	/* offsets within the flash_page program */
	static final int FLASH_ADDR_HIGH	= 8;
	static final int FLASH_ADDR_LOW		= 11;
	static final int RAM_ADDR_HIGH		= 13;
	static final int RAM_ADDR_LOW		= 14;
	static final int FLASH_WORDS_HIGH	= 16;
	static final int FLASH_WORDS_LOW	= 18;
	static final int FLASH_TIMING		= 21;

	/* sleep mode control */
	static final int SLEEP			= (byte) 0xbe;
	static final int  SLEEP_USB_EN		= (byte) 0x80;
	static final int  SLEEP_XOSC_STB	= (byte) 0x40;
	static final int  SLEEP_HFRC_STB	= (byte) 0x20;
	static final int  SLEEP_RST_MASK	= (byte) 0x18;
	static final int   SLEEP_RST_POWERON	= (byte) 0x00;
	static final int   SLEEP_RST_EXTERNAL	= (byte) 0x10;
	static final int   SLEEP_RST_WATCHDOG	= (byte) 0x08;
	static final int  SLEEP_OSC_PD		= (byte) 0x04;
	static final int  SLEEP_MODE_MASK	= (byte) 0x03;
	static final int   SLEEP_MODE_PM0	= (byte) 0x00;
	static final int   SLEEP_MODE_PM1	= (byte) 0x01;
	static final int   SLEEP_MODE_PM2	= (byte) 0x02;
	static final int   SLEEP_MODE_PM3	= (byte) 0x03;

	/* clock controller */
	static final byte CLKCON		= (byte) 0xC6;
	static final byte  CLKCON_OSC32K	= (byte) 0x80;
	static final byte  CLKCON_OSC		= (byte) 0x40;
	static final byte  CLKCON_TICKSPD	= (byte) 0x38;
	static final byte  CLKCON_CLKSPD	= (byte) 0x07;

	static final byte[] flash_page_proto = {

		MOV_direct_data, P1DIR, (byte) 0x02,
		MOV_direct_data, P1,	(byte) 0xFF,

		MOV_direct_data, FADDRH, 0,	/* FLASH_ADDR_HIGH */

		MOV_direct_data, FADDRL, 0,	/* FLASH_ADDR_LOW */

		MOV_DPTR_data16, 0, 0,		/* RAM_ADDR_HIGH, RAM_ADDR_LOW */

		MOV_R7_data, 0,			/* FLASH_WORDS_HIGH */

		MOV_R6_data, 0,			/* FLASH_WORDS_LOW */


		MOV_direct_data, FWT, 0x20,	/* FLASH_TIMING */

		MOV_direct_data, FCTL, FCTL_ERASE,
/* eraseWaitLoop: */
		MOV_A_direct,		FCTL,
		JB, ACC|FCTL_BUSY_BIT, (byte) 0xfb,

		MOV_direct_data, P1, (byte) 0xfd,

		MOV_direct_data, FCTL, FCTL_WRITE,
/* writeLoop: */
		MOV_R5_data, 2,
/* writeWordLoop: */
		MOVX_A_atDPTR,
		INC_DPTR,
		MOV_direct_A, FWDATA,
		DJNZ_R5_rel, (byte) 0xfa,		/* writeWordLoop */
/* writeWaitLoop: */
		MOV_A_direct, FCTL,
		JB, ACC|FCTL_SWBSY_BIT, (byte) 0xfb,	/* writeWaitLoop */
		DJNZ_R6_rel, (byte) 0xf1,		/* writeLoop */
		DJNZ_R7_rel, (byte) 0xef,			/* writeLoop */

		MOV_direct_data, P1DIR, (byte) 0x00,
		MOV_direct_data, P1,	(byte) 0xFF,
		TRAP,
	};

	public byte[] make_flash_page(int flash_addr, int ram_addr, int byte_count) {
		int flash_word_addr = flash_addr >> 1;
		int flash_word_count = ((byte_count + 1) >> 1);

		byte[] flash_page = new byte[flash_page_proto.length];
		for (int i = 0; i < flash_page.length; i++)
			flash_page[i] = flash_page_proto[i];

		flash_page[FLASH_ADDR_HIGH]  = (byte) (flash_word_addr >> 8);
		flash_page[FLASH_ADDR_LOW]   = (byte) (flash_word_addr);
		flash_page[RAM_ADDR_HIGH]    = (byte) (ram_addr >> 8);
		flash_page[RAM_ADDR_LOW]     = (byte) (ram_addr);

		byte flash_words_low = (byte) (flash_word_count);
		byte flash_words_high = (byte) (flash_word_count >> 8);
		/* the flashing code has a minor 'bug' */
		if (flash_words_low != 0)
			flash_words_high++;

		flash_page[FLASH_WORDS_HIGH] = (byte) flash_words_high;
		flash_page[FLASH_WORDS_LOW]  = (byte) flash_words_low;
		return flash_page;
	}

	static byte[] set_clkcon_fast = {
		MOV_direct_data, CLKCON, 0x00
	};

	static byte[] get_sleep = {
		MOV_A_direct, SLEEP
	};

	public void clock_init() throws IOException, InterruptedException {
		if (debug != null) {
			debug.debug_instr(set_clkcon_fast);

			byte	status;
			for (int times = 0; times < 20; times++) {
				Thread.sleep(1);
				status = debug.debug_instr(get_sleep);
				if ((status & SLEEP_XOSC_STB) != 0)
					return;
			}
			throw new IOException("Failed to initialize target clock");
		}
	}

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

	void altos_run(int pc) throws IOException, InterruptedException {
		debug.set_pc(pc);
		int set_pc = debug.get_pc();
		if (pc != set_pc)
			throw new IOException("Failed to set target program counter");
		debug.resume();

		for (int times = 0; times < 20; times++) {
			byte status = debug.read_status();
			if ((status & AltosDebug.STATUS_CPU_HALTED) != 0)
				return;
		}

		throw new IOException("Failed to execute program on target");
	}

	public void flash() {
		try {
			if (!check_rom_config())
				throw new IOException("Invalid rom config settings");
			if (image.address + image.data.length > 0x8000)
				throw new IOException(String.format("Flash image too long %d",
								    image.address +
								    image.data.length));
			if ((image.address & 0x3ff) != 0)
				throw new IOException(String.format("Flash image must start on page boundary (is 0x%x)",
								    image.address));
			int ram_address = 0xf000;
			int flash_prog = 0xf400;

			/*
			 * Store desired config values into image
			 */
			rom_config.write(image);
			/*
			 * Bring up the clock
			 */
			clock_init();

			int remain = image.data.length;
			int flash_addr = image.address;
			int image_start = 0;

			action("start", 0);
			action(0, image.data.length);
			while (remain > 0 && !aborted) {
				int this_time = remain;
				if (this_time > 0x400)
					this_time = 0x400;

				if (debug != null) {
					/* write the data */
					debug.write_memory(ram_address, image.data,
							   image_start, this_time);

					/* write the flash program */
					byte[] flash_page = make_flash_page(flash_addr,
									    ram_address,
									    this_time);
					debug.write_memory(flash_prog, flash_page);

					altos_run(flash_prog);
					byte[] check = debug.read_memory(flash_addr, this_time);
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
				if (debug != null) {
					debug.set_pc(image.address);
					debug.resume();
				}
			}
			if (debug != null)
				debug.close();
		} catch (IOException ie) {
			action(ie.getMessage(), -1);
			abort();
		} catch (InterruptedException ie) {
			abort();
		}
	}

	public void close() {
		if (debug != null)
			debug.close();
	}

	synchronized public void abort() {
		aborted = true;
		close();
	}

	public boolean check_rom_config() throws InterruptedException {
		if (debug == null)
			return true;
		if (rom_config == null)
			rom_config = debug.romconfig();
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

	public AltosFlash(File file, AltosLink link, AltosFlashListener listener)
		throws IOException, FileNotFoundException, InterruptedException {
		this.file = file;
		this.link = link;
		this.listener = listener;
		if (link != null)
			debug = new AltosDebug(link);
		input = new FileInputStream(file);
		image = new AltosHexfile(input);
		if (debug != null && !debug.check_connection()) {
			debug.close();
			throw new IOException("Debug port not connected");
		}
	}
}