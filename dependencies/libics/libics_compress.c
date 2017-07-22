/*
 * libics: Image Cytometry Standard file reading and writing.
 *
 * Copyright 2015-2017:
 *   Scientific Volume Imaging Holding B.V.
 *   Laapersveld 63, 1213 VB Hilversum, The Netherlands
 *   https://www.svi.nl
 *
 * Contact: libics@svi.nl
 *
 * Copyright (C) 2000-2013 Cris Luengo and others
 *
 * Large chunks of this library written by
 *    Bert Gijsbers
 *    Dr. Hans T.M. van der Voort
 * And also Damir Sudar, Geert van Kempen, Jan Jitze Krol,
 * Chiel Baarslag and Fons Laan.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * FILE : libics_compress.c
 *
 * The following internal functions are contained in this file:
 *
 *   IcsReadCompress ()
 *
 * This file is based on code from (N)compress 4.2.4.3, written by
 * Spencer W. Thomas, Jim McKie, Steve Davies, Ken Turkowski, James
 * A. Woods, Joe Orost, Dave Mack and Peter Jannesen between 1984 and
 * 1992. The original code is public domain and obtainable from
 * http://ncompress.sourceforge.net/ .
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libics_intern.h"


#define IBUFSIZ  ICS_BUF_SIZE  /* Input buffer size */
#define IBUFXTRA 64


/* Defines for third byte of header */
#define MAGIC_1 (unsigned char)'\037'   /* First byte of compressed file */
#define MAGIC_2 (unsigned char)'\235'   /* Second byte of compressed file */
#define BIT_MASK 0x1f   /* Mask for 'number of compresssion bits' */
                        /* Masks 0x20 and 0x40 are free. I think 0x20 should
                           mean that there is a fourth header byte (for
                           expansion). */
#define BLOCK_MODE 0x80 /* Block compresssion if table is full and compression
                           rate is dropping flush tables */

/* The next two codes should not be changed lightly, as they must not lie within
   the contiguous general code space. */
#define FIRST 257 /* first free entry */
#define CLEAR 256 /* table clear output code */


#define INIT_BITS 9  /* initial number of bits/code */
#define HBITS 17     /* 50% occupancy */
#define HSIZE (1<<HBITS)
#define BITS  16


#define MAXCODE(n)   (1L << (n))
#define INPUT(b, o, c, n, m) {                      \
   unsigned char *p = &b[o >> 3];                   \
   c = ((((long)(p[0])) | ((long)(p[1]) << 8)       \
         | ((long)(p[2]) << 16)) >> (o & 0x7)) & m; \
   o += n;                                          \
}
#define TAB_PREFIXOF(i)       codeTab[i]
#define TAB_SUFFIXOF(i)       hTab[i]
#define DE_STACK              (&(hTab[HSIZE-1]))
#define CLEAR_TAB_PREFIXOF()  memset(codeTab, 0, 256)


