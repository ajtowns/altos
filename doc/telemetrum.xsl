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
      Welcome to the Altus Metrum community!  Our circuits and software reflect
      our passion for both hobby rocketry and Free Software.  We hope their
      capabilities and performance will delight you in every way, but by
      releasing all of our hardware and software designs under open licenses,
      we also hope to empower you to take as active a role in our collective
      future as you wish!
    </para>
    <para>
      The focal point of our community is TeleMetrum, a dual deploy altimeter 
      with fully integrated GPS and radio telemetry as standard features, and
      a "companion interface" that will support optional capabilities in the 
      future.
    </para>
    <para>    
      Complementing TeleMetrum is TeleDongle, a USB to RF interface for 
      communicating with TeleMetrum.  Combined with your choice of antenna and 
      notebook computer, TeleDongle and our associated user interface software
      form a complete ground station capable of logging and displaying in-flight
      telemetry, aiding rocket recovery, then processing and archiving flight
      data for analysis and review.
    </para>
  </chapter>
  <chapter>
    <title>Specifications</title>
    <itemizedlist>
      <listitem>
        <para>
          Recording altimeter for model rocketry.
        </para>
      </listitem>
      <listitem>
        <para>
          Supports dual deployment (can fire 2 ejection charges).
        </para>
      </listitem>
      <listitem>
        <para>
          70cm ham-band transceiver for telemetry downlink.
        </para>
      </listitem>
      <listitem>
        <para>
          Barometric pressure sensor good to 45k feet MSL.
        </para>
      </listitem>
      <listitem>
        <para>
          1-axis high-g accelerometer for motor characterization, capable of 
          +/- 50g using default part.
        </para>
      </listitem>
      <listitem>
        <para>
          On-board, integrated GPS receiver with 5hz update rate capability.
        </para>
      </listitem>
      <listitem>
        <para>
          On-board 1 megabyte non-volatile memory for flight data storage.
        </para>
      </listitem>
      <listitem>
        <para>
          USB interface for battery charging, configuration, and data recovery.
        </para>
      </listitem>
      <listitem>
        <para>
          Fully integrated support for LiPo rechargeable batteries.
        </para>
      </listitem>
      <listitem>
        <para>
          Uses LiPo to fire e-matches, support for optional separate pyro 
          battery if needed.
        </para>
      </listitem>
      <listitem>
        <para>
          2.75 x 1 inch board designed to fit inside 29mm airframe coupler tube.
        </para>
      </listitem>
    </itemizedlist>
  </chapter>
  <chapter>
    <title>Handling Precautions</title>
    <para>
      TeleMetrum is a sophisticated electronic device.  When handled gently and
      properly installed in an airframe, it will deliver impressive results.
      However, like all electronic devices, there are some precautions you
      must take.
    </para>
    <para>
      The Lithium Polymer rechargeable batteries used with TeleMetrum have an 
      extraordinary power density.  This is great because we can fly with
      much less battery mass than if we used alkaline batteries or previous
      generation rechargeable batteries... but if they are punctured 
      or their leads are allowed to short, they can and will release their 
      energy very rapidly!
      Thus we recommend that you take some care when handling our batteries 
      and consider giving them some extra protection in your airframe.  We 
      often wrap them in suitable scraps of closed-cell packing foam before 
      strapping them down, for example.
    </para>
    <para>
      The TeleMetrum barometric sensor is sensitive to sunlight.  In normal 
      mounting situations, it and all of the other surface mount components 
      are "down" towards whatever the underlying mounting surface is, so
      this is not normally a problem.  Please consider this, though, when
      designing an installation, for example, in a 29mm airframe's see-through
      plastic payload bay.
    </para>
    <para>
      The TeleMetrum barometric sensor sampling port must be able to "breathe",
      both by not being covered by foam or tape or other materials that might
      directly block the hole on the top of the sensor, but also by having a
      suitable static vent to outside air.  
    </para>
    <para>
      As with all other rocketry electronics, TeleMetrum must be protected 
      from exposure to corrosive motor exhaust and ejection charge gasses.
    </para>
  </chapter>
  <chapter>
    <title>Hardware Overview</title>
    <para>
      TeleMetrum is a 1 inch by 2.75 inch circuit board.  It was designed to
      fit inside coupler for 29mm airframe tubing, but using it in a tube that
      small in diameter may require some creativity in mounting and wiring 
      to succeed!  The default 1/4
      wave UHF wire antenna attached to the center of the nose-cone end of
      the board is about 7 inches long, and wiring for a power switch and
      the e-matches for apogee and main ejection charges depart from the 
      fin can end of the board.  Given all this, an ideal "simple" avionics 
      bay for TeleMetrum should have at least 10 inches of interior length.
    </para>
    <para>
      A typical TeleMetrum installation using the on-board GPS antenna and
      default wire UHF antenna involves attaching only a suitable
      Lithium Polymer battery, a single pole switch for power on/off, and 
      two pairs of wires connecting e-matches for the apogee and main ejection
      charges.  
    </para>
    <para>
      By default, we use the unregulated output of the LiPo battery directly
      to fire ejection charges.  This works marvelously with standard 
      low-current e-matches like the J-Tek from MJG Technologies, and with 
      Quest Q2G2 igniters.  However, if you
      want or need to use a separate pyro battery, you can do so by adding
      a second 2mm connector to position B2 on the board and cutting the
      thick pcb trace connecting the LiPo battery to the pyro circuit between
      the two silk screen marks on the surface mount side of the board shown
      here [insert photo]
    </para>
    <para>
      We offer two choices of pyro and power switch connector, or you can 
      choose neither and solder wires directly to the board.  All three choices
      are reasonable depending on the constraints of your airframe.  Our
      favorite option when there is sufficient room above the board is to use
      the Tyco pin header with polarization and locking.  If you choose this
      option, you crimp individual wires for the power switch and e-matches
      into a mating connector, and installing and removing the TeleMetrum
      board from an airframe is as easy as plugging or unplugging two 
      connectors.  If the airframe will not support this much height or if
      you want to be able to directly attach e-match leads to the board, we
      offer a screw terminal block.  This is very similar to what most other
      altimeter vendors provide and so may be the most familiar
      option.  You'll need a very small straight blade screwdriver to connect
      and disconnect the board in this case, such as you might find in a
      jeweler's screwdriver set.  Finally, you can forego both options and
      solder wires directly to the board, which may be the best choice for
      minimum diameter and/or minimum mass designs. 
    </para>
    <para>
      For most airframes, the integrated GPS antenna and wire UHF antenna are
      a great combination.  However, if you are installing in a carbon-fiber
      electronics bay which is opaque to RF signals, you may need to use 
      off-board external antennas instead.  In this case, you can order
      TeleMetrum with an SMA connector for the UHF antenna connection, and
      you can unplug the integrated GPS antenna and select an appropriate 
      off-board GPS antenna with cable terminating in a U.FL connector.
    </para>
  </chapter>
  <chapter>
    <title>Operation</title>
    <section>
      <title>Firmware Modes </title>
