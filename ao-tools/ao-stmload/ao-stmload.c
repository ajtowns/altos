/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "stlink-common.h"

#define AO_USB_DESC_STRING		3

struct sym {
	unsigned	addr;
	char		*name;
	int		required;
} ao_symbols[] = {

	{ 0,	"ao_romconfig_version",	1 },
#define AO_ROMCONFIG_VERSION	(ao_symbols[0].addr)

	{ 0,	"ao_romconfig_check",	1 },
#define AO_ROMCONFIG_CHECK	(ao_symbols[1].addr)

	{ 0,	"ao_serial_number", 1 },
#define AO_SERIAL_NUMBER	(ao_symbols[2].addr)

	{ 0,	"ao_usb_descriptors", 0 },
#define AO_USB_DESCRIPTORS	(ao_symbols[3].addr)

	{ 0,	"ao_radio_cal", 0 },
#define AO_RADIO_CAL		(ao_symbols[4].addr)
};

#define NUM_SYMBOLS		5
#define NUM_REQUIRED_SYMBOLS	3

/*
 * Look through the Elf file for the AltOS symbols
 * that can be adjusted before the image is written
 * to the device
 */
static int
find_symbols (Elf *e)
{
	Elf_Scn 	*scn;
	Elf_Data	*symbol_data = NULL;
	GElf_Shdr	shdr;
	GElf_Sym       	sym;
	int		i, symbol_count, s;
	int		required = 0;
	char		*symbol_name;
	char		*section_name;
	size_t		shstrndx;

	if (elf_getshdrstrndx(e, &shstrndx) < 0)
		return 0;

	/*
	 * Find the symbols
	 */

	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {

		if (gelf_getshdr(scn, &shdr) != &shdr)
			return 0;

#if 0
		section_name = elf_strptr(e, shstrndx, shdr.sh_name);

		printf ("name %s\n", section_name);

		if (shdr.sh_type == SHT_PROGBITS)
		{
			printf ("\ttype %lx\n", shdr.sh_type);
			printf ("\tflags %lx\n", shdr.sh_flags);
			printf ("\taddr %lx\n", shdr.sh_addr);
			printf ("\toffset %lx\n", shdr.sh_offset);
			printf ("\tsize %lx\n", shdr.sh_size);
			printf ("\tlink %lx\n", shdr.sh_link);
			printf ("\tinfo %lx\n", shdr.sh_info);
			printf ("\taddralign %lx\n", shdr.sh_addralign);
			printf ("\tentsize %lx\n", shdr.sh_entsize);
		}
#endif

		if (shdr.sh_type == SHT_SYMTAB) {
			symbol_data = elf_getdata(scn, NULL);
			symbol_count = shdr.sh_size / shdr.sh_entsize;
			break;
		}
	}

	if (!symbol_data)
		return 0;

	for (i = 0; i < symbol_count; i++) {
		gelf_getsym(symbol_data, i, &sym);

		symbol_name = elf_strptr(e, shdr.sh_link, sym.st_name);

		for (s = 0; s < NUM_SYMBOLS; s++)
			if (!strcmp (ao_symbols[s].name, symbol_name)) {
				int	t;
				ao_symbols[s].addr = sym.st_value;
				if (ao_symbols[s].required)
					++required;
			}
	}

	return required >= NUM_REQUIRED_SYMBOLS;
}

struct load {
	uint32_t	addr;
	uint32_t	len;
	uint8_t		buf[0];
};

uint32_t round4(uint32_t a) {
	return (a + 3) & ~3;
}

struct load *
new_load (uint32_t addr, uint32_t len)
{
	struct load *new;

	len = round4(len);
	new = calloc (1, sizeof (struct load) + len);
	if (!new)
		abort();

	new->addr = addr;
	new->len = len;
	return new;
}

void
load_paste(struct load *into, struct load *from)
{
	if (from->addr < into->addr || into->addr + into->len < from->addr + from->len)
		abort();

	memcpy(into->buf + from->addr - into->addr, from->buf, from->len);
}

/*
 * Make a new load structure large enough to hold the old one and
 * the new data
 */
struct load *
expand_load(struct load *from, uint32_t addr, uint32_t len)
{
	struct load	*new;

	if (from) {
		uint32_t	from_last = from->addr + from->len;
		uint32_t	last = addr + len;

		if (addr > from->addr)
			addr = from->addr;
		if (last < from_last)
			last = from_last;

		len = last - addr;

		if (addr == from->addr && len == from->len)
			return from;
	}
	new = new_load(addr, len);
	if (from) {
		load_paste(new, from);
		free (from);
	}
	return new;
}

