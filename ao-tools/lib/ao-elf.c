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

#include <err.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ao-elf.h"
#include "ao-hex.h"
#include "ao-verbose.h"

/*
 * Look through the Elf file for symbols that can be adjusted before
 * the image is written to the device
 */
static struct ao_sym *
load_symbols (Elf *e, int *num_symbolsp)
{
	Elf_Scn 	*scn;
	Elf_Data	*symbol_data = NULL;
	GElf_Shdr	shdr;
	GElf_Sym       	sym;
	int		i, symbol_count;
	char		*symbol_name;
	size_t		shstrndx;
	struct ao_sym	*symbols = NULL;
	struct ao_sym	*symbol;
	int		num_symbols = 0;
	int		size_symbols = 0;

	if (elf_getshdrstrndx(e, &shstrndx) < 0)
		return false;

	/*
	 * Find the symbols
	 */

	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {

		if (gelf_getshdr(scn, &shdr) != &shdr)
			return false;

		if (shdr.sh_type == SHT_SYMTAB) {
			symbol_data = elf_getdata(scn, NULL);
			symbol_count = shdr.sh_size / shdr.sh_entsize;
			break;
		}
	}

	if (!symbol_data)
		return NULL;

	for (i = 0; i < symbol_count; i++) {
		gelf_getsym(symbol_data, i, &sym);

		symbol_name = elf_strptr(e, shdr.sh_link, sym.st_name);
		if (!symbol_name[0])
			continue;

		if (num_symbols == size_symbols) {
			struct ao_sym	*new_symbols;
			int		new_size;

			if (!size_symbols)
				new_size = 16;
			else
				new_size = size_symbols * 2;
			new_symbols = realloc(symbols, new_size * sizeof (struct ao_sym));
			if (!new_symbols)
				goto bail;

			symbols = new_symbols;
			size_symbols = new_size;
		}
		symbol = &symbols[num_symbols];
		memset(symbol, 0, sizeof (struct ao_sym));
		symbol->name = strdup(symbol_name);
		if (!symbol->name)
			goto bail;
		symbol->addr = sym.st_value;
		ao_printf(AO_VERBOSE_EXE, "Add symbol %s: %08x\n", symbol->name, symbol->addr);
		num_symbols++;
	}
	*num_symbolsp = num_symbols;
	return symbols;
bail:
	for (i = 0; i < num_symbols; i++)
		free(symbols[i].name);
	free(symbols);
	return NULL;
}

static uint32_t
round4(uint32_t a) {
	return (a + 3) & ~3;
}

static struct ao_hex_image *
new_load (uint32_t addr, uint32_t len)
{
	struct ao_hex_image *new;

	len = round4(len);
	new = calloc (1, sizeof (struct ao_hex_image) + len);
	if (!new)
		abort();

	new->address = addr;
	new->length = len;
	return new;
}

static void
load_paste(struct ao_hex_image *into, struct ao_hex_image *from)
{
	if (from->address < into->address || into->address + into->length < from->address + from->length)
		abort();

	memcpy(into->data + from->address - into->address, from->data, from->length);
}

/*
 * Make a new load structure large enough to hold the old one and
 * the new data
 */
static struct ao_hex_image *
expand_load(struct ao_hex_image *from, uint32_t address, uint32_t length)
{
	struct ao_hex_image	*new;

	if (from) {
		uint32_t	from_last = from->address + from->length;
		uint32_t	last = address + length;

		if (address > from->address)
			address = from->address;
		if (last < from_last)
			last = from_last;

		length = last - address;

		if (address == from->address && length == from->length)
			return from;
	}
	new = new_load(address, length);
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
static struct ao_hex_image *
load_write(struct ao_hex_image *from, uint32_t address, uint32_t length, void *data)
{
	struct ao_hex_image	*new;

	new = expand_load(from, address, length);
	memcpy(new->data + address - new->address, data, length);
	return new;
}

/*
 * Construct a large in-memory block for all
 * of the loaded sections of the program
 */
static struct ao_hex_image *
get_load(Elf *e)
{
	Elf_Scn 	*scn;
	size_t		shstrndx;
	GElf_Shdr	shdr;
	Elf_Data	*data;
	size_t		nphdr;
	size_t		p;
	GElf_Phdr	phdr;
	GElf_Addr	sh_paddr;
	struct ao_hex_image	*load = NULL;
#if 0
	char		*section_name;
#endif
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

		/* Get the associated file section */

#if 0
		fprintf (stderr, "offset %08x vaddr %08x paddr %08x filesz %08x memsz %08x\n",
			 (uint32_t) phdr.p_offset,
			 (uint32_t) phdr.p_vaddr,
			 (uint32_t) phdr.p_paddr,
			 (uint32_t) phdr.p_filesz,
			 (uint32_t) phdr.p_memsz);
#endif
		
		for (s = 0; s < nshdr; s++) {
			scn = elf_getscn(e, s);

			if (!scn) {
				fprintf (stderr, "getscn failed\n");
				abort();
			}
			if (gelf_getshdr(scn, &shdr) != &shdr) {
				fprintf (stderr, "gelf_getshdr failed\n");
				abort();
			}

#if 0
			section_name = elf_strptr(e, shstrndx, shdr.sh_name);
#endif

			if (phdr.p_offset <= shdr.sh_offset && shdr.sh_offset < phdr.p_offset + phdr.p_filesz) {
					
				if (shdr.sh_size == 0)
					continue;

				sh_paddr = phdr.p_paddr + shdr.sh_offset - phdr.p_offset;

#if 0
				fprintf (stderr, "\tsize %08x rom %08x exec %08x %s\n",
					 (uint32_t) shdr.sh_size,
					 (uint32_t) sh_paddr,
					 (uint32_t) shdr.sh_addr,
					 section_name);
#endif

				data = elf_getdata(scn, NULL);

				/* Write the section data into the memory block */
				load = load_write(load, sh_paddr, shdr.sh_size, data->d_buf);
			}
		}
	}
	return load;
}

/*
 * Open the specified ELF file and
 * check for the symbols we need
 */

struct ao_hex_image *
ao_load_elf(char *name, struct ao_sym **symbols, int *num_symbols)
{
	int		fd;
	Elf		*e;
	size_t		shstrndx;
	struct ao_hex_image	*image;

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

	if (symbols)
		*symbols = load_symbols(e, num_symbols);

	image = get_load(e);
	if (!image) {
		fprintf (stderr, "Cannot create memory image from file\n");
		return NULL;
	}

	return image;
}
