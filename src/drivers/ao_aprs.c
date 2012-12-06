/** 
 * http://ad7zj.net/kd7lmo/aprsbeacon_code.html
 *
 * @mainpage Pico Beacon
 *
 * @section overview_sec Overview
 *
 * The Pico Beacon is an APRS based tracking beacon that operates in the UHF 420-450MHz band.  The device utilizes a 
 * Microchip PIC 18F2525 embedded controller, Motorola M12+ GPS engine, and Analog Devices AD9954 DDS.  The device is capable
 * of generating a 1200bps A-FSK and 9600 bps FSK AX.25 compliant APRS (Automatic Position Reporting System) message.


 *
 * @section history_sec Revision History
 *
 * @subsection v305 V3.05
 * 23 Dec 2006, Change include; (1) change printf format width to conform to ANSI standard when new CCS 4.xx compiler released.
 *
 *
 * @subsection v304 V3.04
 * 10 Jan 2006, Change include; (1) added amplitude control to engineering mode,
 *                                     (2) corrected number of bytes reported in log,
 *                                     (3) add engineering command to set high rate position reports (5 seconds), and
 *                                     (4) corrected size of LOG_COORD block when searching for end of log.
 *
 * @subsection v303 V3.03
 * 15 Sep 2005, Change include; (1) removed AD9954 setting SDIO as input pin, 
 *                                     (2) additional comments and Doxygen tags,
 *                                     (3) integration and test code calculates DDS FTW,
 *                                     (4) swapped bus and reference analog input ports (hardware change),
 *                                     (5) added message that indicates we are reading flash log and reports length,
 *                                     (6) report bus voltage in 10mV steps, and
 *                                     (7) change log type enumerated values to XORed nibbles for error detection.
 *
 *
 * @subsection v302 V3.02
 * 6 Apr 2005, Change include; (1) corrected tracked satellite count in NMEA-0183 $GPGGA message,
 *                                    (2) Doxygen documentation clean up and additions, and
 *                                    (3) added integration and test code to baseline.
 *
 * 
 * @subsection v301 V3.01
 * 13 Jan 2005, Renamed project and files to Pico Beacon.
 *
 *
 * @subsection v300 V3.00
 * 15 Nov 2004, Change include; (1) Micro Beacon extreme hardware changes including integral transmitter,
 *                                     (2) PIC18F2525 processor,
 *                                     (3) AD9954 DDS support functions,
 *                                     (4) added comments and formatting for doxygen,
 *                                     (5) process GPS data with native Motorola protocol,
 *                                     (6) generate plain text $GPGGA and $GPRMC messages,
 *                                     (7) power down GPS 5 hours after lock,
 *                                     (8) added flight data recorder, and
 *                                     (9) added diagnostics terminal mode.
 *
 * 
 * @subsection v201 V2.01
 * 30 Jan 2004, Change include; (1) General clean up of in-line documentation, and 
 *                                     (2) changed temperature resolution to 0.1 degrees F.
 *
 * 
 * @subsection v200 V2.00
 * 26 Oct 2002, Change include; (1) Micro Beacon II hardware changes including PIC18F252 processor,
 *                                     (2) serial EEPROM, 
 *                                     (3) GPS power control, 
 *                                     (4) additional ADC input, and 
 *                                     (5) LM60 temperature sensor.                            
 *
 *
 * @subsection v101 V1.01
 * 5 Dec 2001, Change include; (1) Changed startup message, and 
 *                                    (2) applied SEPARATE pragma to several methods for memory usage.
 *
 *
 * @subsection v100 V1.00
 * 25 Sep 2001, Initial release.  Flew ANSR-3 and ANSR-4.
 * 


 *
 *
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


 * 
 * 
 * @section design Design Details
 *
 * Provides design details on a variety of the components that make up the Pico Beacon.
 *
 *  @subpage power
 */

/**
 *  @page power Power Consumption
 *
 *  Measured DC power consumption.
 * 
 *  3VDC prime power current 

 *
 *    7mA Held in reset 

 *   18mA Processor running, all I/O off 

 *  110mA GPS running 

 *  120mA GPS running w/antenna 

 *  250mA DDS running and GPS w/antenna 

 *  420mA DDS running, GPS w/antenna, and PA chain on with no RF 

 *  900mA Transmit 

 *
 */

