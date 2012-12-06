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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <ao_telemetry.h>

#define AO_APRS_TEST

#include <ao_aprs.c>

/*
 * @section copyright_sec Copyright
 *
 * Copyright (c) 2001-2009 Michael Gray, KD7LMO


 *
 *
 * @section gpl_sec GNU General Public License
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  

 */

static void
audio_gap(int secs)
{
	int	samples = secs * 9600;

	while (samples--)
		putchar(0x7f);
}

// This is where we go after reset.
int main(int argc, char **argv)
{
    uint8_t utcSeconds, lockLostCounter, i;
    gpsInit();
    tncInit();

    audio_gap(1);
    // Transmit software version packet on start up.
    tncTxPacket(TNC_MODE_1200_AFSK);

    // Counters to send packets if the GPS time stamp is not available.
    lockLostCounter = 5;
    utcSeconds = 55;
  
    // This is the main loop that process GPS data and waits for the once per second timer tick.
    for (i = 0; i < 5; i++)
    {
	    audio_gap(10);
        // Read the GPS engine serial port FIFO and process the GPS data.
//        gpsUpdate();

        if (gpsIsReady()) 
        {
            // Start the flight timer when we get a valid 3D fix.
            if (gpsGetFixType() == GPS_3D_FIX)
                timeSetRunFlag();

            // Generate our packets based on the GPS time.
            if (tncIsTimeSlot(gpsPosition.seconds))
                 tncTxPacket(TNC_MODE_1200_AFSK);

            // Sync the internal clock to GPS UTC time.
            utcSeconds = gpsPosition.seconds;

            // This counter is reset every time we receive the GPS message.
            lockLostCounter = 0;

            // Log the data to flash.
//            sysLogGPSData();            
        } // END if gpsIsReady   

        // Processing that occurs once a second.
        if (timeIsUpdate()) 
        {
            // We maintain the UTC time in seconds if we shut off the GPS engine or it fails.
            if (++utcSeconds == 60)
                utcSeconds = 0;

            // If we loose information for more than 5 seconds, 
            // we will determine when to send a packet based on internal time.
            if (lockLostCounter == 5) 
            {
                if (tncIsTimeSlot(utcSeconds))
                    tncTxPacket(TNC_MODE_1200_AFSK);
            } else
                ++lockLostCounter;

            // Update the ADC filters.
//            adcUpdate();

            if (timeHours == 5 && timeMinutes == 0 && timeSeconds == 0)
                gpsPowerOff();

        } // END if timeIsUpdate

    } // END for
    return 0;
}




