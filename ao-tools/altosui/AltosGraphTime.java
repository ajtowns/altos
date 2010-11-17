
// Copyright (c) 2010 Anthony Towns
// GPL v2 or later

package altosui;

import java.awt.Color;
import java.util.ArrayList;
import java.util.HashMap;

import org.jfree.chart.ChartUtilities;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.AxisLocation;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.labels.StandardXYToolTipGenerator;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.plot.ValueMarker;
import org.jfree.chart.renderer.xy.StandardXYItemRenderer;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;
import org.jfree.ui.RectangleAnchor;
import org.jfree.ui.TextAnchor;

class AltosGraphTime extends AltosGraph {
    static interface Element {
        void attachGraph(AltosGraphTime g);
        void gotTimeData(double time, AltosDataPoint d);
        void addToPlot(AltosGraphTime g, XYPlot plot);
    }

    static class TimeAxis implements Element {
        private int axis;
        private Color color;
        private String label;
        private AxisLocation locn;
        private double min_y = Double.NaN;

        public TimeAxis(int axis, String label, Color color, AxisLocation locn)
        {
            this.axis = axis;
            this.color = color;
            this.label = label;
            this.locn = locn;
        }

        public void setLowerBound(double min_y) {
            this.min_y = min_y;
        }

        public void attachGraph(AltosGraphTime g) { return; }
        public void gotTimeData(double time, AltosDataPoint d) { return; }

        public void addToPlot(AltosGraphTime g, XYPlot plot) {
            NumberAxis numAxis = new NumberAxis(label);
            if (!Double.isNaN(min_y))
                numAxis.setLowerBound(min_y);
            plot.setRangeAxis(axis, numAxis);
            plot.setRangeAxisLocation(axis, locn);
            numAxis.setLabelPaint(color);
            numAxis.setTickLabelPaint(color);
            numAxis.setAutoRangeIncludesZero(false);
        }
    }

    abstract static class TimeSeries implements Element {
        protected XYSeries series;
        private String axisName;
        private Color color;

        public TimeSeries(String axisName, String label, Color color) {
            this.series = new XYSeries(label);
            this.axisName = axisName;
            this.color = color;
        }

        public void attachGraph(AltosGraphTime g) {
            g.setAxis(this, axisName, color);
        }
        abstract public void gotTimeData(double time, AltosDataPoint d);

        public void addToPlot(AltosGraphTime g, XYPlot plot) {
            XYSeriesCollection dataset = new XYSeriesCollection();
            dataset.addSeries(this.series);

            XYItemRenderer renderer = new StandardXYItemRenderer();
            renderer.setSeriesPaint(0, color);

            int dataNum = g.getDataNum(this);
            int axisNum = g.getAxisNum(this);

            plot.setDataset(dataNum, dataset);
            plot.mapDatasetToRangeAxis(dataNum, axisNum);
            plot.setRenderer(dataNum, renderer);
        }
    }

    static class StateMarker implements Element {
        private double val = Double.NaN;
        private String name;
        private int state;

        StateMarker(int state, String name) {
            this.state = state;
            this.name = name;
        }

        public void attachGraph(AltosGraphTime g) { return; }
        public void gotTimeData(double time, AltosDataPoint d) {
            if (Double.isNaN(val) || time < val) {
                if (d.state() == state) {
                    val = time;
                }
            }
        }

        public void addToPlot(AltosGraphTime g, XYPlot plot) {
            if (Double.isNaN(val))
                return;

            ValueMarker m = new ValueMarker(val);
            m.setLabel(name);
            m.setLabelAnchor(RectangleAnchor.TOP_RIGHT);
            m.setLabelTextAnchor(TextAnchor.TOP_LEFT);
            plot.addDomainMarker(m);
        }
    }

    private String callsign = null;
    private Integer serial = null;
    private Integer flight = null; 

    private String title;
    private ArrayList<Element> elements;
    private HashMap<String,Integer> axes;
    private HashMap<Element,Integer> datasets;
    private ArrayList<Integer> datasetAxis;

    public AltosGraphTime(String title) {
        this.filename = title.toLowerCase().replaceAll("[^a-z0-9]","_")+".png";
        this.title = title;
        this.elements = new ArrayList<Element>();
        this.axes = new HashMap<String,Integer>();
        this.datasets = new HashMap<Element,Integer>();
        this.datasetAxis = new ArrayList<Integer>();
    }

    public AltosGraphTime addElement(Element e) {
        e.attachGraph(this);
        elements.add(e);
        return this;
    }

    public void setAxis(Element ds, String axisName, Color color) {
        Integer axisNum = axes.get(axisName);
        int dsNum = datasetAxis.size();
        if (axisNum == null) {
            axisNum = newAxis(axisName, color);
        }
        datasets.put(ds, dsNum);
        datasetAxis.add(axisNum);
    }

    public int getAxisNum(Element ds) {
        return datasetAxis.get( datasets.get(ds) ).intValue();
    }
    public int getDataNum(Element ds) {
        return datasets.get(ds).intValue();
    }

    private Integer newAxis(String name, Color color) {
        int cnt = axes.size();
        AxisLocation locn = AxisLocation.BOTTOM_OR_LEFT;
        if (cnt > 0) {
            locn = AxisLocation.TOP_OR_RIGHT;
        }
        Integer res = new Integer(cnt);
        axes.put(name, res);
        this.addElement(new TimeAxis(cnt, name, color, locn));
        return res;
    }

    public void addData(AltosDataPoint d) {
        double time = d.time();
        for (Element e : elements) {
            e.gotTimeData(time, d);
        }
        if (callsign == null) callsign = d.callsign();
        if (serial == null) serial = new Integer(d.serial());
        if (flight == null) flight = new Integer(d.flight());
    }

    public JFreeChart createChart() {
        NumberAxis xAxis = new NumberAxis("Time (s)");
        xAxis.setAutoRangeIncludesZero(false);
        XYItemRenderer renderer = new XYLineAndShapeRenderer(true, false);
        XYPlot plot = new XYPlot();
        plot.setDomainAxis(xAxis);
        plot.setRenderer(renderer);
        plot.setOrientation(PlotOrientation.VERTICAL);

        if (serial != null && flight != null) {
            title = serial + "/" + flight + ": " + title;
        }
        if (callsign != null) {
            title = callsign + " - " + title;
        }

        renderer.setBaseToolTipGenerator(new StandardXYToolTipGenerator());
        JFreeChart chart = new JFreeChart(title, JFreeChart.DEFAULT_TITLE_FONT,
                                plot, true);
        ChartUtilities.applyCurrentTheme(chart);

        plot.setDomainPannable(true);
        plot.setRangePannable(true);
   
        for (Element e : elements) {
            e.addToPlot(this, plot);
        }

        return chart;
    }

    public void toPNG() throws java.io.IOException {
        if (axes.size() > 1) {
            toPNG(800, 500);
        } else {
            toPNG(300, 500);
        }
    }
}