#ifndef AO_APRS_TEST
#include <ao.h>
#endif

#include <ao_aprs.h>

typedef int bool_t;
typedef int32_t int32;
#define false 0
#define true 1

// Public methods, constants, and data structures for each class.

void ddsInit();
void ddsSetAmplitude (uint8_t amplitude);
void ddsSetOutputScale (uint16_t amplitude);
void ddsSetFSKFreq (uint32_t ftw0, uint32_t ftw1);
void ddsSetFreq (uint32_t freq);
void ddsSetFTW (uint32_t ftw);

uint16_t sysCRC16(uint8_t *buffer, uint8_t length, uint16_t crc);

uint8_t timeGetTicks();
void timeInit();
void timeSetDutyCycle (uint8_t dutyCycle);
void timeUpdate();

void tncInit();
void tnc1200TimerTick();
void tncTxByte (uint8_t value);
void tncTxPacket(void);

/** @} */

/**
 *  @defgroup DDS AD9954 DDS (Direct Digital Synthesizer)
 *
 *  Functions to control the Analog Devices AD9954 DDS.
 *
 *  @{
 */

/// Number of digits in DDS frequency to FTW conversion.
#define DDS_FREQ_TO_FTW_DIGITS 9

/// Array of multiplication factors used to convert frequency to the FTW.
const uint32_t DDS_MULT[DDS_FREQ_TO_FTW_DIGITS] = { 11, 7, 7, 3, 4, 8, 4, 9, 1 };

