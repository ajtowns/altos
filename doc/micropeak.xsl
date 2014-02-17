<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
  "/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">
<book>
  <title>MicroPeak Owner's Manual</title>
  <subtitle>A recording altimeter for hobby rocketry</subtitle>
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
	<imagedata fileref="micropeak-dime.jpg" width="6in"/>
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
	<revnumber>0.1</revnumber>
	<date>29 October 2012</date>
	<revremark>
	  Initial release with preliminary hardware.
	</revremark>
      </revision>
      <revision>
	<revnumber>1.0</revnumber>
	<date>18 November 2012</date>
	<revremark>
	  Updates for version 1.0 release.
	</revremark>
      </revision>
      <revision>
	<revnumber>1.1</revnumber>
	<date>12 December 2012</date>
	<revremark>
	  Add comments about EEPROM storage format and programming jig.
	</revremark>
      </revision>
      <revision>
	<revnumber>1.2</revnumber>
	<date>20 January 2013</date>
	<revremark>
	  Add documentation for the MicroPeak USB adapter board. Note
	  the switch to a Kalman filter for peak altitude
	  determination.
	</revremark>
      </revision>
      <revision>
	<revnumber>1.3.2</revnumber>
	<date>12 February 2014</date>
	<revremark>
	  Add a "Download" button to the main window, which makes it
	  quicker to access the download function. Update the data
	  download documentation to reflect the new MicroPeak USB
	  adapter design. Monitor data during download to let you see
	  if the USB connection is working at all by showing the
	  characters received from the MicroPeak USB adapter.
	</revremark>
      </revision>
    </revhistory>
  </bookinfo>
  <dedication>
    <title>Acknowledgements</title>
    <para>
      Thanks to John Lyngdal for suggesting that we build something like this.
    </para>
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
      MicroPeak is designed to be easy to use. Requiring no external
      components, flying takes just a few steps
    </para>
    <itemizedlist>
      <listitem>
	<para>
	  Install the battery. Fit a CR1025 battery into the plastic
	  carrier. The positive (+) terminal should be towards the more
	  open side of the carrier. Slip the carrier into the battery
	  holder with the positive (+) terminal facing away from the
	  circuit board.
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="micropeak-back.jpg" width="4.5in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </listitem>
      <listitem>
	<para>
	  Install MicroPeak in your rocket. This can be as simple as
	  preparing a soft cushion of wadding inside a vented model payload
	  bay. Wherever you mount it, make sure you protect the
	  barometric sensor from corrosive ejection gasses as those
	  will damage the sensor, and shield it from light as that can
	  cause incorrect sensor readings.
	</para>
      </listitem>
      <listitem>
	<para>
	  Turn MicroPeak on. Slide the switch so that the actuator
	  covers the '1' printed on the board. MicroPeak will report
	  the maximum height of the last flight in decimeters using a
	  sequence of flashes on the LED. A sequence of short flashes
	  indicates one digit. A single long flash indicates zero. The
	  height is reported in decimeters, so the last digit will be
	  tenths of a meter. For example, if MicroPeak reports 5 4 4
	  3, then the maximum height of the last flight was 544.3m, or
	  1786 feet.
	</para>
      </listitem>
      <listitem>
	<para>
	  Finish preparing the rocket for flight. After the
	  previous flight data have been reported, MicroPeak waits for
	  one minute before starting to check for launch. This gives
	  you time to finish assembling the rocket. As those
	  activities might cause pressure changes inside the airframe,
	  MicroPeak might accidentally detect boost. If you need to do
	  anything to the airframe after the one minute window passes,
	  make sure to be careful not to disturb the altimeter. The
	  LED will remain dark during the one minute delay, but after
	  that, it will start blinking once every 3 seconds.
	</para>
      </listitem>
      <listitem>
	<para>
	  Fly the rocket. Once the rocket passes about 30m in height
	  (100 feet), the micro-controller will record the ground
	  pressure and track the pressure seen during the flight. In
	  this mode, the LED flickers rapidly. When the rocket lands,
	  and the pressure stabilizes, the micro-controller will record
	  the minimum pressure pressure experienced during the flight,
	  compute the height represented by the difference in air
	  pressure and blink that value out on the LED. After that,
	  MicroPeak powers down to conserve battery power.
	</para>
      </listitem>
      <listitem>
	<para>
	  Recover the data. Turn MicroPeak off and then back on. MicroPeak
	  will blink out the maximum height for the last flight. Turn
	  MicroPeak back off to conserve battery power.
	</para>
      </listitem>
    </itemizedlist>
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
      The CR1025 Lithium batteries have an
      extraordinary power density.  This is great because we can fly with
      much less battery mass... but if they are punctured
      or their contacts are allowed to short, they can and will release their
      energy very rapidly!
      Thus we recommend that you take some care when handling MicroPeak
      to keep conductive material from coming in contact with the exposed metal elements.
    </para>
    <para>
      The barometric sensor used in MicroPeak is sensitive to
      sunlight. Please consider this when designing an
      installation. Many model rockets with payload bays use clear
      plastic for the payload bay. Replacing these with an opaque
      cardboard tube, painting them, or wrapping them with a layer of
      masking tape are all reasonable approaches to keep the sensor
      out of direct sunlight.
    </para>
    <para>
      The barometric sensor sampling ports must be able to "breathe",
      both by not being covered by foam or tape or other materials that might
      directly block the hole on the top of the sensor, and also by having a
      suitable static vent to outside air.
    </para>
    <para>
      As with all other rocketry electronics, Altus Metrum altimeters must 
      be protected from exposure to corrosive motor exhaust and ejection 
      charge gasses.
    </para>
  </chapter>
  <chapter>
    <title>The MicroPeak USB adapter</title>
    <informalfigure>
      <mediaobject>
	<imageobject>
	  <imagedata fileref="MicroPeakUSB-2.0.jpg" width="4.5in"/>
	</imageobject>
      </mediaobject>
    </informalfigure>
    <para>
      MicroPeak stores barometric pressure information for the first
      48 seconds of the flight in on-board non-volatile memory. The
      contents of this memory can be downloaded to a computer using
      the MicroPeak USB adapter.
    </para>
    <section>
      <title>Installing the MicroPeak software</title>
      <para>
	The MicroPeak application runs on Linux, Mac OS X and
	Windows. You can download the latest version from
	<ulink url="http://altusmetrum.org/AltOS"/>.
      </para>
      <para>
	On Mac OS X and Windows, the FTDI USB device driver needs to
	be installed. A compatible version of this driver is included
	with the MicroPeak application, but you may want to download a
	newer version from <ulink
	url="http://www.ftdichip.com/FTDrivers.htm"/>.
      </para>
    </section>
    <section>
      <title>Downloading Micro Peak data</title>
      <itemizedlist>
	<listitem>
	  <para>
	    Plug the MicroPeak USB adapter in to your computer.
	  </para>
	</listitem>
	<listitem>
	  <?dbfo keep-together="always"?>
	  <para>
	    Start the MicroPeak application.
	  </para>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="micropeak-nofont.svg" width="0.5in"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	</listitem>
	<listitem>
	  <?dbfo keep-together="always"?>
	  <para>
	     Click on the Download button at the top of the window.
	  </para>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="micropeak-app.png" width="4.5in"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	</listitem>
	<listitem>
	  <?dbfo keep-together="always"?>
	  <para>
	    Select from the listed devices. There will probably be
	    only one.
	  </para>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="micropeak-device-dialog.png" width="2.3in"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	</listitem>
	<listitem>
	  <para>
	    The application will now wait until it receives valid data
	    from the MicroPeak USB adapter.
	  </para>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="micropeak-download.png" width="2in"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	  <para>
	    The MicroPeak USB adapter has a small phototransistor
	    under the hole in the center of the box.
	    Locate this, turn on the MicroPeak and place the orange LED on the MicroPeak
	    directly inside the hole, resting the MicroPeak itself on
	    the box. You should see the blue LED on the MicroPeak USB
	    adapter blinking in time with the orange LED on the
	    MicroPeak board itself.
	  </para>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="MicroPeakUSB-2.0-inuse.jpg" width="4.5in"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	</listitem>
	<listitem>
	  <?dbfo keep-together="always"?>
	  <para>
	    After the maximum flight height is reported, MicroPeak will
	    pause for a few seconds, blink the LED four times rapidly
	    and then send the data in one long blur on the LED. The
	    MicroPeak application should receive the data. When it does,
	    it will present the data in a graph and offer to save the
	    data to a file. If not, you can power cycle the MicroPeak
	    board and try again.
	  </para>
	  <informalfigure>
	    <mediaobject>
	      <imageobject>
		<imagedata fileref="micropeak-save-dialog.png" width="2.3in"/>
	      </imageobject>
	    </mediaobject>
	  </informalfigure>
	</listitem>
	<listitem>
	  <?dbfo keep-together="always"?>
	  <para>
	    Once the data are saved, a graph will be displayed with
	    height, speed and acceleration values computed from the
	    recorded barometric pressure data. See the next section
	    for more details on that.
	  </para>
	</listitem>
      </itemizedlist>
    </section>
    <section>
      <title>Analyzing MicroPeak Data</title>
      <para>
	The MicroPeak application can present flight data in the form
	of a graph, a collection of computed statistics or in tabular
	form.
      </para>
      <para>
	MicroPeak collects raw barometric pressure data which is
	then used to compute the remaining data. Altitude is computed
	through a standard atmospheric model. Absolute error in this
	data will be affected by local atmospheric
	conditions. Fortunately, these errors tend to mostly cancel
	out, so the error in the height computation is much smaller
	than the error in altitude would be.
      </para>
      <para>
	Speed and acceleration are computed by first smoothing the
	height data with a Gaussian window averaging filter. For speed
	data, this average uses seven samples. For acceleration data,
	eleven samples are used. These were chosen to provide
	reasonably smooth speed and acceleration data, which would
	otherwise be swamped with noise.
      </para>
      <para>
	The File menu has operations to open existing flight logs,
	Download new data from MicroPeak, Save a copy of the flight
	log to a new file, Export the tabular data (as seen in the Raw
	Data tab) to a file, change the application Preferences, Close
	the current window or close all windows and Exit the
	application.
      </para>
      <section>
	<title>MicroPeak Graphs</title>
	<para>
	  Under the Graph tab, the height, speed and acceleration values
	  are displayed together. You can zoom in on the graph by
	  clicking and dragging to sweep out an area of
	  interest. Right-click on the plot to bring up a menu that will
	  let you save, copy or print the graph.
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="micropeak-graph.png" width="4.5in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </section>
      <section>
	<title>MicroPeak Flight Statistics</title>
	<para>
	  The Statistics tab presents overall data from the flight. Note
	  that the Maximum height value is taken from the minumum
	  pressure captured in flight, and may be different from the
	  apparant apogee value as the on-board data are sampled twice
	  as fast as the recorded values, or because the true apogee
	  occurred after the on-board memory was full. Each value is
	  presented in several units as appropriate.
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="micropeak-statistics.png" width="4.5in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </section>
      <section>
	<title>Raw Data</title>
	<para>
	  A table consisting of the both the raw barometric pressure
	  data and values computed from that for each recorded time.
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="micropeak-raw-data.png" width="4.5in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </section>
      <section>
	<title>Configuring the Graph</title>
	<para>
	  This selects which graph elements to show, and lets you
	  switch between metric and imperial units
	</para>
	<informalfigure>
	  <mediaobject>
	    <imageobject>
	      <imagedata fileref="micropeak-graph-configure.png" width="4.5in"/>
	    </imageobject>
	  </mediaobject>
	</informalfigure>
      </section>
    </section>
    <section>
      <title>Setting MicroPeak Preferences</title>
      <informalfigure>
	<mediaobject>
	  <imageobject>
	    <imagedata fileref="micropeak-preferences.png" width="1.8in"/>
	  </imageobject>
	</mediaobject>
      </informalfigure>
      <para>
	The MicroPeak application has a few user settings which are
	configured through the Preferences dialog, which can be
	accessed from the File menu.
      <itemizedlist>
	<listitem>
	  <para>
	    The Log Directory is where flight data will be saved to
	    and loaded from by default. Of course, you can always
	    navigate to other directories in the file chooser windows,
	    this setting is just the starting point.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    If you prefer to see your graph data in feet and
	    miles per hour instead of meters and meters per second,
	    you can select Imperial Units.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    To see what data is actually arriving over the serial
	    port, start the MicroPeak application from a command
	    prompt and select the Serial Debug option. This can be
	    useful in debugging serial communication problems, but
	    most people need never choose this.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    You can adjust the size of the text in the Statistics tab
	    by changing the Font size preference. There are three
	    settings, with luck one will both fit on your screen and
	    provide readable values.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    The Look &amp; feel menu shows a list of available
	    application appearance choices. By default, the MicroPeak
	    application tries to blend in with other applications, but
	    you may choose some other appearance if you like.
	  </para>
	</listitem>
      </itemizedlist>
      </para>
      <para>
	Note that MicroPeak shares a subset of the AltosUI
	preferences, so if you use both of these applications, change
	in one application will affect the other.
      </para>
    </section>
  </chapter>
  <chapter>
    <title>Technical Information</title>
    <section>
      <title>Barometric Sensor</title>
      <para>
	MicroPeak uses the Measurement Specialties MS5607 sensor. This
	has a range of 120kPa to 1kPa with an absolute accuracy of
	150Pa and a resolution of 2.4Pa.
      </para>
      <para>
	The pressure range corresponds roughly to an altitude range of
	-1500m (-4900 feet) to 31000m (102000 feet), while the
	resolution is approximately 20cm (8 inches) near sea level and
	60cm (24in) at 10000m (33000 feet).
      </para>
      <para>
	Ground pressure is computed from an average of 16 samples,
	taken while the altimeter is at rest. The flight pressure used to
	report maximum height is computed from a Kalman filter
	designed to smooth out any minor noise in the sensor
	values. The flight pressure recorded to non-volatile storage
	is unfiltered, coming directly from the pressure sensor.
      </para>
    </section>
    <section>
      <title>Micro-controller</title>
      <para>
	MicroPeak uses an Atmel ATtiny85 micro-controller. This tiny
	CPU contains 8kB of flash for the application, 512B of RAM for
	temporary data storage and 512B of EEPROM for non-volatile
	storage of previous flight data.
      </para>
      <para>
	The ATtiny85 has a low-power mode which turns off all of the
	clocks and powers down most of the internal components. In
	this mode, the chip consumes only .1μA of power. MicroPeak
	uses this mode once the flight has ended to preserve battery
	power.
      </para>
    </section>
    <section>
      <title>Lithium Battery</title>
      <para>
	The CR1025 battery used by MicroPeak holds 30mAh of power,
	which is sufficient to run for over 40 hours. Because
	MicroPeak powers down on landing, run time includes only time
	sitting on the launch pad or during flight.
      </para>
      <para>
	The large positive terminal (+) is usually marked, while the
	smaller negative terminal is not. Make sure you install the
	battery with the positive terminal facing away from the
	circuit board where it will be in contact with the metal
	battery holder. A small pad on the circuit board makes contact
	with the negative battery terminal.
      </para>
      <para>
	Shipping restrictions may prevent us from including a CR1025
	battery with MicroPeak. If so, many stores carry CR1025
	batteries as they are commonly used in small electronic
	devices such as flash lights.
      </para>
    </section>
    <section>
      <title>Atmospheric Model</title>
      <para>
	MicroPeak contains a fixed atmospheric model which is used to
	convert barometric pressure into altitude. The model was
	converted into a 469-element piece-wise linear approximation
	which is then used to compute the altitude of the ground and
	apogee. The difference between these represents the maximum
	height of the flight.
      </para>
      <para>
	The model assumes a particular set of atmospheric conditions,
	which, while a reasonable average, cannot represent the changing
	nature of the real atmosphere. Fortunately, for flights
	reasonably close to the ground, the effect of this global
	inaccuracy are largely canceled out when the computed ground
	altitude is subtracted from the computed apogee altitude, so
	the resulting height is more accurate than either the ground
	or apogee altitudes.
      </para>
      <para>
	Because the raw pressure data is recorded to non-volatile
	storage, you can use that, along with a more sophisticated
	atmospheric model, to compute your own altitude values.
      </para>
    </section>
    <section>
      <title>Mechanical Considerations</title>
      <para>
	MicroPeak is designed to be rugged enough for typical rocketry
	applications. It contains two moving parts, the battery holder
	and the power switch, which were selected for their
	ruggedness.
      </para>
      <para>
	The MicroPeak battery holder is designed to withstand impact
	up to 150g without breaking contact (or, worse yet, causing
	the battery to fall out). That means it should stand up to
	almost any launch you care to try, and should withstand fairly
	rough landings.
      </para>
      <para>
	The power switch is designed to withstand up to 50g forces in
	any direction. Because it is a sliding switch, orienting the
	switch perpendicular to the direction of rocket travel will
	serve to further protect the switch from launch forces.
      </para>
    </section>
    <section>
      <title>On-board data storage</title>
      <para>
	The ATtiny85 has 512 bytes of non-volatile storage, separate
	from the code storage memory. The MicroPeak firmware uses this
	to store information about the last completed
	flight. Barometric measurements from the ground before launch
	and at apogee are stored, and used at power-on to compute the
	height of the last flight.
      </para>
      <para>
	In addition to the data used to present the height of the last
	flight, MicroPeak also stores barometric information sampled
	at regular intervals during the flight. This is the
	information captured with the MicroPeak USB adapter. It can
	also be read from MicroPeak through any AVR programming
	tool.
      </para>
      <table frame='all'>
	<title>MicroPeak EEPROM Data Storage</title>
	<tgroup cols='3' align='center' colsep='1' rowsep='1'>
	  <colspec align='center' colwidth='2*' colname='Address'/>
	  <colspec align='center' colwidth='*' colname='Size (bytes)'/>
	  <colspec align='left' colwidth='7*' colname='Description'/>
	  <thead>
	    <row>
	      <entry align='center'>Address</entry>
	      <entry align='center'>Size (bytes)</entry>
	      <entry align='center'>Description</entry>
	    </row>
	  </thead>
	  <tbody>
	    <row>
	      <entry>0x000</entry>
	      <entry>4</entry>
	      <entry>Average ground pressure (Pa)</entry>
	    </row>
	    <row>
	      <entry>0x004</entry>
	      <entry>4</entry>
	      <entry>Minimum flight pressure (Pa)</entry>
	    </row>
	    <row>
	      <entry>0x008</entry>
	      <entry>2</entry>
	      <entry>Number of in-flight samples</entry>
	    </row>
	    <row>
	      <entry>0x00a … 0x1fe</entry>
	      <entry>2</entry>
	      <entry>Instantaneous flight pressure (Pa) low 16 bits</entry>
	    </row>
	  </tbody>
	</tgroup>
      </table>
      <para>
	All EEPROM data are stored least-significant byte first. The
	instantaneous flight pressure data are stored without the
	upper 16 bits of data. The upper bits can be reconstructed
	from the previous sample, assuming that pressure doesn't
	change by more more than 32kPa in a single sample
	interval. Note that this pressure data is <emphasis>not</emphasis>
	filtered in any way, while both the recorded ground and apogee
	pressure values are, so you shouldn't expect the minimum
	instantaneous pressure value to match the recorded minimum
	pressure value exactly.
      </para>
      <para>
	MicroPeak samples pressure every 96ms, but stores only every
	other sample in the EEPROM. This provides for 251 pressure
	samples at 192ms intervals, or 48.192s of storage. The clock
	used for these samples is a factory calibrated RC circuit
	built into the ATtiny85 and is accurate only to within ±10% at
	25°C. So, you can count on the pressure data being accurate,
	but speed or acceleration data computed from this will be
	limited by the accuracy of this clock.
      </para>
    </section>
    <section>
      <title>MicroPeak Programming Interface</title>
      <para>
	MicroPeak exposes a standard 6-pin AVR programming interface,
	but not using the usual 2x3 array of pins on 0.1"
	centers. Instead, there is a single row of tiny 0.60mm ×
	0.85mm pads on 1.20mm centers exposed near the edge of the
	circuit board. We couldn't find any connector that was
	small enough to include on the circuit board.
      </para>
      <para>
	In lieu of an actual connector, the easiest way to connect to
	the bare pads is through a set of Pogo pins. These
	spring-loaded contacts are designed to connect in precisely
	this way. We've designed a programming jig, the MicroPeak
	Pogo Pin board which provides a standard AVR interface on one
	end and a recessed slot for MicroPeak to align the board with
	the Pogo Pins.
      </para>
      <para>
	The MicroPeak Pogo Pin board is not a complete AVR programmer,
	it is an interface board that provides a 3.3V regulated power
	supply to run the MicroPeak via USB and a standard 6-pin AVR
	programming interface with the usual 2x3 grid of pins on 0.1"
	centers. This can be connected to any AVR programming
	dongle.
      </para>
      <para>
	The AVR programming interface cannot run faster than ¼ of the
	AVR CPU clock frequency. Because MicroPeak runs at 250kHz to
	save power, you must configure your AVR programming system to
	clock the AVR programming interface at no faster than
	62.5kHz, or a clock period of 32µS.
      </para>
    </section>
  </chapter>
</book>
<!--  LocalWords:  Altusmetrum MicroPeak
-->
