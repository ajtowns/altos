<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
"/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">

<article>
  <para>
    Version 1.0.1 is a major release, adding support for the TeleMini
    device and lots of new AltosUI features
  </para>
  <para>
    AltOS Firmware Changes
    <itemizedlist>
      <listitem>
<para>
	Add TeleMini v1.0 support. Firmware images for TeleMini are
	included in AltOS releases.
      </para>
</listitem>
      <listitem>
<para>
	Change telemetry to be encoded in multiple 32-byte packets. This
	enables support for TeleMini and other devices without requiring
	further updates to the TeleDongle firmware.
      </para>
</listitem>
      <listitem>
<para>
	Support operation of TeleMetrum with the antenna pointing
	aft. Previous firmware versions required the antenna to be
	pointing upwards, now there is a configuration option allowing
	the antenna to point aft, to aid installation in some airframes.
      </para>
</listitem>
      <listitem>
<para>
	Ability to disable telemetry. For airframes where an antenna
	just isn't possible, or where radio transmissions might cause
	trouble with other electronics, there's a configuration option
	to disable all telemetry. Note that the board will still
	enable the radio link in idle mode.
      </para>
</listitem>
      <listitem>
<para>
	Arbitrary frequency selection. The radios in Altus Metrum
	devices can be programmed to a wide range of frequencies, so
	instead of limiting devices to 10 pre-selected 'channels', the
	new firmware allows the user to choose any frequency in the
	70cm band. Note that the RF matching circuit on the boards is
	tuned for around 435MHz, so frequencies far from that may
	reduce the available range.
      </para>
</listitem>
      <listitem>
<para>
	Kalman-filter based flight-tracking. The model based sensor
	fusion approach of a Kalman filter means that AltOS now
	computes apogee much more accurately than before, generally
	within a fraction of a second. In addition, this approach
	allows the baro-only TeleMini device to correctly identify
	Mach transitions, avoiding the error-prone selection of a Mach
	delay.
      </para>
</listitem>
    </itemizedlist>
  </para>
  <para>
    AltosUI Changes
    <itemizedlist>
      <listitem>
<para>
	Wait for altimeter when using packet mode. Instead of quicly
	timing out when trying to initialize a packet mode
	configuration connection, AltosUI now waits indefinitely for
	the remote device to appear, providing a cancel button should
	the user get bored. This is necessary as the TeleMini can only
	be placed in "Idle" mode if AltosUI is polling it.
      </para>
</listitem>
      <listitem>
<para>
	Add main/apogee voltage graphs to the data plot. This provides
	a visual indication if the igniters fail before being fired.
      </para>
</listitem>
      <listitem>
<para>
	Scan for altimeter devices by watching the defined telemetry
	frequencies. This avoids the problem of remembering what
	frequency a device was configured to use, which is especially
	important with TeleMini which does not include a USB connection.
      </para>
</listitem>
      <listitem>
<para>
	Monitor altimeter state in "Idle" mode. This provides much of
	the information presented in the "Pad" dialog from the Monitor
	Flight command, monitoring the igniters, battery and GPS
	status withing requiring the flight computer to be armed and
	ready for flight.
      </para>
</listitem>
      <listitem>
<para>
	Pre-load map images from home. For those launch sites which
	don't provide free Wi-Fi, this allows you to download the
	necessary satellite images given the location of the launch
	site. A list of known launch sites is maintained at
	altusmetrum.org which AltosUI downloads to populate a menu; if
	you've got a launch site not on that list, please send the
	name of it, latitude and longitude along with a link to the
	web site of the controlling club to the altusmetrum mailing list.
      </para>
</listitem>
      <listitem>
<para>
	Flight statistics are now displayed in the Graph data
	window. These include max height/speed/accel, average descent
	rates and a few other bits of information. The Graph Data
	window can now be reached from the 'Landed' tab in the Monitor
	Flight window so you can immediately see the results of a
	flight.
      </para>
</listitem>
    </itemizedlist>
  </para>
</article>
