<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE article PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
"/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">

<article>
  <articleinfo>
    <title>AltOS Telemetry</title>
    <subtitle>Packet Definitions</subtitle>
    <author>
      <firstname>Keith</firstname>
      <surname>Packard</surname>
    </author>
    <copyright>
      <year>2011</year>
      <holder>Keith Packard</holder>
    </copyright>
    <legalnotice>
      <para>
	This document is released under the terms of the
	<ulink url="http://creativecommons.org/licenses/by-sa/3.0/">
	  Creative Commons ShareAlike 3.0
	</ulink>
	license.
      </para>
    </legalnotice>
    <revhistory>
      <revision>
	<revnumber>0.1</revnumber>
	<date>01 July 2011</date>
	<revremark>Initial content</revremark>
      </revision>
    </revhistory>
  </articleinfo>
  <section>
    <title>Packet Format Design</title>
    <para>
      AltOS telemetry data is split into multiple different packets,
      all the same size, but each includs an identifier so that the
      ground station can distinguish among different types. A single
      flight board will transmit multiple packet types, each type on a
      different schedule. The ground software need look for only a
      single packet size, and then decode the information within the
      packet and merge data from multiple packets to construct the
      full flight computer state.
    </para>
    <para>
      Each AltOS packet is 32 bytes long. This size was chosen based
      on the known telemetry data requirements. The power of two size
      allows them to be stored easily in flash memory without having
      them split across blocks or leaving gaps at the end.
    </para>
    <para>
      All packet types start with a five byte header which encodes the
      device serial number, device clock value and the packet
      type. The remaining 27 bytes encode type-specific data.
    </para>
  </section>
  <section>
    <title>Packet Formats</title>
    <para>
      This section first defines the packet header common to all packets
      and then the per-packet data layout.
    </para>
    <section>
      <title>Packet Header</title>
      <table frame='all'>
	<title>Telemetry Packet Header</title>
	<tgroup cols='4' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Offset'/>
	  <colspec align='center' colwidth='3*' colname='Data Type'/>
	  <colspec align='left' colwidth='3*' colname='Name'/>
	  <colspec align='left' colwidth='9*' colname='Description'/>
	  <thead>
	    <row>
	      <entry align='center'>Offset</entry>
	      <entry align='center'>Data Type</entry>
	      <entry align='center'>Name</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>0</entry>
	      <entry>uint16_t</entry>
	      <entry>serial</entry>
	      <entry>Device serial Number</entry>
	    </row>
	    <row>
	      <entry>2</entry>
	      <entry>uint16_t</entry>
	      <entry>tick</entry>
	      <entry>Device time in 100ths of a second</entry>
	    </row>
	    <row>
	      <entry>4</entry>
	      <entry>uint8_t</entry>
	      <entry>type</entry>
	      <entry>Packet type</entry>
	    </row>
	    <row>
	      <entry>5</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <para>
      Each packet starts with these five bytes which serve to identify
      which device has transmitted the packet, when it was transmitted
      and what the rest of the packet contains.
      </para>
    </section>
    <section>
      <title>Sensor Data</title>
      <informaltable frame='none' label='' tocentry='0'>
	<tgroup cols='2' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Offset'/>
	  <colspec align='left' colwidth='3*' colname='Description'/>
	  <thead>
	    <row>
	      <entry>Type</entry>
	      <entry>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>0x01</entry>
	      <entry>TeleMetrum Sensor Data</entry>
	    </row>
	    <row>
	      <entry>0x02</entry>
	      <entry>TeleMini Sensor Data</entry>
	    </row>
	    <row>
	      <entry>0x03</entry>
	      <entry>TeleNano Sensor Data</entry>
	    </row>
	  </tbody>
	</tgroup>
      </informaltable>
      <para>
	TeleMetrum, TeleMini and TeleNano share this same packet
	format for sensor data. Each uses a distinct packet type so
	that the receiver knows which data values are valid and which
	are undefined.
      </para>
      <para>
	Sensor Data packets are transmitted once per second on the
	ground, 10 times per second during ascent and once per second
	during descent and landing
      </para>
      <table frame='all'>
	<title>Sensor Packet Contents</title>
	<tgroup cols='4' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Offset'/>
	  <colspec align='center' colwidth='3*' colname='Data Type'/>
	  <colspec align='left' colwidth='3*' colname='Name'/>
	  <colspec align='left' colwidth='9*' colname='Description'/>
	  <thead>
	    <row>
	      <entry align='center'>Offset</entry>
	      <entry align='center'>Data Type</entry>
	      <entry align='center'>Name</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>5</entry><entry>uint8_t</entry><entry>state</entry><entry>Flight state</entry>
	    </row>
	    <row>
	      <entry>6</entry><entry>int16_t</entry><entry>accel</entry><entry>accelerometer (TM only)</entry>
	    </row>
	    <row>
	      <entry>8</entry><entry>int16_t</entry><entry>pres</entry><entry>pressure sensor</entry>
	    </row>
	    <row>
	      <entry>10</entry><entry>int16_t</entry><entry>temp</entry><entry>temperature sensor</entry>
	    </row>
	    <row>
	      <entry>12</entry><entry>int16_t</entry><entry>v_batt</entry><entry>battery voltage</entry>
	    </row>
	    <row>
	      <entry>14</entry><entry>int16_t</entry><entry>sense_d</entry><entry>drogue continuity sense (TM/Tm)</entry>
	    </row>
	    <row>
	      <entry>16</entry><entry>int16_t</entry><entry>sense_m</entry><entry>main continuity sense (TM/Tm)</entry>
	    </row>
	    <row>
	      <entry>18</entry><entry>int16_t</entry><entry>acceleration</entry><entry>m/s² * 16</entry>
	    </row>
	    <row>
	      <entry>20</entry><entry>int16_t</entry><entry>speed</entry><entry>m/s * 16</entry>
	    </row>
	    <row>
	      <entry>22</entry><entry>int16_t</entry><entry>height</entry><entry>m</entry>
	    </row>
	    <row>
	      <entry>24</entry><entry>int16_t</entry><entry>ground_pres</entry><entry>Average barometer reading on ground</entry>
	    </row>
	    <row>
	      <entry>26</entry><entry>int16_t</entry><entry>ground_accel</entry><entry>TM</entry>
	    </row>
	    <row>
	      <entry>28</entry><entry>int16_t</entry><entry>accel_plus_g</entry><entry>TM</entry>
	    </row>
	    <row>
	      <entry>30</entry><entry>int16_t</entry><entry>accel_minus_g</entry><entry>TM</entry>
	    </row>
	    <row>
	      <entry>32</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
    </section>
    <section>
      <title>Configuration Data</title>
      <informaltable frame='none' label='' tocentry='0'>
	<tgroup cols='2' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Offset'/>
	  <colspec align='left' colwidth='3*' colname='Description'/>
	  <thead>
	    <row>
	      <entry>Type</entry>
	      <entry>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>0x04</entry>
	      <entry>Configuration Data</entry>
	    </row>
	  </tbody>
	</tgroup>
      </informaltable>
      <para>
	This provides a description of the software installed on the
	flight computer as well as any user-specified configuration data.
      </para>
      <para>
	Configuration data packets are transmitted once per second
	during all phases of the flight
      </para>
      <table frame='all'>
	<title>Sensor Packet Contents</title>
	<tgroup cols='4' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Offset'/>
	  <colspec align='center' colwidth='3*' colname='Data Type'/>
	  <colspec align='left' colwidth='3*' colname='Name'/>
	  <colspec align='left' colwidth='9*' colname='Description'/>
	  <thead>
	    <row>
	      <entry align='center'>Offset</entry>
	      <entry align='center'>Data Type</entry>
	      <entry align='center'>Name</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>5</entry><entry>uint8_t</entry><entry>type</entry><entry>Device type</entry>
	    </row>
	    <row>
	      <entry>6</entry><entry>uint16_t</entry><entry>flight</entry><entry>Flight number</entry>
	    </row>
	    <row>
	      <entry>8</entry><entry>uint8_t</entry><entry>config_major</entry><entry>Config major version</entry>
	    </row>
	    <row>
	      <entry>9</entry><entry>uint8_t</entry><entry>config_minor</entry><entry>Config minor version</entry>
	    </row>
	    <row>
	      <entry>10</entry><entry>uint16_t</entry><entry>apogee_delay</entry>
	      <entry>Apogee deploy delay in seconds</entry>
	    </row>
	    <row>
	      <entry>12</entry><entry>uint16_t</entry><entry>main_deploy</entry><entry>Main deploy alt in meters</entry>
	    </row>
	    <row>
	      <entry>14</entry><entry>uint16_t</entry><entry>flight_log_max</entry>
	      <entry>Maximum flight log size (kB)</entry>
	    </row>
	    <row>
	      <entry>16</entry><entry>char</entry><entry>callsign[8]</entry><entry>Radio operator identifier</entry>
	    </row>
	    <row>
	      <entry>24</entry><entry>char</entry><entry>version[8]</entry><entry>Software version identifier</entry>
	    </row>
	    <row>
	      <entry>32</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
    </section>
    <section>
      <title>GPS Location</title>
      <informaltable frame='none' label='' tocentry='0'>
	<tgroup cols='2' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Offset'/>
	  <colspec align='left' colwidth='3*' colname='Description'/>
	  <thead>
	    <row>
	      <entry>Type</entry>
	      <entry>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>0x05</entry>
	      <entry>GPS Location</entry>
	    </row>
	  </tbody>
	</tgroup>
      </informaltable>
      <para>
	This packet provides all of the information available from the
	Venus SkyTraq GPS receiver—position, time, speed and precision
	estimates. 
      </para>
      <para>
	GPS Location packets are transmitted once per second during
	all phases of the flight
      </para>
      <table frame='all'>
	<title>GPS Location Packet Contents</title>
	<tgroup cols='4' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Offset'/>
	  <colspec align='center' colwidth='3*' colname='Data Type'/>
	  <colspec align='left' colwidth='3*' colname='Name'/>
	  <colspec align='left' colwidth='9*' colname='Description'/>
	  <thead>
	    <row>
	      <entry align='center'>Offset</entry>
	      <entry align='center'>Data Type</entry>
	      <entry align='center'>Name</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>5</entry><entry>uint8_t</entry><entry>flags</entry>
	      <entry>See GPS Flags table below</entry>
	    </row>
	    <row>
	      <entry>6</entry><entry>int16_t</entry><entry>altitude</entry><entry>m</entry>
	    </row>
	    <row>
	      <entry>8</entry><entry>int32_t</entry><entry>latitude</entry><entry>degrees * 10<superscript>7</superscript></entry>
	    </row>
	    <row>
	      <entry>12</entry><entry>int32_t</entry><entry>longitude</entry><entry>degrees * 10<superscript>7</superscript></entry>
	    </row>
	    <row>
	      <entry>16</entry><entry>uint8_t</entry><entry>year</entry>
	    </row>
	    <row>
	      <entry>17</entry><entry>uint8_t</entry><entry>month</entry>
	    </row>
	    <row>
	      <entry>18</entry><entry>uint8_t</entry><entry>day</entry>
	    </row>
	    <row>
	      <entry>19</entry><entry>uint8_t</entry><entry>hour</entry>
	    </row>
	    <row>
	      <entry>20</entry><entry>uint8_t</entry><entry>minute</entry>
	    </row>
	    <row>
	      <entry>21</entry><entry>uint8_t</entry><entry>second</entry>
	    </row>
	    <row>
	      <entry>22</entry><entry>uint8_t</entry><entry>pdop</entry><entry>* 5</entry>
	    </row>
	    <row>
	      <entry>23</entry><entry>uint8_t</entry><entry>hdop</entry><entry>* 5</entry>
	    </row>
	    <row>
	      <entry>24</entry><entry>uint8_t</entry><entry>vdop</entry><entry>* 5</entry>
	    </row>
	    <row>
	      <entry>25</entry><entry>uint8_t</entry><entry>mode</entry>
	      <entry>See GPS Mode table below</entry>
	    </row>
	    <row>
	      <entry>26</entry><entry>uint16_t</entry><entry>ground_speed</entry><entry>cm/s</entry>
	    </row>
	    <row>
	      <entry>28</entry><entry>int16_t</entry><entry>climb_rate</entry><entry>cm/s</entry>
	    </row>
	    <row>
	      <entry>30</entry><entry>uint8_t</entry><entry>course</entry><entry>/ 2</entry>
	    </row>
	    <row>
	      <entry>31</entry><entry>uint8_t</entry><entry>unused[1]</entry>
	    </row>
	    <row>
	      <entry>32</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <para>
	Packed into a one byte field are status flags and the count of
	satellites used to compute the position fix. Note that this
	number may be lower than the number of satellites being
	tracked; the receiver will not use information from satellites
	with weak signals or which are close enough to the horizon to
	have significantly degraded position accuracy.
      </para>
      <table frame='all'>
	<title>GPS Flags</title>
	<tgroup cols='3' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='bits'/>
	  <colspec align='left' colwidth='2*' colname='name'/>
	  <colspec align='left' colwidth='7*' colname='description'/>
	  <thead>
	    <row>
	      <entry align='center'>Bits</entry>
	      <entry align='center'>Name</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>0-3</entry>
	      <entry>nsats</entry>
	      <entry>Number of satellites in solution</entry>
	    </row>
	    <row>
	      <entry>4</entry>
	      <entry>valid</entry>
	      <entry>GPS solution is valid</entry>
	    </row>
	    <row>
	      <entry>5</entry>
	      <entry>running</entry>
	      <entry>GPS receiver is operational</entry>
	    </row>
	    <row>
	      <entry>6</entry>
	      <entry>date_valid</entry>
	      <entry>Reported date is valid</entry>
	    </row>
	    <row>
	      <entry>7</entry>
	      <entry>course_valid</entry>
	      <entry>ground speed, course and climb rates are valid</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <para>
	Here are all of the valid GPS operational modes. Altus Metrum
	products will only ever report 'N' (not valid), 'A'
	(Autonomous) modes or 'E' (Estimated). The remaining modes
	are either testing modes or require additional data.
      </para>
      <table frame='all'>
	<title>GPS Mode</title>
	<tgroup cols='3' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='value'/>
	  <colspec align='center' colwidth='3*' colname='name'/>
	  <colspec align='left' colwidth='7*' colname='description'/>
	  <thead>
	    <row>
	      <entry align='center'>Mode</entry>
	      <entry align='center'>Name</entry>
	      <entry align='center'>Decsription</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>N</entry>
	      <entry>Not Valid</entry>
	      <entry>All data are invalid</entry>
	    </row>
	    <row>
	      <entry>A</entry>
	      <entry>Autonomous mode</entry>
	      <entry>Data are derived from satellite data</entry>
	    </row>
	    <row>
	      <entry>D</entry>
	      <entry>Differential Mode</entry>
	      <entry>
		  Data are augmented with differential data from a
		  known ground station. The SkyTraq unit in TeleMetrum
		  does not support this mode
		</entry>
	    </row>
	    <row>
	      <entry>E</entry>
	      <entry>Estimated</entry>
	      <entry>
		  Data are estimated using dead reckoning from the
		  last known data
		</entry>
	    </row>
	    <row>
	      <entry>M</entry>
	      <entry>Manual</entry>
	      <entry>Data were entered manually</entry>
	    </row>
	    <row>
	      <entry>S</entry>
	      <entry>Simulated</entry>
	      <entry>GPS receiver testing mode</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
    </section>
    <section>
      <title>GPS Satellite Data</title>
      <informaltable frame='none' label='' tocentry='0'>
	<tgroup cols='2' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Offset'/>
	  <colspec align='left' colwidth='3*' colname='Description'/>
	  <thead>
	    <row>
	      <entry>Type</entry>
	      <entry>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>0x06</entry>
	      <entry>GPS Satellite Data</entry>
	    </row>
	  </tbody>
	</tgroup>
      </informaltable>
      <para>
	This packet provides space vehicle identifiers and signal
	quality information in the form of a C/N1 number for up to 12
	satellites. The order of the svids is not specified.
      </para>
      <para>
	GPS Satellite data are transmitted once per second during all
	phases of the flight.
      </para>
      <table frame='all'>
	<title>GPS Satellite Data Contents</title>
	<tgroup cols='4' align='center' colsep='1' rowsep='1'>
	  <colspec align='right' colwidth='*' colname='Offset'/>
	  <colspec align='center' colwidth='3*' colname='Data Type'/>
	  <colspec align='left' colwidth='3*' colname='Name'/>
	  <colspec align='left' colwidth='9*' colname='Description'/>
	  <thead>
	    <row>
	      <entry align='center'>Offset</entry>
	      <entry align='center'>Data Type</entry>
	      <entry align='center'>Name</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>5</entry><entry>uint8_t</entry><entry>channels</entry>
	      <entry>Number of reported satellite information</entry>
	    </row>
	    <row>
	      <entry>6</entry><entry>sat_info_t</entry><entry>sats[12]</entry>
	      <entry>See Per-Satellite data table below</entry>
	    </row>
	    <row>
	      <entry>30</entry><entry>uint8_t</entry><entry>unused[2]</entry>
	    </row>
	    <row>
	      <entry>32</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <table frame='all'>
	<title>GPS Per-Satellite data (sat_info_t)</title>
	<tgroup cols='4' align='center' colsep='1' rowsep='1'>
	  <colspec align='right' colwidth='*' colname='Offset'/>
	  <colspec align='center' colwidth='3*' colname='Data Type'/>
	  <colspec align='left' colwidth='3*' colname='Name'/>
	  <colspec align='left' colwidth='9*' colname='Description'/>
	  <thead>
	    <row>
	      <entry align='center'>Offset</entry>
	      <entry align='center'>Data Type</entry>
	      <entry align='center'>Name</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>0</entry><entry>uint8_t</entry><entry>svid</entry>
	      <entry>Space Vehicle Identifier</entry>
	    </row>
	    <row>
	      <entry>1</entry><entry>uint8_t</entry><entry>c_n_1</entry>
	      <entry>C/N1 signal quality indicator</entry>
	    </row>
	    <row>
	      <entry>2</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
    </section>
  </section>
  <section>
    <title>Data Transmission</title>
    <para>
      Altus Metrum devices use the Texas Instruments CC1111
      microcontroller which includes an integrated sub-GHz digital
      transceiver. This transceiver is used to both transmit and
      receive the telemetry packets. This section discusses what
      modulation scheme is used and how this device is configured.
    </para>
    <section>
      <title>Modulation Scheme</title>
      <para>
	Texas Instruments provides a tool for computing modulation
	parameters given a desired modulation format and basic bit
	rate. For AltOS, the basic bit rate was specified as 38 kBaud,
	resulting in the following signal parmeters:
      </para>
      <table>
	<title>Modulation Scheme</title>
	<tgroup cols='3'>
	  <colspec align="center" colwidth="*" colname="parameter"/>
	  <colspec align="center" colwidth="*" colname="value"/>
	  <colspec align="center" colwidth="*" colname="description"/>
	  <thead>
	    <row>
	      <entry align='center'>Parameter</entry>
	      <entry align='center'>Value</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>Modulation</entry>
	      <entry>GFSK</entry>
	      <entry>Gaussian Frequency Shift Keying</entry>
	    </row>
	    <row>
	      <entry>Deviation</entry>
	      <entry>20.507812 kHz</entry>
	      <entry>Frequency modulation</entry>
	    </row>
	    <row>
	      <entry>Data rate</entry>
	      <entry>38.360596 kBaud</entry>
	      <entry>Raw bit rate</entry>
	    </row>
	    <row>
	      <entry>RX Filter Bandwidth</entry>
	      <entry>93.75 kHz</entry>
	      <entry>Receiver Band pass filter bandwidth</entry>
	    </row>
	    <row>
	      <entry>IF Frequency</entry>
	      <entry>140.62 kHz</entry>
	      <entry>Receiver intermediate frequency</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
    </section>
    <section>
      <title>Error Correction</title>
      <para>
	The cc1111 provides forward error correction in hardware,
	which AltOS uses to improve reception of weak signals. The
	overall effect of this is to halve the available bandwidth for
	data from 38 kBaud to 19 kBaud.
      </para>
      <table>
	<title>Error Correction</title>
	<tgroup cols='3'>
	  <colspec align="center" colwidth="*" colname="parameter"/>
	  <colspec align="center" colwidth="*" colname="value"/>
	  <colspec align="center" colwidth="*" colname="description"/>
	  <thead>
	    <row>
	      <entry align='center'>Parameter</entry>
	      <entry align='center'>Value</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>Error Correction</entry>
	      <entry>Convolutional coding</entry>
	      <entry>1/2 rate, constraint length m=4</entry>
	    </row>
	    <row>
	      <entry>Interleaving</entry>
	      <entry>4 x 4</entry>
	      <entry>Reduce effect of noise burst</entry>
	    </row>
	    <row>
	      <entry>Data Whitening</entry>
	      <entry>XOR with 9-bit PNR</entry>
	      <entry>Rotate right with bit 8 = bit 0 xor bit 5, initial
	      value 111111111</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
    </section>
  </section>
  <section>
    <title>TeleDongle packet format</title>
    <para>
      TeleDongle does not do any interpretation of the packet data,
      instead it is configured to receive packets of a specified
      length (32 bytes in this case). For each received packet,
      TeleDongle produces a single line of text. This line starts with
      the string "TELEM " and is followed by a list of hexadecimal
      encoded bytes.
    </para>
    <programlisting>TELEM 224f01080b05765e00701f1a1bbeb8d7b60b070605140c000600000000000000003fa988</programlisting>
    <para>
      The hexadecimal encoded string of bytes contains a length byte,
      the packet data, two bytes added by the cc1111 radio receiver
      hardware and finally a checksum so that the host software can
      validate that the line was transmitted without any errors.
    </para>
    <table>
      <title>Packet Format</title>
      <tgroup cols='4'>
	<colspec align="center" colwidth="2*" colname="offset"/>
	<colspec align="center" colwidth="*" colname="name"/>
	<colspec align="center" colwidth="*" colname="value"/>
	<colspec align="center" colwidth="5*" colname="description"/>
	<thead>
	  <row>
	    <entry align='center'>Offset</entry>
	    <entry align='center'>Name</entry>
	    <entry align='center'>Example</entry>
	    <entry align='center'>Description</entry>
	  </row>
	</thead>
	<tbody>
	  <row>
	    <entry>0</entry>
	    <entry>length</entry>
	    <entry>22</entry>
	    <entry>Total length of data bytes in the line. Note that
	    this includes the added RSSI and status bytes</entry>
	  </row>
	  <row>
	    <entry>1 ·· length-3</entry>
	    <entry>packet</entry>
	    <entry>4f ·· 00</entry>
	    <entry>Bytes of actual packet data</entry>
	  </row>
	  <row>
	    <entry>length-2</entry>
	    <entry>rssi</entry>
	    <entry>3f</entry>
	    <entry>Received signal strength. dBm = rssi / 2 - 74</entry>
	  </row>
	  <row>
	    <entry>length-1</entry>
	    <entry>lqi</entry>
	    <entry>a9</entry>
	    <entry>Link Quality Indicator and CRC status. Bit 7
	    is set when the CRC is correct</entry>
	  </row>
	  <row>
	    <entry>length</entry>
	    <entry>checksum</entry>
	    <entry>88</entry>
	    <entry>(0x5a + sum(bytes 1 ·· length-1)) % 256</entry>
	  </row>
	</tbody>
      </tgroup>
    </table>
  </section>
  <section>
    <title>History and Motivation</title>
    <para>
      The original AltoOS telemetry mechanism encoded everything
      available piece of information on the TeleMetrum hardware into a
      single unified packet. Initially, the packets contained very
      little data—some raw sensor readings along with the current GPS
      coordinates when a GPS receiver was connected. Over time, the
      amount of data grew to include sensor calibration data, GPS
      satellite information and a host of internal state information
      designed to help diagnose flight failures in case of a loss of
      the on-board flight data.
    </para>
    <para>
      Because every packet contained all of the data, packets were
      huge—95 bytes long. Much of the information was also specific to
      the TeleMetrum hardware. With the introduction of the TeleMini
      flight computer, most of the data contained in the telemetry
      packets was unavailable. Initially, a shorter, but still
      comprehensive packet was implemented. This required that the
      ground station be pre-configured as to which kind of packet to
      expect.
    </para>
    <para>
      The development of several companion boards also made the
      shortcomings evident—each companion board would want to include
      telemetry data in the radio link; with the original design, the
      packet would have to hold the new data as well, requiring
      additional TeleMetrum and ground station changes.
    </para>
  </section>
</article>
