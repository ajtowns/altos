<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
  "/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">
<book>
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
    <title>TeleMetrum</title>
    <subtitle>Owner's Manual for the TeleMetrum System</subtitle>
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
        <date>30 March 2010</date>
        <revremark>Initial content</revremark>
      </revision>
    </revhistory>
  </bookinfo>
  <chapter>
    <title>Introduction and Overview</title>
    <para>
      Placeholder.
    </para>
  </chapter>
  <chapter>
    <title>Specifications</title>
    <para>
      Placeholder.
    </para>
  </chapter>
  <chapter>
    <title>Handling Precautions</title>
    <para>
      Placeholder.
    </para>
  </chapter>
  <chapter>
    <title>Hardware Overview</title>
    <para>
      Placeholder.
    </para>
  </chapter>
  <chapter>
    <title>Operation</title>
    <para>
      Placeholder.
    </para>
  </chapter>
  <chapter>
    <title>Using Altus Metrum Products</title>
    <section>
      <title>Being Legal</title>
      <para>
        First off, in the US, you need an [amateur radio license](../Radio) or 
        other authorization to legally operate the radio transmitters that are part
        of our products.
      </para>
      <section>
        <title>In the Rocket</title>
        <para>
          In the rocket itself, you just need a [TeleMetrum](../TeleMetrum) board and 
          a LiPo rechargeable battery.  An 860mAh battery weighs less than a 9V 
          alkaline battery, and will run a [TeleMetrum](../TeleMetrum) for hours.
        </para>
        <para>
          By default, we ship TeleMetrum with a simple wire antenna.  If your 
          electronics bay or the airframe it resides within is made of carbon fiber, 
          which is opaque to RF signals, you may choose to have an SMA connector 
          installed so that you can run a coaxial cable to an antenna mounted 
          elsewhere in the rocket.
        </para>
      </section>
      <section>
        <title>On the Ground</title>
        <para>
          To receive the data stream from the rocket, you need an antenna and short 
          feedline connected to one of our [TeleDongle](../TeleDongle) units.  The
          TeleDongle in turn plugs directly into the USB port on a notebook 
          computer.  Because TeleDongle looks like a simple serial port, your computer
          does not require special device drivers... just plug it in.
        </para>
        <para>
          Right now, all of our application software is written for Linux.  However, 
          because we understand that many people run Windows or MacOS, we are working 
          on a new ground station program written in Java that should work on all
          operating systems.
        </para>
        <para>
          After the flight, you can use the RF link to extract the more detailed data 
          logged in the rocket, or you can use a mini USB cable to plug into the 
          TeleMetrum board directly.  Pulling out the data without having to open up
          the rocket is pretty cool!  A USB cable is also how you charge the LiPo 
          battery, so you'll want one of those anyway... the same cable used by lots 
          of digital cameras and other modern electronic stuff will work fine.
        </para>
        <para>
          If your rocket lands out of sight, you may enjoy having a hand-held GPS 
          receiver, so that you can put in a waypoint for the last reported rocket 
          position before touch-down.  This makes looking for your rocket a lot like 
          Geo-Cacheing... just go to the waypoint and look around starting from there.
        </para>
        <para>
          You may also enjoy having a ham radio "HT" that covers the 70cm band... you 
          can use that with your antenna to direction-find the rocket on the ground 
          the same way you can use a Walston or Beeline tracker.  This can be handy 
          if the rocket is hiding in sage brush or a tree, or if the last GPS position 
          doesn't get you close enough because the rocket dropped into a canyon, or 
          the wind is blowing it across a dry lake bed, or something like that...  Keith
          and Bdale both currently own and use the Yaesu VX-7R at launches.
        </para>
        <para>
          So, to recap, on the ground the hardware you'll need includes:
          <orderedlist inheritnum='inherit' numeration='arabic'>
            <listitem> 
              an antenna and feedline
            </listitem>
            <listitem> 
              a TeleDongle
            </listitem>
            <listitem> 
              a notebook computer
            </listitem>
            <listitem> 
              optionally, a handheld GPS receiver
            </listitem>
            <listitem> 
              optionally, an HT or receiver covering 435 Mhz
            </listitem>
          </orderedlist>
        </para>
        <para>
          The best hand-held commercial directional antennas we've found for radio 
          direction finding rockets are from 
	<ulink url="http://www.arrowantennas.com/" >
          Arrow Antennas.
	</ulink>
The 440-3 and 440-5 are both good choices for finding a 
TeleMetrum-equipped rocket when used with a suitable 70cm HT.  
        </para>
      </section>
      <section>
        <title>Data Analysis</title>
        <para>
          Our software makes it easy to log the data from each flight, both the 
          telemetry received over the RF link during the flight itself, and the more
          complete data log recorded in the DataFlash memory on the TeleMetrum 
          board.  Once this data is on your computer, our postflight tools make it
          easy to quickly get to the numbers everyone wants, like apogee altitude, 
          max acceleration, and max velocity.  You can also generate and view a 
          standard set of plots showing the altitude, acceleration, and
          velocity of the rocket during flight.  And you can even export a data file 
          useable with Google Maps and Google Earth for visualizing the flight path 
          in two or three dimensions!
        </para>
        <para>
          Our ultimate goal is to emit a set of files for each flight that can be
          published as a web page per flight, or just viewed on your local disk with 
          a web browser.
        </para>
      </section>
      <section>
        <title>Future Plans</title>
        <para>
          In the future, we intend to offer "companion boards" for the rocket that will
          plug in to TeleMetrum to collect additional data, provide more pyro channels,
          and so forth.  A reference design for a companion board will be documented
          soon, and will be compatible with open source Arduino programming tools.
        </para>
        <para>
          We are also working on the design of a hand-held ground terminal that will
          allow monitoring the rocket's status, collecting data during flight, and
          logging data after flight without the need for a notebook computer on the
          flight line.  Particularly since it is so difficult to read most notebook
          screens in direct sunlight, we think this will be a great thing to have.
        </para>
        <para>
          Because all of our work is open, both the hardware designs and the software,
          if you have some great idea for an addition to the current Altus Metrum family,
          feel free to dive in and help!  Or let us know what you'd like to see that 
          we aren't already working on, and maybe we'll get excited about it too... 
        </para>
      </section>
    </section>
  </chapter>
</book>