/// Array of divisors used to convert frequency to the FTW.
const uint32_t DDS_DIVISOR[DDS_FREQ_TO_FTW_DIGITS - 1] = { 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

/// Lookup table to convert dB amplitude scale in 0.5 steps to a linear DDS scale factor.
const uint16_t DDS_AMP_TO_SCALE[] = 
{ 
    16383, 15467, 14601, 13785, 13013, 12286, 11598, 10949, 10337, 9759, 9213, 8697, 
    8211, 7752, 7318, 6909, 6522, 6157, 5813, 5488, 5181, 4891, 4617, 4359, 4115, 3885, 3668, 3463, 
    3269, 3086, 2913, 2750, 2597, 2451, 2314, 2185, 2062, 1947, 1838, 1735, 1638 
};


/// Frequency Word List - 4.0KHz FM frequency deviation at 81.15MHz  (445.950MHz)
const uint32_t freqTable[256] = 
{
    955418300, 955419456, 955420611, 955421765, 955422916, 955424065, 955425210, 955426351, 
    955427488, 955428618, 955429743, 955430861, 955431971, 955433073, 955434166, 955435249, 
    955436322, 955437385, 955438435, 955439474, 955440500, 955441513, 955442511, 955443495, 
    955444464, 955445417, 955446354, 955447274, 955448176, 955449061, 955449926, 955450773, 
    955451601, 955452408, 955453194, 955453960, 955454704, 955455426, 955456126, 955456803, 
    955457457, 955458088, 955458694, 955459276, 955459833, 955460366, 955460873, 955461354, 
    955461809, 955462238, 955462641, 955463017, 955463366, 955463688, 955463983, 955464250, 
    955464489, 955464701, 955464884, 955465040, 955465167, 955465266, 955465337, 955465380, 
    955465394, 955465380, 955465337, 955465266, 955465167, 955465040, 955464884, 955464701, 
    955464489, 955464250, 955463983, 955463688, 955463366, 955463017, 955462641, 955462238, 
    955461809, 955461354, 955460873, 955460366, 955459833, 955459276, 955458694, 955458088, 
    955457457, 955456803, 955456126, 955455426, 955454704, 955453960, 955453194, 955452408, 
    955451601, 955450773, 955449926, 955449061, 955448176, 955447274, 955446354, 955445417, 
    955444464, 955443495, 955442511, 955441513, 955440500, 955439474, 955438435, 955437385, 
    955436322, 955435249, 955434166, 955433073, 955431971, 955430861, 955429743, 955428618, 
    955427488, 955426351, 955425210, 955424065, 955422916, 955421765, 955420611, 955419456, 
    955418300, 955417144, 955415989, 955414836, 955413684, 955412535, 955411390, 955410249, 
    955409113, 955407982, 955406857, 955405740, 955404629, 955403528, 955402435, 955401351, 
    955400278, 955399216, 955398165, 955397126, 955396100, 955395088, 955394089, 955393105, 
    955392136, 955391183, 955390246, 955389326, 955388424, 955387540, 955386674, 955385827, 
    955385000, 955384192, 955383406, 955382640, 955381896, 955381174, 955380474, 955379797, 
    955379143, 955378513, 955377906, 955377324, 955376767, 955376235, 955375728, 955375246, 
    955374791, 955374362, 955373959, 955373583, 955373234, 955372912, 955372618, 955372350, 
    955372111, 955371900, 955371716, 955371560, 955371433, 955371334, 955371263, 955371220, 
    955371206, 955371220, 955371263, 955371334, 955371433, 955371560, 955371716, 955371900, 
    955372111, 955372350, 955372618, 955372912, 955373234, 955373583, 955373959, 955374362, 
    955374791, 955375246, 955375728, 955376235, 955376767, 955377324, 955377906, 955378513, 
    955379143, 955379797, 955380474, 955381174, 955381896, 955382640, 955383406, 955384192, 
    955385000, 955385827, 955386674, 955387540, 955388424, 955389326, 955390246, 955391183, 
    955392136, 955393105, 955394089, 955395088, 955396100, 955397126, 955398165, 955399216, 
    955400278, 955401351, 955402435, 955403528, 955404629, 955405740, 955406857, 955407982, 
    955409113, 955410249, 955411390, 955412535, 955413684, 955414836, 955415989, 955417144
};

/**
 *  Set DDS frequency tuning word.  The output frequency is equal to RefClock * (ftw / 2 ^ 32).
 *
 *  @param ftw Frequency Tuning Word
 */
void ddsSetFTW (uint32_t ftw)
{
    int	x = ftw - freqTable[0];
    putchar (x > 0 ? 0xc0 : 0x40);
}

/** @} */

/**
 *  @defgroup sys System Library Functions
 *
 *  Generic system functions similiar to the run-time C library.
 *
 *  @{
 */

/**
 *    Calculate the CRC-16 CCITT of buffer that is length bytes long.
 *    The crc parameter allow the calculation on the CRC on multiple buffers.
 *
 *    @param buffer Pointer to data buffer.
 *    @param length number of bytes in data buffer
 *    @param crc starting value
 *
 *    @return CRC-16 of buffer[0 .. length]
 */
uint16_t sysCRC16(uint8_t *buffer, uint8_t length, uint16_t crc)
{
    uint8_t i, bit, value;

    for (i = 0; i < length; ++i) 
    {
        value = buffer[i];

        for (bit = 0; bit < 8; ++bit) 
        {
            crc ^= (value & 0x01);
            crc = ( crc & 0x01 ) ? ( crc >> 1 ) ^ 0x8408 : ( crc >> 1 );
            value = value >> 1;
        } // END for
    } // END for

    return crc ^ 0xffff;
}

/** @} */

/**
 *  @defgroup rtc Real Time Interrupt tick
 *
 *  Manage the built-in real time interrupt.  The interrupt clock PRI is 104uS (9600 bps).
 *
 *  @{
 */

/// A counter that ticks every 100mS.
uint8_t timeTicks;

/// Counts the number of 104uS interrupts for a 100mS time period.
uint16_t timeInterruptCount;

/// Counts the number of 100mS time periods in 1 second.
uint8_t time100ms;

/// System time in seconds.
uint8_t timeSeconds;

/// System time in minutes.
uint8_t timeMinutes;

/// System time in hours.
uint8_t timeHours;

/// Desired LED duty cycle 0 to 9 where 0 = 0% and 9 = 90%.
uint8_t timeDutyCycle;

/// Current value of the timer 1 compare register used to generate 104uS interrupt rate (9600bps).
uint16_t timeCompare;

/// 16-bit NCO where the upper 8-bits are used to index into the frequency generation table.
uint16_t timeNCO;

/// Audio tone NCO update step (phase).
uint16_t timeNCOFreq;

/// Counter used to deciminate down from the 104uS to 833uS interrupt rate.  (9600 to 1200 baud) 
uint8_t timeLowRateCount;

/// Flag set true once per second.
bool_t timeUpdateFlag;

/// Flag that indicate the flight time should run.
bool_t timeRunFlag;

/// The change in the CCP_1 register for each 104uS (9600bps) interrupt period.
#define TIME_RATE 125

/**
 *   Running 8-bit counter that ticks every 100mS.
 *
 *   @return 100mS time tick
 */
uint8_t timeGetTicks()
{
    return timeTicks;
}

/**
 *   Initialize the real-time clock.
 */
void timeInit()
{
    timeTicks = 0;
    timeInterruptCount = 0;
//    time100mS = 0;
    timeSeconds = 0;
    timeMinutes = 0;
    timeHours = 0;
    timeCompare = TIME_RATE;
    timeUpdateFlag = false;
    timeNCO = 0x00;
    timeLowRateCount = 0;
    timeNCOFreq = 0x2000;
    timeRunFlag = false;
}

/**
 *   Function return true once a second based on real-time clock.
 *
 *   @return true on one second tick; otherwise false
 */
bool_t timeIsUpdate()
{
    if (timeUpdateFlag) 
    {
        timeUpdateFlag = false;
        return true;
    } // END if

    return false;
}

/**
 *   Set a flag to indicate the flight time should run.  This flag is typically set when the payload
 *   lifts off.
 */
void timeSetRunFlag()
{
    timeRunFlag = true;
}

/**
 *   Timer interrupt handler called every 104uS (9600 times/second).
 */
void timeUpdate()
{
    // Setup the next interrupt for the operational mode.
    timeCompare += TIME_RATE;

    ddsSetFTW (freqTable[timeNCO >> 8]);

    timeNCO += timeNCOFreq;

    if (++timeLowRateCount == 8) 
    {
	timeLowRateCount = 0;
	tnc1200TimerTick();
    } // END if
}

/** @} */

/**
 *  @defgroup tnc TNC (Terminal Node Controller)
 *
 *  Functions that provide a subset of the TNC functions.
 *
 *  @{
 */

/// The number of start flag bytes to send before the packet message.  (360bits * 1200bps = 300mS)
#define TNC_TX_DELAY 45

/// The size of the TNC output buffer.
#define TNC_BUFFER_SIZE 80

/// States that define the current mode of the 1200 bps (A-FSK) state machine.
typedef enum
{
    /// Stand by state ready to accept new message.
    TNC_TX_READY,

    /// 0x7E bit stream pattern used to define start of APRS message.
    TNC_TX_SYNC,

    /// Transmit the AX.25 header that contains the source/destination call signs, APRS path, and flags.
    TNC_TX_HEADER,

    /// Transmit the message data.
    TNC_TX_DATA,

    /// Transmit the end flag sequence.
    TNC_TX_END
} TNC_TX_1200BPS_STATE;

/// Enumeration of the messages we can transmit. 
typedef enum
{
    /// Startup message that contains software version information.
    TNC_BOOT_MESSAGE,

    /// Plain text status message.
    TNC_STATUS,

    /// Message that contains GPS NMEA-0183 $GPGGA message.
    TNC_GGA,

    /// Message that contains GPS NMEA-0183 $GPRMC message.
    TNC_RMC
}  TNC_MESSAGE_TYPE;

/// AX.25 compliant packet header that contains destination, station call sign, and path.
/// 0x76 for SSID-11, 0x78 for SSID-12
uint8_t TNC_AX25_HEADER[30] = { 
    'A' << 1, 'P' << 1, 'R' << 1, 'S' << 1, ' ' << 1, ' ' << 1, 0x60, \
    'K' << 1, 'D' << 1, '7' << 1, 'S' << 1, 'Q' << 1, 'G' << 1, 0x76, \
    'G' << 1, 'A' << 1, 'T' << 1, 'E' << 1, ' ' << 1, ' ' << 1, 0x60, \
    'W' << 1, 'I' << 1, 'D' << 1, 'E' << 1, '3' << 1, ' ' << 1, 0x67, \
    0x03, 0xf0 };


/// The next bit to transmit.
uint8_t tncTxBit;

/// Current mode of the 1200 bps state machine.
TNC_TX_1200BPS_STATE tncMode;

/// Counter for each bit (0 - 7) that we are going to transmit.
uint8_t tncBitCount;

/// A shift register that holds the data byte as we bit shift it for transmit.
uint8_t tncShift;

/// Index into the APRS header and data array for each byte as we transmit it.
uint8_t tncIndex;

/// The number of bytes in the message portion of the AX.25 message.
uint8_t tncLength;

/// A copy of the last 5 bits we've transmitted to determine if we need to bit stuff on the next bit.
uint8_t tncBitStuff;

/// Pointer to TNC buffer as we save each byte during message preparation.
uint8_t *tncBufferPnt;

/// The type of message to tranmit in the next packet.
TNC_MESSAGE_TYPE tncPacketType;

/// Buffer to hold the message portion of the AX.25 packet as we prepare it.
uint8_t tncBuffer[TNC_BUFFER_SIZE];

/** 
 *   Initialize the TNC internal variables.
 */
void tncInit()
{
    tncTxBit = 0;
    tncMode = TNC_TX_READY;
    tncPacketType = TNC_BOOT_MESSAGE;
}

/**
 *   Method that is called every 833uS to transmit the 1200bps A-FSK data stream.
 *   The provides the pre and postamble as well as the bit stuffed data stream.
 */
void tnc1200TimerTick()
{
    // Set the A-FSK frequency.
    if (tncTxBit == 0x00)
        timeNCOFreq = 0x2000;
    else
        timeNCOFreq = 0x3aab;

    switch (tncMode) 
    {
        case TNC_TX_READY:
            // Generate a test signal alteranting between high and low tones.
            tncTxBit = (tncTxBit == 0 ? 1 : 0);
            break;

        case TNC_TX_SYNC:
            // The variable tncShift contains the lastest data byte.
            // NRZI enocde the data stream.
            if ((tncShift & 0x01) == 0x00) {
                if (tncTxBit == 0)
                    tncTxBit = 1;
                else
                    tncTxBit = 0;
	    }
                    
            // When the flag is done, determine if we need to send more or data.
            if (++tncBitCount == 8) 
            {
                tncBitCount = 0;
                tncShift = 0x7e;

                // Once we transmit x mS of flags, send the data.
                // txDelay bytes * 8 bits/byte * 833uS/bit = x mS
                if (++tncIndex == TNC_TX_DELAY) 
                {
                    tncIndex = 0;
                    tncShift = TNC_AX25_HEADER[0];
                    tncBitStuff = 0;
                    tncMode = TNC_TX_HEADER;
                } // END if
            } else
                tncShift = tncShift >> 1;
            break;

        case TNC_TX_HEADER:
            // Determine if we have sent 5 ones in a row, if we have send a zero.
            if (tncBitStuff == 0x1f) 
            {
                if (tncTxBit == 0)
                    tncTxBit = 1;
                else
                    tncTxBit = 0;

                tncBitStuff = 0x00;
                return;
            }    // END if

            // The variable tncShift contains the lastest data byte.
            // NRZI enocde the data stream.
            if ((tncShift & 0x01) == 0x00) {
                if (tncTxBit == 0)
                    tncTxBit = 1;
                else
                    tncTxBit = 0;
	    }

            // Save the data stream so we can determine if bit stuffing is 
            // required on the next bit time.
            tncBitStuff = ((tncBitStuff << 1) | (tncShift & 0x01)) & 0x1f;

            // If all the bits were shifted, get the next byte.
            if (++tncBitCount == 8) 
            {
                tncBitCount = 0;

                // After the header is sent, then send the data.
                if (++tncIndex == sizeof(TNC_AX25_HEADER)) 
                {
                    tncIndex = 0;
                    tncShift = tncBuffer[0];
                    tncMode = TNC_TX_DATA;
                } else
                    tncShift = TNC_AX25_HEADER[tncIndex];

            } else
                tncShift = tncShift >> 1;

            break;

        case TNC_TX_DATA:
            // Determine if we have sent 5 ones in a row, if we have send a zero.
            if (tncBitStuff == 0x1f) 
            {
                if (tncTxBit == 0)
                    tncTxBit = 1;
                else
                    tncTxBit = 0;

                tncBitStuff = 0x00;
                return;
            }    // END if

            // The variable tncShift contains the lastest data byte.
            // NRZI enocde the data stream.
            if ((tncShift & 0x01) == 0x00) {
                if (tncTxBit == 0)
                    tncTxBit = 1;
                else
                    tncTxBit = 0;
	    }

            // Save the data stream so we can determine if bit stuffing is 
            // required on the next bit time.
            tncBitStuff = ((tncBitStuff << 1) | (tncShift & 0x01)) & 0x1f;

            // If all the bits were shifted, get the next byte.
            if (++tncBitCount == 8) 
            {
                tncBitCount = 0;

                // If everything was sent, transmit closing flags.
                if (++tncIndex == tncLength) 
                {
                    tncIndex = 0;
                    tncShift = 0x7e;
                    tncMode = TNC_TX_END;
                } else
                    tncShift = tncBuffer[tncIndex];

            } else
                tncShift = tncShift >> 1;

            break;

        case TNC_TX_END:
            // The variable tncShift contains the lastest data byte.
            // NRZI enocde the data stream. 
            if ((tncShift & 0x01) == 0x00) {
                if (tncTxBit == 0)
                    tncTxBit = 1;
                else
                    tncTxBit = 0;
	    }

            // If all the bits were shifted, get the next one.
            if (++tncBitCount == 8) 
            {
                tncBitCount = 0;
                tncShift = 0x7e;
    
                // Transmit two closing flags.
                if (++tncIndex == 2) 
                {
                    tncMode = TNC_TX_READY;

                    return;
                } // END if
            } else
                tncShift = tncShift >> 1;

            break;
    } // END switch
}

/**
 *   Generate the plain text position packet. Data is written through the tncTxByte
 *   callback function
 */
void tncPositionPacket(void)
{
    int32_t	latitude = 45.4694766 * 10000000;
    int32_t	longitude = -122.7376250 * 10000000;
    uint32_t	altitude = 10000;
    uint16_t	lat_deg;
    uint16_t	lon_deg;
    uint16_t	lat_min;
    uint16_t	lat_frac;
    uint16_t	lon_min;
    uint16_t	lon_frac;
    int		c;

    char	lat_sign = 'N', lon_sign = 'E';

    if (latitude < 0) {
	lat_sign = 'S';
	latitude = -latitude;
    }

    if (longitude < 0) {
	lon_sign = 'W';
	longitude = -longitude;
    }

    lat_deg = latitude / 10000000;
    latitude -= lat_deg * 10000000;
    latitude *= 60;
    lat_min = latitude / 10000000;
    latitude -= lat_min * 10000000;
    lat_frac = (latitude + 50000) / 100000;

    lon_deg = longitude / 10000000;
    longitude -= lon_deg * 10000000;
    longitude *= 60;
    lon_min = longitude / 10000000;
    longitude -= lon_min * 10000000;
    lon_frac = (longitude + 50000) / 100000;

    c = sprintf ((char *) tncBufferPnt, "=%02u%02u.%02u%c\\%03u%02u.%02u%cO /A=%06u\015",
		lat_deg, lat_min, lat_frac, lat_sign,
		lon_deg, lon_min, lon_frac, lon_sign,
		altitude * 100 / 3048);
    tncBufferPnt += c;
    tncLength += c;
}

/** 
 *    Prepare an AX.25 data packet.  Each time this method is called, it automatically
 *    rotates through 1 of 3 messages.
 *
 *    @param dataMode enumerated type that specifies 1200bps A-FSK or 9600bps FSK
 */
void tncTxPacket(void)
{
    uint16_t crc;

    // Set a pointer to our TNC output buffer.
    tncBufferPnt = tncBuffer;

    // Set the message length counter.
    tncLength = 0;

    tncPositionPacket();

    // Calculate the CRC for the header and message.
    crc = sysCRC16(TNC_AX25_HEADER, sizeof(TNC_AX25_HEADER), 0xffff);
    crc = sysCRC16(tncBuffer, tncLength, crc ^ 0xffff);

    // Save the CRC in the message.
    *tncBufferPnt++ = crc & 0xff;
    *tncBufferPnt = (crc >> 8) & 0xff;

    // Update the length to include the CRC bytes.
    tncLength += 2;

    // Prepare the variables that are used in the real-time clock interrupt.
    tncBitCount = 0;
    tncShift = 0x7e;
    tncTxBit = 0;
    tncIndex = 0;
    tncMode = TNC_TX_SYNC;

    // Turn on the PA chain.
//    output_high (IO_PTT);

    // Wait for the PA chain to power up.
//    delay_ms (10);

    // Key the DDS.
//    output_high (IO_OSK);

    // Log the battery and reference voltage just after we key the transmitter.
//    sysLogVoltage();
    while (tncMode != TNC_TX_READY)
	timeUpdate();
}

/** @} */
