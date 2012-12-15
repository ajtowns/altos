<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"
  "/usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd">
<book>
  <title>The Altus Metrum System</title>
  <subtitle>An Owner's Manual for TeleMetrum, TeleMini and TeleDongle Devices</subtitle>
  <bookinfo>
    <author>
      <firstname>Bdale</firstname>
      <surname>Garbee</surname>
    </author>
    <author>
      <firstname>Keith</firstname>
      <surname>Packard</surname>
    </author>
    <author>
      <firstname>Bob</firstname>
      <surname>Finch</surname>
    </author>
    <author>
      <firstname>Anthony</firstname>
      <surname>Towns</surname>
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
	<revnumber>1.1.1</revnumber>
	<date>16 September 2012</date>
	<revremark>
	  Updated for software version 1.1.1 Version 1.1.1 fixes a few
	  bugs found in version 1.1.
	</revremark>
      </revision>
      <revision>
	<revnumber>1.1</revnumber>
	<date>13 September 2012</date>
	<revremark>
	  Updated for software version 1.1. Version 1.1 has new
	  features but is otherwise compatible with version 1.0.
	</revremark>
      </revision>
      <revision>
        <revnumber>1.0</revnumber>
        <date>24 August 2011</date>
	<revremark>
	  Updated for software version 1.0.  Note that 1.0 represents a
	  telemetry format change, meaning both ends of a link 
	  (TeleMetrum/TeleMini and TeleDongle) must be updated or 
          communications will fail.
	</revremark>
      </revision>
      <revision>
        <revnumber>0.9</revnumber>
        <date>18 January 2011</date>
	<revremark>
	  Updated for software version 0.9.  Note that 0.9 represents a
	  telemetry format change, meaning both ends of a link (TeleMetrum and
	  TeleDongle) must be updated or communications will fail.
	</revremark>
      </revision>
      <revision>
        <revnumber>0.8</revnumber>
        <date>24 November 2010</date>
	<revremark>Updated for software version 0.8 </revremark>
      </revision>
    </revhistory>
  </bookinfo>
  <acknowledgements>
    <para>
      Thanks to Bob Finch, W9YA, NAR 12965, TRA 12350 for writing "The
      Mere-Mortals Quick Start/Usage Guide to the Altus Metrum Starter
      Kit" which formed the basis of the original Getting Started chapter 
      in this manual.  Bob was one of our first customers for a production
      TeleMetrum, and his continued enthusiasm and contributions
      are immensely gratifying and highly appreciated!
    </para>
    <para>
      And thanks to Anthony (AJ) Towns for major contributions including
      the AltosUI graphing and site map code and associated documentation. 
      Free software means that our customers and friends can become our
      collaborators, and we certainly appreciate this level of
      contribution!
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
      The first device created for our community was TeleMetrum, a dual
      deploy altimeter with fully integrated GPS and radio telemetry
      as standard features, and a "companion interface" that will
      support optional capabilities in the future.
    </para>
    <para>
      The newest device is TeleMini, a dual deploy altimeter with
      radio telemetry and radio direction finding. This device is only
      13mm by 38mm (½ inch by 1½ inches) and can fit easily in an 18mm 
      air-frame.
    </para>
    <para>
      Complementing TeleMetrum and TeleMini is TeleDongle, a USB to RF 
      interface for communicating with the altimeters.  Combined with your 
      choice of antenna and
      notebook computer, TeleDongle and our associated user interface software
      form a complete ground station capable of logging and displaying in-flight
      telemetry, aiding rocket recovery, then processing and archiving flight
      data for analysis and review.
    </para>
    <para>
      More products will be added to the Altus Metrum family over time, and
      we currently envision that this will be a single, comprehensive manual
      for the entire product family.
    </para>
  </chapter>
  <chapter>
    <title>Getting Started</title>
    <para>
      The first thing to do after you check the inventory of parts in your
      "starter kit" is to charge the battery.
    </para>
    <para>
      The TeleMetrum battery can be charged by plugging it into the
      corresponding socket of the TeleMetrum and then using the USB A to
      mini B
      cable to plug the TeleMetrum into your computer's USB socket. The
      TeleMetrum circuitry will charge the battery whenever it is plugged
      in, because the TeleMetrum's on-off switch does NOT control the
      charging circuitry.
    </para>
    <para>
      When the GPS chip is initially searching for
      satellites, TeleMetrum will consume more current than it can pull
      from the USB port, so the battery must be attached in order to get
      satellite lock.  Once GPS is locked, the current consumption goes back
      down enough to enable charging while
      running. So it's a good idea to fully charge the battery as your
      first item of business so there is no issue getting and maintaining
      satellite lock.  The yellow charge indicator led will go out when the
      battery is nearly full and the charger goes to trickle charge. It
      can take several hours to fully recharge a deeply discharged battery.
    </para>
    <para>
      The TeleMini battery can be charged by disconnecting it from the
      TeleMini board and plugging it into a standalone battery charger 
      board, and connecting that via a USB cable to a laptop or other USB
      power source
    </para>
    <para>
      The other active device in the starter kit is the TeleDongle USB to
      RF interface.  If you plug it in to your Mac or Linux computer it should
      "just work", showing up as a serial port device.  Windows systems need
      driver information that is part of the AltOS download to know that the
      existing USB modem driver will work.  We therefore recommend installing
      our software before plugging in TeleDongle if you are using a Windows
      computer.  If you are using Linux and are having problems, try moving 
      to a fresher kernel (2.6.33 or newer), as the USB serial driver had 
      ugly bugs in some earlier versions.
    </para>
    <para>
      Next you should obtain and install the AltOS software.  These include
      the AltosUI ground station program, current firmware images for
      TeleMetrum, TeleMini and TeleDongle, and a number of standalone 
      utilities that are rarely needed.  Pre-built binary packages are 
      available for Linux, Microsoft Windows, and recent MacOSX versions.  
      Full source code and build instructions are also available.
      The latest version may always be downloaded from
      <ulink url="http://altusmetrum.org/AltOS"/>.
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
      The Lithium Polymer rechargeable batteries have an
      extraordinary power density.  This is great because we can fly with
      much less battery mass than if we used alkaline batteries or previous
      generation rechargeable batteries... but if they are punctured
      or their leads are allowed to short, they can and will release their
      energy very rapidly!
      Thus we recommend that you take some care when handling our batteries
      and consider giving them some extra protection in your air-frame.  We
      often wrap them in suitable scraps of closed-cell packing foam before
      strapping them down, for example.
    </para>
    <para>
      The barometric sensors used on both TeleMetrum and TeleMini are 
      sensitive to sunlight.  In normal TeleMetrum mounting situations, it 
      and all of the other surface mount components
      are "down" towards whatever the underlying mounting surface is, so
      this is not normally a problem.  Please consider this, though, when
      designing an installation, for example, in an air-frame with a
      see-through plastic payload bay.  It is particularly important to
      consider this with TeleMini, both because the baro sensor is on the
      "top" of the board, and because many model rockets with payload bays
      use clear plastic for the payload bay!  Replacing these with an opaque
      cardboard tube, painting them, or wrapping them with a layer of masking
      tape are all reasonable approaches to keep the sensor out of direct
      sunlight.
    </para>
    <para>
      The barometric sensor sampling port must be able to "breathe",
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
    <title>Hardware Overview</title>
    <para>
      TeleMetrum is a 1 inch by 2.75 inch circuit board.  It was designed to
      fit inside coupler for 29mm air-frame tubing, but using it in a tube that
      small in diameter may require some creativity in mounting and wiring
      to succeed!  The presence of an accelerometer means TeleMetrum should
      be aligned along the flight axis of the airframe, and by default the 1/4
      wave UHF wire antenna should be on the nose-cone end of the board.  The
      antenna wire is about 7 inches long, and wiring for a power switch and
      the e-matches for apogee and main ejection charges depart from the
      fin can end of the board, meaning an ideal "simple" avionics
      bay for TeleMetrum should have at least 10 inches of interior length.
    </para>
    <para>
      TeleMini is a 0.5 inch by 1.5 inch circuit board.   It was designed to
      fit inside an 18mm air-frame tube, but using it in a tube that
      small in diameter may require some creativity in mounting and wiring
      to succeed!  Since there is no accelerometer, TeleMini can be mounted
      in any convenient orientation.  The default 1/4
      wave UHF wire antenna attached to the center of one end of
      the board is about 7 inches long, and wiring for a power switch and
      the e-matches for apogee and main ejection charges depart from the
      other end of the board, meaning an ideal "simple" avionics
      bay for TeleMini should have at least 9 inches of interior length.
    </para>
    <para>
      A typical TeleMetrum or TeleMini installation involves attaching 
      only a suitable Lithium Polymer battery, a single pole switch for 
      power on/off, and two pairs of wires connecting e-matches for the 
      apogee and main ejection charges.  All Altus Metrum products are 
      designed for use with single-cell batteries with 3.7 volts nominal.
    </para>
    <para>
      The battery connectors are a standard 2-pin JST connector and
      match batteries sold by Spark Fun. These batteries are
      single-cell Lithium Polymer batteries that nominally provide 3.7
      volts.  Other vendors sell similar batteries for RC aircraft
      using mating connectors, however the polarity for those is
      generally reversed from the batteries used by Altus Metrum
      products. In particular, the Tenergy batteries supplied for use
      in Featherweight flight computers are not compatible with Altus
      Metrum flight computers or battery chargers. <emphasis>Check
      polarity and voltage before connecting any battery not purchased
      from Altus Metrum or Spark Fun.</emphasis>
    </para>
    <para>
      By default, we use the unregulated output of the Li-Po battery directly
      to fire ejection charges.  This works marvelously with standard
      low-current e-matches like the J-Tek from MJG Technologies, and with
      Quest Q2G2 igniters.  However, if you want or need to use a separate 
      pyro battery, check out the "External Pyro Battery" section in this 
      manual for instructions on how to wire that up. The altimeters are 
      designed to work with an external pyro battery of no more than 15 volts.
    </para>
    <para>
      Ejection charges are wired directly to the screw terminal block
      at the aft end of the altimeter.  You'll need a very small straight 
      blade screwdriver for these screws, such as you might find in a 
      jeweler's screwdriver set.
    </para>
    <para>
      TeleMetrum also uses the screw terminal block for the power
      switch leads. On TeleMini, the power switch leads are soldered
      directly to the board and can be connected directly to a switch.
    </para>
    <para>
      For most air-frames, the integrated antennas are more than
      adequate.   However, if you are installing in a carbon-fiber or
      metal electronics bay which is opaque to RF signals, you may need to
      use off-board external antennas instead.  In this case, you can
      order an altimeter with an SMA connector for the UHF antenna
      connection, and, on TeleMetrum, you can unplug the integrated GPS
      antenna and select an appropriate off-board GPS antenna with
      cable terminating in a U.FL connector.
    </para>
  </chapter>
  <chapter>
    <title>System Operation</title>
    <section>
      <title>Firmware Modes </title>
      <para>
        The AltOS firmware build for the altimeters has two
        fundamental modes, "idle" and "flight".  Which of these modes
        the firmware operates in is determined at start up time. For
        TeleMetrum, the mode is controlled by the orientation of the
        rocket (well, actually the board, of course...) at the time
        power is switched on.  If the rocket is "nose up", then
        TeleMetrum assumes it's on a rail or rod being prepared for
        launch, so the firmware chooses flight mode.  However, if the
        rocket is more or less horizontal, the firmware instead enters
        idle mode.  Since TeleMini doesn't have an accelerometer we can
        use to determine orientation, "idle" mode is selected when the
        board receives a command packet within the first five seconds
        of operation; if no packet is received, the board enters
        "flight" mode.
      </para>
      <para>
        At power on, you will hear three beeps or see three flashes
        ("S" in Morse code for start up) and then a pause while
        the altimeter completes initialization and self test, and decides 
	which mode to enter next.
      </para>
      <para>
        In flight or "pad" mode, the altimeter engages the flight
        state machine, goes into transmit-only mode to
        send telemetry, and waits for launch to be detected.
        Flight mode is indicated by an "di-dah-dah-dit" ("P" for pad)
        on the beeper or lights, followed by beeps or flashes
        indicating the state of the pyrotechnic igniter continuity.
        One beep/flash indicates apogee continuity, two beeps/flashes
        indicate main continuity, three beeps/flashes indicate both
        apogee and main continuity, and one longer "brap" sound or
        rapidly alternating lights indicates no continuity.  For a
        dual deploy flight, make sure you're getting three beeps or
        flashes before launching!  For apogee-only or motor eject
        flights, do what makes sense.
      </para>
      <para>
        If idle mode is entered, you will hear an audible "di-dit" or see 
        two short flashes ("I" for idle), and the flight state machine is 
        disengaged, thus no ejection charges will fire.  The altimeters also 
        listen for the radio link when in idle mode for requests sent via 
        TeleDongle.  Commands can be issued to a TeleMetrum in idle mode 
        over either
        USB or the radio link equivalently. TeleMini only has the radio link.
        Idle mode is useful for configuring the altimeter, for extracting data
        from the on-board storage chip after flight, and for ground testing
        pyro charges.
      </para>
      <para>
        One "neat trick" of particular value when TeleMetrum is used with 
        very large air-frames, is that you can power the board up while the 
        rocket is horizontal, such that it comes up in idle mode.  Then you can
        raise the air-frame to launch position, and issue a 'reset' command 
	via TeleDongle over the radio link to cause the altimeter to reboot and 
        come up in flight mode.  This is much safer than standing on the top 
        step of a rickety step-ladder or hanging off the side of a launch 
        tower with a screw-driver trying to turn on your avionics before 
        installing igniters!
      </para>
    </section>
    <section>
      <title>GPS </title>
      <para>
        TeleMetrum includes a complete GPS receiver.  A complete explanation 
        of how GPS works is beyond the scope of this manual, but the bottom 
        line is that the TeleMetrum GPS receiver needs to lock onto at least 
        four satellites to obtain a solid 3 dimensional position fix and know
        what time it is.
      </para>
      <para>
        TeleMetrum provides backup power to the GPS chip any time a 
        battery is connected.  This allows the receiver to "warm start" on
        the launch rail much faster than if every power-on were a GPS 
	"cold start".  In typical operations, powering up TeleMetrum
        on the flight line in idle mode while performing final air-frame
        preparation will be sufficient to allow the GPS receiver to cold
        start and acquire lock.  Then the board can be powered down during
        RSO review and installation on a launch rod or rail.  When the board
        is turned back on, the GPS system should lock very quickly, typically
        long before igniter installation and return to the flight line are
        complete.
      </para>
    </section>
    <section>
      <title>Controlling An Altimeter Over The Radio Link</title>
      <para>
        One of the unique features of the Altus Metrum system is
        the ability to create a two way command link between TeleDongle
        and an altimeter using the digital radio transceivers built into
        each device. This allows you to interact with the altimeter from
        afar, as if it were directly connected to the computer.
      </para>
      <para>
        Any operation which can be performed with TeleMetrum can
        either be done with TeleMetrum directly connected to the
        computer via the USB cable, or through the radio
        link. TeleMini doesn't provide a USB connector and so it is
        always communicated with over radio.  Select the appropriate 
        TeleDongle device when the list of devices is presented and 
        AltosUI will interact with an altimeter over the radio link.
      </para>
      <para>
	One oddity in the current interface is how AltosUI selects the
	frequency for radio communications. Instead of providing
	an interface to specifically configure the frequency, it uses
	whatever frequency was most recently selected for the target
	TeleDongle device in Monitor Flight mode. If you haven't ever
	used that mode with the TeleDongle in question, select the
	Monitor Flight button from the top level UI, and pick the
	appropriate TeleDongle device.  Once the flight monitoring
	window is open, select the desired frequency and then close it
	down again. All radio communications will now use that frequency.
      </para>
      <itemizedlist>
        <listitem>
          <para>
            Save Flight Data—Recover flight data from the rocket without
            opening it up.
          </para>
        </listitem>
        <listitem>
          <para>
            Configure altimeter apogee delays or main deploy heights
            to respond to changing launch conditions. You can also
            'reboot' the altimeter. Use this to remotely enable the
            flight computer by turning TeleMetrum on in "idle" mode,
            then once the air-frame is oriented for launch, you can
            reboot the altimeter and have it restart in pad mode
            without having to climb the scary ladder.
          </para>
        </listitem>
        <listitem>
          <para>
            Fire Igniters—Test your deployment charges without snaking
            wires out through holes in the air-frame. Simply assembly the
            rocket as if for flight with the apogee and main charges
            loaded, then remotely command the altimeter to fire the
            igniters.
          </para>
        </listitem>
      </itemizedlist>
      <para>
        Operation over the radio link for configuring an altimeter, ground
        testing igniters, and so forth uses the same RF frequencies as flight
        telemetry.  To configure the desired TeleDongle frequency, select
        the monitor flight tab, then use the frequency selector and 
        close the window before performing other desired radio operations.
      </para>
      <para>
        TeleMetrum only enables radio commanding in 'idle' mode, so
        make sure you have TeleMetrum lying horizontally when you turn
        it on. Otherwise, TeleMetrum will start in 'pad' mode ready for
        flight, and will not be listening for command packets from TeleDongle.
      </para>
      <para>
	TeleMini listens for a command packet for five seconds after
	first being turned on, if it doesn't hear anything, it enters
	'pad' mode, ready for flight and will no longer listen for
	command packets. The easiest way to connect to TeleMini is to
	initiate the command and select the TeleDongle device. At this
	point, the TeleDongle will be attempting to communicate with
	the TeleMini. Now turn TeleMini on, and it should immediately
	start communicating with the TeleDongle and the desired
	operation can be performed.
      </para>
      <para>
        You can monitor the operation of the radio link by watching the 
        lights on the devices. The red LED will flash each time a packet
        is transmitted, while the green LED will light up on TeleDongle when 
        it is waiting to receive a packet from the altimeter.
      </para>
    </section>
    <section>
      <title>Ground Testing </title>
      <para>
        An important aspect of preparing a rocket using electronic deployment
        for flight is ground testing the recovery system.  Thanks
        to the bi-directional radio link central to the Altus Metrum system,
        this can be accomplished in a TeleMetrum or TeleMini equipped rocket 
        with less work than you may be accustomed to with other systems.  It 
        can even be fun!
      </para>
      <para>
        Just prep the rocket for flight, then power up the altimeter
        in "idle" mode (placing air-frame horizontal for TeleMetrum or
        selected the Configure Altimeter tab for TeleMini).  This will cause 
        the firmware to go into "idle" mode, in which the normal flight
        state machine is disabled and charges will not fire without
        manual command.  You can now command the altimeter to fire the apogee
        or main charges from a safe distance using your computer and 
        TeleDongle and the Fire Igniter tab to complete ejection testing.
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
        By design, the altimeter firmware listens for the radio link when
        it's in "idle mode", which
        allows us to use the radio link to configure the rocket, do things like
        ejection tests, and extract data after a flight without having to
        crack open the air-frame.  However, when the board is in "flight
        mode", the altimeter only
        transmits and doesn't listen at all.  That's because we want to put
        ultimate priority on event detection and getting telemetry out of
        the rocket through
        the radio in case the rocket crashes and we aren't able to extract
        data later...
      </para>
      <para>
        We don't use a 'normal packet radio' mode like APRS because they're 
        just too inefficient.  The GFSK modulation we use is FSK with the
        base-band pulses passed through a
        Gaussian filter before they go into the modulator to limit the
        transmitted bandwidth.  When combined with the hardware forward error
        correction support in the cc1111 chip, this allows us to have a very
        robust 38.4 kilobit data link with only 10 milliwatts of transmit 
        power, a whip antenna in the rocket, and a hand-held Yagi on the 
        ground.  We've had flights to above 21k feet AGL with great reception, 
        and calculations suggest we should be good to well over 40k feet AGL 
        with a 5-element yagi on the ground.  We hope to fly boards to higher 
        altitudes over time, and would of course appreciate customer feedback 
        on performance in higher altitude flights!
      </para>
    </section>
    <section>
      <title>Configurable Parameters</title>
      <para>
        Configuring an Altus Metrum altimeter for flight is very
        simple.  Even on our baro-only TeleMini board, the use of a Kalman 
        filter means there is no need to set a "mach delay".  The few 
        configurable parameters can all be set using AltosUI over USB or
        or radio link via TeleDongle.
      </para>
      <section>
        <title>Radio Frequency</title>
        <para>
	  Altus Metrum boards support radio frequencies in the 70cm
	  band. By default, the configuration interface provides a
	  list of 10 "standard" frequencies in 100kHz channels starting at
	  434.550MHz.  However, the firmware supports use of
	  any 50kHz multiple within the 70cm band. At any given
	  launch, we highly recommend coordinating when and by whom each
	  frequency will be used to avoid interference.  And of course, both
	  altimeter and TeleDongle must be configured to the same
	  frequency to successfully communicate with each other.
        </para>
      </section>
      <section>
        <title>Apogee Delay</title>
        <para>
          Apogee delay is the number of seconds after the altimeter detects flight
          apogee that the drogue charge should be fired.  In most cases, this
          should be left at the default of 0.  However, if you are flying
          redundant electronics such as for an L3 certification, you may wish
          to set one of your altimeters to a positive delay so that both
          primary and backup pyrotechnic charges do not fire simultaneously.
        </para>
        <para>
          The Altus Metrum apogee detection algorithm fires exactly at
          apogee.  If you are also flying an altimeter like the
          PerfectFlite MAWD, which only supports selecting 0 or 1
          seconds of apogee delay, you may wish to set the MAWD to 0
          seconds delay and set the TeleMetrum to fire your backup 2
          or 3 seconds later to avoid any chance of both charges
          firing simultaneously.  We've flown several air-frames this
          way quite happily, including Keith's successful L3 cert.
        </para>
      </section>
      <section>
        <title>Main Deployment Altitude</title>
        <para>
          By default, the altimeter will fire the main deployment charge at an
          elevation of 250 meters (about 820 feet) above ground.  We think this
          is a good elevation for most air-frames, but feel free to change this
          to suit.  In particular, if you are flying two altimeters, you may
          wish to set the
          deployment elevation for the backup altimeter to be something lower
          than the primary so that both pyrotechnic charges don't fire
          simultaneously.
        </para>
      </section>
      <section>
	<title>Maximum Flight Log</title>
	<para>
	  TeleMetrum version 1.1 and 1.2 have 2MB of on-board flash storage,
	  enough to hold over 40 minutes of data at full data rate
	  (100 samples/second). TeleMetrum 1.0 has 1MB of on-board
	  storage. As data are stored at a reduced rate during descent
	  (10 samples/second), there's plenty of space to store many
	  flights worth of data.
	</para>
	<para>
	  The on-board flash is partitioned into separate flight logs,
	  each of a fixed maximum size. Increase the maximum size of
	  each log and you reduce the number of flights that can be
	  stored. Decrease the size and TeleMetrum can store more
	  flights.
	</para>
	<para>
	  All of the configuration data is also stored in the flash
	  memory, which consumes 64kB on TeleMetrum v1.1/v1.2 and 256B on
	  TeleMetrum v1.0. This configuration space is not available
	  for storing flight log data.
	</para>
	<para>
	  To compute the amount of space needed for a single flight,
	  you can multiply the expected ascent time (in seconds) by
	  800, multiply the expected descent time (in seconds) by 80
	  and add the two together. That will slightly under-estimate
	  the storage (in bytes) needed for the flight. For instance,
	  a flight spending 20 seconds in ascent and 150 seconds in
	  descent will take about (20 * 800) + (150 * 80) = 28000
	  bytes of storage. You could store dozens of these flights in
	  the on-board flash.
	</para>
	<para>
	  The default size, 192kB, allows for 10 flights of storage on
	  TeleMetrum v1.1/v1.2 and 5 flights on TeleMetrum v1.0. This
	  ensures that you won't need to erase the memory before
	  flying each time while still allowing more than sufficient
	  storage for each flight.
	</para>
	<para>
	  As TeleMini does not contain an accelerometer, it stores
	  data at 10 samples per second during ascent and one sample
	  per second during descent. Each sample is a two byte reading
	  from the barometer. These are stored in 5kB of
	  on-chip flash memory which can hold 256 seconds at the
	  ascent rate or 2560 seconds at the descent rate. Because of
	  the limited storage, TeleMini cannot hold data for more than
	  one flight, and so must be erased after each flight or it
	  will not capture data for subsequent flights.
	</para>
      </section>
      <section>
	<title>Ignite Mode</title>
	<para>
	  Instead of firing one charge at apogee and another charge at
	  a fixed height above the ground, you can configure the
	  altimeter to fire both at apogee or both during
	  descent. This was added to support an airframe that has two
	  TeleMetrum computers, one in the fin can and one in the
	  nose.
	</para>
	<para>
	  Providing the ability to use both igniters for apogee or
	  main allows some level of redundancy without needing two
	  flight computers.  In Redundant Apogee or Redundant Main
	  mode, the two charges will be fired two seconds apart.
	</para>
      </section>
      <section>
	<title>Pad Orientation</title>
	<para>
	  TeleMetrum measures acceleration along the axis of the
	  board. Which way the board is oriented affects the sign of
	  the acceleration value. Instead of trying to guess which way
	  the board is mounted in the air frame, TeleMetrum must be
	  explicitly configured for either Antenna Up or Antenna
	  Down. The default, Antenna Up, expects the end of the
	  TeleMetrum board connected to the 70cm antenna to be nearest
	  the nose of the rocket, with the end containing the screw
	  terminals nearest the tail.
	</para>
      </section>
    </section>

  </chapter>
  <chapter>

    <title>AltosUI</title>
    <para>
      The AltosUI program provides a graphical user interface for
      interacting with the Altus Metrum product family, including
      TeleMetrum, TeleMini and TeleDongle. AltosUI can monitor telemetry data,
      configure TeleMetrum, TeleMini and TeleDongle devices and many other
      tasks. The primary interface window provides a selection of
      buttons, one for each major activity in the system.  This manual
      is split into chapters, each of which documents one of the tasks
      provided from the top-level toolbar.
    </para>
    <section>
      <title>Monitor Flight</title>
      <subtitle>Receive, Record and Display Telemetry Data</subtitle>
      <para>
        Selecting this item brings up a dialog box listing all of the
        connected TeleDongle devices. When you choose one of these,
        AltosUI will create a window to display telemetry data as
        received by the selected TeleDongle device.
      </para>
      <para>
        All telemetry data received are automatically recorded in
        suitable log files. The name of the files includes the current
        date and rocket serial and flight numbers.
      </para>
      <para>
        The radio frequency being monitored by the TeleDongle device is
        displayed at the top of the window. You can configure the
        frequency by clicking on the frequency box and selecting the desired
        frequency. AltosUI remembers the last frequency selected for each
        TeleDongle and selects that automatically the next time you use
        that device.
      </para>
      <para>
        Below the TeleDongle frequency selector, the window contains a few
        significant pieces of information about the altimeter providing
        the telemetry data stream:
      </para>
      <itemizedlist>
        <listitem>
          <para>The configured call-sign</para>
        </listitem>
        <listitem>
          <para>The device serial number</para>
        </listitem>
        <listitem>
          <para>The flight number. Each altimeter remembers how many
            times it has flown.
          </para>
        </listitem>
        <listitem>
          <para>
            The rocket flight state. Each flight passes through several
            states including Pad, Boost, Fast, Coast, Drogue, Main and
            Landed.
          </para>
        </listitem>
        <listitem>
          <para>
            The Received Signal Strength Indicator value. This lets
            you know how strong a signal TeleDongle is receiving. The
            radio inside TeleDongle operates down to about -99dBm;
            weaker signals may not be receivable. The packet link uses
            error detection and correction techniques which prevent
            incorrect data from being reported.
          </para>
        </listitem>
      </itemizedlist>
      <para>
        Finally, the largest portion of the window contains a set of
        tabs, each of which contain some information about the rocket.
        They're arranged in 'flight order' so that as the flight
        progresses, the selected tab automatically switches to display
        data relevant to the current state of the flight. You can select
        other tabs at any time. The final 'table' tab displays all of
        the raw telemetry values in one place in a spreadsheet-like format.
      </para>
      <section>
        <title>Launch Pad</title>
        <para>
          The 'Launch Pad' tab shows information used to decide when the
          rocket is ready for flight. The first elements include red/green
          indicators, if any of these is red, you'll want to evaluate
          whether the rocket is ready to launch:
          <itemizedlist>
            <listitem>
              <para>
                Battery Voltage. This indicates whether the Li-Po battery
                powering the TeleMetrum has sufficient charge to last for
                the duration of the flight. A value of more than
                3.7V is required for a 'GO' status.
              </para>
            </listitem>
            <listitem>
              <para>
                Apogee Igniter Voltage. This indicates whether the apogee
                igniter has continuity. If the igniter has a low
                resistance, then the voltage measured here will be close
                to the Li-Po battery voltage. A value greater than 3.2V is
                required for a 'GO' status.
              </para>
            </listitem>
            <listitem>
              <para>
                Main Igniter Voltage. This indicates whether the main
                igniter has continuity. If the igniter has a low
                resistance, then the voltage measured here will be close
                to the Li-Po battery voltage. A value greater than 3.2V is
                required for a 'GO' status.
              </para>
            </listitem>
	    <listitem>
	      <para>
		On-board Data Logging. This indicates whether there is
		space remaining on-board to store flight data for the
		upcoming flight. If you've downloaded data, but failed
		to erase flights, there may not be any space
		left. TeleMetrum can store multiple flights, depending
		on the configured maximum flight log size. TeleMini
		stores only a single flight, so it will need to be
		downloaded and erased after each flight to capture
		data. This only affects on-board flight logging; the
		altimeter will still transmit telemetry and fire
		ejection charges at the proper times.
	      </para>
	    </listitem>
            <listitem>
              <para>
                GPS Locked. For a TeleMetrum device, this indicates whether the GPS receiver is
                currently able to compute position information. GPS requires
                at least 4 satellites to compute an accurate position.
              </para>
            </listitem>
            <listitem>
              <para>
                GPS Ready. For a TeleMetrum device, this indicates whether GPS has reported at least
                10 consecutive positions without losing lock. This ensures
                that the GPS receiver has reliable reception from the
                satellites.
              </para>
            </listitem>
          </itemizedlist>
          <para>
            The Launchpad tab also shows the computed launch pad position
            and altitude, averaging many reported positions to improve the
            accuracy of the fix.
          </para>
        </para>
      </section>
      <section>
        <title>Ascent</title>
        <para>
          This tab is shown during Boost, Fast and Coast
          phases. The information displayed here helps monitor the
          rocket as it heads towards apogee.
        </para>
        <para>
          The height, speed and acceleration are shown along with the
          maximum values for each of them. This allows you to quickly
          answer the most commonly asked questions you'll hear during
          flight.
        </para>
        <para>
          The current latitude and longitude reported by the TeleMetrum GPS are
          also shown. Note that under high acceleration, these values
          may not get updated as the GPS receiver loses position
          fix. Once the rocket starts coasting, the receiver should
          start reporting position again.
        </para>
        <para>
          Finally, the current igniter voltages are reported as in the
          Launch Pad tab. This can help diagnose deployment failures
          caused by wiring which comes loose under high acceleration.
        </para>
      </section>
      <section>
        <title>Descent</title>
        <para>
          Once the rocket has reached apogee and (we hope) activated the
          apogee charge, attention switches to tracking the rocket on
          the way back to the ground, and for dual-deploy flights,
          waiting for the main charge to fire.
        </para>
        <para>
          To monitor whether the apogee charge operated correctly, the
          current descent rate is reported along with the current
          height. Good descent rates vary based on the choice of recovery
	  components, but generally range from 15-30m/s on drogue and should
	  be below 10m/s when under the main parachute in a dual-deploy flight.
        </para>
        <para>
          For TeleMetrum altimeters, you can locate the rocket in the
          sky using the elevation and bearing information to figure
          out where to look. Elevation is in degrees above the
          horizon. Bearing is reported in degrees relative to true
          north. Range can help figure out how big the rocket will
          appear. Ground Distance shows how far it is to a point
          directly under the rocket and can help figure out where the
          rocket is likely to land. Note that all of these values are
          relative to the pad location. If the elevation is near 90°,
          the rocket is over the pad, not over you.
        </para>
        <para>
          Finally, the igniter voltages are reported in this tab as
          well, both to monitor the main charge as well as to see what
          the status of the apogee charge is.  Note that some commercial
	  e-matches are designed to retain continuity even after being
	  fired, and will continue to show as green or return from red to
	  green after firing.
        </para>
      </section>
      <section>
        <title>Landed</title>
        <para>
          Once the rocket is on the ground, attention switches to
          recovery. While the radio signal is often lost once the
          rocket is on the ground, the last reported GPS position is
          generally within a short distance of the actual landing location.
        </para>
        <para>
          The last reported GPS position is reported both by
          latitude and longitude as well as a bearing and distance from
          the launch pad. The distance should give you a good idea of
          whether to walk or hitch a ride.  Take the reported
          latitude and longitude and enter them into your hand-held GPS
          unit and have that compute a track to the landing location.
        </para>
	<para>
	  Both TeleMini and TeleMetrum will continue to transmit RDF
	  tones after landing, allowing you to locate the rocket by
	  following the radio signal if necessary. You may need to get 
	  away from the clutter of the flight line, or even get up on 
	  a hill (or your neighbor's RV roof) to receive the RDF signal.
	</para>
        <para>
          The maximum height, speed and acceleration reported
          during the flight are displayed for your admiring observers.
	  The accuracy of these immediate values depends on the quality
	  of your radio link and how many packets were received.  
	  Recovering the on-board data after flight will likely yield
	  more precise results.
        </para>
	<para>
	  To get more detailed information about the flight, you can
	  click on the 'Graph Flight' button which will bring up a
	  graph window for the current flight.
	</para>
      </section>
      <section>
        <title>Site Map</title>
        <para>
          When the TeleMetrum has a GPS fix, the Site Map tab will map
          the rocket's position to make it easier for you to locate the
          rocket, both while it is in the air, and when it has landed. The
          rocket's state is indicated by color: white for pad, red for
          boost, pink for fast, yellow for coast, light blue for drogue,
          dark blue for main, and black for landed.
        </para>
        <para>
          The map's scale is approximately 3m (10ft) per pixel. The map
          can be dragged using the left mouse button. The map will attempt
          to keep the rocket roughly centered while data is being received.
        </para>
        <para>
          Images are fetched automatically via the Google Maps Static API,
          and cached on disk for reuse. If map images cannot be downloaded,
          the rocket's path will be traced on a dark gray background
          instead.
        </para>
	<para>
	  You can pre-load images for your favorite launch sites
	  before you leave home; check out the 'Preload Maps' section below.
	</para>
      </section>
    </section>
    <section>
      <title>Save Flight Data</title>
      <para>
        The altimeter records flight data to its internal flash memory.
        TeleMetrum data is recorded at a much higher rate than the telemetry
        system can handle, and is not subject to radio drop-outs. As
        such, it provides a more complete and precise record of the
        flight. The 'Save Flight Data' button allows you to read the
        flash memory and write it to disk. As TeleMini has only a barometer, it
	records data at the same rate as the telemetry signal, but there will be
	no data lost due to telemetry drop-outs.
      </para>
      <para>
        Clicking on the 'Save Flight Data' button brings up a list of
        connected TeleMetrum and TeleDongle devices. If you select a
        TeleMetrum device, the flight data will be downloaded from that
        device directly. If you select a TeleDongle device, flight data
        will be downloaded from an altimeter over radio link via the 
	specified TeleDongle. See the chapter on Controlling An Altimeter 
	Over The Radio Link for more information.
      </para>
      <para>
	After the device has been selected, a dialog showing the
	flight data saved in the device will be shown allowing you to
	select which flights to download and which to delete. With
	version 0.9 or newer firmware, you must erase flights in order
	for the space they consume to be reused by another
	flight. This prevents accidentally losing flight data
	if you neglect to download data before flying again. Note that
	if there is no more space available in the device, then no
	data will be recorded during the next flight.
      </para>
      <para>
        The file name for each flight log is computed automatically
        from the recorded flight date, altimeter serial number and
        flight number information.
      </para>
    </section>
    <section>
      <title>Replay Flight</title>
      <para>
        Select this button and you are prompted to select a flight
        record file, either a .telem file recording telemetry data or a
        .eeprom file containing flight data saved from the altimeter
        flash memory.
      </para>
      <para>
        Once a flight record is selected, the flight monitor interface
        is displayed and the flight is re-enacted in real time. Check
        the Monitor Flight chapter above to learn how this window operates.
      </para>
    </section>
    <section>
      <title>Graph Data</title>
      <para>
        Select this button and you are prompted to select a flight
        record file, either a .telem file recording telemetry data or a
        .eeprom file containing flight data saved from
        flash memory.
      </para>
      <para>
        Once a flight record is selected, a window with two tabs is
        opened. The first tab contains a graph with acceleration
        (blue), velocity (green) and altitude (red) of the flight,
        measured in metric units. The
        apogee(yellow) and main(magenta) igniter voltages are also
        displayed; high voltages indicate continuity, low voltages
        indicate open circuits. The second tab contains some basic
	flight statistics.
      </para>
      <para>
        The graph can be zoomed into a particular area by clicking and
        dragging down and to the right. Once zoomed, the graph can be
        reset by clicking and dragging up and to the left. Holding down
        control and clicking and dragging allows the graph to be panned.
        The right mouse button causes a pop-up menu to be displayed, giving
        you the option save or print the plot.
      </para>
      <para>
        Note that telemetry files will generally produce poor graphs
        due to the lower sampling rate and missed telemetry packets.
        Use saved flight data in .eeprom files for graphing where possible.
      </para>
    </section>
    <section>
      <title>Export Data</title>
      <para>
        This tool takes the raw data files and makes them available for
        external analysis. When you select this button, you are prompted to 
	select a flight
        data file (either .eeprom or .telem will do, remember that
        .eeprom files contain higher resolution and more continuous
        data). Next, a second dialog appears which is used to select
        where to write the resulting file. It has a selector to choose
        between CSV and KML file formats.
      </para>
      <section>
        <title>Comma Separated Value Format</title>
        <para>
          This is a text file containing the data in a form suitable for
          import into a spreadsheet or other external data analysis
          tool. The first few lines of the file contain the version and
          configuration information from the altimeter, then
          there is a single header line which labels all of the
          fields. All of these lines start with a '#' character which
          many tools can be configured to skip over.
        </para>
        <para>
          The remaining lines of the file contain the data, with each
          field separated by a comma and at least one space. All of
          the sensor values are converted to standard units, with the
          barometric data reported in both pressure, altitude and
          height above pad units.
        </para>
      </section>
      <section>
        <title>Keyhole Markup Language (for Google Earth)</title>
        <para>
          This is the format used by Google Earth to provide an overlay 
	  within that application. With this, you can use Google Earth to 
	  see the whole flight path in 3D.
        </para>
      </section>
    </section>
    <section>
      <title>Configure Altimeter</title>
      <para>
        Select this button and then select either a TeleMetrum or
        TeleDongle Device from the list provided. Selecting a TeleDongle
        device will use the radio link to configure a remote altimeter. 
      </para>
      <para>
        The first few lines of the dialog provide information about the
        connected device, including the product name,
        software version and hardware serial number. Below that are the
        individual configuration entries.
      </para>
      <para>
        At the bottom of the dialog, there are four buttons:
      </para>
      <itemizedlist>
        <listitem>
          <para>
            Save. This writes any changes to the
            configuration parameter block in flash memory. If you don't
            press this button, any changes you make will be lost.
          </para>
        </listitem>
        <listitem>
          <para>
            Reset. This resets the dialog to the most recently saved values,
            erasing any changes you have made.
          </para>
        </listitem>
        <listitem>
          <para>
            Reboot. This reboots the device. Use this to
            switch from idle to pad mode by rebooting once the rocket is
            oriented for flight, or to confirm changes you think you saved 
	    are really saved.
          </para>
        </listitem>
        <listitem>
          <para>
            Close. This closes the dialog. Any unsaved changes will be
            lost.
          </para>
        </listitem>
      </itemizedlist>
      <para>
        The rest of the dialog contains the parameters to be configured.
      </para>
      <section>
        <title>Main Deploy Altitude</title>
        <para>
          This sets the altitude (above the recorded pad altitude) at
          which the 'main' igniter will fire. The drop-down menu shows
          some common values, but you can edit the text directly and
          choose whatever you like. If the apogee charge fires below
          this altitude, then the main charge will fire two seconds
          after the apogee charge fires.
        </para>
      </section>
      <section>
        <title>Apogee Delay</title>
        <para>
          When flying redundant electronics, it's often important to
          ensure that multiple apogee charges don't fire at precisely
          the same time, as that can over pressurize the apogee deployment
          bay and cause a structural failure of the air-frame. The Apogee
          Delay parameter tells the flight computer to fire the apogee
          charge a certain number of seconds after apogee has been
          detected.
        </para>
      </section>
      <section>
        <title>Radio Frequency</title>
        <para>
          This configures which of the configured frequencies to use for both
          telemetry and packet command mode. Note that if you set this
          value via packet command mode, you will have to reconfigure
          the TeleDongle frequency before you will be able to use packet
          command mode again.
        </para>
      </section>
      <section>
        <title>Radio Calibration</title>
        <para>
          The radios in every Altus Metrum device are calibrated at the
          factory to ensure that they transmit and receive on the
          specified frequency.  If you need to you can adjust the calibration 
	  by changing this value.  Do not do this without understanding what
	  the value means, read the appendix on calibration and/or the source
	  code for more information.  To change a TeleDongle's calibration, 
	  you must reprogram the unit completely.
        </para>
      </section>
      <section>
        <title>Callsign</title>
        <para>
          This sets the call sign included in each telemetry packet. Set this
          as needed to conform to your local radio regulations.
        </para>
      </section>
      <section>
        <title>Maximum Flight Log Size</title>
        <para>
          This sets the space (in kilobytes) allocated for each flight
          log. The available space will be divided into chunks of this
          size. A smaller value will allow more flights to be stored,
          a larger value will record data from longer flights.
	</para>
      </section>
      <section>
        <title>Ignite Mode</title>
	<para>
	  TeleMetrum and TeleMini provide two igniter channels as they
	  were originally designed as dual-deploy flight
	  computers. This configuration parameter allows the two
	  channels to be used in different configurations.
	</para>
	<itemizedlist>
	  <listitem>
	    <para>
	      Dual Deploy. This is the usual mode of operation; the
	      'apogee' channel is fired at apogee and the 'main'
	      channel at the height above ground specified by the
	      'Main Deploy Altitude' during descent.
	    </para>
	  </listitem>
	  <listitem>
	    <para>
	      Redundant Apogee. This fires both channels at
	      apogee, the 'apogee' channel first followed after a two second
	      delay by the 'main' channel.
	    </para>
	  </listitem>
	  <listitem>
	    <para>
	      Redundant Main. This fires both channels at the
	      height above ground specified by the Main Deploy
	      Altitude setting during descent. The 'apogee'
	      channel is fired first, followed after a two second
	      delay by the 'main' channel.
	    </para>
	  </listitem>
	</itemizedlist>
      </section>
      <section>
        <title>Pad Orientation</title>
	<para>
	  Because it includes an accelerometer, TeleMetrum is
	  sensitive to the orientation of the board. By default, it
	  expects the antenna end to point forward. This parameter
	  allows that default to be changed, permitting the board to
	  be mounted with the antenna pointing aft instead.
	</para>
	<itemizedlist>
	  <listitem>
	    <para>
	      Antenna Up. In this mode, the antenna end of the
	      TeleMetrum board must point forward, in line with the
	      expected flight path.
	    </para>
	  </listitem>
	  <listitem>
	    <para>
	      Antenna Down. In this mode, the antenna end of the
	      TeleMetrum board must point aft, in line with the
	      expected flight path.
	    </para>
	  </listitem>
	</itemizedlist>
      </section>
    </section>
    <section>
      <title>Configure AltosUI</title>
      <para>
        This button presents a dialog so that you can configure the AltosUI global settings.
      </para>
      <section>
        <title>Voice Settings</title>
        <para>
          AltosUI provides voice announcements during flight so that you
          can keep your eyes on the sky and still get information about
          the current flight status. However, sometimes you don't want
          to hear them.
        </para>
        <itemizedlist>
          <listitem>
            <para>Enable—turns all voice announcements on and off</para>
          </listitem>
          <listitem>
            <para>
              Test Voice—Plays a short message allowing you to verify
              that the audio system is working and the volume settings
              are reasonable
            </para>
          </listitem>
        </itemizedlist>
      </section>
      <section>
        <title>Log Directory</title>
        <para>
          AltosUI logs all telemetry data and saves all TeleMetrum flash
          data to this directory. This directory is also used as the
          staring point when selecting data files for display or export.
        </para>
        <para>
          Click on the directory name to bring up a directory choosing
          dialog, select a new directory and click 'Select Directory' to
          change where AltosUI reads and writes data files.
        </para>
      </section>
      <section>
        <title>Callsign</title>
        <para>
          This value is transmitted in each command packet sent from 
	  TeleDongle and received from an altimeter.  It is not used in 
	  telemetry mode, as the callsign configured in the altimeter board
	  is included in all telemetry packets.  Configure this
          with the AltosUI operators call sign as needed to comply with
          your local radio regulations.
        </para>
      </section>
      <section>
	<title>Imperial Units</title>
	<para>
	  This switches between metric units (meters) and imperial
	  units (feet and miles). This affects the display of values
	  use during flight monitoring, data graphing and all of the
	  voice announcements. It does not change the units used when
	  exporting to CSV files, those are always produced in metric units.
	</para>
      </section>
      <section>
	<title>Font Size</title>
	<para>
	  Selects the set of fonts used in the flight monitor
	  window. Choose between the small, medium and large sets.
	</para>
      </section>
      <section>
        <title>Serial Debug</title>
        <para>
          This causes all communication with a connected device to be
          dumped to the console from which AltosUI was started. If
          you've started it from an icon or menu entry, the output
          will simply be discarded. This mode can be useful to debug
          various serial communication issues.
        </para>
      </section>
      <section>
	<title>Manage Frequencies</title>
	<para>
	  This brings up a dialog where you can configure the set of
	  frequencies shown in the various frequency menus. You can
	  add as many as you like, or even reconfigure the default
	  set. Changing this list does not affect the frequency
	  settings of any devices, it only changes the set of
	  frequencies shown in the menus.
	</para>
      </section>
    </section>
    <section>
      <title>Configure Groundstation</title>
      <para>
        Select this button and then select a TeleDongle Device from the list provided.
      </para>
      <para>
        The first few lines of the dialog provide information about the
        connected device, including the product name,
        software version and hardware serial number. Below that are the
        individual configuration entries.
      </para>
      <para>
	Note that the TeleDongle itself doesn't save any configuration
	data, the settings here are recorded on the local machine in
	the Java preferences database. Moving the TeleDongle to
	another machine, or using a different user account on the same
	machine will cause settings made here to have no effect.
      </para>
      <para>
        At the bottom of the dialog, there are three buttons:
      </para>
      <itemizedlist>
        <listitem>
          <para>
            Save. This writes any changes to the
            local Java preferences file. If you don't
            press this button, any changes you make will be lost.
          </para>
        </listitem>
        <listitem>
          <para>
            Reset. This resets the dialog to the most recently saved values,
            erasing any changes you have made.
          </para>
        </listitem>
        <listitem>
          <para>
            Close. This closes the dialog. Any unsaved changes will be
            lost.
          </para>
        </listitem>
      </itemizedlist>
      <para>
        The rest of the dialog contains the parameters to be configured.
      </para>
      <section>
        <title>Frequency</title>
        <para>
          This configures the frequency to use for both telemetry and
          packet command mode. Set this before starting any operation
          involving packet command mode so that it will use the right
          frequency. Telemetry monitoring mode also provides a menu to
          change the frequency, and that menu also sets the same Java
          preference value used here.
        </para>
      </section>
      <section>
        <title>Radio Calibration</title>
        <para>
          The radios in every Altus Metrum device are calibrated at the
          factory to ensure that they transmit and receive on the
          specified frequency.  To change a TeleDongle's calibration, 
	  you must reprogram the unit completely, so this entry simply
	  shows the current value and doesn't allow any changes.
        </para>
      </section>
    </section>
    <section>
      <title>Flash Image</title>
      <para>
        This reprograms any Altus Metrum device by using a TeleMetrum
        or TeleDongle as a programming dongle. Please read the
        directions for flashing devices in the Updating Device
        Firmware chapter below.
      </para>
      <para>
        Once you have the programmer and target devices connected,
        push the 'Flash Image' button. That will present a dialog box
        listing all of the connected devices. Carefully select the
        programmer device, not the device to be programmed.
      </para>
      <para>
        Next, select the image to flash to the device. These are named
        with the product name and firmware version. The file selector
        will start in the directory containing the firmware included
        with the AltosUI package. Navigate to the directory containing
        the desired firmware if it isn't there.
      </para>
      <para>
        Next, a small dialog containing the device serial number and
        RF calibration values should appear. If these values are
        incorrect (possibly due to a corrupted image in the device),
        enter the correct values here.
      </para>
      <para>
        Finally, a dialog containing a progress bar will follow the
        programming process.
      </para>
      <para>
        When programming is complete, the target device will
        reboot. Note that if the target device is connected via USB, you
        will have to unplug it and then plug it back in for the USB
        connection to reset so that you can communicate with the device
        again.
      </para>
    </section>
    <section>
      <title>Fire Igniter</title>
      <para>
	This activates the igniter circuits in TeleMetrum to help test
	recovery systems deployment. Because this command can operate
	over the Packet Command Link, you can prepare the rocket as
	for flight and then test the recovery system without needing
	to snake wires inside the air-frame.
      </para>
      <para>
	Selecting the 'Fire Igniter' button brings up the usual device
	selection dialog. Pick the desired TeleDongle or TeleMetrum
	device. This brings up another window which shows the current
	continuity test status for both apogee and main charges.
      </para>
      <para>
	Next, select the desired igniter to fire. This will enable the
	'Arm' button.
      </para>
      <para>
	Select the 'Arm' button. This enables the 'Fire' button. The
	word 'Arm' is replaced by a countdown timer indicating that
	you have 10 seconds to press the 'Fire' button or the system
	will deactivate, at which point you start over again at
	selecting the desired igniter.
      </para>
    </section>
    <section>
      <title>Scan Channels</title>
      <para>
	This listens for telemetry packets on all of the configured
	frequencies, displaying information about each device it
	receives a packet from. You can select which of the three
	telemetry formats should be tried; by default, it only listens
	for the standard telemetry packets used in v1.0 and later
	firmware.
      </para>
    </section>
    <section>
      <title>Load Maps</title>
      <para>
	Before heading out to a new launch site, you can use this to
	load satellite images in case you don't have internet
	connectivity at the site. This loads a fairly large area
	around the launch site, which should cover any flight you're likely to make.
      </para>
      <para>
	There's a drop-down menu of launch sites we know about; if
	your favorites aren't there, please let us know the lat/lon
	and name of the site. The contents of this list are actually
	downloaded at run-time, so as new sites are sent in, they'll
	get automatically added to this list.
      </para>
      <para>
	If the launch site isn't in the list, you can manually enter the lat/lon values
      </para>
      <para>
	Clicking the 'Load Map' button will fetch images from Google
	Maps; note that Google limits how many images you can fetch at
	once, so if you load more than one launch site, you may get
	some gray areas in the map which indicate that Google is tired
	of sending data to you. Try again later.
      </para>
    </section>
    <section>
      <title>Monitor Idle</title>
      <para>
	This brings up a dialog similar to the Monitor Flight UI,
	except it works with the altimeter in "idle" mode by sending
	query commands to discover the current state rather than
	listening for telemetry packets.
      </para>
    </section>
  </chapter>
  <chapter>
    <title>Using Altus Metrum Products</title>
    <section>
      <title>Being Legal</title>
      <para>
        First off, in the US, you need an <ulink url="http://www.altusmetrum.org/Radio/">amateur radio license</ulink> or
        other authorization to legally operate the radio transmitters that are part
        of our products.
      </para>
      </section>
      <section>
        <title>In the Rocket</title>
        <para>
          In the rocket itself, you just need a <ulink url="http://www.altusmetrum.org/TeleMetrum/">TeleMetrum</ulink> or
	  <ulink url="http://www.altusmetrum.org/TeleMini/">TeleMini</ulink> board and
          a single-cell, 3.7 volt nominal Li-Po rechargeable battery.  An 
	  850mAh battery weighs less than a 9V alkaline battery, and will 
	  run a TeleMetrum for hours.
	  A 110mAh battery weighs less than a triple A battery and will run a TeleMetrum for
	  a few hours, or a TeleMini for much (much) longer.
        </para>
        <para>
          By default, we ship the altimeters with a simple wire antenna.  If your
          electronics bay or the air-frame it resides within is made of carbon fiber,
          which is opaque to RF signals, you may choose to have an SMA connector
          installed so that you can run a coaxial cable to an antenna mounted
          elsewhere in the rocket.
        </para>
      </section>
      <section>
        <title>On the Ground</title>
        <para>
          To receive the data stream from the rocket, you need an antenna and short
          feed-line connected to one of our <ulink url="http://www.altusmetrum.org/TeleDongle/">TeleDongle</ulink> units.  The
          TeleDongle in turn plugs directly into the USB port on a notebook
          computer.  Because TeleDongle looks like a simple serial port, your computer
          does not require special device drivers... just plug it in.
        </para>
        <para>
	  The GUI tool, AltosUI, is written in Java and runs across
	  Linux, Mac OS and Windows. There's also a suite of C tools
	  for Linux which can perform most of the same tasks.
        </para>
        <para>
          After the flight, you can use the radio link to extract the more detailed data
          logged in either TeleMetrum or TeleMini devices, or you can use a mini USB cable to plug into the
          TeleMetrum board directly.  Pulling out the data without having to open up
          the rocket is pretty cool!  A USB cable is also how you charge the Li-Po
          battery, so you'll want one of those anyway... the same cable used by lots
          of digital cameras and other modern electronic stuff will work fine.
        </para>
        <para>
          If your TeleMetrum-equipped rocket lands out of sight, you may enjoy having a hand-held GPS
          receiver, so that you can put in a way-point for the last reported rocket
          position before touch-down.  This makes looking for your rocket a lot like
          Geo-Caching... just go to the way-point and look around starting from there.
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
              an antenna and feed-line
            </listitem>
            <listitem>
              a TeleDongle
            </listitem>
            <listitem>
              a notebook computer
            </listitem>
            <listitem>
              optionally, a hand-held GPS receiver
            </listitem>
            <listitem>
              optionally, an HT or receiver covering 435 MHz
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
          TeleMetrum- or TeleMini- equipped rocket when used with a suitable 70cm HT.
        </para>
      </section>
      <section>
        <title>Data Analysis</title>
        <para>
          Our software makes it easy to log the data from each flight, both the
          telemetry received during the flight itself, and the more
          complete data log recorded in the flash memory on the altimeter
          board.  Once this data is on your computer, our post-flight tools make it
          easy to quickly get to the numbers everyone wants, like apogee altitude,
          max acceleration, and max velocity.  You can also generate and view a
          standard set of plots showing the altitude, acceleration, and
          velocity of the rocket during flight.  And you can even export a TeleMetrum data file
          usable with Google Maps and Google Earth for visualizing the flight path
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
          and so forth.  
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
  </chapter>
  <chapter>
    <title>Altimeter Installation Recommendations</title>
    <para>
      Building high-power rockets that fly safely is hard enough. Mix
      in some sophisticated electronics and a bunch of radio energy
      and oftentimes you find few perfect solutions. This chapter
      contains some suggestions about how to install Altus Metrum
      products into the rocket air-frame, including how to safely and
      reliably mix a variety of electronics into the same air-frame.
    </para>
    <section>
      <title>Mounting the Altimeter</title>
      <para>
	The first consideration is to ensure that the altimeter is
	securely fastened to the air-frame. For TeleMetrum, we use
	nylon standoffs and nylon screws; they're good to at least 50G
	and cannot cause any electrical issues on the board. For
	TeleMini, we usually cut small pieces of 1/16" balsa to fit
	under the screw holes, and then take 2x56 nylon screws and
	screw them through the TeleMini mounting holes, through the
	balsa and into the underlying material.
      </para>
      <orderedlist inheritnum='inherit' numeration='arabic'>
	<listitem>
	  Make sure TeleMetrum is aligned precisely along the axis of
	  acceleration so that the accelerometer can accurately
	  capture data during the flight.
	</listitem>
	<listitem>
	  Watch for any metal touching components on the
	  board. Shorting out connections on the bottom of the board
	  can cause the altimeter to fail during flight.
	</listitem>
      </orderedlist>
    </section>
    <section>
      <title>Dealing with the Antenna</title>
      <para>
	The antenna supplied is just a piece of solid, insulated,
	wire. If it gets damaged or broken, it can be easily
	replaced. It should be kept straight and not cut; bending or
	cutting it will change the resonant frequency and/or
	impedance, making it a less efficient radiator and thus
	reducing the range of the telemetry signal.
      </para>
      <para>
	Keeping metal away from the antenna will provide better range
	and a more even radiation pattern. In most rockets, it's not
	entirely possible to isolate the antenna from metal
	components; there are often bolts, all-thread and wires from other
	electronics to contend with. Just be aware that the more stuff
	like this around the antenna, the lower the range.
      </para>
      <para>
	Make sure the antenna is not inside a tube made or covered
	with conducting material. Carbon fiber is the most common
	culprit here -- CF is a good conductor and will effectively
	shield the antenna, dramatically reducing signal strength and
	range. Metallic flake paint is another effective shielding
	material which is to be avoided around any antennas.
      </para>
      <para>
	If the ebay is large enough, it can be convenient to simply
	mount the altimeter at one end and stretch the antenna out
	inside. Taping the antenna to the sled can keep it straight
	under acceleration. If there are metal rods, keep the
	antenna as far away as possible.
      </para>
      <para>
	For a shorter ebay, it's quite practical to have the antenna
	run through a bulkhead and into an adjacent bay. Drill a small
	hole in the bulkhead, pass the antenna wire through it and
	then seal it up with glue or clay. We've also used acrylic
	tubing to create a cavity for the antenna wire. This works a
	bit better in that the antenna is known to stay straight and
	not get folded by recovery components in the bay. Angle the
	tubing towards the side wall of the rocket and it ends up
	consuming very little space.
      </para>
      <para>
	If you need to place the antenna at a distance from the
	altimeter, you can replace the antenna with an edge-mounted
	SMA connector, and then run 50Ω coax from the board to the
	antenna. Building a remote antenna is beyond the scope of this
	manual.
      </para>
    </section>
    <section>
      <title>Preserving GPS Reception</title>
      <para>
	The GPS antenna and receiver in TeleMetrum are highly
	sensitive and normally have no trouble tracking enough
	satellites to provide accurate position information for
	recovering the rocket. However, there are many ways to
	attenuate the GPS signal.
      <orderedlist inheritnum='inherit' numeration='arabic'>
	<listitem>
	  Conductive tubing or coatings. Carbon fiber and metal
	  tubing, or metallic paint will all dramatically attenuate the
	  GPS signal. We've never heard of anyone successfully
	  receiving GPS from inside these materials.
	</listitem>
	<listitem>
	  Metal components near the GPS patch antenna. These will
	  de-tune the patch antenna, changing the resonant frequency
	  away from the L1 carrier and reduce the effectiveness of the
	  antenna. You can place as much stuff as you like beneath the
	  antenna as that's covered with a ground plane. But, keep
	  wires and metal out from above the patch antenna.
	</listitem>
      </orderedlist>
      </para>
    </section>
    <section>
      <title>Radio Frequency Interference</title>
      <para>
	Any altimeter will generate RFI; the digital circuits use
	high-frequency clocks that spray radio interference across a
	wide band. Altus Metrum altimeters generate intentional radio
	signals as well, increasing the amount of RF energy around the board.
      </para>
      <para>
	Rocketry altimeters also use precise sensors measuring air
	pressure and acceleration. Tiny changes in voltage can cause
	these sensor readings to vary by a huge amount. When the
	sensors start mis-reporting data, the altimeter can either
	fire the igniters at the wrong time, or not fire them at all.
      </para>
      <para>
	Voltages are induced when radio frequency energy is
	transmitted from one circuit to another. Here are things that
	influence the induced voltage and current:
      </para>
      <itemizedlist>
	<listitem>
	  Keep wires from different circuits apart. Moving circuits
	  further apart will reduce RFI.
	</listitem>
	<listitem>
	  Avoid parallel wires from different circuits. The longer two
	  wires run parallel to one another, the larger the amount of
	  transferred energy. Cross wires at right angles to reduce
	  RFI.
	</listitem>
	<listitem>
	  Twist wires from the same circuits. Two wires the same
	  distance from the transmitter will get the same amount of
	  induced energy which will then cancel out. Any time you have
	  a wire pair running together, twist the pair together to
	  even out distances and reduce RFI. For altimeters, this
	  includes battery leads, switch hookups and igniter
	  circuits.
	</listitem>
	<listitem>
	  Avoid resonant lengths. Know what frequencies are present
	  in the environment and avoid having wire lengths near a
	  natural resonant length. Altusmetrum products transmit on the
	  70cm amateur band, so you should avoid lengths that are a
	  simple ratio of that length; essentially any multiple of 1/4
	  of the wavelength (17.5cm).
	</listitem>
      </itemizedlist>
    </section>
    <section>
      <title>The Barometric Sensor</title>
      <para>
	Altusmetrum altimeters measure altitude with a barometric
	sensor, essentially measuring the amount of air above the
	rocket to figure out how high it is. A large number of
	measurements are taken as the altimeter initializes itself to
	figure out the pad altitude. Subsequent measurements are then
	used to compute the height above the pad.
      </para>
      <para>
	To accurately measure atmospheric pressure, the ebay
	containing the altimeter must be vented outside the
	air-frame. The vent must be placed in a region of linear
	airflow, have smooth edges, and away from areas of increasing or 
	decreasing pressure.
      </para>
      <para>
	The barometric sensor in the altimeter is quite sensitive to
	chemical damage from the products of APCP or BP combustion, so
	make sure the ebay is carefully sealed from any compartment
	which contains ejection charges or motors.
      </para>
    </section>
    <section>
      <title>Ground Testing</title>
      <para>
	The most important aspect of any installation is careful
	ground testing. Bringing an air-frame up to the LCO table which
	hasn't been ground tested can lead to delays or ejection
	charges firing on the pad, or, even worse, a recovery system
	failure.
      </para>
      <para>
	Do a 'full systems' test that includes wiring up all igniters
	without any BP and turning on all of the electronics in flight
	mode. This will catch any mistakes in wiring and any residual
	RFI issues that might accidentally fire igniters at the wrong
	time. Let the air-frame sit for several minutes, checking for
	adequate telemetry signal strength and GPS lock.  If any igniters
	fire unexpectedly, find and resolve the issue before loading any
	BP charges!
      </para>
      <para>
	Ground test the ejection charges. Prepare the rocket for
	flight, loading ejection charges and igniters. Completely
	assemble the air-frame and then use the 'Fire Igniters'
	interface through a TeleDongle to command each charge to
	fire. Make sure the charge is sufficient to robustly separate
	the air-frame and deploy the recovery system.
      </para>
    </section>
  </chapter>
  <chapter>
    <title>Updating Device Firmware</title>
    <para>
      The big concept to understand is that you have to use a
      TeleDongle as a programmer to update a TeleMetrum or TeleMini,
      and a TeleMetrum or other TeleDongle to program the TeleDongle
      Due to limited memory resources in the cc1111, we don't support
      programming directly over USB. 
    </para>
    <para>
      You may wish to begin by ensuring you have current firmware images.
      These are distributed as part of the AltOS software bundle that
      also includes the AltosUI ground station program.  Newer ground
      station versions typically work fine with older firmware versions,
      so you don't need to update your devices just to try out new
      software features.  You can always download the most recent
      version from <ulink url="http://www.altusmetrum.org/AltOS/"/>.
    </para>
    <para>
      We recommend updating the altimeter first, before updating TeleDongle.
    </para>
    <section>
      <title>Updating TeleMetrum Firmware</title>
      <orderedlist inheritnum='inherit' numeration='arabic'>
        <listitem>
          Find the 'programming cable' that you got as part of the starter
          kit, that has a red 8-pin MicroMaTch connector on one end and a
          red 4-pin MicroMaTch connector on the other end.
        </listitem>
        <listitem>
          Take the 2 screws out of the TeleDongle case to get access
          to the circuit board.
        </listitem>
        <listitem>
          Plug the 8-pin end of the programming cable to the
          matching connector on the TeleDongle, and the 4-pin end to the
          matching connector on the TeleMetrum.
	  Note that each MicroMaTch connector has an alignment pin that
	  goes through a hole in the PC board when you have the cable
	  oriented correctly.
        </listitem>
        <listitem>
          Attach a battery to the TeleMetrum board.
        </listitem>
        <listitem>
          Plug the TeleDongle into your computer's USB port, and power
          up the TeleMetrum.
        </listitem>
        <listitem>
          Run AltosUI, and select 'Flash Image' from the File menu.
        </listitem>
        <listitem>
          Pick the TeleDongle device from the list, identifying it as the
          programming device.
        </listitem>
        <listitem>
          Select the image you want put on the TeleMetrum, which should have a
          name in the form telemetrum-v1.2-1.0.0.ihx.  It should be visible
	in the default directory, if not you may have to poke around
	your system to find it.
        </listitem>
        <listitem>
          Make sure the configuration parameters are reasonable
          looking. If the serial number and/or RF configuration
          values aren't right, you'll need to change them.
        </listitem>
        <listitem>
          Hit the 'OK' button and the software should proceed to flash
          the TeleMetrum with new firmware, showing a progress bar.
        </listitem>
        <listitem>
          Confirm that the TeleMetrum board seems to have updated OK, which you
          can do by plugging in to it over USB and using a terminal program
          to connect to the board and issue the 'v' command to check
          the version, etc.
        </listitem>
        <listitem>
          If something goes wrong, give it another try.
        </listitem>
      </orderedlist>
    </section>
    <section>
      <title>Updating TeleMini Firmware</title>
      <orderedlist inheritnum='inherit' numeration='arabic'>
        <listitem>
	  You'll need a special 'programming cable' to reprogram the
	  TeleMini. It's available on the Altus Metrum web store, or
	  you can make your own using an 8-pin MicroMaTch connector on
	  one end and a set of four pins on the other.
        </listitem>
        <listitem>
          Take the 2 screws out of the TeleDongle case to get access
          to the circuit board.
        </listitem>
        <listitem>
          Plug the 8-pin end of the programming cable to the matching
          connector on the TeleDongle, and the 4-pins into the holes
          in the TeleMini circuit board.  Note that the MicroMaTch
          connector has an alignment pin that goes through a hole in
          the PC board when you have the cable oriented correctly, and
          that pin 1 on the TeleMini board is marked with a square pad
          while the other pins have round pads.
        </listitem>
        <listitem>
          Attach a battery to the TeleMini board.
        </listitem>
        <listitem>
          Plug the TeleDongle into your computer's USB port, and power
          up the TeleMini
        </listitem>
        <listitem>
          Run AltosUI, and select 'Flash Image' from the File menu.
        </listitem>
        <listitem>
          Pick the TeleDongle device from the list, identifying it as the
          programming device.
        </listitem>
        <listitem>
          Select the image you want put on the TeleMini, which should have a
          name in the form telemini-v1.0-1.0.0.ihx.  It should be visible
	in the default directory, if not you may have to poke around
	your system to find it.
        </listitem>
        <listitem>
          Make sure the configuration parameters are reasonable
          looking. If the serial number and/or RF configuration
          values aren't right, you'll need to change them.
        </listitem>
        <listitem>
          Hit the 'OK' button and the software should proceed to flash
          the TeleMini with new firmware, showing a progress bar.
        </listitem>
        <listitem>
          Confirm that the TeleMini board seems to have updated OK, which you
          can do by configuring it over the radio link through the TeleDongle, or
	  letting it come up in "flight" mode and listening for telemetry.
        </listitem>
        <listitem>
          If something goes wrong, give it another try.
        </listitem>
      </orderedlist>
    </section>
    <section>
      <title>Updating TeleDongle Firmware</title>
      <para>
        Updating TeleDongle's firmware is just like updating TeleMetrum or TeleMini
	firmware, but you use either a TeleMetrum or another TeleDongle as the programmer.
	</para>
      <orderedlist inheritnum='inherit' numeration='arabic'>
        <listitem>
          Find the 'programming cable' that you got as part of the starter
          kit, that has a red 8-pin MicroMaTch connector on one end and a
          red 4-pin MicroMaTch connector on the other end.
        </listitem>
        <listitem>
	  Find the USB cable that you got as part of the starter kit, and
	  plug the "mini" end in to the mating connector on TeleMetrum or TeleDongle.
        </listitem>
        <listitem>
          Take the 2 screws out of the TeleDongle case to get access
          to the circuit board.
        </listitem>
        <listitem>
          Plug the 8-pin end of the programming cable to the
          matching connector on the programmer, and the 4-pin end to the
          matching connector on the TeleDongle.
	  Note that each MicroMaTch connector has an alignment pin that
	  goes through a hole in the PC board when you have the cable
	  oriented correctly.
        </listitem>
        <listitem>
          Attach a battery to the TeleMetrum board if you're using one.
        </listitem>
        <listitem>
          Plug both the programmer and the TeleDongle into your computer's USB
	  ports, and power up the programmer.
        </listitem>
        <listitem>
          Run AltosUI, and select 'Flash Image' from the File menu.
        </listitem>
        <listitem>
          Pick the programmer device from the list, identifying it as the
          programming device.
        </listitem>
        <listitem>
          Select the image you want put on the TeleDongle, which should have a
          name in the form teledongle-v0.2-1.0.0.ihx.  It should be visible
	in the default directory, if not you may have to poke around
	your system to find it.
        </listitem>
        <listitem>
          Make sure the configuration parameters are reasonable
          looking. If the serial number and/or RF configuration
          values aren't right, you'll need to change them.  The TeleDongle
	  serial number is on the "bottom" of the circuit board, and can
	  usually be read through the translucent blue plastic case without
	  needing to remove the board from the case.
        </listitem>
        <listitem>
          Hit the 'OK' button and the software should proceed to flash
          the TeleDongle with new firmware, showing a progress bar.
        </listitem>
        <listitem>
          Confirm that the TeleDongle board seems to have updated OK, which you
          can do by plugging in to it over USB and using a terminal program
          to connect to the board and issue the 'v' command to check
          the version, etc.  Once you're happy, remove the programming cable
	  and put the cover back on the TeleDongle.
        </listitem>
        <listitem>
          If something goes wrong, give it another try.
        </listitem>
      </orderedlist>
      <para>
        Be careful removing the programming cable from the locking 8-pin
        connector on TeleMetrum.  You'll need a fingernail or perhaps a thin
        screwdriver or knife blade to gently pry the locking ears out
        slightly to extract the connector.  We used a locking connector on
        TeleMetrum to help ensure that the cabling to companion boards
        used in a rocket don't ever come loose accidentally in flight.
      </para>
    </section>
  </chapter>
  <chapter>
    <title>Hardware Specifications</title>
    <section>
      <title>TeleMetrum Specifications</title>
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
	    70cm ham-band transceiver for telemetry down-link.
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
	    On-board, integrated GPS receiver with 5Hz update rate capability.
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
	    Fully integrated support for Li-Po rechargeable batteries.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    Uses Li-Po to fire e-matches, can be modified to support 
	    optional separate pyro battery if needed.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    2.75 x 1 inch board designed to fit inside 29mm air-frame coupler tube.
	  </para>
	</listitem>
      </itemizedlist>
    </section>
    <section>
      <title>TeleMini Specifications</title>
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
	    70cm ham-band transceiver for telemetry down-link.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    Barometric pressure sensor good to 45k feet MSL.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    On-board 5 kilobyte non-volatile memory for flight data storage.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    RF interface for configuration, and data recovery.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    Support for Li-Po rechargeable batteries, using an external charger.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    Uses Li-Po to fire e-matches, can be modified to support 
	    optional separate pyro battery if needed.
	  </para>
	</listitem>
	<listitem>
	  <para>
	    1.5 x .5 inch board designed to fit inside 18mm air-frame coupler tube.
	  </para>
	</listitem>
      </itemizedlist>
    </section>
  </chapter>
  <chapter>
    <title>FAQ</title>
      <para>
        TeleMetrum seems to shut off when disconnected from the
        computer.  Make sure the battery is adequately charged.  Remember the
        unit will pull more power than the USB port can deliver before the
        GPS enters "locked" mode.  The battery charges best when TeleMetrum
        is turned off.
      </para>
      <para>
        It's impossible to stop the TeleDongle when it's in "p" mode, I have
        to unplug the USB cable?  Make sure you have tried to "escape out" of
        this mode.  If this doesn't work the reboot procedure for the
        TeleDongle *is* to simply unplug it. 'cu' however will retain it's
        outgoing buffer IF your "escape out" ('~~') does not work.
        At this point using either 'ao-view' (or possibly
        'cutemon') instead of 'cu' will 'clear' the issue and allow renewed
        communication.
      </para>
      <para>
        The amber LED (on the TeleMetrum) lights up when both
        battery and USB are connected. Does this mean it's charging?
        Yes, the yellow LED indicates the charging at the 'regular' rate.
        If the led is out but the unit is still plugged into a USB port,
        then the battery is being charged at a 'trickle' rate.
      </para>
      <para>
        There are no "dit-dah-dah-dit" sound or lights like the manual mentions?
        That's the "pad" mode.  Weak batteries might be the problem.
        It is also possible that the TeleMetrum is horizontal and the output
        is instead a "dit-dit" meaning 'idle'. For TeleMini, it's possible that
	it received a command packet which would have left it in "pad" mode.
      </para>
      <para>
        How do I save flight data?
        Live telemetry is written to file(s) whenever AltosUI is connected
        to the TeleDongle.  The file area defaults to ~/TeleMetrum
        but is easily changed using the menus in AltosUI. The files that
        are written end in '.telem'. The after-flight
        data-dumped files will end in .eeprom and represent continuous data
        unlike the .telem files that are subject to losses
        along the RF data path.
        See the above instructions on what and how to save the eeprom stored
        data after physically retrieving your altimeter.  Make sure to save
        the on-board data after each flight; while the TeleMetrum can store
	multiple flights, you never know when you'll lose the altimeter...
      </para>
  </chapter>
  <appendix>
    <title>Notes for Older Software</title>
    <para>
      <emphasis>
      Before AltosUI was written, using Altus Metrum devices required
      some finesse with the Linux command line. There was a limited
      GUI tool, ao-view, which provided functionality similar to the
      Monitor Flight window in AltosUI, but everything else was a
      fairly 80's experience. This appendix includes documentation for
      using that software.
      </emphasis>
    </para>
    <para>
      Both TeleMetrum and TeleDongle can be directly communicated
      with using USB ports. The first thing you should try after getting
      both units plugged into to your computer's USB port(s) is to run
      'ao-list' from a terminal-window to see what port-device-name each
      device has been assigned by the operating system.
      You will need this information to access the devices via their
      respective on-board firmware and data using other command line
      programs in the AltOS software suite.
    </para>
    <para>
      TeleMini can be communicated with through a TeleDongle device
      over the radio link. When first booted, TeleMini listens for a
      TeleDongle device and if it receives a packet, it goes into
      'idle' mode. Otherwise, it goes into 'pad' mode and waits to be
      launched. The easiest way to get it talking is to start the
      communication link on the TeleDongle and the power up the
      TeleMini board.
    </para>
    <para>
      To access the device's firmware for configuration you need a terminal
      program such as you would use to talk to a modem.  The software
      authors prefer using the program 'cu' which comes from the UUCP package
      on most Unix-like systems such as Linux.  An example command line for
      cu might be 'cu -l /dev/ttyACM0', substituting the correct number
      indicated from running the
      ao-list program.  Another reasonable terminal program for Linux is
      'cutecom'.  The default 'escape'
      character used by CU (i.e. the character you use to
      issue commands to cu itself instead of sending the command as input
      to the connected device) is a '~'. You will need this for use in
      only two different ways during normal operations. First is to exit
      the program by sending a '~.' which is called a 'escape-disconnect'
      and allows you to close-out from 'cu'. The
      second use will be outlined later.
    </para>
    <para>
      All of the Altus Metrum devices share the concept of a two level
      command set in their firmware.
      The first layer has several single letter commands. Once
      you are using 'cu' (or 'cutecom') sending (typing) a '?'
      returns a full list of these
      commands. The second level are configuration sub-commands accessed
      using the 'c' command, for
      instance typing 'c?' will give you this second level of commands
      (all of which require the
      letter 'c' to access).  Please note that most configuration options
      are stored only in Flash memory; TeleDongle doesn't provide any storage
      for these options and so they'll all be lost when you unplug it.
    </para>
    <para>
      Try setting these configuration ('c' or second level menu) values.  A good
      place to start is by setting your call sign.  By default, the boards
      use 'N0CALL' which is cute, but not exactly legal!
      Spend a few minutes getting comfortable with the units, their
      firmware, and 'cu' (or possibly 'cutecom').
      For instance, try to send
      (type) a 'c r 2' and verify the channel change by sending a 'c s'.
      Verify you can connect and disconnect from the units while in your
      terminal program by sending the escape-disconnect mentioned above.
    </para>
        <para>
          To set the radio frequency, use the 'c R' command to specify the
	  radio transceiver configuration parameter. This parameter is computed
	  using the desired frequency, 'F', the radio calibration parameter, 'C' (showed by the 'c s' command) and
	  the standard calibration reference frequency, 'S', (normally 434.550MHz):
	  <programlisting>
	    R = F / S * C
	  </programlisting>
	  Round the result to the nearest integer value.
          As with all 'c' sub-commands, follow this with a 'c w' to write the
          change to the parameter block in the on-board flash on
          your altimeter board if you want the change to stay in place across reboots.
        </para>
        <para>
          To set the apogee delay, use the 'c d' command.
          As with all 'c' sub-commands, follow this with a 'c w' to write the
          change to the parameter block in the on-board DataFlash chip.
        </para>
        <para>
          To set the main deployment altitude, use the 'c m' command.
          As with all 'c' sub-commands, follow this with a 'c w' to write the
          change to the parameter block in the on-board DataFlash chip.
        </para>
        <para>
          To calibrate the radio frequency, connect the UHF antenna port to a
          frequency counter, set the board to 434.550MHz, and use the 'C'
          command to generate a CW carrier.  Wait for the transmitter temperature
          to stabilize and the frequency to settle down.
          Then, divide 434.550 MHz by the
          measured frequency and multiply by the current radio cal value show
          in the 'c s' command.  For an unprogrammed board, the default value
          is 1186611.  Take the resulting integer and program it using the 'c f'
          command.  Testing with the 'C' command again should show a carrier
          within a few tens of Hertz of the intended frequency.
          As with all 'c' sub-commands, follow this with a 'c w' to write the
          change to the parameter block in the on-board DataFlash chip.
        </para>
    <para>
      Note that the 'reboot' command, which is very useful on the altimeters,
      will likely just cause problems with the dongle.  The *correct* way
      to reset the dongle is just to unplug and re-plug it.
    </para>
    <para>
      A fun thing to do at the launch site and something you can do while
      learning how to use these units is to play with the radio link access
      between an altimeter and the TeleDongle.  Be aware that you *must* create
      some physical separation between the devices, otherwise the link will
      not function due to signal overload in the receivers in each device.
    </para>
    <para>
      Now might be a good time to take a break and read the rest of this
      manual, particularly about the two "modes" that the altimeters
      can be placed in. TeleMetrum uses the position of the device when booting
      up will determine whether the unit is in "pad" or "idle" mode. TeleMini
      enters "idle" mode when it receives a command packet within the first 5 seconds
      of being powered up, otherwise it enters "pad" mode.
    </para>
    <para>
      You can access an altimeter in idle mode from the TeleDongle's USB
      connection using the radio link
      by issuing a 'p' command to the TeleDongle. Practice connecting and
      disconnecting ('~~' while using 'cu') from the altimeter.  If
      you cannot escape out of the "p" command, (by using a '~~' when in
      CU) then it is likely that your kernel has issues.  Try a newer version.
    </para>
    <para>
      Using this radio link allows you to configure the altimeter, test
      fire e-matches and igniters from the flight line, check pyro-match
      continuity and so forth. You can leave the unit turned on while it
      is in 'idle mode' and then place the
      rocket vertically on the launch pad, walk away and then issue a
      reboot command.  The altimeter will reboot and start sending data
      having changed to the "pad" mode. If the TeleDongle is not receiving
      this data, you can disconnect 'cu' from the TeleDongle using the
      procedures mentioned above and THEN connect to the TeleDongle from
      inside 'ao-view'. If this doesn't work, disconnect from the
      TeleDongle, unplug it, and try again after plugging it back in.
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
    <para>
      On TeleMetrum, the GPS will eventually find enough satellites, lock in on them,
      and 'ao-view' will both auditorily announce and visually indicate
      that GPS is ready.
      Now you can launch knowing that you have a good data path and
      good satellite lock for flight data and recovery.  Remember
      you MUST tell ao-view to connect to the TeleDongle explicitly in
      order for ao-view to be able to receive data.
    </para>
    <para>
      The altimeters provide RDF (radio direction finding) tones on
      the pad, during descent and after landing. These can be used to
      locate the rocket using a directional antenna; the signal
      strength providing an indication of the direction from receiver to rocket.
    </para>
    <para>
      TeleMetrum also provides GPS tracking data, which can further simplify
      locating the rocket once it has landed. (The last good GPS data
      received before touch-down will be on the data screen of 'ao-view'.)
    </para>
    <para>
      Once you have recovered the rocket you can download the eeprom
      contents using either 'ao-dumplog' (or possibly 'ao-eeprom'), over
      either a USB cable or over the radio link using TeleDongle.
      And by following the man page for 'ao-postflight' you can create
      various data output reports, graphs, and even KML data to see the
      flight trajectory in Google-earth. (Moving the viewing angle making
      sure to connect the yellow lines while in Google-earth is the proper
      technique.)
    </para>
    <para>
      As for ao-view.... some things are in the menu but don't do anything
      very useful.  The developers have stopped working on ao-view to focus
      on a new, cross-platform ground station program.  So ao-view may or
      may not be updated in the future.  Mostly you just use
      the Log and Device menus.  It has a wonderful display of the incoming
      flight data and I am sure you will enjoy what it has to say to you
      once you enable the voice output!
    </para>
  </appendix>
  <appendix>
      <title>Calibration</title>
      <para>
        There are only two calibrations required for a TeleMetrum board, and
        only one for TeleDongle and TeleMini.  All boards are shipped from
        the factory pre-calibrated, but the procedures are documented here
	in case they are ever needed.  Re-calibration is not supported by
	AltosUI, you must connect to the board with a serial terminal program
	and interact directly with the on-board command interpreter to effect
	calibration.
      </para>
      <section>
        <title>Radio Frequency</title>
        <para>
          The radio frequency is synthesized from a clock based on the 48 MHz
          crystal on the board.  The actual frequency of this oscillator 
          must be measured to generate a calibration constant.  While our 
          GFSK modulation
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
          frequency counter, set the board to 434.550MHz, and use the 'C'
          command in the on-board command interpreter to generate a CW 
          carrier.  For TeleMetrum, this is best done over USB.  For TeleMini,
	  note that the only way to escape the 'C' command is via power cycle
	  since the board will no longer be listening for commands once it
	  starts generating a CW carrier.
	</para>
	<para>
	  Wait for the transmitter temperature to stabilize and the frequency 
          to settle down.  Then, divide 434.550 MHz by the
          measured frequency and multiply by the current radio cal value show
          in the 'c s' command.  For an unprogrammed board, the default value
          is 1186611.  Take the resulting integer and program it using the 'c f'
          command.  Testing with the 'C' command again should show a carrier
          within a few tens of Hertz of the intended frequency.
          As with all 'c' sub-commands, follow this with a 'c w' to write the
          change to the parameter block in the on-board DataFlash chip.
        </para>
	<para>
	  Note that any time you re-do the radio frequency calibration, the
	  radio frequency is reset to the default 434.550 Mhz.  If you want
	  to use another frequency, you will have to set that again after
	  calibration is completed.
	</para>
      </section>
      <section>
        <title>TeleMetrum Accelerometer</title>
        <para>
          The TeleMetrum accelerometer we use has its own 5 volt power 
	  supply and
          the output must be passed through a resistive voltage divider to match
          the input of our 3.3 volt ADC.  This means that unlike the barometric
          sensor, the output of the acceleration sensor is not ratio-metric to
          the ADC converter, and calibration is required.  Explicitly 
	  calibrating the accelerometers also allows us to load any device
	  from a Freescale family that includes at least +/- 40g, 50g, 100g, 
	  and 200g parts.  Using gravity,
          a simple 2-point calibration yields acceptable results capturing both
          the different sensitivities and ranges of the different accelerometer
          parts and any variation in power supply voltages or resistor values
          in the divider network.
        </para>
        <para>
          To calibrate the acceleration sensor, use the 'c a 0' command.  You
          will be prompted to orient the board vertically with the UHF antenna
          up and press a key, then to orient the board vertically with the
          UHF antenna down and press a key.  Note that the accuracy of this
	  calibration depends primarily on how perfectly vertical and still
	  the board is held during the cal process.  As with all 'c' 
	  sub-commands, follow this with a 'c w' to write the
          change to the parameter block in the on-board DataFlash chip.
        </para>
        <para>
          The +1g and -1g calibration points are included in each telemetry
          frame and are part of the header stored in onboard flash to be
	  downloaded after flight.  We always store and return raw ADC 
	  samples for each sensor... so nothing is permanently "lost" or 
	  "damaged" if the calibration is poor.
        </para>
        <para>
         In the unlikely event an accel cal goes badly, it is possible
         that TeleMetrum may always come up in 'pad mode' and as such not be
         listening to either the USB or radio link.  If that happens,
         there is a special hook in the firmware to force the board back
         in to 'idle mode' so you can re-do the cal.  To use this hook, you
         just need to ground the SPI clock pin at power-on.  This pin is
         available as pin 2 on the 8-pin companion connector, and pin 1 is
         ground.  So either carefully install a fine-gauge wire jumper
         between the two pins closest to the index hole end of the 8-pin
         connector, or plug in the programming cable to the 8-pin connector
         and use a small screwdriver or similar to short the two pins closest
         to the index post on the 4-pin end of the programming cable, and
         power up the board.  It should come up in 'idle mode' (two beeps),
	 allowing a re-cal.
        </para>
      </section>
  </appendix>
  <appendix
      xmlns:xi="http://www.w3.org/2001/XInclude">
    <title>Release Notes</title>
    <xi:include	href="release-notes-1.1.1.xsl"  xpointer="xpointer(/article/*)"/>
    <xi:include	href="release-notes-1.1.xsl"  xpointer="xpointer(/article/*)"/>
    <xi:include	href="release-notes-1.0.1.xsl"  xpointer="xpointer(/article/*)"/>
    <xi:include	href="release-notes-0.9.2.xsl"  xpointer="xpointer(/article/*)"/>
    <xi:include	href="release-notes-0.9.xsl"  xpointer="xpointer(/article/*)"/>
    <xi:include	href="release-notes-0.8.xsl"  xpointer="xpointer(/article/*)"/>
    <xi:include	href="release-notes-0.7.1.xsl"  xpointer="xpointer(/article/*)"/>
  </appendix>
</book>

<!--  LocalWords:  Altusmetrum
-->