/* Read the full COMPRESS-compressed data stream. */
Ics_Error IcsReadCompress(Ics_Header *IcsStruct,
                          void       *outBuffer,
                          size_t      len)
{
    ICSINIT;
    Ics_BlockRead  *br      = (Ics_BlockRead*)IcsStruct->blockRead;
    unsigned char  *stackPtr;
    long int        code;
    int             fInChar;
    long int        oldCode;
    long int        inCode;
    int             inBits;
    int             posBits;
    size_t          outPos  = 0;
    size_t          inSize;
    int             bitMask;
    long int        freeEnt;
    long int        maxCode;
    long int        maxMaxCode;
    int             nBits;
    size_t          rSize;
    int             blockMode;
    int             maxBits;
    size_t          i;
    size_t          offset;
    unsigned char  *inBuffer   = NULL;
    unsigned char  *hTab    = NULL;
    unsigned short *codeTab = NULL;


        /* Dynamically allocate memory that's static in (N)compress. */
    inBuffer = (unsigned char*)malloc(IBUFSIZ + IBUFXTRA);
    if (inBuffer == NULL) {
        error = IcsErr_Alloc;
        goto exit;
    }
        /* Not sure about the size of this thing, original code uses a long int
           array that's cast to char: */
    hTab = (unsigned char*)malloc(HSIZE * 4);
    if (hTab == NULL) {
        error = IcsErr_Alloc;
        goto exit;
    }
    codeTab = (unsigned short*)malloc(HSIZE * sizeof(unsigned short));
    if (codeTab == NULL) {
        error = IcsErr_Alloc;
        goto exit;
    }

    if ((rSize = fread(inBuffer, 1, IBUFSIZ, br->dataFilePtr)) <= 0) {
        error = IcsErr_FReadIds;
        goto exit;
    }
    inSize = rSize;
    if (inSize < 3 || inBuffer[0] != MAGIC_1 || inBuffer[1] != MAGIC_2) {
        error = IcsErr_CorruptedStream;
        goto exit;
    }

    maxBits = inBuffer[2] & BIT_MASK;
    blockMode = inBuffer[2] & BLOCK_MODE;
    maxMaxCode = MAXCODE(maxBits);
    if (maxBits > BITS) {
        error = IcsErr_DecompressionProblem;
        goto exit;
    }

    maxCode = MAXCODE(nBits = INIT_BITS) - 1;
    bitMask = (1 << nBits) - 1;
    oldCode = -1;
    fInChar = 0;
    posBits = 3 << 3;

    freeEnt = blockMode ? FIRST : 256;

        /* As above, initialize the first 256 entries in the table. */
    CLEAR_TAB_PREFIXOF();
    for (code = 255; code >= 0; --code) {
        TAB_SUFFIXOF(code) = (unsigned char)code;
    }

    do {

      resetbuf:

        offset = posBits >> 3;
        inSize = offset <= inSize ? inSize - offset : 0;
        for (i = 0 ; i < inSize ; ++i) {
            inBuffer[i] = inBuffer[i + offset];
        }
        posBits = 0;

        if (inSize < IBUFXTRA) {
            rSize = fread(inBuffer + inSize, 1, IBUFSIZ, br->dataFilePtr);
            if (rSize <= 0 && !feof(br->dataFilePtr)) {
                error = IcsErr_FReadIds;
                goto exit;
            }
            inSize += rSize;
        }

        if (rSize > 0) {
            inBits = (inSize - inSize%nBits) << 3;
        } else {
            inBits = (inSize << 3) - (nBits - 1);
        }

        while (inBits > posBits) {
            if (freeEnt > maxCode) {
                posBits = ((posBits - 1)
                           + ((nBits << 3)
                              - (posBits - 1
                                 + (nBits << 3)) % (nBits << 3)));
                ++nBits;
                if (nBits == maxBits) {
                    maxCode = maxMaxCode;
                } else {
                    maxCode = MAXCODE(nBits) - 1;
                }
                bitMask = (1 << nBits) - 1;
                goto resetbuf;
            }

            INPUT(inBuffer, posBits, code, nBits, bitMask);

            if (oldCode == -1) {
                if (code >= 256) {
                    error = IcsErr_CorruptedStream;
                    goto exit;
                }
                oldCode = code;
                fInChar = (int)oldCode;
                ((unsigned char*)outBuffer)[outPos++] = (unsigned char)fInChar;
                continue;
            }

            if (code == CLEAR && blockMode) {
                CLEAR_TAB_PREFIXOF();
                freeEnt = FIRST - 1;
                posBits = ((posBits - 1)
                           + ((nBits << 3)
                              - (posBits - 1 + (nBits << 3)) % (nBits << 3)));
                maxCode = MAXCODE(nBits = INIT_BITS) - 1;
                bitMask = (1 << nBits) - 1;
                goto resetbuf;
            }

            inCode = code;
            stackPtr = DE_STACK;

            if (code >= freeEnt) { /* Special case for KwKwK string.   */
                if (code > freeEnt) {
                    error = IcsErr_CorruptedStream;
                    goto exit;
                }
                *--stackPtr = (unsigned char)fInChar;
                code = oldCode;
            }

                /* Generate output characters in reverse order */
            while (code >= 256) {
                *--stackPtr = TAB_SUFFIXOF(code);
                code = TAB_PREFIXOF(code);
            }
            fInChar = TAB_SUFFIXOF(code);
            *--stackPtr = (unsigned char)fInChar;

                /* And put them out in forward order */
            i = DE_STACK - stackPtr;
            if (outPos+i > len) {
                i = len-outPos; /* do not write more in buffer than fits! */
            }
            memcpy(((unsigned char*)outBuffer) + outPos, stackPtr, i);
            outPos += i;
            if (outPos == len) {
                goto exit;
            }

            code = freeEnt;
            if (code < maxMaxCode) { /* Generate the new entry. */
                TAB_PREFIXOF(code) = (unsigned short)oldCode;
                TAB_SUFFIXOF(code) = (unsigned char)fInChar;
                freeEnt = code + 1;
            }

            oldCode = inCode; /* Remember previous code. */
        }

    } while (rSize > 0);

    if (outPos != len) {
        error = IcsErr_OutputNotFilled;
    }

  exit:
    if (inBuffer) free(inBuffer);
    if (hTab) free(hTab);
    if (codeTab) free(codeTab);
    return error;
}
