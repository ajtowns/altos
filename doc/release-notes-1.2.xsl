<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
"/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">

<article>
  <para>
    Version 1.2 is a major release. It adds support for MicroPeak and
    the MicroPeak USB adapter.
  </para>
  <para>
    AltOS Firmware Changes
    <itemizedlist>
      <listitem>
	Add MicroPeak support. This includes support for the ATtiny85
	processor and adaptations to the core code to allow for
	devices too small to run the multi-tasking scheduler.
      </listitem>
    </itemizedlist>
  </para>
  <para>
    MicroPeak UI changes
    <itemizedlist>
      <listitem>
	Added this new application
      </listitem>
    </itemizedlist>
  </para>
  <para>
    Distribution Changes
    <itemizedlist>
      <listitem>
	Distribute Mac OS X packages in disk image ('.dmg') format to
	greatly simplify installation.
      </listitem>
      <listitem>
	Provide version numbers for the shared Java libraries to
	ensure that upgrades work properly, and to allow for multiple
	Altus Metrum software packages to be installed in the same
	directory at the same time.
      </listitem>
    </itemizedlist>
  </para>
</article>
