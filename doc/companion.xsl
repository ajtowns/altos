<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE article PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
"/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">

<article>
  <articleinfo>
    <title>AltOS Companion Port</title>
    <subtitle>Protocol Definitions</subtitle>
    <author>
      <firstname>Keith</firstname>
      <surname>Packard</surname>
    </author>
    <copyright>
      <year>2012</year>
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
	<date>13 January 2012</date>
	<revremark>Initial content</revremark>
      </revision>
    </revhistory>
  </articleinfo>
  <section>
    <title>Companion Port</title>
    <para>
      Many Altus Metrum products come with an eight pin Micro MaTch
      connector, called the Companion Port. This is often used to
      program devices using a programming cable. However, it can also
      be used to connect TeleMetrum to external companion boards
      (hence the name).
    </para>
    <para>
      The Companion Port provides two different functions:
      <itemizedlist>
	<listitem>
	  <para>
	  Power. Both battery-level and 3.3V regulated power are
	  available. Note that the amount of regulated power is not
	  huge; TeleMetrum contains a 150mA regulator and uses, at
	  peak, about 120mA or so. For applications needing more than
	  a few dozen mA, placing a separate regulator on them and
	  using the battery for power is probably a good idea.
	  </para>
	</listitem>
	<listitem>
	  <para>
	  SPI. The flight computer operates as a SPI master, using
	  a protocol defined in this document. Companion boards
	  provide a matching SPI slave implementation which supplies
	  telemetry information for the radio downlink during flight
	  </para>
	</listitem>
      </itemizedlist>
    </para>
  </section>
  <section>
    <title>Companion SPI Protocol</title>
    <para>
      The flight computer implements a SPI master communications
      channel over the companion port, and uses this to get
      information about a connected companion board and then to get
      telemetry data for transmission during flight.
    </para>
    <para>
      At startup time, the flight computer sends a setup request
      packet, and the companion board returns a board identifier, the
      desired telemetry update period and the number of data channels
      provided. The flight computer doesn't interpret the telemetry
      data at all, simply packing it up and sending it over the link.
      Telemetry packets are 32 bytes long, and companion packets use 8
      bytes as a header leaving room for a maximum of 12 16-bit data
      values.
    </para>
    <para>
      Because of the limits of the AVR processors used in the first
      two companion boards, the SPI data rate is set to 187.5kbaud.
    </para>
  </section>
  <section>
    <title>SPI Message Formats</title>
    <para>
    This section first defines the command message format sent from
    the flight computer to the companion board, and then the various
    reply message formats for each type of command message.
    </para>
    <section>
      <title>Command Message</title>
      <table frame='all'>
	<title>Companion Command Message</title>
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
	      <entry>uint8_t</entry>
	      <entry>command</entry>
	      <entry>Command identifier</entry>
	    </row>
	    <row>
	      <entry>1</entry>
	      <entry>uint8_t</entry>
	      <entry>flight_state</entry>
	      <entry>Current flight computer state</entry>
	    </row>
	    <row>
	      <entry>2</entry>
	      <entry>uint16_t</entry>
	      <entry>tick</entry>
	      <entry>Flight computer clock (100 ticks/second)</entry>
	    </row>
	    <row>
	      <entry>4</entry>
	      <entry>uint16_t</entry>
	      <entry>serial</entry>
	      <entry>Flight computer serial number</entry>
	    </row>
	    <row>
	      <entry>6</entry>
	      <entry>uint16_t</entry>
	      <entry>flight</entry>
	      <entry>Flight number</entry>
	    </row>
	    <row>
	      <entry>8</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <table frame='all'>
	<title>Companion Command Identifiers</title>
	<tgroup cols='3' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Value'/>
	  <colspec align='left' colwidth='3*' colname='Name'/>
	  <colspec align='left' colwidth='9*' colname='Description'/>
	  <thead>
	    <row>
	      <entry>Value</entry>
	      <entry>Name</entry>
	      <entry>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>1</entry>
	      <entry>SETUP</entry>
	      <entry>Supply the flight computer with companion
	      information</entry>
	    </row>
	    <row>
	      <entry>2</entry>
	      <entry>FETCH</entry>
	      <entry>Return telemetry information</entry>
	    </row>
	    <row>
	      <entry>3</entry>
	      <entry>NOTIFY</entry>
	      <entry>Tell companion board when flight state
	      changes</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <para>
	The flight computer will send a SETUP message shortly after
	power-up and will then send FETCH messages no more often than
	the rate specified in the SETUP reply. NOTIFY messages will be
	sent whenever the flight state changes.
      </para>
      <para>
	'flight_state' records the current state of the flight,
	whether on the pad, under power, coasting to apogee or
	descending on the drogue or main chute.
      </para>
      <para>
	'tick' provides the current flight computer clock, which 
	be used to synchronize data recorded on the flight computer
	with that recorded on the companion board in post-flight analysis.
      </para>
      <para>
	'serial' is the product serial number of the flight computer,
	'flight' is the flight sequence number. Together, these two
	uniquely identify the flight and can be recorded with any
	companion board data logging to associate the companion data
	with the proper flight.
      </para>
      <para>
	NOTIFY commands require no reply at all, they are used solely
	to inform the companion board when the state of the flight, as
	computed by the flight computer, changes. Companion boards can
	use this to change data collection parameters, disabling data
	logging until the flight starts and terminating it when the
	flight ends.
      </para>
    </section>
    <section>
      <title>SETUP reply message</title>
      <table frame='all'>
	<title>SETUP reply contents</title>
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
	      <entry>board_id</entry>
	      <entry>Board identifier</entry>
	    </row>
	    <row>
	      <entry>2</entry>
	      <entry>uint16_t</entry>
	      <entry>board_id_inverse</entry>
	      <entry>~board_id—used to tell if a board is present</entry>
	    </row>
	    <row>
	      <entry>4</entry>
	      <entry>uint8_t</entry>
	      <entry>update_period</entry>
	      <entry>Minimum time (in 100Hz ticks) between FETCH commands</entry>
	    </row>
	    <row>
	      <entry>5</entry>
	      <entry>uint8_t</entry>
	      <entry>channels</entry>
	      <entry>Number of data channels to retrieve in FETCH command</entry>
	    </row>
	    <row>
	      <entry>6</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <para>
	The SETUP reply contains enough information to uniquely
	identify the companion board to the end user as well as for
	the flight computer to know how many data values to expect in
	reply to a FETCH command, and how often to fetch that data.
      </para>
      <para>
	To detect the presence of a companion board, the flight
	computer checks to make sure that board_id_inverse is the
	bit-wise inverse of board_id. Current companion boards use
	USB product ID as the board_id, but the flight computer does
	not interpret this data and so it can be any value.
      </para>
    </section>
    <section>
      <title>FETCH reply message</title>
      <table frame='all'>
	<title>FETCH reply contents</title>
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
	      <entry>data0</entry>
	      <entry>0th data item</entry>
	    </row>
	    <row>
	      <entry>2</entry>
	      <entry>uint16_t</entry>
	      <entry>data1</entry>
	      <entry>1st data item</entry>
	    </row>
	    <row>
	      <entry>...</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <para>
	The FETCH reply contains arbitrary data to be reported over
	the flight computer telemetry link. The number of 16-bit data items
	must match the 'channels' value provided in the SETUP reply
	message.
      </para>
    </section>
  </section>
  <section>
    <title>History and Motivation</title>
    <para>
      To allow cross-programming, the original TeleMetrum and
      TeleDongle designs needed to include some kind of
      connector. With that in place, adding the ability to connect
      external cards to TeleMetrum was fairly simple. We set the
      software piece of this puzzle aside until we had a companion
      board to use.
    </para>
    <para>
      The first companion board was TeleScience. Designed to collect
      temperature data from the nose and fin of the airframe, the main
      requirement for the companion port was that it be able to report
      telemetry data during flight as a back-up in case the
      TeleScience on-board data was lost.
    </para>
    <para>
      The second companion board, TelePyro, provides 8 additional
      channels for deployment, staging or other activities. To avoid
      re-programming the TeleMetrum to use TelePyro, we decided to
      provide enough information over the companion link for it to
      independently control those channels.
    </para>
    <para>
      Providing a standard, constant interface between the flight
      computer and companion boards allows for the base flight
      computer firmware to include support for companion boards.
    </para>
  </section>
</article>
