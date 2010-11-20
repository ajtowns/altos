/*
 * Copyright © 2010 Anthony Towns <aj@erisian.com.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package altosui;

import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import javax.swing.*;
import javax.imageio.ImageIO;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.net.URL;
import java.net.URLConnection;

public class AltosSiteMapLabel extends JLabel {
    public static boolean fetchMap(File file, String url) {
        URL u;
        try {
            u = new URL(url);
        } catch (java.net.MalformedURLException e) {
            return false;
        }

        byte[] data;
        try {
            URLConnection uc = u.openConnection();
            int contentLength = uc.getContentLength();
            InputStream in = new BufferedInputStream(uc.getInputStream());
            int bytesRead = 0;
            int offset = 0;
            data = new byte[contentLength];
            while (offset < contentLength) {
                bytesRead = in.read(data, offset, data.length - offset);
                if (bytesRead == -1)
                    break;
                offset += bytesRead;
            }
            in.close();

            if (offset != contentLength) {
                return false;
            }
        } catch (IOException e) {
            return false;
        }
   
        try {
            FileOutputStream out = new FileOutputStream(file);
            out.write(data);
            out.flush();
            out.close();
        } catch (FileNotFoundException e) {
            return false;
        } catch (IOException e) {
            if (file.exists()) {
                file.delete();
            }
            return false;
        }
        return true;
    }

    public void fetchAndLoadMap(final File pngfile, final String url) {
        System.out.printf("# Trying to fetch %s...\n", pngfile);

        Thread thread = new Thread() {
            public void run() {
                try {
                    if (fetchMap(pngfile, url)) {
                        setIcon(new ImageIcon(ImageIO.read(pngfile)));
                    }
                } catch (Exception e) {
                    System.out.printf("# Failed to fetch file %s\n", pngfile);
                    System.out.printf(" wget -O '%s' ''\n", pngfile, url);
                }
            }
        };
        thread.start();
    }

    public void fetchMap(double lat, double lng, int zoom, int px_size) {
        File pngfile = MapFile(lat, lng, zoom);
        String url = MapURL(lat, lng, zoom, px_size);

        if (!pngfile.exists()) {
            fetchMap(pngfile, url);
        }
    }

    public void loadMap(double lat, double lng, int zoom, int px_size) {
        File pngfile = MapFile(lat, lng, zoom);
        String url = MapURL(lat, lng, zoom, px_size);
        
        if (!pngfile.exists()) {
            fetchAndLoadMap(pngfile, url);
            return;
        }

        try {
            setIcon(new ImageIcon(ImageIO.read(pngfile)));
            return;
        } catch (IOException e) { 
            System.out.printf("# IO error trying to load %s\n", pngfile);
            return;
        }
    }

    private static File MapFile(double lat, double lng, int zoom) {
        char chlat = lat < 0 ? 'S' : 'N';
        char chlng = lng < 0 ? 'E' : 'W';
        if (lat < 0) lat = -lat;
        if (lng < 0) lng = -lng;
        return new File(AltosPreferences.logdir(), 
                String.format("map-%c%.6f,%c%.6f-%d.png",
                    chlat, lat, chlng, lng, zoom));
    }

    private static String MapURL(double lat, double lng, 
            int zoom, int px_size) 
    {
        return String.format("http://maps.google.com/maps/api/staticmap?center=%.6f,%.6f&zoom=%d&size=%dx%d&sensor=false&maptype=hybrid&format=png32", lat, lng, zoom, px_size, px_size);
    }
}

