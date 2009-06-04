/*-------------------------------------------------------------------------
   Register Declarations for the ChipCon CC1111 Processor Range

   Copyright © 2008 Keith Packard <keithp@keithp.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Adapted from the Cygnal C8051F12x config file which is:

   Copyright (C) 2003 - Maarten Brock, sourceforge.brock@dse.nl

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
-------------------------------------------------------------------------*/

#ifndef _CC1111_H_
#define _CC1111_H_


/*  BYTE Registers  */

sfr at 0x80 P0       ;  /* PORT 0                                        */
sfr at 0x81 SP       ;  /* STACK POINTER                                 */
sfr at 0x82 DPL      ;  /* DATA POINTER - LOW BYTE                       */
sfr at 0x83 DPH      ;  /* DATA POINTER - HIGH BYTE                      */
sfr at 0x84 DPL1     ;  /* DATA POINTER 1 - LOW BYTE                     */
sfr at 0x85 DPH1     ;  /* DATA POINTER 1 - HIGH BYTE                    */
sfr at 0x86 U0CSR    ;  /* USART 0 Control and status                    */
sfr at 0x87 PCON     ;  /* POWER CONTROL                                 */
sfr at 0x88 TCON     ;  /* TIMER CONTROL                                 */
sfr at 0x89 P0IFG    ;  /* TIMER MODE                                    */
sfr at 0x8A P1IFG    ;  /* TIMER 0 - LOW BYTE                            */
sfr at 0x8B P2IFG    ;  /* TIMER 1 - LOW BYTE                            */
sfr at 0x8C PICTL    ;  /* TIMER 0 - HIGH BYTE                           */
sfr at 0x8D P1IEN    ;  /* TIMER 1 - HIGH BYTE                           */