<para>
	The AltOS firmware build for TeleMetrum has two fundamental modes,
	"idle" and "flight".  Which of these modes the firmware operates in
	is determined by the orientation of the rocket (well, actually the
	board, of course...) at the time power is switched on.  If the rocket
	is "nose up", then TeleMetrum assumes it's on a rail or rod being
	prepared for launch, so the firmware chooses flight mode.  However,
	if the rocket is more or less horizontal, the firmware instead enters
	idle mode.
</para>
<para>
	At power on, you will hear three beeps ("S" in Morse code for startup)
        and then a pause while 
	TeleMetrum completes initialization and self tests, and decides which
	mode to enter next.
</para>
<para>
	In flight mode, TeleMetrum turns on the GPS system, engages the flight
	state machine, goes into transmit-only mode on the RF link sending 
	telemetry, and waits for launch to be detected.  Flight mode is
	indicated by an audible "di-dah-dah-dit" ("P" for pad) on the 
        beeper, followed by
	beeps indicating the state of the pyrotechnic igniter continuity.
	One beep indicates apogee continuity, two beeps indicate
	main continuity, three beeps indicate both apogee and main continuity,
	and one longer "brap" sound indicates no continuity.  For a dual
	deploy flight, make sure you're getting three beeps before launching!
	For apogee-only or motor eject flights, do what makes sense.
</para>
<para>
	In idle mode, you will hear an audible "di-dit" ("I" for idle), and
        the normal flight state machine is disengaged, thus
	no ejection charges will fire.  TeleMetrum also listens on the RF
	link when in idle mode for packet mode requests sent from TeleDongle.
	Commands can be issued to a TeleMetrum in idle mode over either
	USB or the RF link equivalently.
	Idle mode is useful for configuring TeleMetrum, for extracting data 
	from the on-board storage chip after flight, and for ground testing
	pyro charges.
</para>
<para>
	One "neat trick" of particular value when TeleMetrum is used with very
	large airframes, is that you can power the board up while the rocket
	is horizontal, such that it comes up in idle mode.  Then you can 
	raise the airframe to launch position, use a TeleDongle to open
	a packet connection, and issue a 'reset' command which will cause
	TeleMetrum to reboot, realize it's now nose-up, and thus choose
	flight mode.  This is much safer than standing on the top step of a
	rickety step-ladder or hanging off the side of a launch tower with
	a screw-driver trying to turn on your avionics before installing
	igniters!
