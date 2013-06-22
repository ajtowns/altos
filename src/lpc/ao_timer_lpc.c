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

#include <ao.h>

volatile __data AO_TICK_TYPE ao_tick_count;

uint16_t
ao_time(void)
{
	return ao_tick_count;
}

#if AO_DATA_ALL
volatile __data uint8_t	ao_data_interval = 1;
volatile __data uint8_t	ao_data_count;
#endif

void lpc_systick_isr(void)
{
	if (lpc_systick.csr & (1 << LPC_SYSTICK_CSR_COUNTFLAG)) {
		++ao_tick_count;
#if HAS_TASK_QUEUE
		if (ao_task_alarm_tick && (int16_t) (ao_tick_count - ao_task_alarm_tick) >= 0)
			ao_task_check_alarm((uint16_t) ao_tick_count);
#endif
#if AO_DATA_ALL
		if (++ao_data_count == ao_data_interval) {
			ao_data_count = 0;
			ao_adc_poll();
#if (AO_DATA_ALL & ~(AO_DATA_ADC))
			ao_wakeup((void *) &ao_data_count);
#endif
		}
#endif
	}
}

#if HAS_ADC
void
ao_timer_set_adc_interval(uint8_t interval)
{
	ao_arch_critical(
		ao_data_interval = interval;
		ao_data_count = 0;
		);
}
#endif

#define SYSTICK_RELOAD ((AO_LPC_SYSCLK / 2) / 100 - 1)

/* Initialize our 100Hz clock */
void
ao_timer_init(void)
{
	lpc_systick.rvr = SYSTICK_RELOAD;
	lpc_systick.cvr = 0;
	lpc_systick.csr = ((1 << LPC_SYSTICK_CSR_ENABLE) |
			   (1 << LPC_SYSTICK_CSR_TICKINT) |
			   (LPC_SYSTICK_CSR_CLKSOURCE_CPU_OVER_2 << LPC_SYSTICK_CSR_CLKSOURCE));
}

#define AO_LPC_M	((AO_LPC_CLKOUT / AO_LPC_CLKIN) - 1)

#define AO_LPC_FCCO_MIN	156000000

static void
ao_clock_delay(void)
{
	uint32_t	i;
	for (i = 0; i < 200; i++)
		ao_arch_nop();
}

