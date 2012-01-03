
// Copyright (c) 2010 Anthony Towns
// GPL v2 or later

package altosui;

import java.io.IOException;
import java.text.ParseException;
import java.lang.UnsupportedOperationException;
import java.util.NoSuchElementException;
import java.util.Iterator;
import org.altusmetrum.AltosLib.*;

class AltosDataPointReader implements Iterable<AltosDataPoint> {
    Iterator<AltosRecord> iter;
    AltosState state;
    AltosRecord record;
    boolean has_gps;
    boolean has_accel;
    boolean has_ignite;

    final static int MISSING = AltosRecord.MISSING;

    public AltosDataPointReader(AltosRecordIterable reader) {
        this.iter = reader.iterator();
        this.state = null;
	has_accel = reader.has_accel();
	has_gps = reader.has_gps();
	has_ignite = reader.has_ignite();
    }

    private void read_next_record() 
        throws NoSuchElementException
    {
        record = iter.next();
        state = new AltosState(record, state);
    }

    private AltosDataPoint current_dp() {
        assert this.record != null;
        
        return new AltosDataPoint() {
            public int version() { return record.version; }
            public int serial() { return record.serial; }
            public int flight() { return record.flight; }
            public String callsign() { return record.callsign; }
            public double time() { return record.time; }
            public double rssi() { return record.rssi; }

            public int state() { return record.state; }
            public String state_name() { return record.state(); }

            public double acceleration() { return record.acceleration(); }
            public double pressure() { return record.raw_pressure(); }
            public double altitude() { return record.raw_altitude(); }
	    public double height() { return record.raw_height(); }
            public double accel_speed() { return record.accel_speed(); }
            public double baro_speed() { return state.baro_speed; }
            public double temperature() { return record.temperature(); }
            public double battery_voltage() { return record.battery_voltage(); }
            public double drogue_voltage() { return record.drogue_voltage(); }
            public double main_voltage() { return record.main_voltage(); }
	    public boolean has_accel() { return has_accel; }
        };
    }

    public Iterator<AltosDataPoint> iterator() {
        return new Iterator<AltosDataPoint>() {
            public void remove() { 
                throw new UnsupportedOperationException(); 
            }
            public boolean hasNext() {
		if (record != null && record.state == Altos.ao_flight_landed)
		    return false;
                return iter.hasNext();
            }
            public AltosDataPoint next() {
		do {
		    read_next_record();
		} while (record.time < -1.0 && hasNext());
                return current_dp();
            }
        };
    }
}