sfr at 0x8F P0INP    ;  /* FLASH WRITE/ERASE CONTROL                     */
sfr at 0x90 P1       ;  /* PORT 1                                        */
sfr at 0x91 RFIM     ;  /* UART 0 STATUS                                 */
sfr at 0x92 DPS      ;  /* */
sfr at 0x93 MPAGE    ;  /* */
sfr at 0x94 _SFR94_  ;  /* */
sfr at 0x95 ENDIAN   ;  /* */
sfr at 0x96 _SFR96_  ;  /* */
sfr at 0x97 _SFR97_  ;  /* */
sfr at 0x98 S0CON    ;  /* UART 0 CONTROL                                */
sfr at 0x99 _SFR99_  ;  /* UART 0 BUFFER                                 */
sfr at 0x9A IEN2     ;  /* SPI 0 CONFIGURATION                           */
sfr at 0x9B S1CON    ;  /* SPI 0 DATA                                    */
sfr at 0x9C T2CT     ;  /* SPI 0 DATA                                    */
sfr at 0x9D T2PR     ;  /* SPI 0 CLOCK RATE CONTROL                      */
sfr at 0x9E T2CTL    ;  /* SPI 0 CLOCK RATE CONTROL                      */
sfr at 0x9F _SFR9F_  ;  /* SPI 0 CLOCK RATE CONTROL                      */
sfr at 0xA0 P2       ;  /* PORT 2                                        */
sfr at 0xA1 WORIRQ   ;  /* EMIF TIMING CONTROL                           */
sfr at 0xA2 WORCTRL  ;  /* EMIF CONTROL                                  */
sfr at 0xA3 WOREVT0  ;  /* EMIF CONFIGURATION                            */
sfr at 0xA4 WOREVT1  ;  /* EMIF CONFIGURATION                            */
sfr at 0xA5 WORTIME0 ;  /* EMIF CONFIGURATION                            */
sfr at 0xA6 WORTIME1 ;  /* EMIF CONFIGURATION                            */
sfr at 0xA7 _SFRA7_  ;  /* EMIF CONFIGURATION                            */
sfr at 0xA8 IEN0     ;  /* INTERRUPT ENABLE                              */
sfr at 0xA9 IP0      ;  /* UART 0 SLAVE ADDRESS                          */
sfr at 0xAA _SFRAA_  ;  /*                                               */
sfr at 0xAB FWT      ;  /*                                               */
sfr at 0xAC FADDRL   ;  /*                                               */
sfr at 0xAD FADDRH   ;  /*                                               */
sfr at 0xAE FCTL     ;  /*                                               */
sfr at 0xAF FWDATA   ;  /*                                               */
sfr at 0xB0 _SFRB0_  ;  /*                                               */
sfr at 0xB1 ENCDI    ;  /* FLASH BANK SELECT                             */
sfr at 0xB2 ENCDO    ;  /*                                               */
sfr at 0xB3 ENCCS    ;  /*                                               */
sfr at 0xB4 ADCCON1  ;  /*                                               */
sfr at 0xB5 ADCCON2  ;  /*                                               */
sfr at 0xB6 ADCCON3  ;  /*                                               */
sfr at 0xB8 IEN1     ;  /* INTERRUPT PRIORITY                            */
sfr at 0xB9 IP1      ;  /*                                               */
sfr at 0xBA ADCL     ;  /*                                               */
sfr at 0xBB ADCH     ;  /*                                               */
sfr at 0xBC RNDL     ;  /*                                               */
sfr at 0xBD RNDH     ;  /*                                               */
sfr at 0xBE SLEEP    ;  /*                                               */
sfr at 0xC0 IRCON    ;  /*                                               */
sfr at 0xC1 U0DBUF   ;  /*                                               */
sfr at 0xC2 U0BAUD   ;  /*                                               */
sfr at 0xC4 U0UCR    ;  /*                                               */
sfr at 0xC5 U0GCR    ;  /*                                               */
sfr at 0xC6 CLKCON   ;  /*                                               */
sfr at 0xC7 MEMCTR   ;  /*                                               */
sfr at 0xC9 WDCTL    ;  /*                                               */
sfr at 0xCA T3CNT    ;  /*                                               */
sfr at 0xCB T3CTL    ;  /*                                               */
sfr at 0xCC T3CCTL0  ;  /*                                               */
sfr at 0xCD T3CC0    ;  /*                                               */
sfr at 0xCE T3CCTL1  ;  /*                                               */
sfr at 0xCF T3CC1    ;  /*                                               */
sfr at 0xD0 PSW      ;  /*                                               */
sfr at 0xD1 DMAIRQ   ;  /*                                               */
sfr at 0xD2 DMA1CFGL ;  /*                                               */
sfr at 0xD3 DMA1CFGH ;  /*                                               */
sfr at 0xD4 DMA0CFGL ;  /*                                               */
sfr at 0xD5 DMA0CFGH ;  /*                                               */
sfr at 0xD6 DMAARM   ;  /*                                               */
sfr at 0xD7 DMAREQ   ;  /*                                               */
sfr at 0xD8 TIMIF    ;  /*                                               */
sfr at 0xD9 RFD      ;  /*                                               */
sfr at 0xDA T1CC0L   ;  /*                                               */
sfr at 0xDB T1CC0H   ;  /*                                               */
sfr at 0xDC T1CC1L   ;  /*                                               */
sfr at 0xDD T1CC1H   ;  /*                                               */
sfr at 0xDE T1CC2L   ;  /*                                               */
sfr at 0xDF T1CC2H   ;  /*                                               */
sfr at 0xE0 ACC      ;  /* ACCUMULATOR                                   */
sfr at 0xE1 RFST     ;  /*                                               */
sfr at 0xE2 T1CNTL   ;  /*                                               */
sfr at 0xE3 T1CNTH   ;  /*                                               */
sfr at 0xE4 T1CTL    ;  /*                                               */
sfr at 0xE5 T1CCTL0  ;  /*                                               */
sfr at 0xE6 T1CCTL1  ;  /*                                               */
sfr at 0xE7 T1CCTL2  ;  /*                                               */
sfr at 0xE8 IRCON2   ;  /*                                               */
sfr at 0xE9 RFIF     ;  /*                                               */
sfr at 0xEA T4CNT    ;  /*                                               */
sfr at 0xEB T4CTL    ;  /*                                               */
sfr at 0xEC T4CCTL0  ;  /*                                               */
sfr at 0xED T4CC0    ;  /*                                               */
sfr at 0xEE T4CCTL1  ;  /*                                               */
sfr at 0xEF T4CC1    ;  /*                                               */
sfr at 0xF0 B        ;  /*                                               */
sfr at 0xF1 PERCFG   ;  /*                                               */
sfr at 0xF2 ADCCFG   ;  /*                                               */
sfr at 0xF3 P0SEL    ;  /*                                               */
sfr at 0xF4 P1SEL    ;  /*                                               */
sfr at 0xF5 P2SEL    ;  /*                                               */
sfr at 0xF6 P1INP    ;  /*                                               */
sfr at 0xF7 P2INP    ;  /*                                               */
sfr at 0xF8 U1CSR    ;  /*                                               */
sfr at 0xF9 U1DBUF   ;  /*                                               */
sfr at 0xFA U1BAUD   ;  /*                                               */
sfr at 0xFB U1UCR    ;  /*                                               */
sfr at 0xFC U1GCR    ;  /*                                               */
sfr at 0xFD P0DIR    ;  /*                                               */
sfr at 0xFE P1DIR    ;  /*                                               */
sfr at 0xFF P2DIR    ;  /*                                               */

