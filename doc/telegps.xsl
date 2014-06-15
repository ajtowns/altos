<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
  "/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">
<book>
  <title>TeleGPS Owner's Manual</title>
  <subtitle>A recording GPS tracker</subtitle>
  <bookinfo>
    <author>
      <firstname>Keith</firstname>
      <surname>Packard</surname>
    </author>
    <copyright>
      <year>2014</year>
      <holder>Bdale Garbee and Keith Packard</holder>
    </copyright>
    <mediaobject>
      <imageobject>
	<imagedata fileref="telegps-v1.0-top.jpg" width="4in"/>
      </imageobject>
    </mediaobject>
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
	<revnumber>1.4</revnumber>
	<date>13 June 2014</date>
	<revremark>
	  Initial release
	</revremark>
      </revision>
    </revhistory>
  </bookinfo>
  <dedication>
    <title>Acknowledgements</title>
    <para>
      Have fun using these products, and we hope to meet all of you
      out on the rocket flight line somewhere.
      <literallayout>
Bdale Garbee, KB0G
NAR #87103, TRA #12201

Keith Packard, KD7SQG
NAR #88757, TRA #12200
      </literallayout>
    </para>
  </dedication>
  <chapter>
    <title>Quick Start Guide</title>
    <para>
      TeleGPS is designed to be easy to use. Requiring no external
      components, flying takes just a few steps.
    </para>
    <para>
      First, download and install the software from <ulink
      url="http://altusmetrum.org/AltOS"/>. This will make sure that
      you have the right device drivers installed.
    </para>
    <para>
      Next, plug in the battery and USB cable and connect TeleGPS to
      your computer. This will charge the battery and allow you to
      configure the device.
    </para>
    <para>
      Start the TeleGPS application and set the callsign and frequency
      on your TeleGPS device; refer to the Configure TeleGPS section
      in the TeleGPS Application chapter for instructions.
    </para>
    <para>
      Unplug TeleGPS when the battery charger light goes green. This
      will enable the radio and logging portions of the TeleGPS
      firmware.
    </para>
    <para>
      Connect TeleDongle to your computer and start TeleGPS or start
      AltosDroid on your android device and connect to TeleBT. Set the
      frequency to match the TeleGPS and you should be receiving telemetry.
    </para>
  </chapter>
  <chapter>
    <title>Handling Precautions</title>
    <para>
      All Altus Metrum products are sophisticated electronic devices.  
      When handled gently and properly installed in an air-frame, they
      will deliver impressive results.  However, as with all electronic 
      devices, there are some precautions you must take.
    </para>
    <para>
      The Lithium polymer batteries have an
      extraordinary power density.  This is great because we can fly with
      much less battery mass... but if they are punctured
      or their contacts are allowed to short, they can and will release their
      energy very rapidly!
      Thus we recommend that you take some care when handling TeleGPS
      to keep conductive material from coming in contact with the exposed metal elements.
    </para>
    <para>
      As with all other rocketry electronics, Altus Metrum devices must 
      be protected from exposure to corrosive motor exhaust and ejection 
      charge gasses.
    </para>
  </chapter>
  <chapter>
    <title>TeleGPS Hardware</title>
    <section>
      <title>Hooking Up Lithium Polymer Batteries</title>
      <para>
	TeleGPS has a two pin JST PH series connector to connect up
	a single-cell Lithium Polymer cell (3.7V nominal). You can
	purchase matching batteries from the Altus Metrum store, or
	other vendors, or you can make your own. Pin 1 of the
	connector is positive, pin 2 is negative. Spark Fun sells a
	cable with the connector attached, which they call a <ulink
	url="https://www.sparkfun.com/products/9914">JST Jumper 2
	Wire Assembly</ulink>.
      </para>
      <para>
	Many RC vendors also sell lithium polymer batteries with
	this same connector. All that we have found use the opposite
	polarity, and if you use them that way, you will damage or
	destroy TeleGPS.
      </para>
    </section>
    <section>
      <title>On-board Data Recording</title>
      <para>
	TeleGPS logs GPS data at a user-configurable rate. Data are
	logged to a 2MB on-board flash memory part, which can be
	partitioned into several equal-sized blocks, one for each
	flight. 64kB of this storage are reserved to hold
	configuration data, leaving 1984kB for flight data.
      </para>
      <para>
	The on-board flash is partitioned into separate flight logs,
	each of a fixed maximum size. Increase the maximum size of
	each log and you reduce the number of flights that can be
	stored. Decrease the size and you can store more flights.
      </para>
      <para>
	To compute the amount of space needed for a single log, you
	can divide the expected time (in seconds) by the sample period
	(by default, 1 second per sample) and then multiply the result
	by 32 bytes per sample. For instance, a sample period of 1
	second and a flight lasting one hour will take 32 * 3600 =
	115200 bytes. TeleGPS does try to reduce log space used by not
	recording position information when it isn't moving, so actual
	space consumed may be less than this.
      </para>
      <para>
	The default size allows for four flights of 496kB each, which
	provides over four hours of logging at 1 sample per second.
      </para>
      <para>
	TeleGPS will not overwrite existing flight data, so be sure to
	download flight data and erase it from the onboard flash
	before it fills up. TeleGPS will still report telemetry even
	if memory is full, so the only thing you will lose is the
	on-board data log.
      </para>
    </section>
    <section>
      <title>Installation</title>
      <para>
	The battery connectors are a standard 2-pin JST connector and
	match batteries sold by Spark Fun. These batteries are
	single-cell Lithium Polymer batteries that nominally provide 3.7
	volts.  Other vendors sell similar batteries for RC aircraft
	using mating connectors, however the polarity for those is
	generally reversed from the batteries used by Altus Metrum
	products. In particular, the Tenergy batteries supplied for use
	in Featherweight flight computers are not compatible with Altus
	Metrum flight computers or battery chargers. <emphasis>Check
	polarity and voltage before connecting any battery not purchased
	from Altus Metrum or Spark Fun.</emphasis>
      </para>
      <para>
	TeleGPS uses an integrate GPS patch antenna and won't
	receive GPS signals if installed inside a metal or carbon
	fiber compartment. Test GPS reception and telemetry
	transmission with the system installed and all other
	electronics powered up to verify signal reception and make
	sure there isn't any interference from other systems.
      </para>
    </section>
  </chapter>
  <chapter>
    <title>System Operation</title>
    <section>
      <title>GFSK Telemetry</title>
      <para>
        TeleGPS's native telemetry system doesn't use a 'normal packet
        radio' mode like APRS because it's not very efficient.  The
        GFSK modulation we use is FSK with the base-band pulses passed
        through a Gaussian filter before they go into the modulator to
        limit the transmitted bandwidth.  When combined with forward
        error correction and interleaving, this allows us to have a
        very robust 19.2 kilobit data link with only 10-40 milliwatts
        of transmit power, a whip antenna in the rocket, and a
        hand-held Yagi on the ground.  We've had flights to above 21k
        feet AGL with great reception, and calculations suggest we
        should be good to well over 40k feet AGL with a 5-element yagi
        on the ground with our 10mW units and over 100k feet AGL with
        the 40mW devices.
      </para>
    </section>
    <section>
      <title>APRS</title>
      <para>
	TeleGPS can send APRS if desired, and the
	interval between APRS packets can be configured. As each APRS
	packet takes a full second to transmit, we recommend an
	interval of at least 5 seconds to avoid consuming too much
	battery power or radio channel bandwidth. You can configure
	the APRS interval using AltosUI; that process is described in
	the Configure Altimeter section of the AltosUI chapter.
      </para>
      <para>
	AltOS uses the APRS compressed position report data format,
	which provides for higher position precision and shorter
	packets than the original APRS format. It also includes
	altitude data, which is invaluable when tracking rockets. We
	haven't found a receiver which doesn't handle compressed
	positions, but it's just possible that you have one, so if you
	have an older device that can receive the raw packets but
	isn't displaying position information, it's possible that this
	is the cause.
      </para>
      <para>
	The APRS packet format includes a comment field that can have
	arbitrary text in it. AltOS uses this to send status
	information about the flight computer. It sends four fields as
	shown in the following table.
      </para>
      <table frame='all'>
	<title>Altus Metrum APRS Comments</title>
	<?dbfo keep-together="always"?>
	<tgroup cols='3' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='*' colname='Field'/>
	  <colspec align='center' colwidth='*' colname='Example'/>
	  <colspec align='center' colwidth='4*' colname='Description'/>
	  <thead>
	    <row>
	      <entry align='center'>Field</entry>
	      <entry align='center'>Example</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>1</entry>
	      <entry>L</entry>
	      <entry>GPS Status U for unlocked, L for locked</entry>
	    </row>
	    <row>
	      <entry>2</entry>
	      <entry>6</entry>
	      <entry>Number of Satellites in View</entry>
	    </row>
	    <row>
	      <entry>3</entry>
	      <entry>B4.0</entry>
	      <entry>Battery Voltage</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <para>
	Here's an example of an APRS comment showing GPS lock with 6
	satellites in view and a battery at 4.0V.
	<screen>
	  L6 B4.0
	</screen>
      </para>
      <para>
	Make sure your primary battery is above 3.8V and GPS is locked
	with at least 5 or 6 satellites in view before starting. If GPS
	is switching between L and U regularly, then it doesn't have a
	good lock and you should wait until it becomes stable.
      </para>
      <para>
	If the GPS receiver loses lock, the APRS data transmitted will
	contain the last position for which GPS lock was
	available. You can tell that this has happened by noticing
	that the GPS status character switches from 'L' to 'U'. Before
	GPS has locked, APRS will transmit zero for latitude,
	longitude and altitude.
      </para>
    </section>
    <section>
      <title>Configurable Parameters</title>
      <para>
        Configuring TeleGPS is very
        simple; the few configurable parameters can all be set
        using the TeleGPS application over USB. Read
	the Configure TeleGPS section in the TeleGPS Software chapter below
	for more information.
      </para>
      <section>
        <title>Radio Frequency</title>
        <para>
	  Altus Metrum boards support radio frequencies in the 70cm
	  band. By default, the configuration interface provides a
	  list of 10 “standard” frequencies in 100kHz channels starting at
	  434.550MHz.  However, the firmware supports use of
	  any 50kHz multiple within the 70cm band. At any given
	  launch, we highly recommend coordinating when and by whom each
	  frequency will be used to avoid interference.  And of course, both
	  TeleGPS and the receiver must be configured to the same
	  frequency to successfully communicate with each other.
        </para>
      </section>
      <section>
	<title>Callsign</title>
	<para>
	  This sets the callsign used for telemetry and APRS to
	  identify the device.
	</para>
      </section>
      <section>
	<title>Telemetry/RDF/APRS Enable</title>
	<para>
	  You can completely disable the radio, if necessary, leaving
	  TeleGPS only logging data to internal memory.
	</para>
      </section>
      <section>
	<title>APRS Interval</title>
	<para>
	  This selects how often APRS packets are transmitted. Set
	  this to zero to disable APRS without also disabling the
	  regular telemetry and RDF transmissions. As APRS takes a
	  full second to transmit a single position report, we
	  recommend sending packets no more than once every 5 seconds.
	</para>
      </section>
      <section>
	<title>Maximum Flight Log</title>
	<para>
	  Changing this value will set the maximum amount of flight
	  log storage that an individual flight will use. The
	  available storage is divided into as many flights of the
	  specified size as can fit in the available space. You can
	  download and erase individual flight logs. If you fill up
	  the available storage, future flights will not get logged
	  until you erase some of the stored ones.
	</para>
      </section>
      <section>
	<title>Logging Trigger Motion</title>
	<para>
	  If TeleGPS moves less than this distance over a long period
	  of time, it will not log that location, saving storage space.
	</para>
      </section>
      <section>
	<title>Position Reporting Interval</title>
	<para>
	  This sets how often TeleGPS reports position information via
	  telemetry and to the on-board log. Reducing this value will
	  save power and logging memory consumption.
	</para>
      </section>
    </section>
  </chapter>
  <chapter>
    <title>TeleGPS Application</title>
    <para>
      The TeleGPS application provides a graphical user interface for
      interacting with the Altus Metrum product family. TeleGPS can
      monitor telemetry data, configure devices and many other
      tasks. The primary interface window is for displaying data
      received over the telemetry link. There are additional
      tasks available from the main window menu bar. This chapter
      is split into sections, each of which documents one of the tasks
      provided from the top-level toolbar.
    </para>
    <section>
      <title>Telemetry Monitoring</title>
      <para>
	This is the window brought up when you start the
	application. If you have a TeleDongle device connected to the
	computer, it will automatically be selected for telemetry monitoring
      </para>
      <para>
        All telemetry data received are automatically recorded in
        suitable log files. The name of the files includes the current
        date and TeleGPS serial and flight numbers.
      </para>
      <para>
        The radio frequency being monitored by the TeleDongle device
        is displayed at the top of the window. You can configure the
        frequency by clicking on the frequency box and selecting the
        desired frequency. The TeleGPS application remembers the last
        frequency selected for each TeleDongle and selects that
        automatically the next time you use that device.
      </para>
      <para>
        Below the TeleDongle frequency selector, the window contains a few
        significant pieces of information about the altimeter providing
        the telemetry data stream:
      </para>
      <itemizedlist>
        <listitem>
          <para>The configured call-sign</para>
        </listitem>
        <listitem>
          <para>The device serial number</para>
        </listitem>
        <listitem>
          <para>The flight number. TeleGPS remembers how many
          times it has flown.
          </para>
        </listitem>
        <listitem>
          <para>
            The Received Signal Strength Indicator value. This lets
            you know how strong a signal TeleDongle is receiving. The
            radio inside TeleDongle operates down to about -100dBm;
            weaker signals may not be receivable. The packet link uses
            error detection and correction techniques which prevent
            incorrect data from being reported.
          </para>
        </listitem>
        <listitem>
          <para>
            The age of the displayed data, in seconds since the last 
	    successfully received telemetry packet.  In normal operation
	    this will stay in the low single digits.  If the number starts
	    counting up, then you are no longer receiving data over the radio
	    link from the flight computer.
          </para>
        </listitem>
      </itemizedlist>
      <para>
        Finally, the largest portion of the window contains a set of
        tabs, each of which contain some information about the TeleGPS
        board. The final 'table' tab displays many of the raw telemetry
        values in one place in a spreadsheet-like format.
      </para>
      <section>
        <title>Map</title>
        <para>
          The Map tab shows the TeleGPS track over time on top of map
	  data making it easy to locate the device.
        </para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="telegps-map.png" width="5.5in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
        <para>
          The map's default scale is approximately 3m (10ft) per pixel. The map
          can be dragged using the left mouse button. The map will attempt
          to keep the rocket roughly centered while data is being received.
        </para>
	<para>
	  You can adjust the style of map and the zoom level with
	  buttons on the right side of the map window. You can draw a
	  line on the map by moving the mouse over the map with a
	  button other than the left one pressed, or by pressing the
	  left button while also holding down the shift key. The
	  length of the line in real-world units will be shown at the
	  start of the line.
	</para>
        <para>
          Images are fetched automatically via the Google Maps Static API,
          and cached on disk for reuse. If map images cannot be downloaded,
          the rocket's path will be traced on a dark gray background
          instead.
        </para>
	<para>
	  You can pre-load images for your favorite launch sites
	  before you leave home; check out the 'Preload Maps' section below.
	</para>
      </section>
      <section>
	<title>Location</title>
	<para>
	  The Location tab shows the raw GPS data received from TeleGPS.
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="telegps-location.png" width="5.5in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </section>
      <section>
	<title>Status</title>
	<para>
	  The Status tab shows data relative to the location of
	  TeleGPS when the application first received telemetry from
	  it.
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="telegps-status.png" width="5.5in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </section>
      <section>
	<title>Table</title>
	<para>
	  The Table tab shows detailed information about the GPS
	  receiver
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="telegps-table.png" width="5.5in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </section>
    </section>
    <!--
	<variablelist>
	  <varlistentry>
	    <term></term>
	    <listitem>
	      <para>
	      </para>
	    </listitem>
	  </varlistentry>
	</variablelist>
    -->
    <section>
      <title>TeleGPS Menus</title>
      <para>
	TeleGPS has three or four menus at the top of the window:
	<variablelist>
	  <varlistentry>
	    <term>File</term>
	    <listitem>
	      <para>
		New Window, Graph Data, Export Data, Load Maps, Preferences, Close and Exit
	      </para>
	    </listitem>
	  </varlistentry>
	  <varlistentry>
	    <term>Monitor</term>
	    <listitem>
	      <para>
		Connect Device, Disconnect, Scan Channels and Replay Saved Data
	      </para>
	    </listitem>
	  </varlistentry>
	  <varlistentry>
	    <term>Device</term>
	    <listitem>
	      <para>
		Download Data, Configure Device and Flash Device
	      </para>
	    </listitem>
	  </varlistentry>
	  <varlistentry>
	    <term>Frequency</term>
	    <listitem>
	      <para>
		This shows the current monitoring frequency with a
		drop-down menu listing other configured
		frequencies. You can change the set of frequencies
		shown here from the Preferences dialog. This menu is
		only shown when the TeleGPS application is connected
		to a TeleDongle or TeleBT device.
	      </para>
	    </listitem>
	  </varlistentry>
	</variablelist>
      </para>
      <section>
	<title>New Window</title>
	<para>
	  This creates another telemetry monitoring window, in case
	  you have multiple TeleDongle devices connected to the
	  computer.
	</para>
      </section>
      <section>
	<title>Graph Data</title>
	<para>
	  This brings up a file dialog to load a saved log, either
	  a .telem file of recorded telemetry or .eeprom of saved
	  data from on-board memory. It looks a bit like the flight
	  monitoring window, using a selection of tabs to show
	  different views of the saved data.
	</para>
	<section>
	  <title>Graph</title>
	  <para>
	    The Graph tab shows a plot of the the GPS data
	    collected. The X axis is time in seconds; there are a
	    variety of Y axes available for different kinds of data.
	  </para>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="telegps-graph-graph.png" width="6in" scalefit="1"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	</section>
	<section>
	  <title>Configure Graph</title>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="telegps-graph-configure.png" width="6in" scalefit="1"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	  <para>
	    This selects which graph elements to show, and, at the
	    bottom, lets you switch between metric and imperial units
	  </para>
	</section>
	<section>
	  <title>Statistics</title>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="telegps-graph-stats.png" width="6in" scalefit="1"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	  <para>
	    Shows overall data computed from the flight.
	  </para>
	</section>
	<section>
	  <title>Map</title>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="telegps-graph-map.png" width="6in" scalefit="1"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	  <para>
	    Shows a map of the area overlaid with the GPS track. As with
	    the telemetry monitoring window, you can select the style
	    of map and zoom level using buttons along the side;
	    you can scroll the map by dragging within the map pressing
	    the left button and you can draw a line to measure
	    distances using either the left button with the shift key,
	    or any other button.
	  </para>
	</section>
      </section>
      <section>
	<title>Export Data</title>
	<para>
          This tool takes the raw data files and makes them available for
          external analysis. When you select this button, you are prompted to 
	  select a data file, which can be either a .eeprom or .telem.
	  The .eeprom files contain higher resolution and more continuous data, 
	  while .telem files contain receiver signal strength information.  
	  Next, a second dialog appears which is used to select
          where to write the resulting file. It has a selector to choose
          between CSV and KML file formats.
	</para>
	<section>
          <title>Comma Separated Value Format</title>
          <para>
            This is a text file containing the data in a form suitable for
            import into a spreadsheet or other external data analysis
            tool. The first few lines of the file contain the version and
            configuration information from TeleGPS, then
            there is a single header line which labels all of the
            fields. All of these lines start with a '#' character which
            many tools can be configured to skip over.
          </para>
          <para>
            The remaining lines of the file contain the data, with each
            field separated by a comma and at least one space. All of
            the sensor values are converted to standard units, with the
            barometric data reported in both pressure, altitude and
            height above pad units.
          </para>
	</section>
	<section>
          <title>Keyhole Markup Language (for Google Earth)</title>
          <para>
            This is the format used by Google Earth to provide an overlay 
	    within that application. With this, you can use Google Earth to 
	    see the whole flight path in 3D.
          </para>
	</section>
      </section>
      <section>
	<title>Load Maps</title>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="load-maps.png" width="5.2in" scalefit="1"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
	<para>
	  Before using TeleGPS, you can use Load Maps to load map data
	  in case you don't have access to the internet while
	  receiving telemetry.
	</para>
	<para>
	  There's a drop-down menu of rocket launch sites we know
	  about; if your favorites aren't there, please let us know
	  the lat/lon and name of the site. The contents of this list
	  are actually downloaded from our server at run-time, so as
	  new sites are sent in, they'll get automatically added to
	  this list.  If the launch site isn't in the list, you can
	  manually enter the lat/lon values
	</para>
	<para>
	  There are four different kinds of maps you can view; you can
	  select which to download by selecting as many as you like from
	  the available types:
	  <variablelist>
	    <varlistentry>
	      <term>Hybrid</term>
	      <listitem>
		<para>
		  A combination of satellite imagery and road data. This
		  is the default view.
		</para>
	      </listitem>
	    </varlistentry>
	    <varlistentry>
	      <term>Satellite</term>
	      <listitem>
		<para>
		  Just the satellite imagery without any annotation.
		</para>
	      </listitem>
	    </varlistentry>
	    <varlistentry>
	      <term>Roadmap</term>
	      <listitem>
		<para>
		  Roads, political boundaries and a few geographic features.
		</para>
	      </listitem>
	    </varlistentry>
	    <varlistentry>
	      <term>Terrain</term>
	      <listitem>
		<para>
		  Contour intervals and shading that show hills and
		  valleys.
		</para>
	      </listitem>
	    </varlistentry>
	  </variablelist>
	</para>
	<para>
	  You can specify the range of zoom levels to download; smaller
	  numbers show more area with less resolution. The default
	  level, 0, shows about 3m/pixel. One zoom level change
	  doubles or halves that number.
	</para>
	<para>
	  The Tile Radius value sets how large an area around the center
	  point to download. Each tile is 512x512 pixels, and the
	  'radius' value specifies how many tiles away from the center
	  will be downloaded. Specify a radius of 0 and you get only the
	  center tile. A radius of 1 loads a 3x3 grid, centered on the
	  specified location.
	</para>
	<para>
	  Clicking the 'Load Map' button will fetch images from Google
	  Maps; note that Google limits how many images you can fetch at
	  once, so if you load more than one launch site, you may get
	  some gray areas in the map which indicate that Google is tired
	  of sending data to you. Try again later.
	</para>
      </section>
      <section>
	<title>Preferences</title>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="telegps-preferences.png" width="2.4in" scalefit="1"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
	<section>
          <title>Voice Settings</title>
          <para>
            AltosUI provides voice announcements during flight so that you
            can keep your eyes on the sky and still get information about
            the current flight status. However, sometimes you don't want
            to hear them.
          </para>
          <variablelist>
	    <varlistentry>
	      <term>Enable</term>
	      <listitem>
		<para>Turns all voice announcements on and off</para>
	      </listitem>
	    </varlistentry>
	    <varlistentry>
	      <term>Test Voice</term>
	      <listitem>
		<para>
		  Plays a short message allowing you to verify
		  that the audio system is working and the volume settings
		  are reasonable
		</para>
	      </listitem>
	    </varlistentry>
          </variablelist>
	</section>
	<section>
          <title>Log Directory</title>
          <para>
            AltosUI logs all telemetry data and saves all TeleMetrum flash
            data to this directory. This directory is also used as the
            staring point when selecting data files for display or export.
          </para>
          <para>
            Click on the directory name to bring up a directory choosing
            dialog, select a new directory and click 'Select Directory' to
            change where AltosUI reads and writes data files.
          </para>
	</section>
	<section>
          <title>Callsign</title>
          <para>
            This value is transmitted in each command packet sent from 
	    TeleDongle and received from an altimeter.  It is not used in 
	    telemetry mode, as the callsign configured in the altimeter board
	    is included in all telemetry packets.  Configure this
            with the AltosUI operators call sign as needed to comply with
            your local radio regulations.
          </para>
          <para>
	    Note that to successfully command a flight computer over the radio
	    (to configure the altimeter, monitor idle, or fire pyro charges), 
	    the callsign configured here must exactly match the callsign
	    configured in the flight computer.  This matching is case 
	    sensitive.
          </para>
	</section>
	<section>
	  <title>Imperial Units</title>
	  <para>
	    This switches between metric units (meters) and imperial
	    units (feet and miles). This affects the display of values
	    use during flight monitoring, configuration, data graphing
	    and all of the voice announcements. It does not change the
	    units used when exporting to CSV files, those are always
	    produced in metric units.
	  </para>
	</section>
	<section>
          <title>Serial Debug</title>
          <para>
            This causes all communication with a connected device to be
            dumped to the console from which AltosUI was started. If
            you've started it from an icon or menu entry, the output
            will simply be discarded. This mode can be useful to debug
            various serial communication issues.
          </para>
	</section>
	<section>
	  <title>Font Size</title>
	  <para>
	    Selects the set of fonts used in the flight monitor
	    window. Choose between the small, medium and large sets.
	  </para>
	</section>
	<section>
	  <title>Look &amp; Feel</title>
	  <para>
	    Adjust the style of the windows. By default, the TeleGPS
	    application attempts to blend in with the native style.
	  </para>
	</section>
	<section>
	  <title>Manage Frequencies</title>
	  <para>
	    This brings up a dialog where you can configure the set of
	    frequencies shown in the various frequency menus. You can
	    add as many as you like, or even reconfigure the default
	    set. Changing this list does not affect the frequency
	    settings of any devices, it only changes the set of
	    frequencies shown in the menus.
	  </para>
	</section>
      </section>
      <section>
	<title>Close</title>
	<para>
	  This closes the current window, leaving any other windows
	  open and the application running.
	</para>
      </section>
      <section>
	<title>Exit</title>
	<para>
	  This closes all TeleGPS windows and terminates the application.
	</para>
      </section>
      <section>
	<title>Connect Device</title>
	<para>
          Selecting this item brings up a dialog box listing all of
          the connected TeleDongle devices. When you choose one of
          these, AltosUI will display telemetry data as received by
          the selected TeleDongle device.
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="device-selection.png" width="3.1in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </section>
      <section>
	<title>Disconnect</title>
	<para>
	  Disconnects the currently connected TeleDongle or TeleBT
	</para>
      </section>
      <section>
	<title>Scan Channels</title>
	<para>
	  Scans the configured set of frequencies looking for
	  telemetry signals. A list of all of the discovered signals
	  is show; selecting one of those and clicking on 'Monitor'
	  will select that frequency in the associated TeleGPS
	  application window.
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="telegps-scan.png" width="3.1in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </section>
      <section>
	<title>Replay Saved Data</title>
	<para>
          Replays saved data through the monitoring interface. While
          replaying, the frequency menu is replaced with a menu to allow
          you to set the replay speed.
	</para>
      </section>
      <section>
	<title>Download Data</title>
	<para>
          TeleGPS records data to its internal flash memory.
          On-board data is recorded at the same rate as telemetry
          but is not subject to radio drop-outs. As
          such, it generally provides a more complete and precise record.
          The 'Download Data' menu entry allows you to read the
          flash memory and write it to disk. 
	</para>
	<para>
          Select the 'Download Data' menu entry to bring up a list of
          connected TeleGPS devices. After the device has been
          selected, a dialog showing the data stored in the
          device will be shown allowing you to select which entries to
          download and which to delete. You must erase flights in order for the space they
          consume to be reused by another track. This prevents
          accidentally losing data if you neglect to download
          data before starting TeleGPS again. Note that if there is no more
          space available in the device, then no data will be recorded.
	</para>
	<para>
          The file name for each data log is computed automatically
          from the recorded date, altimeter serial number and flight
          number information.
	</para>
      </section>
      <section>
	<title>Configure Device</title>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="telegps-configure.png" width="3.6in" scalefit="1"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
	<para>
          Select this button and then select any connected TeleGPS
          device from the list provided.
	</para>
	<para>
          The first few lines of the dialog provide information about the
          connected device, including the product name,
          software version and hardware serial number. Below that are the
          individual configuration entries.
	</para>
	<para>
          At the bottom of the dialog, there are four buttons:
	</para>
	<variablelist>
	  <varlistentry>
	    <term>Save</term>
	    <listitem>
	      <para>
		This writes any changes to the
		configuration parameter block in flash memory. If you don't
		press this button, any changes you make will be lost.
	      </para>
	    </listitem>
	  </varlistentry>
	  <varlistentry>
	    <term>Reset</term>
	    <listitem>
	      <para>
		This resets the dialog to the most recently saved values,
		erasing any changes you have made.
	      </para>
	    </listitem>
	  </varlistentry>
	  <varlistentry>
	    <term>Reboot</term>
	    <listitem>
	      <para>
		This reboots the device. This will restart logging for
		a new flight number, if any log information has been
		saved for the current flight.
	      </para>
	    </listitem>
	  </varlistentry>
	  <varlistentry>
	    <term>Close</term>
	    <listitem>
	      <para>
		This closes the dialog. Any unsaved changes will be
		lost.
	      </para>
	    </listitem>
	  </varlistentry>
	</variablelist>
	<para>
          The rest of the dialog contains the parameters to be configured.
	</para>
	<section>
          <title>Frequency</title>
          <para>
            This configures which of the frequencies to use for both
            telemetry and packet command mode. Note that if you set this
            value via packet command mode, the TeleDongle frequency will
            also be automatically reconfigured to match so that
            communication will continue afterwards.
          </para>
	</section>
	<section>
          <title>RF Calibration</title>
          <para>
            The radios in every Altus Metrum device are calibrated at the
            factory to ensure that they transmit and receive on the
            specified frequency.  If you need to you can adjust the calibration 
	    by changing this value.  Do not do this without understanding what
	    the value means, read the appendix on calibration and/or the source
	    code for more information.  To change a TeleDongle's calibration, 
	    you must reprogram the unit completely.
          </para>
	</section>
	<section>
	  <title>Telemetry/RDF/APRS Enable</title>
	  <para>
	    Enables the radio for transmission during flight. When
	    disabled, the radio will not transmit anything during flight
	    at all.
	  </para>
	</section>
	<section>
	  <title>APRS Interval</title>
	  <para>
	    How often to transmit GPS information via APRS (in
	    seconds). When set to zero, APRS transmission is
	    disabled. This option is available on TeleMetrum v2 and
	    TeleMega boards. TeleMetrum v1 boards cannot transmit APRS
	    packets. Note that a single APRS packet takes nearly a full
	    second to transmit, so enabling this option will prevent
	    sending any other telemetry during that time.
	  </para>
	</section>
	<section>
          <title>Callsign</title>
          <para>
            This sets the call sign included in each telemetry packet. Set this
            as needed to conform to your local radio regulations.
          </para>
	</section>
	<section>
          <title>Maximum Log Size</title>
          <para>
            This sets the space (in kilobytes) allocated for each data
            log. The available space will be divided into chunks of this
            size. A smaller value will allow more logs to be stored,
            a larger value will record data for longer times.
	  </para>
	</section>
	<section>
	  <title>Logging Trigger Motion</title>
	  <para>
	    If TeleGPS moves less than this distance over a long period
	    of time, it will not log that location, saving storage space.
	  </para>
	</section>
	<section>
	  <title>Position Reporting Interval</title>
	  <para>
	    This sets how often TeleGPS reports position information via
	    telemetry and to the on-board log. Reducing this value will
	    save power and logging memory consumption.
	  </para>
	</section>
      </section>
      <section>
	<title>Flash Device</title>
	<para>
          This reprograms TeleGPS devices with new firmware. Please
          read the directions for flashing devices in the Updating
          Device Firmware chapter below.
	</para>
      </section>
    </section>
  </chapter>
  <chapter>
    <title>Updating Device Firmware</title>
    <para>
      TeleGPS is programmed directly over its USB connectors.
    </para>
    <para>
      You may wish to begin by ensuring you have current firmware images.
      These are distributed as part of the TeleGPS software bundle that
      also includes the TeleGPS ground station program.  Newer ground
      station versions typically work fine with older firmware versions,
      so you don't need to update your devices just to try out new
      software features.  You can always download the most recent
      version from <ulink url="http://www.altusmetrum.org/AltOS/"/>.
    </para>
    <section>
      <title>
	Updating TeleGPS Firmware
      </title>
      <orderedlist inheritnum='inherit' numeration='arabic'>
	<listitem>
	  <para>
	    Attach a battery and power switch to the target
	    device. Power up the device.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    Using a Micro USB cable, connect the target device to your
	    computer's USB socket.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    Run TeleGPS, and select 'Flash Device' from the Device menu.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    Select the target device in the Device Selection dialog.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    Select the image you want to flash to the device, which
	    should have a name in the form
	    &lt;product&gt;-v&lt;product-version&gt;-&lt;software-version&gt;.ihx, such
	    as TeleGPS-v1.0-1.4.0.ihx.
	  </para>
	</listitem>
        <listitem>
	  <para>
	    Make sure the configuration parameters are reasonable
	    looking. If the serial number and/or RF configuration
	    values aren't right, you'll need to change them.
	  </para>
        </listitem>
        <listitem>
	  <para>
	    Hit the 'OK' button and the software should proceed to flash
	    the device with new firmware, showing a progress bar.
	  </para>
        </listitem>
	<listitem>
	  <para>
	    Verify that the device is working by using the 'Configure
	    Altimeter' item to check over the configuration.
	  </para>
	</listitem>
      </orderedlist>