/*
 * Create a new load structure with data from the existing one
 * and the new data
 */
struct load *
load_write(struct load *from, uint32_t addr, uint32_t len, void *data)
{
	struct load	*new;

	new = expand_load(from, addr, len);
	memcpy(new->buf + addr - new->addr, data, len);
	return new;
}

/*
 * Construct a large in-memory block for all
 * of the loaded sections of the program
 */
static struct load *
get_load(Elf *e)
{
	Elf_Scn 	*scn;
	size_t		shstrndx;
	GElf_Shdr	shdr;
	Elf_Data	*data;
	uint8_t		*buf;
	char		*got_name;
	size_t		nphdr;
	size_t		p;
	GElf_Phdr	phdr;
	GElf_Addr	p_paddr;
	GElf_Off	p_offset;
	GElf_Addr	sh_paddr;
	struct load	*load = NULL;
	char		*section_name;
	size_t		nshdr;
	size_t		s;
	
	if (elf_getshdrstrndx(e, &shstrndx) < 0)
		return 0;

	if (elf_getphdrnum(e, &nphdr) < 0)
		return 0;

	if (elf_getshdrnum(e, &nshdr) < 0)
		return 0;

	/*
	 * As far as I can tell, all of the phdr sections should
	 * be flashed to memory
	 */
	for (p = 0; p < nphdr; p++) {

		/* Find this phdr */
		gelf_getphdr(e, p, &phdr);

		if (phdr.p_type != PT_LOAD)
			continue;

		p_offset = phdr.p_offset;
		/* Get the associated file section */

#if 0
		printf ("offset %08x vaddr %08x paddr %08x filesz %08x memsz %08x\n",
			(uint32_t) phdr.p_offset,
			(uint32_t) phdr.p_vaddr,
			(uint32_t) phdr.p_paddr,
			(uint32_t) phdr.p_filesz,
			(uint32_t) phdr.p_memsz);
#endif
		
		for (s = 0; s < nshdr; s++) {
			scn = elf_getscn(e, s);

			if (!scn) {
				printf ("getscn failed\n");
				abort();
			}
			if (gelf_getshdr(scn, &shdr) != &shdr) {
				printf ("gelf_getshdr failed\n");
				abort();
			}

			section_name = elf_strptr(e, shstrndx, shdr.sh_name);

			if (phdr.p_offset <= shdr.sh_offset && shdr.sh_offset < phdr.p_offset + phdr.p_filesz) {
					
				if (shdr.sh_size == 0)
					continue;

				sh_paddr = phdr.p_paddr + shdr.sh_offset - phdr.p_offset;

				printf ("\tsize %08x rom %08x exec %08x %s\n",
					(uint32_t) shdr.sh_size,
					(uint32_t) sh_paddr,
					(uint32_t) shdr.sh_addr,
					section_name);

				data = elf_getdata(scn, NULL);

				/* Write the section data into the memory block */
				load = load_write(load, sh_paddr, shdr.sh_size, data->d_buf);
			}
		}
	}
	return load;
}

/*
 * Edit the to-be-written memory block
 */
static int
rewrite(struct load *load, unsigned addr, uint8_t *data, int len)
{
	int 		i;

	if (addr < load->addr || load->addr + load->len < addr + len)
		return 0;

	printf("rewrite %04x:", addr);
	for (i = 0; i < len; i++)
		printf (" %02x", load->buf[addr - load->addr + i]);
	printf(" ->");
	for (i = 0; i < len; i++)
		printf (" %02x", data[i]);
	printf("\n");
	memcpy(load->buf + addr - load->addr, data, len);
}

/*
 * Open the specified ELF file and
 * check for the symbols we need
 */

Elf *
ao_open_elf(char *name)
{
	int		fd;
	Elf		*e;
	Elf_Scn 	*scn;
	Elf_Data	*symbol_data = NULL;
	GElf_Shdr	shdr;
	GElf_Sym       	sym;
	size_t		n, shstrndx, sz;
	int		i, symbol_count, s;
	int		required = 0;

	if (elf_version(EV_CURRENT) == EV_NONE)
		return NULL;

	fd = open(name, O_RDONLY, 0);

	if (fd < 0)
		return NULL;

	e = elf_begin(fd, ELF_C_READ, NULL);

	if (!e)
		return NULL;

	if (elf_kind(e) != ELF_K_ELF)
		return NULL;

	if (elf_getshdrstrndx(e, &shstrndx) != 0)
		return NULL;

	if (!find_symbols(e)) {
		fprintf (stderr, "Cannot find required symbols\n");
		return NULL;
	}

	return e;
}

