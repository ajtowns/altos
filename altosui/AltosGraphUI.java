
// Copyright (c) 2010 Anthony Towns
// GPL v2 or later

package altosui;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.AltosLib.*;
import org.altusmetrum.altosuilib.*;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.ui.RefineryUtilities;

public class AltosGraphUI extends AltosUIFrame 
{
    JTabbedPane	pane;

    static final private Color red = new Color(194,31,31);
    static final private Color green = new Color(31,194,31);
    static final private Color blue = new Color(31,31,194);
    //static final private Color black = new Color(31,31,31);
    static final private Color yellow = new Color(194,194,31);
    //static final private Color cyan = new Color(31,194,194);
    static final private Color magenta = new Color(194,31,194);

    static private class OverallGraphs {
        AltosGraphTime.Element height = 
		new AltosGraphTime.TimeSeries("Height", AltosConvert.height.show_units(), "Height (AGL)", red) {
                public void gotTimeData(double time, AltosDataPoint d) {
			double	height = d.height();
			if (height != AltosRecord.MISSING)
				series.add(time, AltosConvert.height.value(height));
                } 
            };
    
        AltosGraphTime.Element speed =
		new AltosGraphTime.TimeSeries("Speed", AltosConvert.speed.show_units(), "Vertical Speed", green) { 
                public void gotTimeData(double time, AltosDataPoint d) {
		    double	speed = d.speed();
		    if (speed != AltosRecord.MISSING)
			    series.add(time, AltosConvert.speed.value(speed));
                }
            };
    
        AltosGraphTime.Element acceleration =
		new AltosGraphTime.TimeSeries("Acceleration",
					      AltosConvert.accel.show_units(),
					      "Axial Acceleration", blue)
            {
                public void gotTimeData(double time, AltosDataPoint d) {
		    double acceleration = d.acceleration();
		    if (acceleration != AltosRecord.MISSING)
			    series.add(time, AltosConvert.accel.value(acceleration));
                }
            };
    
        AltosGraphTime.Element temperature =
	    new AltosGraphTime.TimeSeries("Temperature", "\u00B0C", 
					  "Board temperature", red) 
            {
                public void gotTimeData(double time, AltosDataPoint d) {
		    double temp = d.temperature();
		    if (temp != AltosRecord.MISSING)
			series.add(time, d.temperature());
                }
            };
    
        AltosGraphTime.Element drogue_voltage =
            new AltosGraphTime.TimeSeries("Voltage", "(V)", "Drogue Continuity", yellow) 
            {
                public void gotTimeData(double time, AltosDataPoint d) {
		    double v = d.drogue_voltage();
		    if (v != AltosRecord.MISSING)
			series.add(time, v);
                }
            };
    
        AltosGraphTime.Element main_voltage =
            new AltosGraphTime.TimeSeries("Voltage", "(V)", "Main Continuity", magenta) 
            {
                public void gotTimeData(double time, AltosDataPoint d) {
		    double v = d.main_voltage();
		    if (v != AltosRecord.MISSING)
			series.add(time, v);
                }
            };
    
        //AltosGraphTime.Element e_pad    = new AltosGraphTime.StateMarker(Altos.ao_flight_pad, "Pad");
        AltosGraphTime.Element e_boost  = new AltosGraphTime.StateMarker(Altos.ao_flight_boost, "Boost");
        AltosGraphTime.Element e_fast   = new AltosGraphTime.StateMarker(Altos.ao_flight_fast, "Fast");
        AltosGraphTime.Element e_coast  = new AltosGraphTime.StateMarker(Altos.ao_flight_coast, "Coast");
	AltosGraphTime.Element e_drogue = new AltosGraphTime.StateMarker(Altos.ao_flight_drogue, "Drogue");
	AltosGraphTime.Element e_main   = new AltosGraphTime.StateMarker(Altos.ao_flight_main, "Main");
        AltosGraphTime.Element e_landed = new AltosGraphTime.StateMarker(Altos.ao_flight_landed, "Landed");
    
        protected AltosGraphTime myAltosGraphTime(String suffix) {
            return (new AltosGraphTime("Overall " + suffix))
                .addElement(e_boost)
		.addElement(e_fast)
		.addElement(e_coast)
                .addElement(e_drogue)
                .addElement(e_main)
                .addElement(e_landed);
        }
    
        public ArrayList<AltosGraph> graphs() {
            ArrayList<AltosGraph> graphs = new ArrayList<AltosGraph>();
    
	    graphs.add( myAltosGraphTime("Summary")
			.addElement(height)
			.addElement(speed)
			.addElement(acceleration) );

	    graphs.add( myAltosGraphTime("Summary")
			.addElement(height)
			.addElement(speed));
    
            graphs.add( myAltosGraphTime("Altitude")
                    .addElement(height) );
    
            graphs.add( myAltosGraphTime("Speed")
                    .addElement(speed) );
    
	    graphs.add( myAltosGraphTime("Acceleration")
			.addElement(acceleration) );
    
            graphs.add( myAltosGraphTime("Temperature")
                    .addElement(temperature) );
    
	    graphs.add( myAltosGraphTime("Continuity")
			.addElement(drogue_voltage)
			.addElement(main_voltage) );
    
            return graphs;
        }
    }

    /*
    static private class AscentGraphs extends OverallGraphs {
        protected AltosGraphTime myAltosGraphTime(String suffix) {
            return (new AltosGraphTime("Ascent " + suffix) {
                public void addData(AltosDataPoint d) {
                    int state = d.state();
                    if (Altos.ao_flight_boost <= state && state <= Altos.ao_flight_coast) {
                        super.addData(d);
                    }
                }
            }).addElement(e_boost)
              .addElement(e_fast)
              .addElement(e_coast);
        }
    }
    */

