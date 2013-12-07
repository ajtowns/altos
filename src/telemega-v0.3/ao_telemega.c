/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
#include <ao_hmc5883.h>
#include <ao_mpu6000.h>
#include <ao_mma655x.h>
#include <ao_log.h>
#include <ao_exti.h>
#include <ao_packet.h>
#include <ao_companion.h>
#include <ao_profile.h>
#include <ao_eeprom.h>
#if HAS_SAMPLE_PROFILE
#include <ao_sample_profile.h>
#endif
#include <ao_pyro.h>
#if HAS_STACK_GUARD
#include <ao_mpu.h>
#endif

int
main(void)
{
	ao_clock_init();
	
#if HAS_STACK_GUARD
	ao_mpu_init();
#endif

	ao_task_init();
	ao_serial_init();
	ao_led_init(LEDS_AVAILABLE);
	ao_led_on(AO_LED_GREEN);
	ao_timer_init();

	ao_i2c_init();
	ao_spi_init();
	ao_dma_init();
	ao_exti_init();

	ao_adc_init();
#if HAS_BEEP
	ao_beep_init();
#endif
	ao_cmd_init();

#if HAS_MS5607
	ao_ms5607_init();
#endif
#if HAS_HMC5883
	ao_hmc5883_init();
#endif
#if HAS_MPU6000
	ao_mpu6000_init();
#endif
#if HAS_MMA655X
	ao_mma655x_init();
#endif

	ao_eeprom_init();
	ao_storage_init();
	
	ao_flight_init();
	ao_log_init();
	ao_report_init();

	ao_usb_init();
	ao_gps_init();
	ao_gps_report_mega_init();
	ao_telemetry_init();
	ao_radio_init();
	ao_packet_slave_init(FALSE);
	ao_igniter_init();
	ao_companion_init();
	ao_pyro_init();

	ao_config_init();
#if AO_PROFILE
	ao_profile_init();
#endif
#if HAS_SAMPLE_PROFILE
	ao_sample_profile_init();
#endif
	
	ao_start_scheduler();
	return 0;
}