/*
 * Read a 32-bit value from the target device with arbitrary
 * alignment
 */
static uint32_t
get_uint32(stlink_t *sl, uint32_t addr)
{
	const 		uint8_t *data = sl->q_buf;
	uint32_t	actual_addr;
	int		off;
	uint32_t	result;

	sl->q_len = 0;

	printf ("read 0x%x\n", addr);

	actual_addr = addr & ~3;
	
	stlink_read_mem32(sl, actual_addr, 8);

	if (sl->q_len != 8)
		abort();

	off = addr & 3;
	result = data[off] | (data[off + 1] << 8) | (data[off+2] << 16) | (data[off+3] << 24);
	printf ("read 0x%08x = 0x%08x\n", addr, result);
	return result;
}

/*
 * Read a 16-bit value from the target device with arbitrary
 * alignment
 */
static uint16_t
get_uint16(stlink_t *sl, uint32_t addr)
{
	const 		uint8_t *data = sl->q_buf;
	uint32_t	actual_addr;
	int		off;
	uint16_t	result;

	sl->q_len = 0;


	actual_addr = addr & ~3;
	
	stlink_read_mem32(sl, actual_addr, 8);

	if (sl->q_len != 8)
		abort();

	off = addr & 3;
	result = data[off] | (data[off + 1] << 8);
	printf ("read 0x%08x = 0x%04x\n", addr, result);
	return result;
}

/*
 * Check to see if the target device has been
 * flashed with a similar firmware image before
 *
 * This is done by looking for the same romconfig version,
 * which should be at the same location as the linker script
 * places this at 0x100 from the start of the rom section
 */
static int
check_flashed(stlink_t *sl)
{
	uint16_t	romconfig_version = get_uint16(sl, AO_ROMCONFIG_VERSION);
	uint16_t	romconfig_check = get_uint16(sl, AO_ROMCONFIG_CHECK);

	if (romconfig_version != (uint16_t) ~romconfig_check) {
		fprintf (stderr, "Device has not been flashed before\n");
		return 0;
	}
	return 1;
}

static const struct option options[] = {
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "cal", .has_arg = 1, .val = 'c' },
	{ .name = "serial", .has_arg = 1, .val = 's' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--cal=<radio-cal>] [--serial=<serial>] file.elf\n", program);
	exit(1);
}

void
done(stlink_t *sl, int code)
{
	if (sl) {
		stlink_reset(sl);
		stlink_run(sl);
		stlink_exit_debug_mode(sl);
		stlink_close(sl);
	}
	exit (code);
}

