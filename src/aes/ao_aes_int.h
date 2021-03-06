/* Copyright (C) 2000-2009 Peter Selinger.
   This file is part of ccrypt. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* rijndael.h */
/* $Id: rijndael.h 258 2009-08-26 17:46:10Z selinger $ */

/* derived from original source: rijndael-alg-ref.h   v2.0   August '99
 * Reference ANSI C code for NIST competition
 * authors: Paulo Barreto
 *          Vincent Rijmen
 */

#ifndef __RIJNDAEL_H
#define __RIJNDAEL_H

#include <stdint.h>

typedef uint8_t word8;
typedef uint32_t word32;

/* a type to hold 32 bits accessible as 1 integer or 4 bytes */
union word8x4_u {
  word8 w8[4];
  word32 w32;
};
typedef union word8x4_u word8x4;

#include "ao_aes_tables.h"

#define MAXBC		(256/32)
#define MAXKC		(256/32)
#define MAXROUNDS	14
#define MAXRK           ((MAXROUNDS+1)*MAXBC)

typedef struct {
  int BC;
  int KC;
  int ROUNDS;
  int shift[2][4];
  word32 rk[MAXRK];
} roundkey;

#if 0
/* keys and blocks are externally treated as word32 arrays, to
   make sure they are aligned on 4-byte boundaries on architectures
   that require it. */

/* make a roundkey rkk from key. key must have appropriate size given
   by keyBits. keyBits and blockBits may only be 128, 196, or
   256. Returns non-zero if arguments are invalid. */

int xrijndaelKeySched(word32 key[], int keyBits, int blockBits,
		      roundkey *rkk);

/* encrypt, resp. decrypt, block using rijndael roundkey rkk. rkk must
   have been created with xrijndaelKeySched. Size of block, in bits,
   must be equal to blockBits parameter that was used to make rkk. In
   all other cases, behavior is undefined - for reasons of speed, no
   check for error conditions is done. */

void xrijndaelEncrypt(word32 block[], roundkey *rkk);
void xrijndaelDecrypt(word32 block[], roundkey *rkk);
#endif

#endif				/* __RIJNDAEL_H */
