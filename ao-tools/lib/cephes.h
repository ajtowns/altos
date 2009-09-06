/*
 * This file comes from the cephes math library, which was
 * released under the GPLV2+ license as a part of the Debian labplot
 * package (I've included the GPLV2 license reference here to make
 * this clear) - Keith Packard <keithp@keithp.com>
 *
 * Cephes Math Library Release 2.0:  April, 1987
 * Copyright 1984, 1987 by Stephen L. Moshier
 * Direct inquiries to 30 Frost Street, Cambridge, MA 02140
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
/*
 * Prototypes of Cephes functions
 */

#ifndef _CEPHES_H_
#define _CEPHES_H_

/* Variable for error reporting.  See mtherr.c.  */
extern int merror;

#if 0
extern int airy ( double x, double *ai, double *aip, double *bi, double *bip );
extern double beta ( double a, double b );
extern double lbeta ( double a, double b );
extern double chdtrc ( double df, double x );
extern double chdtr ( double df, double x );
extern double chdtri ( double df, double y );
extern double dawsn ( double xx );
extern double ellie ( double phi, double m );
extern double ellik ( double phi, double m );
extern double ellpe ( double x );
extern double ellpk ( double x );
extern double expn ( int n, double x );
extern double fac ( int i );
extern double fdtrc ( int ia, int ib, double x );
extern double fdtr ( int ia, int ib, double x );
extern double fdtri ( int ia, int ib, double y );
extern double frexp ( double x, int *pw2 );
extern double ldexp ( double x, int pw2 );
extern int fresnl ( double xxa, double *ssa, double *cca );
extern double gdtr ( double a, double b, double x );
extern double gdtrc ( double a, double b, double x );
extern double hyp2f0 ( double a, double b, double x, int type, double *err );
extern double hyp2f1 ( double a, double b, double c, double x );
extern double hyperg ( double a, double b, double x );
#endif
extern double i0 ( double x );
extern double i0e ( double x );
#if 0
extern double i1 ( double x );
extern double i1e ( double x );
extern double iv ( double v, double x );
extern double igamc ( double a, double x );
extern double igam ( double a, double x );
extern double igami ( double a, double y0_ );
extern double incbet ( double aa, double bb, double xx );
extern double incbi ( double aa, double bb, double yy0 );
extern double jv ( double n, double x );
extern double k0 ( double x );
extern double k0e ( double x );
extern double k1 ( double x );
extern double k1e ( double x );
extern double kn ( int nn, double x );
extern int mtherr ( char *name, int code );
extern double ndtr ( double a );
extern double ndtri ( double y0_ );
extern double pdtrc ( int k, double m );
extern double pdtr ( int k, double m );
extern double pdtri ( int k, double y );
extern double psi ( double x );
extern void revers ( double y[], double x[], int n );
extern double true_gamma ( double x );
extern double rgamma ( double x );
extern int shichi ( double x, double *si, double *ci );
extern int sici ( double x, double *si, double *ci );
extern double spence ( double x );
extern double stdtr ( int k, double t );
extern double stdtri ( int k, double p );
extern double onef2 ( double a, double b, double c, double x, double *err );
extern double threef0 ( double a, double b, double c, double x, double *err );
extern double struve ( double v, double x );
extern double log1p ( double x );
extern double expm1 ( double x );
extern double cosm1 ( double x );
extern double yv ( double v, double x );
extern double zeta ( double x, double q );
extern double zetac ( double x );

#endif
extern double chbevl ( double x, void *P, int n );
#if 0
extern double polevl ( double x, void *P, int n );
extern double p1evl ( double x, void *P, int n );

/* polyn.c */
extern void polini ( int maxdeg );
extern void polprt ( double a[], int na, int d );
extern void polclr ( double *a, int n );
extern void polmov ( double *a, int na, double *b );
extern void polmul ( double a[], int na, double b[], int nb, double c[] );
extern void poladd ( double a[], int na, double b[], int nb, double c[] );
extern void polsub ( double a[], int na, double b[], int nb, double c[] );
extern int poldiv ( double a[], int na, double b[], int nb, double c[] );
extern void polsbt ( double a[], int na, double b[], int nb, double c[] );
extern double poleva ( double a[], int na, double x );

#endif

#endif /* _CEPHES_H_ */
