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
    </itemizedlist>
  </para>
</article>