void
ao_clock_init(void)
{
	uint8_t		p;
	uint32_t	i;

	/* Turn off all perhipherals except for GPIO configuration */
	lpc_scb.sysahbclkctrl = ((1 << LPC_SCB_SYSAHBCLKCTRL_SYS) |
				 (1 << LPC_SCB_SYSAHBCLKCTRL_ROM) |
				 (1 << LPC_SCB_SYSAHBCLKCTRL_RAM0) |
				 (1 << LPC_SCB_SYSAHBCLKCTRL_FLASHARRAY) |
				 (1 << LPC_SCB_SYSAHBCLKCTRL_GPIO) |
				 (1 << LPC_SCB_SYSAHBCLKCTRL_IOCON));
				 
	/* Enable the brown-out detection at the highest voltage to
	 * make sure the flash part remains happy
	 */

	lpc_scb.pdruncfg &= ~(1 << LPC_SCB_PDRUNCFG_BOD_PD);
	lpc_scb.bodctrl = ((LPC_SCB_BOD_BODRSTLEV_2_63 << LPC_SCB_BOD_BODRSTLEV) |
			   (LPC_SCB_BOD_BODINTVAL_RESERVED << LPC_SCB_BOD_BODINTVAL) |
			   (1 << LPC_SCB_BOD_BODRSTENA));

	/* Turn the IRC clock back on */
	lpc_scb.pdruncfg &= ~(1 << LPC_SCB_PDRUNCFG_IRC_PD);
	ao_clock_delay();
	
	/* Switch to the IRC clock */
	lpc_scb.mainclksel = LPC_SCB_MAINCLKSEL_SEL_IRC << LPC_SCB_MAINCLKSEL_SEL;
	lpc_scb.mainclkuen = (0 << LPC_SCB_MAINCLKUEN_ENA);
	lpc_scb.mainclkuen = (1 << LPC_SCB_MAINCLKUEN_ENA);
	while (!(lpc_scb.mainclkuen & (1 << LPC_SCB_MAINCLKUEN_ENA)))
		;
	
	/* Switch USB to the main clock */
	lpc_scb.usbclksel = (LPC_SCB_USBCLKSEL_SEL_MAIN_CLOCK << LPC_SCB_USBCLKSEL_SEL);
	lpc_scb.usbclkuen = (0 << LPC_SCB_USBCLKUEN_ENA);
	lpc_scb.usbclkuen = (1 << LPC_SCB_USBCLKUEN_ENA);
	while (!(lpc_scb.usbclkuen & (1 << LPC_SCB_USBCLKUEN_ENA)))
		;
	
	/* Find a PLL post divider ratio that gets the FCCO in range */
	for (p = 0; p < 4; p++)
		if (AO_LPC_CLKOUT << (1 + p) >= AO_LPC_FCCO_MIN)
			break;

	if (p == 4)
		ao_panic(AO_PANIC_CRASH);

	/* Power down the PLL before touching the registers */
	lpc_scb.pdruncfg |= (1 << LPC_SCB_PDRUNCFG_SYSPLL_PD);
	ao_clock_delay();

	/* Set PLL divider values */
	lpc_scb.syspllctrl = ((AO_LPC_M << LPC_SCB_SYSPLLCTRL_MSEL) |
			      (p << LPC_SCB_SYSPLLCTRL_PSEL));

	/* Turn off the external crystal clock */
	lpc_scb.pdruncfg |= (1 << LPC_SCB_PDRUNCFG_SYSOSC_PD);
	ao_clock_delay();

	/* Configure the crystal clock */
	lpc_scb.sysoscctrl = ((0 << LPC_SCB_SYSOSCCTRL_BYPASS) |			   /* using a crystal */
			      ((AO_LPC_CLKIN > 15000000) << LPC_SCB_SYSOSCCTRL_FREQRANGE));/* set range */

	/* Turn on the external crystal clock */
	lpc_scb.pdruncfg &= ~(1 << LPC_SCB_PDRUNCFG_SYSOSC_PD);
	ao_clock_delay();

	/* Select crystal as PLL input */

	lpc_scb.syspllclksel = (LPC_SCB_SYSPLLCLKSEL_SEL_SYSOSC << LPC_SCB_SYSPLLCLKSEL_SEL);
	lpc_scb.syspllclkuen = (1 << LPC_SCB_SYSPLLCLKUEN_ENA);
	lpc_scb.syspllclkuen = (0 << LPC_SCB_SYSPLLCLKUEN_ENA);
	lpc_scb.syspllclkuen = (1 << LPC_SCB_SYSPLLCLKUEN_ENA);
	while (!(lpc_scb.syspllclkuen & (1 << LPC_SCB_SYSPLLCLKUEN_ENA)))
		;
	
	/* Turn on the PLL */
	lpc_scb.pdruncfg &= ~(1 << LPC_SCB_PDRUNCFG_SYSPLL_PD);

	/* Wait for it to lock */
	
	for (i = 0; i < 20000; i++)
		if (lpc_scb.syspllstat & (1 << LPC_SCB_SYSPLLSTAT_LOCK))
			break;
	if (i == 20000)
		ao_panic(AO_PANIC_CRASH);

	/* Switch to the PLL */
	lpc_scb.mainclksel = LPC_SCB_MAINCLKSEL_SEL_PLL_OUTPUT << LPC_SCB_MAINCLKSEL_SEL;
	lpc_scb.mainclkuen = (1 << LPC_SCB_MAINCLKUEN_ENA);
	lpc_scb.mainclkuen = (0 << LPC_SCB_MAINCLKUEN_ENA);
	lpc_scb.mainclkuen = (1 << LPC_SCB_MAINCLKUEN_ENA);
	while (!(lpc_scb.mainclkuen & (1 << LPC_SCB_MAINCLKUEN_ENA)))
		;

	/* Set system clock divider */
	lpc_scb.sysahbclkdiv = AO_LPC_CLKOUT / AO_LPC_SYSCLK;

	/* Shut down perhipheral clocks (enabled as needed) */
	lpc_scb.ssp0clkdiv = 0;
	lpc_scb.uartclkdiv = 0;
	lpc_scb.ssp1clkdiv = 0;
	lpc_scb.usbclkdiv = 0;
	lpc_scb.clkoutdiv = 0;

	/* Switch USB PLL source to system osc so we can power down the IRC */
	lpc_scb.usbpllclksel = (LPC_SCB_USBPLLCLKSEL_SEL_SYSOSC << LPC_SCB_USBPLLCLKSEL_SEL);
	lpc_scb.usbpllclkuen = (0 << LPC_SCB_USBPLLCLKUEN_ENA);
	lpc_scb.usbpllclkuen = (1 << LPC_SCB_USBPLLCLKUEN_ENA);
	while (!(lpc_scb.usbpllclkuen & (1 << LPC_SCB_USBPLLCLKUEN_ENA)))
		;
	
	/* Power down everything we don't need */
	lpc_scb.pdruncfg = ((1 << LPC_SCB_PDRUNCFG_IRCOUT_PD) |
			    (1 << LPC_SCB_PDRUNCFG_IRC_PD) |
			    (0 << LPC_SCB_PDRUNCFG_BOD_PD) |
			    (1 << LPC_SCB_PDRUNCFG_ADC_PD) |
			    (1 << LPC_SCB_PDRUNCFG_WDTOSC_PD) |
			    (1 << LPC_SCB_PDRUNCFG_USBPLL_PD) |
			    (1 << LPC_SCB_PDRUNCFG_USBPAD_PD) |
			    (1 << 11) |
			    (7 << 13));
}
