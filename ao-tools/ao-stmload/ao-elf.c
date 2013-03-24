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

#include "ao-elf.h"
#include <err.h>
#include <gelf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ccdbg.h"
#include "ao-stmload.h"

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

		for (s = 0; s < ao_num_symbols; s++)
			if (!strcmp (ao_symbols[s].name, symbol_name)) {
				int	t;
				ao_symbols[s].addr = sym.st_value;
				if (ao_symbols[s].required)
					++required;
			}
	}

	return required >= ao_num_required_symbols;
}

uint32_t round4(uint32_t a) {
	return (a + 3) & ~3;
}

struct hex_image *
new_load (uint32_t addr, uint32_t len)
{
	struct hex_image *new;

	len = round4(len);
	new = calloc (1, sizeof (struct hex_image) + len);
	if (!new)
		abort();

	new->address = addr;
	new->length = len;
	return new;
}

void
load_paste(struct hex_image *into, struct hex_image *from)
{
	if (from->address < into->address || into->address + into->length < from->address + from->length)
		abort();

	memcpy(into->data + from->address - into->address, from->data, from->length);
}

/*
 * Make a new load structure large enough to hold the old one and
 * the new data
 */
struct hex_image *
expand_load(struct hex_image *from, uint32_t address, uint32_t length)
{
	struct hex_image	*new;

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
struct hex_image *
load_write(struct hex_image *from, uint32_t address, uint32_t length, void *data)
{
	struct hex_image	*new;

	new = expand_load(from, address, length);
	memcpy(new->data + address - new->address, data, length);
	return new;
}

/*
 * Construct a large in-memory block for all
 * of the loaded sections of the program
 */
static struct hex_image *
get_load(Elf *e)
{
	Elf_Scn 	*scn;
	size_t		shstrndx;
	GElf_Shdr	shdr;
	Elf_Data	*data;
	char		*got_name;
	size_t		nphdr;
	size_t		p;
	GElf_Phdr	phdr;
	GElf_Addr	p_paddr;
	GElf_Off	p_offset;
	GElf_Addr	sh_paddr;
	struct hex_image	*load = NULL;
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
 * Open the specified ELF file and
 * check for the symbols we need
 */

struct hex_image *
ao_load_elf(char *name)
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
	struct hex_image	*image;

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

	image = get_load(e);
	if (!image) {
		fprintf (stderr, "Cannot create memory image from file\n");
		return NULL;
	}

	return image;
}
