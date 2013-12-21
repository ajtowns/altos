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

#ifndef _AO_MMA655X_H_
#define _AO_MMA655X_H_


#define AO_MMA655X_SN0		0x00
#define AO_MMA655X_SN1		0x01
#define AO_MMA655X_SN2		0x02
#define AO_MMA655X_SN3		0x03
#define AO_MMA655X_SN3		0x03
#define AO_MMA655X_STDEFL	0x04
#define AO_MMA655X_FCTCFG	0x06
# define AO_MMA655X_FCTCFG_STMAG	7

#define AO_MMA655X_PN		0x08
#define AO_MMA655X_DEVCTL	0x0a
#define AO_MMA655X_DEVCTL_RES_1		7
#define AO_MMA655X_DEVCTL_RES_0		6
#define AO_MMA655X_DEVCTL_OCPHASE	4
#define AO_MMA655X_DEVCTL_OCPHASE_MASK	3
#define AO_MMA655X_DEVCTL_OFFCFG_EN	3

#define AO_MMA655X_DEVCFG	0x0b
#define AO_MMA655X_DEVCFG_OC		7
#define AO_MMA655X_DEVCFG_ENDINIT	5
#define AO_MMA655X_DEVCFG_SD		4
#define AO_MMA655X_DEVCFG_OFMON		3
#define AO_MMA655X_DEVCFG_A_CFG		0
#define AO_MMA655X_DEVCFG_A_CFG_DISABLE		0
#define AO_MMA655X_DEVCFG_A_CFG_PCM		1
#define AO_MMA655X_DEVCFG_A_CFG_MOVING_AVG_HIGH	2
#define AO_MMA655X_DEVCFG_A_CFG_MOVING_AVG_LOW	3
#define AO_MMA655X_DEVCFG_A_CFG_COUNT_HIGH	4
#define AO_MMA655X_DEVCFG_A_CFG_COUNT_LOW	5
#define AO_MMA655X_DEVCFG_A_CFG_UNFILTERED_HIGH	6
#define AO_MMA655X_DEVCFG_A_CFG_UNFILTERED_LOW	7
#define AO_MMA655X_DEVCFG_A_CFG_MASK		7

#define AO_MMA655X_AXISCFG	0x0c
#define AO_MMA655X_AXISCFG_ST		7
#define AO_MMA655X_AXISCFG_LPF		0
#define AO_MMA655X_AXISCFG_LPF_MASK	0xf

#define AO_MMA655X_ARMCFG	0x0e
#define AO_MMA655X_ARMCFG_APS		4
#define AO_MMA655X_ARMCFG_APS_MASK	3
#define AO_MMA655X_ARMCFG_AWS_N		2
#define AO_MMA655X_ARMCFG_AWS_N_MASK	3
#define AO_MMA655X_ARMCFG_AWS_P		0
#define AO_MMA655X_ARMCFG_AWS_P_MASK	3

#define AO_MMA655X_ARMT_P	0x10
#define AO_MMA655X_ARMT_N	0x12

#define AO_MMA655X_DEVSTAT	0x14
#define AO_MMA655X_DEVSTAT_IDE		6
#define AO_MMA655X_DEVSTAT_DEVINIT	4
#define AO_MMA655X_DEVSTAT_MISOERR	3
#define AO_MMA655X_DEVSTAT_OFFSET	1
#define AO_MMA655X_DEVSTAT_DEVRES	0

#define AO_MMA655X_COUNT	0x15
#define AO_MMA655X_OFFCORR	0x16

/*
 * Range of valid self-test difference from
 * normal measurement
 */

#define AO_ST_MIN	300
#define AO_ST_MAX	800

extern uint16_t	ao_mma655x_current;

void
ao_mma655x_init(void);

#endif /* _AO_MMA655X_H_ */