    /*
    static private class DescentGraphs extends OverallGraphs {
        protected AltosGraphTime myAltosGraphTime(String suffix) {
            return (new AltosGraphTime("Descent " + suffix) {
                public void addData(AltosDataPoint d) {
                    int state = d.state();
                    if (Altos.ao_flight_drogue <= state && state <= Altos.ao_flight_main) {
                        super.addData(d);
                    }
                }
            }).addElement(e_drogue)
              .addElement(e_main);
            // ((XYGraph)graph[8]).ymin = new Double(-50);
        }
    }
    */

	public AltosGraphUI(AltosRecordIterable records, String name) throws InterruptedException, IOException {
		super(String.format("Altos Graph %s", name));

		AltosDataPointReader reader = new AltosDataPointReader (records);
        
		if (reader.has_accel)
		    init(reader, records, 0);
		else
		    init(reader, records, 1);
	}

//    public AltosGraphUI(AltosDataPointReader data, int which)
    //  {
//        super("Altos Graph");
//        init(data, which);
//    }

    private void init(AltosDataPointReader data, AltosRecordIterable records, int which) throws InterruptedException, IOException {
	pane = new JTabbedPane();

        AltosGraph graph = createGraph(data, which);

        JFreeChart chart = graph.createChart();
        ChartPanel chartPanel = new ChartPanel(chart);
        chartPanel.setMouseWheelEnabled(true);
        chartPanel.setPreferredSize(new java.awt.Dimension(800, 500));
        pane.add(graph.title, chartPanel);

	AltosFlightStatsTable stats = new AltosFlightStatsTable(new AltosFlightStats(records));
	pane.add("Flight Statistics", stats);

	setContentPane (pane);

        pack();

        RefineryUtilities.centerFrameOnScreen(this);

        setDefaultCloseOperation(DISPOSE_ON_CLOSE);
        setVisible(true);
    }

    private static AltosGraph createGraph(Iterable<AltosDataPoint> data,
            int which)
    {
        return createGraphsWhich(data, which).get(0);
    }

    /*
    private static ArrayList<AltosGraph> createGraphs(
            Iterable<AltosDataPoint> data)
    {
        return createGraphsWhich(data, -1);
    }
    */

    private static ArrayList<AltosGraph> createGraphsWhich(
            Iterable<AltosDataPoint> data, int which)
    {
        ArrayList<AltosGraph> graph = new ArrayList<AltosGraph>();
        graph.addAll((new OverallGraphs()).graphs());
//        graph.addAll((new AscentGraphs()).graphs());
//        graph.addAll((new DescentGraphs()).graphs());

        if (which > 0) {
            if (which >= graph.size()) {
                which = 0;
            }
            AltosGraph g = graph.get(which);
            graph = new ArrayList<AltosGraph>();
            graph.add(g);
        }

        for (AltosDataPoint dp : data) {
            for (AltosGraph g : graph) {
                g.addData(dp);
            }
        }

        return graph;
    }
}

/* gnuplot bits...
 *
300x400

--------------------------------------------------------
TOO HARD! :)

"ascent-gps-accuracy.png" "Vertical error margin to apogee - GPS v Baro (m)"
    5:($7 < 6 ? $24-$11 : 1/0)
"descent-gps-accuracy.png" "Vertical error margin during descent - GPS v Baro (m)"
    5:($7 < 6 ? 1/0 : $24-$11)

set output "overall-gps-accuracy.png"
set ylabel "distance above sea level (m)"
plot "telemetry.csv" using 5:11 with lines ti "baro altitude" axis x1y1, \
    "telemetry.csv" using 5:24 with lines ti "gps altitude" axis x1y1

set term png tiny size 700,700 enhanced
set xlabel "m"
set ylabel "m"
set polar
set grid polar
set rrange[*:*]
set angles degrees

set output "overall-gps-path.png"
#:30 with yerrorlines
plot "telemetry.csv" using (90-$33):($7 == 2 ? $31 : 1/0) with lines ti "pad", \
    "telemetry.csv" using (90-$33):($7 == 3 ? $31 : 1/0) with lines ti "boost", \
    "telemetry.csv" using (90-$33):($7 == 4 ? $31 : 1/0) with lines ti "fast", \
    "telemetry.csv" using (90-$33):($7 == 5 ? $31 : 1/0) with lines ti "coast", \
    "telemetry.csv" using (90-$33):($7 == 6 ? $31 : 1/0) with lines ti "drogue", \
    "telemetry.csv" using (90-$33):($7 == 7 ? $31 : 1/0) with lines ti "main", \
    "telemetry.csv" using (90-$33):($7 == 8 ? $31 : 1/0) with lines ti "landed"

set output "ascent-gps-path.png"
plot "telemetry.csv" using (90-$33):($7 == 2 ? $31 : 1/0):30 with lines ti "pad", \
    "telemetry.csv" using (90-$33):($7 == 3 ? $31 : 1/0):20 with lines ti "boost", \
    "telemetry.csv" using (90-$33):($7 == 4 ? $31 : 1/0):10 with lines ti "fast", \
    "telemetry.csv" using (90-$33):($7 == 5 ? $31 : 1/0):5 with lines ti "coast"

set output "descent-gps-path.png"
plot "telemetry.csv" using (90-$33):($7 == 6 ? $31 : 1/0) with lines ti "drogue", \
    "telemetry.csv" using (90-$33):($7 == 7 ? $31 : 1/0) with lines ti "main", \
    "telemetry.csv" using (90-$33):($7 == 8 ? $31 : 1/0) with lines ti "landed"

 */