</para>
    </section>
    <section>
      <title>GPS </title>
<para>
	TeleMetrum includes a complete GPS receiver.  See a later section for
	a brief explanation of how GPS works that will help you understand
	the information in the telemetry stream.  The bottom line is that
	the TeleMetrum GPS receiver needs to lock onto at least four 
	satellites to obtain a solid 3 dimensional position fix and know 
	what time it is!
</para>
<para>
	TeleMetrum provides backup power to the GPS chip any time a LiPo
	battery is connected.  This allows the receiver to "warm start" on
	the launch rail much faster than if every power-on were a "cold start"
	for the GPS receiver.  In typical operations, powering up TeleMetrum
	on the flight line in idle mode while performing final airframe
	preparation will be sufficient to allow the GPS receiver to cold
	start and acquire lock.  Then the board can be powered down during
	RSO review and installation on a launch rod or rail.  When the board
	is turned back on, the GPS system should lock very quickly, typically
	long before igniter installation and return to the flight line are
	complete.
</para>
    </section>
    <section>
      <title>Ground Testing </title>
	<para>
	An important aspect of preparing a rocket using electronic deployment
	for flight is ground testing the recovery system.  Thanks
	to the bi-directional RF link central to the Altus Metrum system, 
	this can be accomplished in a TeleMetrum-equipped rocket without as
	much work as you may be accustomed to with other systems.  It can
	even be fun!
	</para>
	<para>
	Just prep the rocket for flight, then power up TeleMetrum while the
	airframe is horizontal.  This will cause the firmware to go into 
	"idle" mode, in which the normal flight state machine is disabled and
	charges will not fire without manual command.  Then, establish an
	RF packet connection from a TeleDongle-equipped computer using the 
	P command from a safe distance.  You can now command TeleMetrum to
	fire the apogee or main charges to complete your testing.
	</para>
	<para>
	In order to reduce the chance of accidental firing of pyrotechnic
	charges, the command to fire a charge is intentionally somewhat
	difficult to type, and the built-in help is slightly cryptic to 
	prevent accidental echoing of characters from the help text back at
	the board from firing a charge.  The command to fire the apogee
	drogue charge is 'i DoIt drogue' and the command to fire the main
	charge is 'i DoIt main'.
	</para>
    </section>
    <section>
      <title>Radio Link </title>
      <para>
        The chip our boards are based on incorporates an RF transceiver, but
        it's not a full duplex system... each end can only be transmitting or
        receiving at any given moment.  So we had to decide how to manage the
        link.
      </para>
      <para>
        By design, TeleMetrum firmware listens for an RF connection when
        it's in "idle mode" (turned on while the rocket is horizontal), which
        allows us to use the RF link to configure the rocket, do things like
        ejection tests, and extract data after a flight without having to 
        crack open the airframe.  However, when the board is in "flight 
        mode" (turned on when the rocket is vertical) the TeleMetrum only 
        transmits and doesn't listen at all.  That's because we want to put 
        ultimate priority on event detection and getting telemetry out of 
        the rocket and out over
        the RF link in case the rocket crashes and we aren't able to extract
        data later... 
      </para>
      <para>
        We don't use a 'normal packet radio' mode because they're just too
        inefficient.  The GFSK modulation we use is just FSK with the 
	baseband pulses passed through a
        Gaussian filter before they go into the modulator to limit the
        transmitted bandwidth.  When combined with the hardware forward error
        correction support in the cc1111 chip, this allows us to have a very
        robust 38.4 kilobit data link with only 10 milliwatts of transmit power,
        a whip antenna in the rocket, and a hand-held Yagi on the ground.  We've
        had a test flight above 12k AGL with good reception, and calculations
        suggest we should be good to 40k AGL or more with a 5-element yagi on
        the ground.  We hope to fly boards to higher altitudes soon, and would
	of course appreciate customer feedback on performance in higher
	altitude flights!
      </para>
    </section>
    <section>
	<title>Configurable Parameters</title>
	<para>
	Configuring a TeleMetrum board for flight is very simple.  Because we
	have both acceleration and pressure sensors, there is no need to set
	a "mach delay", for example.  The few configurable parameters can all
	be set using a simple terminal program over the USB port or RF link
	via TeleDongle.
	</para>
	<section>
	<title>Radio Channel</title>
	<para>
	Our firmware supports 10 channels.  The default channel 0 corresponds
	to a center frequency of 434.550 Mhz, and channels are spaced every 
	100 khz.  Thus, channel 1 is 434.650 Mhz, and channel 9 is 435.550 Mhz.
	At any given launch, we highly recommend coordinating who will use
	each channel and when to avoid interference.  And of course, both 
	TeleMetrum and TeleDongle must be configured to the same channel to
	successfully communicate with each other.
	</para>
	<para>
	To set the radio channel, use the 'c r' command, like 'c r 3' to set
	channel 3.  
	As with all 'c' sub-commands, follow this with a 'c w' to write the 
	change to the parameter block in the on-board DataFlash chip.
	</para>
	</section>
	<section>
	<title>Apogee Delay</title>
	<para>
	Apogee delay is the number of seconds after TeleMetrum detects flight
	apogee that the drogue charge should be fired.  In most cases, this
	should be left at the default of 0.  However, if you are flying
	redundant electronics such as for an L3 certification, you may wish 
	to set one of your altimeters to a positive delay so that both 
	primary and backup pyrotechnic charges do not fire simultaneously.
	</para>
	<para>
	To set the apogee delay, use the [FIXME] command.
	As with all 'c' sub-commands, follow this with a 'c w' to write the 
	change to the parameter block in the on-board DataFlash chip.
	</para>
	</section>
	<section>
	<title>Main Deployment Altitude</title>
	<para>
	By default, TeleMetrum will fire the main deployment charge at an
	elevation of 250 meters (about 820 feet) above ground.  We think this
	is a good elevation for most airframes, but feel free to change this 
	to suit.  In particular, if you are flying two altimeters, you may
	wish to set the
	deployment elevation for the backup altimeter to be something lower
	than the primary so that both pyrotechnic charges don't fire
	simultaneously.
	</para>
	<para>
	To set the main deployment altitude, use the [FIXME] command.
	As with all 'c' sub-commands, follow this with a 'c w' to write the 
	change to the parameter block in the on-board DataFlash chip.
	</para>
	</section>
    </section>
    <section>
	<title>Calibration</title>
	<para>
	There are only two calibrations required for a TeleMetrum board, and
	only one for TeleDongle.
	</para>
	<section>
	<title>Radio Frequency</title>
	<para>
	The radio frequency is synthesized from a clock based on the 48 Mhz
	crystal on the board.  The actual frequency of this oscillator must be
	measured to generate a calibration constant.  While our GFSK modulation
	bandwidth is wide enough to allow boards to communicate even when 
	their oscillators are not on exactly the same frequency, performance
	is best when they are closely matched.
	Radio frequency calibration requires a calibrated frequency counter.
	Fortunately, once set, the variation in frequency due to aging and
	temperature changes is small enough that re-calibration by customers
	should generally not be required.
	</para>
	<para>
	To calibrate the radio frequency, connect the UHF antenna port to a
	frequency counter, set the board to channel 0, and use the 'C' 
	command to generate a CW carrier.  Wait for the transmitter temperature
	to stabilize and the frequency to settle down.  
	Then, divide 434.550 Mhz by the 
	measured frequency and multiply by the current radio cal value show
	in the 'c s' command.  For an unprogrammed board, the default value
	is 1186611.  Take the resulting integer and program it using the 'c f'
	command.  Testing with the 'C' command again should show a carrier
	within a few tens of Hertz of the intended frequency.
	As with all 'c' sub-commands, follow this with a 'c w' to write the 
	change to the parameter block in the on-board DataFlash chip.
	</para>
	</section>
	<section>
	<title>Accelerometer</title>
	<para>
	The accelerometer we use has its own 5 volt power supply and
	the output must be passed through a resistive voltage divider to match
	the input of our 3.3 volt ADC.  This means that unlike the barometric
	sensor, the output of the acceleration sensor is not ratiometric to 
	the ADC converter, and calibration is required.  We also support the 
	use of any of several accelerometers from a Freescale family that 
	includes at least +/- 40g, 50g, 100g, and 200g parts.  Using gravity,
	a simple 2-point calibration yields acceptable results capturing both
	the different sensitivities and ranges of the different accelerometer
	parts and any variation in power supply voltages or resistor values
	in the divider network.
	</para>
	<para>
	To calibrate the acceleration sensor, use the 'c a 0' command.  You
	will be prompted to orient the board vertically with the UHF antenna
	up and press a key, then to orient the board vertically with the 
	UHF antenna down and press a key.
	As with all 'c' sub-commands, follow this with a 'c w' to write the 
	change to the parameter block in the on-board DataFlash chip.
	</para>
	<para>
	The +1g and -1g calibration points are included in each telemetry
	frame and are part of the header extracted by ao-dumplog after flight.
	Note that we always store and return raw ADC samples for each
	sensor... nothing is permanently "lost" or "damaged" if the 
	calibration is poor.
	</para>
	</section>
    </section>
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
    <section>
	<title>
	How GPS Works
	</title>
	<para>
	Placeholder.
	</para>
    </section>
  </chapter>
</book>

