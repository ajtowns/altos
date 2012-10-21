
// Copyright (c) 2010 Anthony Towns
// GPL v2 or later

package altosui;

import java.lang.UnsupportedOperationException;
import java.util.NoSuchElementException;
import java.util.Iterator;
import org.altusmetrum.AltosLib.*;

class AltosDataPointReader implements Iterable<AltosDataPoint> {
    Iterator<AltosRecord> iter;
    AltosState state;
    boolean has_gps;
    boolean has_accel;
    boolean has_ignite;

    final static int MISSING = AltosRecord.MISSING;

    public AltosDataPointReader(AltosRecordIterable reader) {
        this.iter = reader.iterator();
        this.state = null;
	has_accel = true;
	has_gps = reader.has_gps();
	has_ignite = reader.has_ignite();
    }

    private void read_next_record() 
        throws NoSuchElementException
    {
        state = new AltosState(iter.next(), state);
    }

    private AltosDataPoint current_dp() {
        assert this.state != null;
        
        return new AltosDataPoint() {
            public int version() { return state.data.version; }
            public int serial() { return state.data.serial; }
            public int flight() { return state.data.flight; }
            public String callsign() { return state.data.callsign; }
            public double time() { return state.data.time; }
            public double rssi() { return state.data.rssi; }

            public int state() { return state.state; }
            public String state_name() { return state.data.state(); }

            public double acceleration() { return state.acceleration; }
	    public double height() { return state.height; }
	    public double speed() { return state.speed(); }
            public double temperature() { return state.temperature; }
            public double battery_voltage() { return state.battery; }
            public double drogue_voltage() { return state.drogue_sense; }
            public double main_voltage() { return state.main_sense; }
	    public boolean has_accel() { return true; } // return state.acceleration != AltosRecord.MISSING; }
        };
    }

    public Iterator<AltosDataPoint> iterator() {
        return new Iterator<AltosDataPoint>() {
            public void remove() { 
                throw new UnsupportedOperationException(); 
            }
            public boolean hasNext() {
		if (state != null && state.state == Altos.ao_flight_landed)
		    return false;
                return iter.hasNext();
            }
            public AltosDataPoint next() {
		do {
		    read_next_record();
		} while (state.data.time < -1.0 && hasNext());
                return current_dp();
            }
        };
    }
}

