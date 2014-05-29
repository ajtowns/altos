<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
  "/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">
<book>
  <title>TeleGPS Owner's Manual</title>
  <subtitle>A recording GPS tracker</subtitle>
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
	<revnumber>1.0</revnumber>
	<date>28 May 2014</date>
	<revremark>
	  Initial release with preliminary hardware.
	</revremark>
      </revision>
    </revhistory>
  </bookinfo>
  <dedication>
    <title>Acknowledgements</title>
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
      TeleGPS is designed to be easy to use. Requiring no external
      components, flying takes just a few steps.
    </para>
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
      The Lithium polymer batteries have an
      extraordinary power density.  This is great because we can fly with
      much less battery mass... but if they are punctured
      or their contacts are allowed to short, they can and will release their
      energy very rapidly!
      Thus we recommend that you take some care when handling TeleGPS
      to keep conductive material from coming in contact with the exposed metal elements.
    </para>
    <para>
      As with all other rocketry electronics, Altus Metrum devices must 
      be protected from exposure to corrosive motor exhaust and ejection 
      charge gasses.
    </para>
  </chapter>
  <chapter>
    <title>Technical Information</title>
    <section>
      <title>GPS Receiver</title>
      <para>
	TeleGPS uses the u-Blox Max-7Q GPS receiver.
      </para>
    </section>
    <section>
      <title>Micro-controller</title>
      <para>
	TeleGPS uses an NXP LPC11U14 micro-controller. This tiny
	CPU contains 32kB of flash for the application and 4kB of RAM for
	temporary data storage.
      </para>
    </section>
    <section>
      <title>Lithium Polymer Battery</title>
      <para>
	Shipping restrictions may prevent us from including a battery
	battery with TeleGPS.
      </para>
    </section>
    <section>
      <title>Mechanical Considerations</title>
      <para>
	TeleGPS is designed to be rugged enough for typical rocketry
	applications. 
      </para>
    </section>
    <section>
      <title>On-board data storage</title>
      <para>
	TeleGPS has some bytes of non-volatile storage, separate
	from the code storage memory. The TeleGPS firmware uses this
	to store information about the last completed
	flight.
      </para>
    </section>
  </chapter>
</book>
<!--  LocalWords:  Altusmetrum TeleGPS
-->
