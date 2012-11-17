<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
  "/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">
<book>
  <title>MicroPeak Owner's Manual</title>
  <subtitle>A peak-recording altimeter for hobby rocketry</subtitle>
  <bookinfo>
    <author>
      <firstname>Keith</firstname>
      <surname>Packard</surname>
    </author>
    <copyright>
      <year>2012</year>
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
	<date>29 October 2012</date>
	<revremark>
	  Initial release with preliminary hardware.
	</revremark>
      </revision>
    </revhistory>
  </bookinfo>
  <acknowledgements>
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
  </acknowledgements>
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
      </listitem>
      <listitem>
	<para>
	  Install MicroPeak in your rocket. This can be as simple as
	  preparing a soft cushion of wadding inside a vented model payload
	  bay. Wherever you mount it, make sure you protect the
	  barometric sensor from corrosive ejection gasses as those
	  will damage the sensor.
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
	  30 seconds before starting to check for launch. This gives
	  you time to finish assembling the rocket. As those
	  activities might cause pressure changes inside the airframe,
	  MicroPeak might accidentally detect boost. If you need to do
	  anything to the airframe after the 30 second window passes,
	  make sure to be careful not to disturb the altimeter. The
	  LED will remain dark during the 30 second delay, but after
	  that, it will start blinking once every 3 seconds.
	</para>
      </listitem>
      <listitem>
	<para>
	  Fly the rocket. Once the rocket passes about 10m in height
	  (32 feet), the micro-controller will record the ground
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
	  Recover the data. Turn MicroPeak off for a couple of seconds
	  (to discharge the capacitors) and then back on. MicroPeak
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
      The barometric sensors used in MicroPeak is
      sensitive to sunlight. Please consider this when
      designing an installation, for example, in an air-frame with a
      see-through plastic payload bay. Many model rockets with payload bays
      use clear plastic for the payload bay. Replacing these with an opaque
      cardboard tube, painting them, or wrapping them with a layer of masking
      tape are all reasonable approaches to keep the sensor out of direct
      sunlight.
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
	taken while the altimeter is at rest. Flight pressure is
	computed from an exponential IIR filter designed to smooth out
	transients caused by mechanical stress on the barometer.
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
	The CR1025 battery used by MicroPeak holes 30mAh of power,
	which is sufficient to run for over 15 hours. Because
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
	Shipping restrictions prevent us from including a CR1025
	battery with MicroPeak. Many stores carry CR1025 batteries as
	they are commonly used in small electronic devices such as
	flash lights.
      </para>
    </section>
    <section>
      <title>Atmospheric Model</title>
      <para>
	MicroPeak contains a fixed atmospheric model which is used to
	convert barometric pressure into altitude. The model was
	converted into a 469-element piece wise linear approximation
	which is then used to compute the altitude of the ground and
	apogee. The difference between these represents the maximum
	height of the flight.
      </para>
      <para>
	The model assumes a particular set of atmospheric conditions,
	which while a reasonable average cannot represent the changing
	nature of the real atmosphere. Fortunately, for flights
	reasonably close to the ground, the effect of this global
	inaccuracy are largely canceled out when the computed ground
	altitude is subtracted from the computed apogee altitude, so
	the resulting height is more accurate than either the ground
	or apogee altitudes.
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
  </chapter>
</book>
<!--  LocalWords:  Altusmetrum MicroPeak
-->