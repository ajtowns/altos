<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
"/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">

<article>
  <para>
    Version 0.9 adds a few new firmware features and accompanying
    AltosUI changes, along with new hardware support.
  </para>
  <itemizedlist>
    <listitem>
      <para>
      Support for TeleMetrum v1.1 hardware. Sources for the flash
      memory part used in v1.0 dried up, so v1.1 uses a different part
      which required a new driver and support for explicit flight log
      erasing.
</para>
    </listitem>
    <listitem>
      <para>
      Multiple flight log support. This stores more than one flight
      log in the on-board flash memory. It also requires the user to
      explicitly erase flights so that you won't lose flight logs just
      because you fly the same board twice in one day.
</para>
    </listitem>
    <listitem>
      <para>
      Telemetry support for devices with serial number >=
      256. Previous versions used a telemetry packet format that
      provided only 8 bits for the device serial number. This change
      requires that both ends of the telemetry link be running the 0.9
      firmware or they will not communicate.
</para>
    </listitem>
  </itemizedlist>
</article>
