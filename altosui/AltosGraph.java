
// Copyright (c) 2010 Anthony Towns
// GPL v2 or later

package altosui;

import java.io.*;

import org.jfree.chart.JFreeChart;
import org.jfree.chart.ChartUtilities;
import org.altusmetrum.AltosLib.*;

abstract class AltosGraph {
    public String filename;
    public abstract void addData(AltosDataPoint d);
    public abstract JFreeChart createChart();
    public String title;
    public void toPNG() throws java.io.IOException { toPNG(300, 500); }
    public void toPNG(int width, int height)
        throws java.io.IOException
    {
        File pngout = new File(filename);
        JFreeChart chart = createChart();
        ChartUtilities.saveChartAsPNG(pngout, chart, width, height);
        System.out.println("Created " + filename);
    }
}
