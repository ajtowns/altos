/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

public class AltosMs5607 {
	int	reserved;
	int	sens;
	int	off;
	int	tcs;
	int	tco;
	int	tref;
	int	tempsens;
	int	crc;

	int		raw_pres;
	int		raw_temp;
	public int	pa;
	public int	cc;

	void convert() {
		int	dT;
		int TEMP;
		long OFF;
		long SENS;
		int P;

		dT = raw_temp - ((int) tref << 8);
	
		TEMP = (int) (2000 + (((long) dT * tempsens) >> 23));

		OFF = ((long) off << 17) + (((long) tco * dT) >> 6);

		SENS = ((long) sens << 16) + (((long) tcs * dT) >> 7);

		if (TEMP < 2000) {
			int	T2 = (int) (((long) dT * (long) dT) >> 31);
			int TEMPM = TEMP - 2000;
			long OFF2 = (61 * (long) TEMPM * (long) TEMPM) >> 4;
			long SENS2 = 2 * (long) TEMPM * (long) TEMPM;
			if (TEMP < 1500) {
				int TEMPP = TEMP + 1500;
				long TEMPP2 = TEMPP * TEMPP;
				OFF2 = OFF2 + 15 * TEMPP2;
				SENS2 = SENS2 + 8 * TEMPP2;
			}
			TEMP -= T2;
			OFF -= OFF2;
			SENS -= SENS2;
		}

		pa = (int) (((((long) raw_pres * SENS) >> 21) - OFF) >> 15);
		cc = TEMP;
	}

	public int set(int in_pres, int in_temp) {
		raw_pres = in_pres;
		raw_temp = in_temp;
		convert();
		return pa;
	}
}
