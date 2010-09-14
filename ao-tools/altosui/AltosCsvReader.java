
// Copyright (c) 2010 Anthony Towns
// GPL v2 or later

package altosui;

import java.lang.UnsupportedOperationException;
import java.util.HashMap;
import java.util.NoSuchElementException;
import java.util.Iterator;
import java.io.*;
import com.csvreader.CsvReader;

import altosui.AltosDataPoint;

class AltosCsvReader implements Iterable<AltosDataPoint>
{
    public CsvReader csv;
    public AltosDataPoint next = null;

    static protected String [] headers = "version serial flight call time rssi state state_name acceleration pressure altitude height accel_speed baro_speed temperature battery_voltage drogue_voltage main_voltage connected locked nsat latitude longitude altitude year month day hour minute second pad_dist pad_range pad_az pad_el".split(" ");

    AltosCsvReader(Reader stream) {
        csv = new CsvReader(stream);
        csv.setComment('#');
        csv.setUseComments(true);
        csv.setHeaders(headers);
    }
    AltosCsvReader(String filename) throws FileNotFoundException {
        csv = new CsvReader(filename);
        csv.setComment('#');
        csv.setUseComments(true);
        csv.setHeaders(headers);
    }

    public Iterator<AltosDataPoint> iterator() {
        return new Iterator<AltosDataPoint>() {
            public void remove() { 
                throw new UnsupportedOperationException(); 
            }
            public boolean hasNext() {
                if (next == null) {
                    try {
                        if (csv.readRecord()) {
                            next = new CsvRow();
                        } else {
                            close();
                            return false;
                        }
                    } catch (IOException e) {
                        close();
                        return false;
                    }
                }
                return true;
            }
            public AltosDataPoint next() {
                if (!hasNext())
                    throw new NoSuchElementException();
                AltosDataPoint res = next;
                next = null;
                return res;
            }
        };
    }

    public void close() {
        csv.close();
    }

    private class CsvRow extends HashMap<String,String>
            implements AltosDataPoint 
    {
        CsvRow() throws IOException {
            for (int i = 0; i < headers.length; i++) {
                this.put(headers[i], csv.get(headers[i]).trim());
            }
        }

        private int intField(String name) {
            return Integer.parseInt(get(name).trim());
        }
        private double doubleField(String name) {
            return Double.valueOf(get(name)).doubleValue();
        }
        private String stringField(String name) {
            return get(name);
        }

        public int version() { return intField("version"); }
        public int serial() { return intField("serial"); }
        public int flight() { return intField("flight"); }
        public String callsign() { return stringField("call"); }
        public double time() { return doubleField("time"); }
        public double rssi() { return doubleField("rssi"); }

        public int state() { return intField("state"); }
        public String state_name() { return stringField("state_name"); }

        public double acceleration() { return doubleField("acceleration"); }
        public double pressure() { return doubleField("pressure"); }
        public double altitude() { return doubleField("altitude"); }
        public double height() { return doubleField("height"); }
        public double accel_speed() { return doubleField("accel_speed"); }
        public double baro_speed() { return doubleField("baro_speed"); }
        public double temperature() { return doubleField("temperature"); }
        public double battery_voltage() { 
            return doubleField("battery_voltage"); 
        }
        public double drogue_voltage() { return doubleField("drogue_voltage"); }
        public double main_voltage() { return doubleField("main_voltage"); }
    }
}