<!--
      <section>
	<title>Recovering From Self-Flashing Failure</title>
	<para>
	  If the firmware loading fails, it can leave the device
	  unable to boot. Not to worry, you can force the device to
	  start the boot loader instead, which will let you try to
	  flash the device again.
	</para>
	<para>
	  On each device, connecting two pins from one of the exposed
	  connectors will force the boot loader to start, even if the
	  regular operating system has been corrupted in some way.
	</para>
	<variablelist>
	  <varlistentry>
	    <term>TeleMega</term>
	    <listitem>
	      <para>
		Connect pin 6 and pin 1 of the companion connector. Pin 1
		can be identified by the square pad around it, and then
		the pins could sequentially across the board. Be very
		careful to <emphasis>not</emphasis> short pin 8 to
		anything as that is connected directly to the battery. Pin
		7 carries 3.3V and the board will crash if that is
		connected to pin 1, but shouldn't damage the board.
	      </para>
	    </listitem>
	  </varlistentry>
	  <varlistentry>
	    <term>TeleMetrum v2</term>
	    <listitem>
	      <para>
		Connect pin 6 and pin 1 of the companion connector. Pin 1
		can be identified by the square pad around it, and then
		the pins could sequentially across the board. Be very
		careful to <emphasis>not</emphasis> short pin 8 to
		anything as that is connected directly to the battery. Pin
		7 carries 3.3V and the board will crash if that is
		connected to pin 1, but shouldn't damage the board.
	      </para>
	    </listitem>
	  </varlistentry>
	  <varlistentry>
	    <term>EasyMini</term>
	    <listitem>
	      <para>
		Connect pin 6 and pin 1 of the debug connector, which is
		the six holes next to the beeper. Pin 1 can be identified
		by the square pad around it, and then the pins could
		sequentially across the board, making Pin 6 the one on the
		other end of the row.
	      </para>
	    </listitem>
	  </varlistentry>
	</variablelist>
      </section>
    -->
    </section>
  </chapter>
  <chapter>
    <title>Technical Information</title>
    <section>
      <title>GPS Receiver</title>
      <para>
	TeleGPS uses the u-Blox Max-7Q GPS receiver.
      </para>
    </section>
    <section>
      <title>Micro-controller</title>
      <para>
	TeleGPS uses an NXP LPC11U14 micro-controller. This tiny
	CPU contains 32kB of flash for the application and 4kB of RAM for
	temporary data storage.
      </para>
    </section>
    <section>
      <title>Lithium Polymer Battery</title>
      <para>
	Shipping restrictions may prevent us from including a battery
	battery with TeleGPS.
      </para>
    </section>
    <section>
      <title>Mechanical Considerations</title>
      <para>
	TeleGPS is designed to be rugged enough for typical rocketry
	applications. 
      </para>
    </section>
    <section>
      <title>On-board data storage</title>
      <para>
	TeleGPS has 2MB of non-volatile storage, separate from the
	code storage memory. The TeleGPS firmware uses this to log
	information during flight.
      </para>
    </section>
  </chapter>
</book>
<!--  LocalWords:  Altusmetrum TeleGPS
-->
