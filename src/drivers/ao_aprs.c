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

// Public methods, constants, and data structures for each class.

static void timeInit(void);

static void tncInit(void);
static void tnc1200TimerTick(void);

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
static uint16_t sysCRC16(const uint8_t *buffer, uint8_t length, uint16_t crc)
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

/// 16-bit NCO where the upper 8-bits are used to index into the frequency generation table.
static uint16_t timeNCO;

/// Audio tone NCO update step (phase).
static uint16_t timeNCOFreq;

/**
 *   Initialize the real-time clock.
 */
static void timeInit()
{
    timeNCO = 0x00;
    timeNCOFreq = 0x2000;
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
#define TNC_BUFFER_SIZE 40

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

/// AX.25 compliant packet header that contains destination, station call sign, and path.
/// 0x76 for SSID-11, 0x78 for SSID-12
static uint8_t TNC_AX25_HEADER[] = { 
    'A' << 1, 'P' << 1, 'A' << 1, 'M' << 1, ' ' << 1, ' ' << 1, 0x60,
    'N' << 1, '0' << 1, 'C' << 1, 'A' << 1, 'L' << 1, 'L' << 1, 0x78,
    'W' << 1, 'I' << 1, 'D' << 1, 'E' << 1, '2' << 1, ' ' << 1, 0x65,
    0x03, 0xf0 };

#define TNC_CALLSIGN_OFF	7
#define TNC_CALLSIGN_LEN	6

static void
tncSetCallsign(void)
{
#ifndef AO_APRS_TEST
	uint8_t	i;

	for (i = 0; i < TNC_CALLSIGN_LEN; i++) {
		if (!ao_config.callsign[i])
			break;
		TNC_AX25_HEADER[TNC_CALLSIGN_OFF + i] = ao_config.callsign[i] << 1;
	}
	for (; i < TNC_CALLSIGN_LEN; i++)
		TNC_AX25_HEADER[TNC_CALLSIGN_OFF + i] = ' ' << 1;
#endif
}

/// The next bit to transmit.
static uint8_t tncTxBit;

/// Current mode of the 1200 bps state machine.
static TNC_TX_1200BPS_STATE tncMode;

/// Counter for each bit (0 - 7) that we are going to transmit.
static uint8_t tncBitCount;

/// A shift register that holds the data byte as we bit shift it for transmit.
static uint8_t tncShift;

/// Index into the APRS header and data array for each byte as we transmit it.
static uint8_t tncIndex;

/// The number of bytes in the message portion of the AX.25 message.
static uint8_t tncLength;

/// A copy of the last 5 bits we've transmitted to determine if we need to bit stuff on the next bit.
static uint8_t tncBitStuff;

/// Buffer to hold the message portion of the AX.25 packet as we prepare it.
static uint8_t tncBuffer[TNC_BUFFER_SIZE];

/** 
 *   Initialize the TNC internal variables.
 */
static void tncInit()
{
    tncTxBit = 0;
    tncMode = TNC_TX_READY;
}

/**
 *   Method that is called every 833uS to transmit the 1200bps A-FSK data stream.
 *   The provides the pre and postamble as well as the bit stuffed data stream.
 */
static void tnc1200TimerTick()
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

static void tncCompressInt(uint8_t *dest, int32_t value, int len) {
	int i;
	for (i = len - 1; i >= 0; i--) {
		dest[i] = value % 91 + 33;
		value /= 91;
	}
}

static int ao_num_sats(void)
{
    int i;
    int n = 0;

    for (i = 0; i < ao_gps_tracking_data.channels; i++) {
	if (ao_gps_tracking_data.sats[i].svid)
	    n++;
    }
    return n;
}

static char ao_gps_locked(void)
{
    if (ao_gps_data.flags & AO_GPS_VALID)
	return 'L';
    else
	return 'U';
}

static int tncComment(uint8_t *buf)
{
#if HAS_ADC
	struct ao_data packet;

	ao_arch_critical(ao_data_get(&packet););

	int16_t battery = ao_battery_decivolt(packet.adc.v_batt);
#ifdef AO_SENSE_DROGUE
	int16_t apogee = ao_ignite_decivolt(AO_SENSE_DROGUE(&packet));
#endif
#ifdef AO_SENSE_MAIN
	int16_t main = ao_ignite_decivolt(AO_SENSE_MAIN(&packet));
#endif

	return sprintf((char *) buf,
		       "%c%d B%d.%d"
#ifdef AO_SENSE_DROGUE
		       " A%d.%d"
#endif
#ifdef AO_SENSE_MAIN
		       " M%d.%d"
#endif
		       , ao_gps_locked(),
		       ao_num_sats(),
		       battery/10,
		       battery % 10
#ifdef AO_SENSE_DROGUE
		       , apogee/10,
		       apogee%10
#endif
#ifdef AO_SENSE_MAIN
		       , main/10,
		       main%10
#endif
		);
#else
	return sprintf((char *) buf,
		       "%c%d",
		       ao_gps_locked(),
		       ao_num_sats());
#endif
}

/*
 * APRS use a log encoding of altitude with a base of 1.002, such that
 *
 *	feet = 1.002 ** encoded_altitude
 *
 *	meters = (1.002 ** encoded_altitude) * 0.3048
 *
 *	log2(meters) = log2(1.002 ** encoded_altitude) + log2(0.3048)
 *
 *	log2(meters) = encoded_altitude * log2(1.002) + log2(0.3048)
 *
 *	encoded_altitude = (log2(meters) - log2(0.3048)) / log2(1.002)
 *
 *	encoded_altitude = (log2(meters) + log2(1/0.3048)) * (1/log2(1.002))
 *
 * We need 9 bits of mantissa to hold 1/log2(1.002) (~ 347), which leaves us
 * 23 bits of fraction. That turns out to be *just* enough to avoid any
 * errors in the result (cool, huh?).
 */

#define fixed23_int(x)		((uint32_t) ((x) << 23))
#define fixed23_one		fixed23_int(1)
#define fixed23_two 		fixed23_int(2)
#define fixed23_half		(fixed23_one >> 1)
#define fixed23_floor(x)	((x) >> 23)
#define fixed23_real(x)		((uint32_t) ((x) * fixed23_one + 0.5))

static inline uint64_t
fixed23_mul(uint32_t x, uint32_t y)
{
	return ((uint64_t) x * y + fixed23_half) >> 23;
}

/*
 * Use 30 fraction bits for the altitude. We need two bits at the
 * top as we need to handle x, where 0 <= x < 4. We don't
 * need 30 bits, but it's actually easier this way as we normalize
 * the incoming value to 1 <= x < 2, and having the integer portion
 * way up high means we don't have to deal with shifting in both
 * directions to cover from 0 to 2**30-1.
 */

#define fixed30_int(x)	((uint32_t) ((x) << 30))
#define fixed30_one 	fixed30_int(1)
#define fixed30_half 	(fixed30_one >> 1)
#define fixed30_two	fixed30_int(2)

static inline uint32_t
fixed30_mul(uint32_t x, uint32_t y)
{
	return ((uint64_t) x * y + fixed30_half) >> 30;
}

/*
 * Fixed point log2. Takes integer argument, returns
 * fixed point result with 23 bits of fraction
 */

static uint32_t
ao_fixed_log2(uint32_t x)
{
	uint32_t	result;
	uint32_t	frac = fixed23_one;

	/* Bounds check for sanity */
	if (x <= 0)
		return 0;

	if (x >= fixed30_one)
		return 0xffffffff;

	/*
	 * Normalize and compute integer log portion
	 *
	 * This makes 1 <= x < 2, and computes result to be
	 * the integer portion of the log2 of x
	 */

	for (result = fixed23_int(30); x < fixed30_one; result -= fixed23_one, x <<= 1)
		;

	/*
	 * Given x, find y and n such that:
	 *
	 *	x = y * 2**n 		1 <= y < 2
	 *
	 * That means:
	 *
	 *	lb(x) = n + lb(y)
	 *
	 * Now, repeatedly square y to find find z and m such that:
	 *
	 *	z = y ** (2**m)	2 <= z < 4
	 *
	 * This is possible because 1 <= y < 2
	 *
	 *	lb(y) = lb(z) / 2**m
 	 *
	 *	        (1 + lb(z/2))
	 *	      = -------------
	 *                  2**m
	 *
	 *            = 2**-m + 2**-m * lb(z/2)
	 *
	 * Note that if 2 <= z < 4, then 1 <= (z/2) < 2, so we can
	 * iterate to find lb(z/2)
	 *
	 * In this implementation, we don't care about the 'm' value,
	 * instead we only care about 2**-m, which we store in 'frac'
	 */

	while (frac != 0 && x != fixed30_one) {
		/* Repeatedly square x until 2 <= x < 4 */
		while (x < fixed30_two) {
			x = fixed30_mul(x, x);

			/* Divide the fractional result bit by 2 */
			frac >>= 1;
		}

		/* Add in this result bit */
		result |= frac;

		/* Make 1 <= x < 2 again and iterate */
		x >>= 1;
	}
	return result;
}

#define APRS_LOG_CONVERT	fixed23_real(1.714065192056127)
#define APRS_LOG_BASE		fixed23_real(346.920048461100941)

static int
ao_aprs_encode_altitude(int meters)
{
	return fixed23_floor(fixed23_mul(ao_fixed_log2(meters) + APRS_LOG_CONVERT, APRS_LOG_BASE) + fixed23_half);
}

/**
 *   Generate the plain text position packet.
 */
static int tncPositionPacket(void)
{
    static int32_t	latitude;
    static int32_t	longitude;
    static int32_t	altitude;
    int32_t		lat, lon, alt;
    uint8_t	*buf;

    if (ao_gps_data.flags & AO_GPS_VALID) {
	latitude = ao_gps_data.latitude;
	longitude = ao_gps_data.longitude;
	altitude = ao_gps_data.altitude;
	if (altitude < 0)
	    altitude = 0;
    }

    buf = tncBuffer;
    *buf++ = '!';

    /* Symbol table ID */
    *buf++ = '/';

    lat = ((uint64_t) 380926 * (900000000 - latitude)) / 10000000;
    lon = ((uint64_t) 190463 * (1800000000 + longitude)) / 10000000;

    alt = ao_aprs_encode_altitude(altitude);

    tncCompressInt(buf, lat, 4);
    buf += 4;
    tncCompressInt(buf, lon, 4);
    buf += 4;

    /* Symbol code */
    *buf++ = '\'';

    tncCompressInt(buf, alt, 2);
    buf += 2;

    *buf++ = 33 + ((1 << 5) | (2 << 3));

    buf += tncComment(buf);

    return buf - tncBuffer;
}

static int16_t
tncFill(uint8_t *buf, int16_t len)
{
    int16_t	l = 0;
    uint8_t	b;
    uint8_t	bit;

    while (tncMode != TNC_TX_READY && l < len) {
	b = 0;
	for (bit = 0; bit < 8; bit++) {
	    b = b << 1 | (timeNCO >> 15);
	    timeNCO += timeNCOFreq;
	}
	*buf++ = b;
	l++;
	tnc1200TimerTick();
    }
    if (tncMode == TNC_TX_READY)
	l = -l;
    return l;
}

/** 
 *    Prepare an AX.25 data packet.  Each time this method is called, it automatically
 *    rotates through 1 of 3 messages.
 *
 *    @param dataMode enumerated type that specifies 1200bps A-FSK or 9600bps FSK
 */
void ao_aprs_send(void)
{
    uint16_t crc;

    timeInit();
    tncInit();
    tncSetCallsign();

    tncLength = tncPositionPacket();

    // Calculate the CRC for the header and message.
    crc = sysCRC16(TNC_AX25_HEADER, sizeof(TNC_AX25_HEADER), 0xffff);
    crc = sysCRC16(tncBuffer, tncLength, crc ^ 0xffff);

    // Save the CRC in the message.
    tncBuffer[tncLength++] = crc & 0xff;
    tncBuffer[tncLength++] = (crc >> 8) & 0xff;

    // Prepare the variables that are used in the real-time clock interrupt.
    tncBitCount = 0;
    tncShift = 0x7e;
    tncTxBit = 0;
    tncIndex = 0;
    tncMode = TNC_TX_SYNC;

    ao_radio_send_aprs(tncFill);
}

/** @} */
