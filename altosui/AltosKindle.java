/*
 * Copyright © 2011 Anthony Towns <aj@erisian.com.au>
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

import java.net.URI;
import java.net.URL;
import java.net.InetSocketAddress;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import com.sun.net.httpserver.*;
import java.util.*;

public class AltosKindle implements HttpHandler {
	public void handle(HttpExchange exchange) throws IOException {
		String requestMethod = exchange.getRequestMethod();
		if (requestMethod.equalsIgnoreCase("GET")) {
			URI uri = exchange.getRequestURI();
			String path = uri.getPath();
			System.out.print("GET " + path + "\n");
			if (path.equals("") || path.equals("/")) {
				do_index(exchange);
			} else if (path.equals("/replay")) {
				do_serial_page(exchange, -1);
			} else if (path.startsWith("/serial/")) {
				do_serial_page(exchange, Integer.parseInt(path.substring(8)));
			} else if (path.equals("/launch.html")) {
				do_fixed_page(exchange, "launch.html", "text/html");
			} else if (path.equals("/launch.js")) {
				do_fixed_page(exchange, "launch.js", "application/javascript");
			} else if (path.equals("/raphael-min.js")) {
				do_fixed_page(exchange, "raphael-min.js", "application/javascript");
			} else if (path.equals("/launch.css")) {
				do_fixed_page(exchange, "launch.css", "text/css");
			} else {
				do_headers(exchange);
			}
		}
	}

	private class JSONStr {
		String whatever = "";
		public void add(String key, int value) {
			whatever = whatever + String.format(", \"%s\": %d", key, value);
		}
		public void add(String key, double value) {
			whatever = whatever + String.format(", \"%s\": %f", key, value);
		}
		public void add(String key, String value) {
			whatever = whatever + String.format(", \"%s\": \"%s\"", key, value);
		}
		public void add(String key, JSONStr value) {
			whatever = whatever + String.format(", \"%s\": %s", key, value.toString());
		}
		public void add(String key, AltosGPS value) {
			JSONStr gps = new JSONStr();
			gps.add("lat", value.lat);
			gps.add("lon", value.lon);
			gps.add("alt", value.alt);
			gps.add("Y", value.year);
			gps.add("M", value.month);
			gps.add("D", value.day);
			gps.add("h", value.hour);
			gps.add("m", value.minute);
			gps.add("s", value.second);
			this.add(key, gps);
		}
		public String toString() {
			return "{" + whatever.substring(2) + "}";
		}
	}

	private void do_serial_page(HttpExchange exchange, int serial)
	throws IOException
	{
		OutputStream responseBody = exchange.getResponseBody();
		AltosState s = get_record(serial);
		if (s == null) {
			exchange.sendResponseHeaders(404, 0);
			responseBody.close();
			return;
		}

		exchange.getResponseHeaders().set("Content-Type", "text/json");
		exchange.sendResponseHeaders(200, 0);
		JSONStr out = new JSONStr();
		out.add("serial", s.data.serial);
		out.add("callsign", s.data.callsign);
		out.add("flight", s.data.flight);
		out.add("rssi", s.data.rssi);
		out.add("state", s.data.state);
		out.add("statename", Altos.state_name(s.data.state));

		out.add("height", s.height);
		out.add("speed", s.speed);
		out.add("baro_speed", s.baro_speed);
		out.add("acceleration", s.acceleration);

		out.add("battery", s.battery);
		out.add("drogue", s.drogue_sense);
		out.add("main", s.main_sense);

		out.add("gpsn", s.data.gps.nsat);
		out.add("gps", s.data.gps);

		out.add("c_lat", -27.843933);
		out.add("c_lon", 152.957153);

		responseBody.write(out.toString().getBytes());
		responseBody.write("\n".getBytes());
		responseBody.close();
	}

	private void do_fixed_page(HttpExchange exchange, String page, String type)
	throws IOException
	{
		OutputStream responseBody = exchange.getResponseBody();
		URL pageURL = AltosUI.class.getResource("/" + page);
		exchange.getResponseHeaders().set("Content-Type", type);
		exchange.sendResponseHeaders(200, 0);
		copy(pageURL.openStream(), responseBody);
		responseBody.close();
	}

	private void do_index(HttpExchange exchange) throws IOException {
		OutputStream responseBody = exchange.getResponseBody();
		exchange.getResponseHeaders().set("Content-Type", "text/html");
		exchange.sendResponseHeaders(200, 0);
		String out = "<html><head><title>Flight Monitor</title></head><body><h1>Monitoring</h1><ul>";
		Set<Integer> serials = AltosKindle.records();
		for (Integer i : serials) {
			if (i >= 0) {
				out += String.format("<li><a href=\"launch.html#%d\">Serial %d</a></li>\n", i, i);
			} else {
				out += String.format("<li><a href=\"launch.html\">Replay</a></li>\n");
			}
		}
		out += "</ul>\n</body></html>\n";
		responseBody.write(out.getBytes());
		responseBody.close();
	}

	private void do_headers(HttpExchange exchange) throws IOException {
		exchange.sendResponseHeaders(200, 0);
		exchange.getResponseHeaders().set("Content-Type", "text/plain");
		OutputStream responseBody = exchange.getResponseBody();
		String uri = exchange.getRequestURI().toString();
		uri += "\n";
		responseBody.write(uri.getBytes());
		Headers requestHeaders = exchange.getRequestHeaders();
		Set<String> keySet = requestHeaders.keySet();
		Iterator<String> iter = keySet.iterator();
		while (iter.hasNext()) {
			String key = iter.next();
			List values = requestHeaders.get(key);
			String s = key + " = " + values.toString() + "\n";
			responseBody.write(s.getBytes());
		}
		responseBody.close();
	}

	private static HashMap<Integer, AltosState> telem = new HashMap<Integer,AltosState>();

	public static synchronized void add_record(int serial, AltosState state) {
		telem.put(serial, state);
	}
	public static synchronized void finish_record(int serial) {
		telem.remove(serial);
	}
	private static AltosState get_record(int serial) {
		return telem.get(serial);
	}
	private static synchronized Set<Integer> records() {
		return new HashSet<Integer>(telem.keySet());
	}

	public static void runServer() throws IOException {
		HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
		server.createContext("/", new AltosKindle());
		server.setExecutor(null); // creates a default executor
		server.start();
	}

	private static final int IO_BUFFER_SIZE = 4 * 1024;

	private static void copy(InputStream in, OutputStream out)
	throws IOException
	{
		byte[] b = new byte[IO_BUFFER_SIZE];
		int read;
		while ((read = in.read(b)) != -1) {
			out.write(b, 0, read);
		}
	}
}