int
main (int argc, char **argv)
{
	char			*device = NULL;
	char			*filename;
	Elf			*e;
	char			*serial_end;
	unsigned int		serial = 0;
	char			*serial_ucs2;
	int			serial_ucs2_len;
	char			serial_int[2];
	unsigned int		s;
	int			i;
	int			string_num;
	uint32_t		cal = 0;
	char			cal_int[4];
	char			*cal_end;
	int			c;
	stlink_t		*sl;
	int			was_flashed = 0;
	struct load		*load;
	int			tries;

	while ((c = getopt_long(argc, argv, "D:c:s:", options, NULL)) != -1) {
		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 'c':
			cal = strtoul(optarg, &cal_end, 10);
			if (cal_end == optarg || *cal_end != '\0')
				usage(argv[0]);
			break;
		case 's':
			serial = strtoul(optarg, &serial_end, 10);
			if (serial_end == optarg || *serial_end != '\0')
				usage(argv[0]);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	filename = argv[optind];
	if (filename == NULL)
		usage(argv[0]);

	/*
	 * Open the source file and load the symbols and
	 * flash data
	 */
	
	e = ao_open_elf(filename);
	if (!e) {
		fprintf(stderr, "Cannot open file \"%s\"\n", filename);
		exit(1);
	}

	if (!find_symbols(e)) {
		fprintf(stderr, "Cannot find symbols in \"%s\"\n", filename);
		exit(1);
	}

	if (!(load = get_load(e))) {
		fprintf(stderr, "Cannot find program data in \"%s\"\n", filename);
		exit(1);
	}
		
	/* Connect to the programming dongle
	 */
	
	for (tries = 0; tries < 3; tries++) {
		if (device) {
			sl = stlink_v1_open(50);
		} else {
			sl = stlink_open_usb(50);
		
		}
		if (!sl) {
			fprintf (stderr, "No STLink devices present\n");
			done (sl, 1);
		}

		if (sl->chip_id != 0)
			break;
		stlink_reset(sl);
		stlink_close(sl);
	}
	if (sl->chip_id == 0) {
		fprintf (stderr, "Debugger connection failed\n");
		done(sl, 1);
	}

	/* Verify that the loaded image fits entirely within device flash
	 */
	if (load->addr < sl->flash_base ||
	    sl->flash_base + sl->flash_size < load->addr + load->len) {
		fprintf (stderr, "\%s\": Invalid memory range 0x%08x - 0x%08x\n", filename,
			 load->addr, load->addr + load->len);
		done(sl, 1);
	}

	/* Enter debugging mode
	 */
	if (stlink_current_mode(sl) == STLINK_DEV_DFU_MODE)
		stlink_exit_dfu_mode(sl);

	if (stlink_current_mode(sl) != STLINK_DEV_DEBUG_MODE)
		stlink_enter_swd_mode(sl);

	/* Go fetch existing config values
	 * if available
	 */
	was_flashed = check_flashed(sl);

	if (!serial) {
		if (!was_flashed) {
			fprintf (stderr, "Must provide serial number\n");
			done(sl, 1);
		}
		serial = get_uint16(sl, AO_SERIAL_NUMBER);
		if (!serial || serial == 0xffff) {
			fprintf (stderr, "Invalid existing serial %d\n", serial);
			done(sl, 1);
		}
	}

	if (!cal && AO_RADIO_CAL && was_flashed) {
		cal = get_uint32(sl, AO_RADIO_CAL);
		if (!cal || cal == 0xffffffff) {
			fprintf (stderr, "Invalid existing rf cal %d\n", cal);
			done(sl, 1);
		}
	}

	/* Write the config values into the flash image
	 */

	serial_int[0] = serial & 0xff;
	serial_int[1] = (serial >> 8) & 0xff;

	if (!rewrite(load, AO_SERIAL_NUMBER, serial_int, sizeof (serial_int))) {
		fprintf(stderr, "Cannot rewrite serial integer at %08x\n",
			AO_SERIAL_NUMBER);
		done(sl, 1);
	}

	if (AO_USB_DESCRIPTORS) {
		unsigned	usb_descriptors;
		usb_descriptors = AO_USB_DESCRIPTORS - load->addr;
		string_num = 0;

		while (load->buf[usb_descriptors] != 0 && usb_descriptors < load->len) {
			if (load->buf[usb_descriptors+1] == AO_USB_DESC_STRING) {
				++string_num;
				if (string_num == 4)
					break;
			}
			usb_descriptors += load->buf[usb_descriptors];
		}
		if (usb_descriptors >= load->len || load->buf[usb_descriptors] == 0 ) {
			fprintf(stderr, "Cannot rewrite serial string at %08x\n", AO_USB_DESCRIPTORS);
			done(sl, 1);
		}

		serial_ucs2_len = load->buf[usb_descriptors] - 2;
		serial_ucs2 = malloc(serial_ucs2_len);
		if (!serial_ucs2) {
			fprintf(stderr, "Malloc(%d) failed\n", serial_ucs2_len);
			done(sl, 1);
		}
		s = serial;
		for (i = serial_ucs2_len / 2; i; i--) {
			serial_ucs2[i * 2 - 1] = 0;
			serial_ucs2[i * 2 - 2] = (s % 10) + '0';
			s /= 10;
		}
		if (!rewrite(load, usb_descriptors + 2 + load->addr, serial_ucs2, serial_ucs2_len)) {
			fprintf (stderr, "Cannot rewrite USB descriptor at %08x\n", AO_USB_DESCRIPTORS);
			done(sl, 1);
		}
	}

	if (cal && AO_RADIO_CAL) {
		cal_int[0] = cal & 0xff;
		cal_int[1] = (cal >> 8) & 0xff;
		cal_int[2] = (cal >> 16) & 0xff;
		cal_int[3] = (cal >> 24) & 0xff;

		if (!rewrite(load, AO_RADIO_CAL, cal_int, sizeof (cal_int))) {
			fprintf(stderr, "Cannot rewrite radio calibration at %08x\n", AO_RADIO_CAL);
			exit(1);
		}
	}

	/* And flash the resulting image to the device
	 */
	if (stlink_write_flash(sl, load->addr, load->buf, load->len) < 0) {
		fprintf (stderr, "\"%s\": Write failed\n", filename);
		done(sl, 1);
	}

	done(sl, 0);
}