/*  BIT Registers  */

/*  P0  0x80 */
sbit at 0x80 P0_0    ;
sbit at 0x81 P0_1    ;
sbit at 0x82 P0_2    ;
sbit at 0x83 P0_3    ;
sbit at 0x84 P0_4    ;
sbit at 0x85 P0_5    ;
sbit at 0x86 P0_6    ;
sbit at 0x87 P0_7    ;

/*  TCON  0x88 */
sbit at 0x89 RFTXRXIF;  /*                                               */
sbit at 0x8B URX0IF  ;  /*                                               */
sbit at 0x8D ADCIF   ;  /*                                               */
sbit at 0x8F URX1IF  ;  /*                                               */
sbit at 0x8F I2SRXIF ;  /*                                               */

/*  SCON0  0x98 */
sbit at 0x98 ENCIF_0 ;  /* UART 0 RX INTERRUPT FLAG                      */
sbit at 0x99 ENCIF_1 ;  /* UART 0 RX INTERRUPT FLAG                      */

/*  IEN0  0xA8 */
sbit at 0xA8 RFTXRXIE;  /* RF TX/RX done interrupt enable                */
sbit at 0xA9 ADCIE   ;  /* ADC interrupt enable                          */
sbit at 0xAA URX0IE  ;  /* USART0 RX interrupt enable                    */
sbit at 0xAB URX1IE  ;  /* USART1 RX interrupt enable                    */
sbit at 0xAB I2SRXIE ;  /* I2S RX interrupt enable                       */
sbit at 0xAC ENCIE   ;  /* AES interrupt enable                          */
sbit at 0xAD STIE    ;  /* Sleep Timer interrupt enable                  */
sbit at 0xAF EA      ;  /* GLOBAL INTERRUPT ENABLE                       */

/*  IEN1  0xB8 */
sbit at 0xB8 DMAIE   ;  /* DMA transfer interrupt enable                 */
sbit at 0xB9 T1IE    ;  /* Timer 1 interrupt enable                      */
sbit at 0xBA T2IE    ;  /* Timer 2 interrupt enable                      */
sbit at 0xBB T3IE    ;  /* Timer 3 interrupt enable                      */
sbit at 0xBC T4IE    ;  /* Timer 4 interrupt enable                      */
sbit at 0xBD P0IE    ;  /* Port 0 interrupt enable                       */

/* IRCON 0xC0 */
sbit at 0xC0 DMAIF   ;  /*                       			 */
sbit at 0xC1 T1IF    ;  /*                       			 */
sbit at 0xC2 T2IF    ;  /*                       			 */
sbit at 0xC3 T3IF    ;  /*                       			 */
sbit at 0xC4 T4IF    ;  /*                       			 */
sbit at 0xC5 P0IF    ;  /*                       			 */
sbit at 0xC7 STIF    ;  /*                       			 */

