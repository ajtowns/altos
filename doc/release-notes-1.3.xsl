<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
"/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">

<article>
  <para>
    Version 1.3 is a major release. It adds support for TeleMega,
    TeleMetrum v2.0, TeleMini v2.0 and EasyMini.
  </para>
  <para>
    AltOS Firmware Changes
    <itemizedlist>
      <listitem>
	<para>
	  Add STM32L processor support. This includes enhancements to
	  the scheduler to support products with many threads.
	</para>
      </listitem>
      <listitem>
	<para>
	  Add NXP LPC11U14 processor support.
	</para>
      </listitem>
      <listitem>
	<para>
	  Support additional pyro channels. These are configurable
	  through the UI to handle air starts, staging, additional
	  recovery events and external devices such as cameras.
	</para>
      </listitem>
      <listitem>
	<para>
	  Add 3-axis gyro support for orientation tracking. This
	  integrates the gyros to compute the angle from vertical during
	  flight, allowing the additional pyro events to be controlled
	  by this value.
	</para>
      </listitem>
      <listitem>
	<para>
	  Many more device drivers, including u-Blox Max 7Q GPS,
	  Freescale MMA6555 digital single-axis accelerometer,
	  Invensense MPU6000 3-axis accelerometer + 3 axis gyro,
	  Honeywell HMC5883 3-axis magnetic sensor and the TI CC1120 and
	  CC115L digital FM transceivers
	</para>
      </listitem>
    </itemizedlist>
  </para>
  <para>
    AltosUI changes
    <itemizedlist>
      <listitem>
	<para>
	  Support TeleMega, TeleMetrum v2.0, TeleMini v2.0 and EasyMini telemetry and log formats.
	</para>
      </listitem>
      <listitem>
	<para>
	  Use preferred units for main deployment height configuration,
	  instead of always doing configuration in meters.
	</para>
      </listitem>
    </itemizedlist>
  </para>
  <para>
    MicroPeak UI changes
    <itemizedlist>
      <listitem>
	<para>
	  Add 'Download' button to menu bar.
	</para>
      </listitem>
      <listitem>
	<para>
	  Save the last log directory and offer that as the default for new downloads
	</para>
      </listitem>
    </itemizedlist>
  </para>
</article>
