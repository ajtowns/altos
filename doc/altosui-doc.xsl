<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
  "/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">
<book>
  <title>AltosUI</title>
  <subtitle>Altos Metrum Graphical User Interface Manual</subtitle>
  <bookinfo>
    <author>
      <firstname>Bdale</firstname>
      <surname>Garbee</surname>
    </author>
    <author>
      <firstname>Keith</firstname>
      <surname>Packard</surname>
    </author>
    <copyright>
      <year>2010</year>
      <holder>Bdale Garbee and Keith Packard</holder>
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
        <date>19 November 2010</date>
        <revremark>Initial content</revremark>
      </revision>
    </revhistory>
  </bookinfo>
  <chapter>
    <title>Introduction</title>
    <para>
      The AltosUI program provides a graphical user interface for
      interacting with the Altus Metrum product family, including
      TeleMetrum and TeleDongle. AltosUI can monitor telemetry data,
      configure TeleMetrum and TeleDongle devices and many other
      tasks. The primary interface window provides a selection of
      buttons, one for each major activity in the system.  This manual
      is split into chapters, each of which documents one of the tasks
      provided from the top-level toolbar.
    </para>
  </chapter>
  <chapter>
    <title>Packet Command Mode</title>
    <subtitle>Controlling TeleMetrum Over The Radio Link</subtitle>
    <para>
      One of the unique features of the Altos Metrum environment is
      the ability to create a two way command link between TeleDongle
      and TeleMetrum using the digital radio transceivers built into
      each device. This allows you to interact with TeleMetrum from
      afar, as if it were directly connected to the computer.
    </para>
    <para>
      Any operation which can be performed with TeleMetrum
      can either be done with TeleMetrum directly connected to
      the computer via the USB cable, or through the packet
      link. Simply select the appropriate TeleDongle device when
      the list of devices is presented and AltosUI will use packet
      command mode.
    </para>
    <itemizedlist>
      <listitem>
	<para>
	  Save Flight Data—Recover flight data from the rocket without
	  opening it up.
	</para>
      </listitem>
      <listitem>
	<para>
	  Configure TeleMetrum—Reset apogee delays or main deploy
	  heights to respond to changing launch conditions. You can
	  also 'reboot' the TeleMetrum device. Use this to remotely
	  enable the flight computer by turning TeleMetrum on while
	  horizontal, then once the airframe is oriented for launch,
	  you can reboot TeleMetrum and have it restart in pad mode
	  without having to climb the scary ladder.
	</para>
      </listitem>
      <listitem>
	<para>
	  Fire Igniters—Test your deployment charges without snaking
	  wires out through holes in the airframe. Simply assembly the
	  rocket as if for flight with the apogee and main charges
	  loaded, then remotely command TeleMetrum to fire the
	  igniters.
	</para>
      </listitem>
    </itemizedlist>
    <para>
      Packet command mode uses the same RF channels as telemetry
      mode. Configure the desired TeleDongle channel using the
      flight monitor window channel selector and then close that
      window before performing the desired operation.
    </para>
    <para>
      TeleMetrum only enables packet command mode in 'idle' mode, so
      make sure you have TeleMetrum lying horizontally when you turn
      it on. Otherwise, TeleMetrum will start in 'pad' mode ready for
      flight and will not be listening for command packets from TeleDongle.
    </para>
    <para>
      When packet command mode is enabled, you can monitor the link
      by watching the lights on the TeleDongle and TeleMetrum
      devices. The red LED will flash each time TeleDongle or
      TeleMetrum transmit a packet while the green LED will light up
      on TeleDongle while it is waiting to receive a packet from
      TeleMetrum.
    </para>
  </chapter>
  <chapter>
    <title>Monitor Flight</title>
    <subtitle>Receive, Record and Display Telemetry Data</subtitle>
    <para>
      Selecting this item brings up a dialog box listing all of the
      connected TeleDongle devices. When you choose one of these,
      AltosUI will create a window to display telemetry data as
      received by the selected TeleDongle device.
    </para>
    <para>
      All telemetry data received are automatically recorded in
      suitable log files. The name of the files includes the current
      date and rocket serial and flight numbers.
    </para>
    <para>
      The radio channel being monitored by the TeleDongle device is
      displayed at the top of the window. You can configure the
      channel by clicking on the channel box and selecting the desired
      channel. AltosUI remembers the last channel selected for each
      TeleDongle and selects that automatically the next time you use
      that device.
    </para>
    <para>
      Below the TeleDongle channel selector, the window contains a few
      significant pieces of information about the TeleMetrum providing
      the telemetry data stream:
    </para>
    <itemizedlist>
      <listitem>
	<para>The TeleMetrum callsign</para>
      </listitem>
      <listitem>
	<para>The TeleMetrum serial number</para>
      </listitem>
      <listitem>
	<para>The flight number. Each TeleMetrum remembers how many
	times it has flown.</para>
      </listitem>
      <listitem>
	<para>
	  The rocket flight state. Each flight passes through several
	  states including Pad, Boost, Fast, Coast, Drogue, Main and
	  Landed.
	</para>
      </listitem>
      <listitem>
	<para>
	  The Received Signal Strength Indicator value. This lets
	  you know how strong a signal TeleDongle is receiving. The
	  radio inside TeleDongle operates down to about -99dBm;
	  weaker signals may not be receiveable. The packet link uses
	  error correction and detection techniques which prevent
	  incorrect data from being reported.
	</para>
      </listitem>
    </itemizedlist>
    <para>
      Finally, the largest portion of the window contains a set of
      tabs, each of which contain some information about the rocket.
      They're arranged in 'flight order' so that as the flight
      progresses, the selected tab automatically switches to display
      data relevant to the current state of the flight. You can select
      other tabs at any time. The final 'table' tab contains all of
      the telemetry data in one place.
    </para>
    <section>
      <title>Launch Pad</title>
      <para>
	The 'Launch Pad' tab shows information used to decide when the
	rocket is ready for flight. The first elements include red/green
	indicators, if any of these is red, you'll want to evaluate
	whether the rocket is ready to launch:
	<itemizedlist>
	  <listitem>
	    <para>
	      Battery Voltage. This indicates whether the LiPo battery
	      powering the TeleMetrum has sufficient charge to last for
	      the duration of the flight. A value of more than
	      3.7V is required for a 'GO' status.
	    </para>
	  </listitem>
	  <listitem>
	    <para>
	      Apogee Igniter Voltage. This indicates whether the apogee
	      igniter has continuity. If the igniter has a low
	      resistance, then the voltage measured here will be close
	      to the LiPo battery voltage. A value greater than 3.2V is
	      required for a 'GO' status.
	    </para>
	  </listitem>
	  <listitem>
	    <para>
	      Main Igniter Voltage. This indicates whether the main
	      igniter has continuity. If the igniter has a low
	      resistance, then the voltage measured here will be close
	      to the LiPo battery voltage. A value greater than 3.2V is
	      required for a 'GO' status.
	    </para>
	  </listitem>
	  <listitem>
	    <para>
	      GPS Locked. This indicates whether the GPS receiver is
	      currently able to compute position information. GPS requires
	      at least 4 satellites to compute an accurate position.
	    </para>
	  </listitem>
	  <listitem>
	    <para>
	      GPS Ready. This indicates whether GPS has reported at least
	      10 consecutive positions without losing lock. This ensures
	      that the GPS receiver has reliable reception from the
	      satellites.
	    </para>
	  </listitem>
	</itemizedlist>
	<para>
	  The LaunchPad tab also shows the computed launch pad position
	  and altitude, averaging many reported positions to improve the
	  accuracy of the fix.
	</para>
      </para>
    </section>
    <section>
      <title>Ascent</title>
      <para>
	This tab is shown during Boost, Fast and Coast
	phases. The information displayed here helps monitor the
	rocket as it heads towards apogee.
      </para>
      <para>
	The height, speed and acceleration are shown along with the
	maxium values for each of them. This allows you to quickly
	answer the most commonly asked questions you'll hear during
	flight.
      </para>
      <para>
	The current latitude and longitude reported by the GPS are
	also shown. Note that under high acceleration, these values
	may not get updated as the GPS receiver loses position
	fix. Once the rocket starts coasting, the receiver should
	start reporting position again.
      </para>
      <para>
	Finally, the current igniter voltages are reported as in the
	Launch Pad tab. This can help diagnose deployment failures
	caused by wiring which comes loose under high acceleration.
      </para>
    </section>
    <section>
      <title>Descent</title>
      <para>
	Once the rocket has reached apogee and (we hope) activated the
	apogee charge, attention switches to tracking the rocket on
	the way back to the ground, and for dual-deploy flights,
	waiting for the main charge to fire.
      </para>
      <para>
	To monitor whether the apogee charge operated correctly, the
	current descent rate is reported along with the current
	height. Good descent rates generally range from 15-30m/s.
      </para>
      <para>
	To help locate the rocket in the sky, use the elevation and
	bearing information to figure out where to look. Elevation is
	in degrees above the horizon. Bearing is reported in degrees
	relative to true north. Range can help figure out how big the
	rocket will appear. Note that all of these values are relative
	to the pad location. If the elevation is near 90°, the rocket
	is over the pad, not over you.
      </para>
      <para>
	Finally, the igniter voltages are reported in this tab as
	well, both to monitor the main charge as well as to see what
	the status of the apogee charge is.
      </para>
    </section>
    <section>
      <title>Landed</title>
      <para>
	Once the rocket is on the ground, attention switches to
	recovery. While the radio signal is generally lost once the
	rocket is on the ground, the last reported GPS position is
	generally within a short distance of the actual landing location.
      </para>
      <para>
	The last reported GPS position is reported both by
	latitude and longitude as well as a bearing and distance from
	the launch pad. The distance should give you a good idea of
	whether you'll want to walk or hitch a ride. Take the reported
	latitude and longitude and enter them into your handheld GPS
	unit and have that compute a track to the landing location.
      </para>
      <para>
	Finally, the maximum height, speed and acceleration reported
	during the flight are displayed for your admiring observers.
      </para>
    </section>
  </chapter>
  <chapter>
    <title>Save Flight Data</title>
    <para>
      TeleMetrum records flight data to its internal flash memory.
      This data is recorded at a much higher rate than the telemetry
      system can handle, and is not subject to radio drop-outs. As
      such, it provides a more complete and precise record of the
      flight. The 'Save Flight Data' button allows you to read the
      flash memory and write it to disk.
    </para>
    <para>
      Clicking on the 'Save Flight Data' button brings up a list of
      connected TeleMetrum and TeleDongle devices. If you select a
      TeleMetrum device, the flight data will be downloaded from that
      device directly. If you select a TeleDongle device, flight data
      will be downloaded from a TeleMetrum device connected via the
      packet command link to the specified TeleDongle. See the chapter
      on Packet Command Mode for more information about this.
    </para>
    <para>
      The filename for the data is computed automatically from the recorded
      flight date, TeleMetrum serial number and flight number
      information.
    </para>
  </chapter>
  <chapter>
    <title>Replay Flight</title>
    <para>
      Select this button and you are prompted to select a flight
      record file, either a .telem file recording telemetry data or a
      .eeprom file containing flight data saved from the TeleMetrum
      flash memory.
    </para>
    <para>
      Once a flight record is selected, the flight monitor interface
      is displayed and the flight is re-enacted in real time. Check
      the Monitor Flight chapter above to learn how this window operates.
    </para>
  </chapter>
  <chapter>
    <title>Graph Data</title>
    <para>
      This section should be written by AJ.
    </para>
  </chapter>
  <chapter>
    <title>Export Data</title>
    <para>
     This tool takes the raw data files and makes them available for
     external analysis. When you select this button, you are prompted to select a flight
      data file (either .eeprom or .telem will do, remember that
      .eeprom files contain higher resolution and more continuous
      data). Next, a second dialog appears which is used to select
      where to write the resulting file. It has a selector to choose
      between CSV and KML file formats.
    </para>
    <section>
      <title>Comma Separated Value Format</title>
      <para>
	This is a text file containing the data in a form suitable for
	import into a spreadsheet or other external data analysis
	tool. The first few lines of the file contain the version and
	configuration information from the TeleMetrum device, then
	there is a single header line which labels all of the
	fields. All of these lines start with a '#' character which
	most tools can be configured to skip over.
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
	This is the format used by
	Googleearth to provide an overlay within that
	application. With this, you can use Googleearth to see the
	whole flight path in 3D.
      </para>
    </section>
  </chapter>
  <chapter>
    <title>Configure TeleMetrum</title>
    <para>
      Select this button and then select either a TeleMetrum or
      TeleDongle Device from the list provided. Selecting a TeleDongle
      device will use Packet Comamnd Mode to configure remote
      TeleMetrum device. Learn how to use this in the Packet Command
      Mode chapter.
    </para>
    <para>
      The first few lines of the dialog provide information about the
      connected TeleMetrum device, including the product name,
      software version and hardware serial number. Below that are the
      individual configuration entries.
    </para>
    <para>
      At the bottom of the dialog, there are four buttons:
    </para>
    <itemizedlist>
      <listitem>
	<para>
	  Save. This writes any changes to the TeleMetrum
	  configuration parameter block in flash memory. If you don't
	  press this button, any changes you make will be lost.
	</para>
      </listitem>
      <listitem>
	<para>
	  Reset. This resets the dialog to the most recently saved values,
	  erasing any changes you have made.
	</para>
      </listitem>
      <listitem>
	<para>
	  Reboot. This reboots the TeleMetrum device. Use this to
	  switch from idle to pad mode by rebooting once the rocket is
	  oriented for flight.
	</para>
      </listitem>
      <listitem>
	<para>
	  Close. This closes the dialog. Any unsaved changes will be
	  lost.
	</para>
      </listitem>
    </itemizedlist>
    <para>
      The rest of the dialog contains the parameters to be configured.
    </para>
    <section>
      <title>Main Deploy Altitude</title>
      <para>
	This sets the altitude (above the recorded pad altitude) at
	which the 'main' igniter will fire. The drop-down menu shows
	some common values, but you can edit the text directly and
	choose whatever you like. If the apogee charge fires below
	this altitude, then the main charge will fire two seconds
	after the apogee charge fires.
      </para>
    </section>
    <section>
      <title>Apogee Delay</title>
      <para>
	When flying redundant electronics, it's often important to
	ensure that multiple apogee charges don't fire at precisely
	the same time as that can overpressurize the apogee deployment
	bay and cause a structural failure of the airframe. The Apogee
	Delay parameter tells the flight computer to fire the apogee
	charge a certain number of seconds after apogee has been
	detected.
      </para>
    </section>
    <section>
      <title>Radio Channel</title>
      <para>
	This configures which of the 10 radio channels to use for both
	telemetry and packet command mode. Note that if you set this
	value via packet command mode, you will have to reconfigure
	the TeleDongle channel before you will be able to use packet
	command mode again.
      </para>
    </section>
    <section>
      <title>Radio Calibration</title>
      <para>
	The radios in every Altus Metrum device are calibrated at the
	factory to ensure that they transmit and receive on the
	specified frequency for each channel. You can adjust that
	calibration by changing this value. To change the TeleDongle's
	calibration, you must reprogram the unit completely.
      </para>
    </section>
    <section>
      <title>Callsign</title>
      <para>
	This sets the callsign included in each telemetry packet. Set this
	as needed to conform to your local radio regulations.
      </para>
    </section>
  </chapter>
  <chapter>
    <title>Configure AltosUI</title>
    <para>
      This button presents a dialog so that you can configure the AltosUI global settings.
    </para>
    <section>
      <title>Voice Settings</title>
      <para>
	AltosUI provides voice annoucements during flight so that you
	can keep your eyes on the sky and still get information about
	the current flight status. However, sometimes you don't want
	to hear them.
      </para>
      <itemizedlist>
	<listitem>
	  <para>Enable—turns all voice announcements on and off</para>
	</listitem>
	<listitem>
	  <para>
	    Test Voice—Plays a short message allowing you to verify
	    that the audio systme is working and the volume settings
	    are reasonable
	  </para>
	</listitem>
      </itemizedlist>
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
	This value is used in command packet mode and is transmitted
	in each packet sent from TeleDongle and received from
	TeleMetrum. It is not used in telemetry mode as that transmits
	packets only from TeleMetrum to TeleDongle. Configure this
	with the AltosUI operators callsign as needed to comply with
	your local radio regulations.
      </para>
    </section>
  </chapter>
  <chapter>
    <title>Flash Image</title>
    <para>
    </para>
  </chapter>
  <chapter>
    <title>Fire Igniter</title>
    <para>
    </para>
  </chapter>
</book>