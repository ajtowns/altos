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

#include <ao.h>
#include <ao_mpu.h>

static uint32_t	stm_mpu_dregion;

void
ao_mpu_init(void)
{
	uint32_t	region;

	/* Check to see how many regions we have */
	stm_mpu_dregion = (stm_mpu.typer >> STM_MPU_TYPER_DREGION) & STM_MPU_TYPER_DREGION_MASK;

	/* No MPU at all */
	if (stm_mpu_dregion == 0)
		return;

	/* Disable MPU */
	stm_mpu.cr = ((0 << STM_MPU_CR_PRIVDEFENA) |
		      (0 << STM_MPU_CR_HFNMIENA) |
		      (0 << STM_MPU_CR_ENABLE));

	/* Disable all regions */
	for (region = 0; region < stm_mpu_dregion; region++) {
		/* Set the base address and RNR value */
		stm_mpu.rbar = ((0 & (STM_MPU_RBAR_ADDR_MASK << STM_MPU_RBAR_ADDR)) |
				(1 << STM_MPU_RBAR_VALID) |
				(region << STM_MPU_RBAR_REGION));

		/* Disable this region */
		stm_mpu.rasr = 0;
	}

	region = 0;

	/* Flash */
	/* 0x00000000 - 0x1fffffff */
	stm_mpu.rbar = (0x0000000 |
			(1 << STM_MPU_RBAR_VALID) |
			(region << STM_MPU_RBAR_REGION));

	stm_mpu.rasr = ((0 << STM_MPU_RASR_XN) |
			(STM_MPU_RASR_AP_RO_RO << STM_MPU_RASR_AP) |
			(5 << STM_MPU_RASR_TEX) |
			(0 << STM_MPU_RASR_C) |
			(1 << STM_MPU_RASR_B) |
			(0 << STM_MPU_RASR_S) |
			(0 << STM_MPU_RASR_SRD) |
			(28 << STM_MPU_RASR_SIZE) |
			(1 << STM_MPU_RASR_ENABLE));
	region++;

	/* Ram */
	/* 0x20000000 - 0x3fffffff */
	stm_mpu.rbar = (0x20000000 |
			(1 << STM_MPU_RBAR_VALID) |
			(region << STM_MPU_RBAR_REGION));

	stm_mpu.rasr = ((0 << STM_MPU_RASR_XN) |
			(STM_MPU_RASR_AP_RW_RW << STM_MPU_RASR_AP) |
			(5 << STM_MPU_RASR_TEX) |
			(0 << STM_MPU_RASR_C) |
			(1 << STM_MPU_RASR_B) |
			(0 << STM_MPU_RASR_S) |
			(0 << STM_MPU_RASR_SRD) |
			(28 << STM_MPU_RASR_SIZE) |
			(1 << STM_MPU_RASR_ENABLE));
	region++;

	/* Peripherals */

	/* 0x4000000 - 0x7ffffff */
	stm_mpu.rbar = (0x40000000 |
			(1 << STM_MPU_RBAR_VALID) |
			(region << STM_MPU_RBAR_REGION));

	stm_mpu.rasr = ((1 << STM_MPU_RASR_XN) |
			(STM_MPU_RASR_AP_RW_RW << STM_MPU_RASR_AP) |
			(2 << STM_MPU_RASR_TEX) |
			(0 << STM_MPU_RASR_C) |
			(0 << STM_MPU_RASR_B) |
			(0 << STM_MPU_RASR_S) |
			(0 << STM_MPU_RASR_SRD) |
			(29 << STM_MPU_RASR_SIZE) |
			(1 << STM_MPU_RASR_ENABLE));
	region++;

	/* 0x8000000 - 0xffffffff */
	stm_mpu.rbar = (0x80000000 |
			(1 << STM_MPU_RBAR_VALID) |
			(region << STM_MPU_RBAR_REGION));

	stm_mpu.rasr = ((1 << STM_MPU_RASR_XN) |
			(STM_MPU_RASR_AP_RW_RW << STM_MPU_RASR_AP) |
			(2 << STM_MPU_RASR_TEX) |
			(0 << STM_MPU_RASR_C) |
			(0 << STM_MPU_RASR_B) |
			(0 << STM_MPU_RASR_S) |
			(0 << STM_MPU_RASR_SRD) |
			(30 << STM_MPU_RASR_SIZE) |
			(1 << STM_MPU_RASR_ENABLE));
	region++;

	/* Enable MPU */
	stm_mpu.cr = ((0 << STM_MPU_CR_PRIVDEFENA) |
		      (0 << STM_MPU_CR_HFNMIENA) |
		      (1 << STM_MPU_CR_ENABLE));
}

/*
 * Protect the base of the stack from CPU access
 */

void
ao_mpu_stack_guard(void *base)
{
	uintptr_t	addr = (uintptr_t) base;

	/* Round up to cover the lowest possible 32-byte region */
	addr = (addr + ~(STM_MPU_RBAR_ADDR_MASK << STM_MPU_RBAR_ADDR)) & (STM_MPU_RBAR_ADDR_MASK << STM_MPU_RBAR_ADDR);

	stm_mpu.rbar = addr | (1 << STM_MPU_RBAR_VALID) | (7 << STM_MPU_RBAR_REGION);
	stm_mpu.rasr = ((1 << STM_MPU_RASR_XN) |
			(STM_MPU_RASR_AP_NONE_NONE << STM_MPU_RASR_AP) |
			(5 << STM_MPU_RASR_TEX) |
			(0 << STM_MPU_RASR_C) |
			(1 << STM_MPU_RASR_B) |
			(0 << STM_MPU_RASR_S) |
			(0 << STM_MPU_RASR_SRD) |
			(4 << STM_MPU_RASR_SIZE) |
			(1 << STM_MPU_RASR_ENABLE));
}
