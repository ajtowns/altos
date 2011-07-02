<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
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
    This section first defines the packet header common to all packets
    and then the per-packet data layout.
    <section>
      <title>Packet Header</title>
      <table frame='all'>
	<title>Telemetry Packet Header</title>
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
      <table frame='all'>
	<title>Sensor Packet Contents</title>
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
	      <entry>18</entry><entry>int16_t</entry><entry>accel</entry><entry>m/s² * 16</entry>
	    </row>
	    <row>
	      <entry>20</entry><entry>int16_t</entry><entry>speed</entry><entry>m/s * 16</entry>
	    </row>
	    <row>
	      <entry>22</entry><entry>int16_t</entry><entry>height</entry><entry>m</entry>
	    </row>
	    <row>
	      <entry>24</entry><entry>int16_t</entry><entry>ground_accel</entry><entry>TM</entry>
	    </row>
	    <row>
	      <entry>26</entry><entry>int16_t</entry><entry>ground_pres</entry><entry>Average barometer reading on ground</entry>
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
      <table frame='all'>
	<title>Sensor Packet Contents</title>
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
	      <entry>10</entry><entry>uint16_t</entry><entry>main_deploy</entry><entry>Main deploy alt in meters</entry>
	    </row>
	    <row>
	      <entry>12</entry><entry>uint32_t</entry><entry>flight_log_max</entry><entry>Maximum flight log size (B)</entry>
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
      <table frame='all'>
	<title>GPS Location Packet Contents</title>
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
	      <entry>5</entry><entry>uint8_t</entry><entry>flags</entry><entry>GPS Flags (see below)</entry>
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
	      <entry>25</entry><entry>uint8_t</entry><entry>mode</entry><entry>N, A, D, E, M, S</entry>
	    </row>
	    <row>
	      <entry>26</entry><entry>uint16_t</entry><entry>ground_speed</entry><entry>cm/s</entry>
	    </row>
	    <row>
	      <entry>28</entry><entry>uint8_t</entry><entry>course</entry><entry>/ 2</entry>
	    </row>
	    <row>
	      <entry>29</entry><entry>uint8_t</entry><entry>unused[2]</entry>
	    </row>
	    <row>
	      <entry>32</entry>
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
	    </row>
	    <row>
	      <entry>6</entry><entry>uint8_t</entry><entry>sat_0_id</entry>
	    </row>
	    <row>
	      <entry>7</entry><entry>uint8_t</entry><entry>sat_0_c_n_1</entry>
	    </row>
	    <row>
	      <entry>8</entry><entry>uint8_t</entry><entry>sat_1_id</entry>
	    </row>
	    <row>
	      <entry>9</entry><entry>uint8_t</entry><entry>sat_1_c_n_1</entry>
	    </row>
	    <row>
	      <entry>10</entry><entry>uint8_t</entry><entry>sat_2_id</entry>
	    </row>
	    <row>
	      <entry>11</entry><entry>uint8_t</entry><entry>sat_2_c_n_1</entry>
	    </row>
	    <row>
	      <entry>12</entry><entry>uint8_t</entry><entry>sat_3_id</entry>
	    </row>
	    <row>
	      <entry>13</entry><entry>uint8_t</entry><entry>sat_3_c_n_1</entry>
	    </row>
	    <row>
	      <entry>14</entry><entry>uint8_t</entry><entry>sat_4_id</entry>
	    </row>
	    <row>
	      <entry>15</entry><entry>uint8_t</entry><entry>sat_4_c_n_1</entry>
	    </row>
	    <row>
	      <entry>16</entry><entry>uint8_t</entry><entry>sat_5_id</entry>
	    </row>
	    <row>
	      <entry>17</entry><entry>uint8_t</entry><entry>sat_5_c_n_1</entry>
	    </row>
	    <row>
	      <entry>18</entry><entry>uint8_t</entry><entry>sat_6_id</entry>
	    </row>
	    <row>
	      <entry>19</entry><entry>uint8_t</entry><entry>sat_6_c_n_1</entry>
	    </row>
	    <row>
	      <entry>20</entry><entry>uint8_t</entry><entry>sat_7_id</entry>
	    </row>
	    <row>
	      <entry>21</entry><entry>uint8_t</entry><entry>sat_7_c_n_1</entry>
	    </row>
	    <row>
	      <entry>22</entry><entry>uint8_t</entry><entry>sat_8_id</entry>
	    </row>
	    <row>
	      <entry>23</entry><entry>uint8_t</entry><entry>sat_8_c_n_1</entry>
	    </row>
	    <row>
	      <entry>24</entry><entry>uint8_t</entry><entry>sat_9_id</entry>
	    </row>
	    <row>
	      <entry>25</entry><entry>uint8_t</entry><entry>sat_9_c_n_1</entry>
	    </row>
	    <row>
	      <entry>26</entry><entry>uint8_t</entry><entry>sat_10_id</entry>
	    </row>
	    <row>
	      <entry>27</entry><entry>uint8_t</entry><entry>sat_10_c_n_1</entry>
	    </row>
	    <row>
	      <entry>28</entry><entry>uint8_t</entry><entry>sat_11_id</entry>
	    </row>
	    <row>
	      <entry>29</entry><entry>uint8_t</entry><entry>sat_11_c_n_1</entry>
	    </row>
	    <row>
	      <entry>30</entry><entry>uin8_t</entry><entry>unused30</entry>
	    </row>
	    <row>
	      <entry>31</entry><entry>uin8_t</entry><entry>unused31</entry>
	    </row>
	    <row>
	      <entry>32</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
    </section>
  </section>
</article>
      
<!--

      
      
  
 Rethinking this (over IRC), and thinking about telemetry from a
 companion board, it looks like 32 bytes per packet would be better (12
 channels of A/D from telescience would require 24 bytes).

 Packet type 0x06: GPS satellites
 1 packet per second.


 TeleScience

<entry>5</entry><entry>uint8_t</entry><entry>channels</entry>
<entry>6</entry><entry>uint16_t</entry><entry>data[12]</entry>
<entry>30</entry><entry>uint8_t</entry><entry>unused[2]</entry>
<entry>32</entry>

 [ 2-line signature. Click/Enter to show. ]
 - - 
 keith.packard@intel.com
 [ application/pgp-signature ]
  Keith Packard <keithp@keithp.com> (Mon. 18:59) (me sent)
  Subject: Re: New telemetry ideas
  To: bdale@gag.com
  Date: Mon, 27 Jun 2011 18:59:08 -0700

  [ multipart/signed ]
  [ text/plain ]
  On Mon, 27 Jun 2011 14:25:00 -0700, Keith Packard <keithp@keithp.com> wrote:
  Non-text part: multipart/signed

  > Packet type 0x05: GPS location
  > 1 packet per second
  > 
  > 5       uint8_t         flags

  Let's add another field here:

<entry>6</entry><entry>int16_t</entry><entry>gps_tick</entry><entry>tick when GPS data was received</entry>

  That leaves a single byte unused
-->