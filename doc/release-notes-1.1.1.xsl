<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
"/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">

<article>
  <para>
    Version 1.1.1 is a bug-fix release. It fixes a couple of bugs in
    AltosUI and one firmware bug that affects TeleMetrum version 1.0
    boards. Thanks to Bob Brown for help diagnosing the Google Earth
    file export issue, and for suggesting the addition of the Ground
    Distance value in the Descent tab.
  </para>
  <para>
    AltOS Firmware Changes
    <itemizedlist>
      <listitem>
<para>
	TeleMetrum v1.0 boards use the AT45DB081D flash memory part to
	store flight data, which is different from later TeleMetrum
	boards. The AltOS v1.1 driver for this chip couldn't erase
	memory, leaving it impossible to delete flight data or update
	configuration values. This bug doesn't affect newer TeleMetrum
	boards, and it doesn't affect the safety of rockets flying
	version 1.1 firmware.
      </para>
</listitem>
    </itemizedlist>
  </para>
  <para>
    AltosUI Changes
    <itemizedlist>
      <listitem>
<para>
	Creating a Google Earth file (KML) from on-board flight data
	(EEPROM) would generate an empty file. The code responsible
	for reading the EEPROM file wasn't ever setting the GPS valid
	bits, and so the KML export code thought there was no GPS data
	in the file.
      </para>
</listitem>
      <listitem>
<para>
	The “Landed” tab was displaying all values in metric units,
	even when AltosUI was configured to display imperial
	units. Somehow I just missed this tab when doing the units stuff.
      </para>
</listitem>
      <listitem>
<para>
	The “Descent” tab displays the range to the rocket, which is a
	combination of the over-the-ground distance to the rockets
	current latitude/longitude and the height of the rocket. As
	such, it's useful for knowing how far away the rocket is, but
	difficult to use when estimating where the rocket might
	eventually land. A new “Ground Distance” field has been added
	which displays the distance to a spot right underneath the
	rocket.
      </para>
</listitem>
      <listitem>
<para>
	Sensor data wasn't being displayed for TeleMini flight
	computers in Monitor Idle mode, including things like battery
	voltage. The code that picked which kinds of data to fetch
	from the flight computer was missing a check for TeleMini when
	deciding whether to fetch the analog sensor data.
      </para>
</listitem>
    </itemizedlist>
  </para>
</article>
