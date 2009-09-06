/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include "cc.h"
#include "cephes.h"
#include <math.h>
#include <stdlib.h>

static inline double sqr (double x) { return x * x; }

/*
 * Kaiser Window digital filter
 */

#if 0
/* not used in this program */
static double highpass (double n, double m, double wc)
{
	double  alpha = m/2;
	double  dist;

	dist = n - alpha;
	if (dist == 0)
		return (M_PI/2 - wc) / M_PI;
	return -sin(dist * (M_PI/2-wc)) / (M_PI * dist);
}
#endif

static double lowpass (double n, double m, double wc)
{
	double  alpha = m/2;
	double  dist;
	dist = n - alpha;
	if (dist == 0)
		return wc / M_PI;
	return sin (wc * dist) / (M_PI * dist);
}

static double kaiser (double n, double m, double beta)
{
	double alpha = m / 2;
	return i0 (beta * sqrt (1 - sqr((n - alpha) / alpha))) / i0(beta);
}

static double beta (double A)
{
	if (A > 50)
		return 0.1102 * (A - 8.7);
	else if (A >= 21)
		return 0.5842 * pow((A - 21), 0.4) + 0.07886 * (A - 21);
	else
		return 0.0;
}

static int M (double A, double delta_omega)
{
	if (A > 21)
		return ceil ((A - 7.95) / (2.285 * delta_omega));
	else
		return ceil(5.79 / delta_omega);
}

struct filter_param {
	double omega_pass;
	double delta_omega;
	double A;
	double beta;
	int M;
} filter_param_t;

static struct filter_param
filter (double omega_pass, double omega_stop, double error)
{
	struct filter_param  p;
	p.omega_pass = omega_pass;
	p.delta_omega = omega_stop - omega_pass;
	p.A = -20 * log10 (error);
	p.beta = beta (p.A);
	p.M = M (p.A, p.delta_omega);
	if ((p.M & 1) == 1)
		p.M++;
	return p;
}

static double *
make_low_pass_filter(double omega_pass, double omega_stop, double error, int *length_p)
{
	struct filter_param	p = filter(omega_pass, omega_stop, error);
	int		length;
	int		n;
	double		*lpf;

	length = p.M + 1;
	lpf = calloc (length, sizeof(double));
	for (n = 0; n < length; n++)
		lpf[n] = lowpass(n, p.M, omega_pass) * kaiser(n, p.M, p.beta);
	*length_p = length;
	return lpf;
}

static double *
convolve(double *d, int d_len, double *e, int e_len)
{
	int w = (e_len - 1) / 2;
	int n;
	double *con = calloc (d_len, sizeof (double));

	for (n = 0; n < d_len; n++) {
		double	v = 0;
		int o;
		for (o = -w; o <= w; o++) {
			int	p = n + o;
			double sample = p < 0 ? d[0] : p >= d_len ? d[d_len-1] : d[p];
			v += sample * e[o + w];
		}
		con[n] = v;
	}
	return con;
}

double *
cc_low_pass(double *data, int data_len, double omega_pass, double omega_stop, double error)
{
	int fir_len;
	double *fir = make_low_pass_filter(omega_pass, omega_stop, error, &fir_len);
	double *result;

	result = convolve(data, data_len, fir, fir_len);
	free(fir);
	return result;
}
