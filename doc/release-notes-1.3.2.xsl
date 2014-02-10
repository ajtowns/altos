<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
"/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">

<article>
  <para>
    Version 1.3.2 is a minor release. It includes small bug fixes for
    the TeleMega flight software and AltosUI ground station
  </para>
  <para>
    AltOS Firmware Changes
    <itemizedlist>
      <listitem>
	<para>
	  On TeleMega, limit number of logged GPS status information
	  to 12 satellites. That's all there is room for in the log
	  structure.
	</para>
      </listitem>
      <listitem>
	<para>
	  Improve APRS behavior. Remembers last known GPS position and
	  keeps sending that if we lose GPS lock. Marks
	  locked/unlocked by sending L/U in the APRS comment field
	  along with the number of sats in view and voltages.
	</para>
      </listitem>
    </itemizedlist>
  </para>
  <para>
    AltosUI changes
    <itemizedlist>
      <listitem>
	<para>
	  If the TeleMega flight firmware reports that it has logged
	  information about more than 12 satellites, don't believe it
	  as the log only holds 12 satellite records.
	</para>
      </listitem>
      <listitem>
	<para>
	  Track the maximum height as computed from GPS altitude
	  data and report that in the flight summary data.
	</para>
      </listitem>
      <listitem>
	<para>
	  Use letters (A, B, C, D) for alternate pyro channel names
	  instead of numbers (0, 1, 2, 3) in the Fire Igniter dialog.
	</para>
      </listitem>
    </itemizedlist>
  </para>
</article>