/*  PSW  0xD0 */
sbit at 0xD0 P       ;  /* ACCUMULATOR PARITY FLAG                       */
sbit at 0xD1 F1      ;  /* USER FLAG 1                                   */
sbit at 0xD2 OV      ;  /* OVERFLOW FLAG                                 */
sbit at 0xD3 RS0     ;  /* REGISTER BANK SELECT 0                        */
sbit at 0xD4 RS1     ;  /* REGISTER BANK SELECT 1                        */
sbit at 0xD5 F0      ;  /* USER FLAG 0                                   */
sbit at 0xD6 AC      ;  /* AUXILIARY CARRY FLAG                          */
sbit at 0xD7 CY      ;  /* CARRY FLAG                                    */

/* TIMIF D8H */
sbit at 0xD8 T3OVFIF ;  /*                                               */
sbit at 0xD9 T3CH0IF ;  /*                                               */
sbit at 0xDA T3CH1IF ;  /*                                               */
sbit at 0xDB T4OVFIF ;  /*                                               */
sbit at 0xDC T4CH0IF ;  /*                                               */
sbit at 0xDD T4CH1IF ;  /*                                               */
sbit at 0xDE OVFIM   ;  /*                                               */

/* IRCON2  E8H */
sbit at 0xE8 P2IF    ;  /*                             			 */
sbit at 0xE8 USBIF   ;  /*                             			 */
sbit at 0xE9 UTX0IF  ;  /*                             			 */
sbit at 0xEA UTX1IF  ;  /*                             			 */
sbit at 0xEA I2STXIF ;  /*                             			 */
sbit at 0xEB P1IF    ;  /*                             			 */
sbit at 0xEC WDTIF   ;  /*                             			 */

/* U1CSR F8H */
sbit at 0xF8 U1_ACTIVE  ;  /*                                               */
sbit at 0xF9 U1_TX_BYTE ;  /*                                               */
sbit at 0xFA U1_RX_BYTE ;  /*                                               */
sbit at 0xFB U1_ERR     ;  /*                                               */
sbit at 0xFC U1_FE      ;  /*                                               */
sbit at 0xFD U1_SLAVE   ;  /*                                               */
sbit at 0xFE U1_RE      ;  /*                                               */
sbit at 0xFF U1_MODE    ;  /*                                               */

#define T1CTL_MODE_SUSPENDED	(0 << 0)
#define T1CTL_MODE_FREE		(1 << 0)
#define T1CTL_MODE_MODULO	(2 << 0)
#define T1CTL_MODE_UP_DOWN	(3 << 0)
#define T1CTL_MODE_MASK		(3 << 0)
#define T1CTL_DIV_1		(0 << 2)
#define T1CTL_DIV_8		(1 << 2)
#define T1CTL_DIV_32		(2 << 2)
#define T1CTL_DIV_128		(3 << 2)
#define T1CTL_DIV_MASK		(3 << 2)
#define T1CTL_OVFIF		(1 << 4)
#define T1CTL_CH0IF		(1 << 5)
#define T1CTL_CH1IF		(1 << 6)
#define T1CTL_CH2IF		(1 << 7)

#define T1CCTL_NO_CAPTURE	(0 << 0)
#define T1CCTL_CAPTURE_RISING	(1 << 0)
#define T1CCTL_CAPTURE_FALLING	(2 << 0)
#define T1CCTL_CAPTURE_BOTH	(3 << 0)
#define T1CCTL_CAPTURE_MASK	(3 << 0)

#define T1CCTL_MODE_CAPTURE	(0 << 2)
#define T1CCTL_MODE_COMPARE	(1 << 2)

#define T1CTL_CMP_SET		(0 << 3)
#define T1CTL_CMP_CLEAR		(1 << 3)
#define T1CTL_CMP_TOGGLE	(2 << 3)
#define T1CTL_CMP_SET_CLEAR	(3 << 3)
#define T1CTL_CMP_CLEAR_SET	(4 << 3)

#define T1CTL_IM_DISABLED	(0 << 6)
#define T1CTL_IM_ENABLED	(1 << 6)

#define T1CTL_CPSEL_NORMAL	(0 << 7)
#define T1CTL_CPSEL_RF		(1 << 7)

#endif
