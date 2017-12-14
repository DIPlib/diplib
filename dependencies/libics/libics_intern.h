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
 * FILE : libics_intern.h
 *
 * Only needed to build the library.
 */

#ifndef LIBICS_INTERN_H
#define LIBICS_INTERN_H

#include <stdio.h>
#include "libics.h"
#include "libics_ll.h"
#include "libics_conf.h"

/* Declare and initialize the error variable. */
#define ICSINIT Ics_Error error = IcsErr_Ok

/* Forcing the proper locale. */
#ifdef ICS_FORCE_C_LOCALE
#include <locale.h>
#define ICS_INIT_LOCALE \
    char* Ics_CurrentLocale = 0
#define ICS_SET_LOCALE                           \
    Ics_CurrentLocale = setlocale(LC_ALL, NULL); \
    setlocale (LC_ALL, "C")
#define ICS_REVERT_LOCALE \
    setlocale(LC_ALL, Ics_CurrentLocale)
#else
#define ICS_INIT_LOCALE
#define ICS_SET_LOCALE
#define ICS_REVERT_LOCALE
#endif


/* Below the IcsTokens are defined . Each token corresponds to an ICS
   keyword. Several token are defined for intenal bookkeeping: LASTMAIN, FIRST,
   LAST. These should not be moved! Note: If a token is added/removed the
   corresponding arrays which relate token to strings in libics_data.c MUST be
   synchronized! */
typedef enum {
        /* Main category tokens: */
    ICSTOK_SOURCE = 0,
    ICSTOK_LAYOUT,
    ICSTOK_REPRES,
    ICSTOK_PARAM,
    ICSTOK_HISTORY,
    ICSTOK_SENSOR,
    ICSTOK_END,
    ICSTOK_LASTMAIN,

        /* Subcategory tokens: */
    ICSTOK_FIRSTSUB,
    ICSTOK_FILE,
    ICSTOK_OFFSET,
    ICSTOK_PARAMS,
    ICSTOK_ORDER,
    ICSTOK_SIZES,
    ICSTOK_COORD,
    ICSTOK_SIGBIT,
    ICSTOK_FORMAT,
    ICSTOK_SIGN,
    ICSTOK_COMPR,
    ICSTOK_BYTEO,
    ICSTOK_ORIGIN,
    ICSTOK_SCALE,
    ICSTOK_UNITS,
    ICSTOK_LABELS,
    ICSTOK_SCILT,
    ICSTOK_TYPE,
    ICSTOK_MODEL,
    ICSTOK_SPARAMS,
    ICSTOK_SSTATES,
    ICSTOK_LASTSUB,

        /* SubsubCategory tokens: */
    ICSTOK_FIRSTSUBSUB,
    ICSTOK_CHANS,
    ICSTOK_IMDIR,
    ICSTOK_NUMAPER,
    ICSTOK_OBJQ,
    ICSTOK_REFRIME,
    ICSTOK_REFRILM,
    ICSTOK_PINHRAD,
    ICSTOK_ILLPINHRAD,
    ICSTOK_PINHSPA,
    ICSTOK_EXBFILL,
    ICSTOK_LAMBDEX,
    ICSTOK_LAMBDEM,
    ICSTOK_PHOTCNT,
    ICSTOK_IFACE1,
    ICSTOK_IFACE2,
    ICSTOK_DETMAG,
    ICSTOK_DETPPU,
    ICSTOK_DETBASELINE,
    ICSTOK_DETLNAVGCNT,
    ICSTOK_STEDDEPLMODE,
    ICSTOK_STEDLAMBDA,
    ICSTOK_STEDSATFACTOR,
    ICSTOK_STEDIMMFRACTION,
    ICSTOK_STEDVPPM,
    ICSTOK_SPIMEXCTYPE,
    ICSTOK_SPIMFILLFACTOR,
    ICSTOK_SPIMPLANENA,
    ICSTOK_SPIMPLANEGAUSSWIDTH,
    ICSTOK_SPIMPLANEPROPDIR,
    ICSTOK_SPIMPLANECENTEROFF,
    ICSTOK_SPIMPLANEFOCUSOF,
    ICSTOK_SCATTERMODEL,
    ICSTOK_SCATTERFREEPATH,
    ICSTOK_SCATTERRELCONTRIB,
    ICSTOK_SCATTERBLURRING,
    ICSTOK_LASTSUBSUB,

        /* Value tokens: */
    ICSTOK_FIRSTVALUE,
    ICSTOK_COMPR_UNCOMPRESSED,
    ICSTOK_COMPR_COMPRESS,
    ICSTOK_COMPR_GZIP,
    ICSTOK_FORMAT_INTEGER,
    ICSTOK_FORMAT_REAL,
    ICSTOK_FORMAT_COMPLEX,
    ICSTOK_SIGN_SIGNED,
    ICSTOK_SIGN_UNSIGNED,
    ICSTOK_STATE_DEFAULT,
    ICSTOK_STATE_ESTIMATED,
    ICSTOK_STATE_REPORTED,
    ICSTOK_STATE_VERIFIED,
    ICSTOK_LASTVALUE,

    ICSTOK_NONE
} Ics_Token;


/* Definition keyword relating to imel representation */
#define ICS_ORDER_BITS "bits"
#define ICS_LABEL_BITS "intensity"


/* Definition of other keywords */
#define ICS_HISTORY         "history"
#define ICS_COORD_VIDEO     "video"
#define ICS_FILENAME        "filename"
#define ICS_VERSION         "ics_version"
#define ICS_UNITS_RELATIVE  "relative"
#define ICS_UNITS_UNDEFINED "undefined"


/* The following structure links names to (enumerated) tokens: */
typedef struct {
    const char *name;
    Ics_Token   token;
} Ics_Symbol;


typedef struct {
    int         entries;
    Ics_Symbol *list;
} Ics_SymbolList;


extern Ics_Symbol     G_CatSymbols[];
extern Ics_Symbol     G_SubCatSymbols[];
extern Ics_Symbol     G_SubSubCatSymbols[];
extern Ics_Symbol     G_ValueSymbols[];
extern Ics_SymbolList G_Categories;
extern Ics_SymbolList G_SubCategories;
extern Ics_SymbolList G_SubSubCategories;
extern Ics_SymbolList G_Values;


/* This is the struct behind the "void* History" in the ICS structure: */
typedef struct {
    char   **strings; /* History strings */
    size_t   length;  /* Size of the Strings array */
    int      nStr;    /* Index past the last one in the array; sort of the
                         number of strings in the array, except that some array
                         elements might be NULL */
} Ics_History;

/* This is the struct behind the "void* BlockRead" in the ICS structure: */
typedef struct {
    FILE*          dataFilePtr;     /* Input data file */
#ifdef ICS_ZLIB
    void          *zlibStream;      /* z_stream* (or gzFile) for zlib */
    void          *zlibInputBuffer; /* Input buffer for compressed data */
    unsigned long  zlibCRC;         /* running CRC */
#endif
    int            compressRead;    /* set to non-zero when IcsReadCompress has
                                      been called */
} Ics_BlockRead;


/* Assorted support functions */
FILE *IcsFOpen(const char *path,
               const char *mode);

size_t IcsStrToSize(const char *str);

void IcsStrCpy(char       *dest,
               const char *src,
               int         len);

void IcsAppendChar(char *Line,
                   char ch);

int IcsGetBytesPerSample(const Ics_Header *IcsStruct);

void IcsGetFileName(char       *dest,
                    const char *src);

Ics_Error IcsOpenIcs(FILE **fpp,
                     char  *filename,
                     int    forceName);

Ics_Error IcsInternAddHistory(Ics_Header *ics,
                              const char *key,
                              const char *stuff,
                              const char *seps);

/* Binary data support functions */
void IcsFillByteOrder(int bytes,
                      int machineByteOrder[ICS_MAX_IMEL_SIZE]);

Ics_Error IcsWritePlainWithStrides(const void      *src,
                                   const size_t    *dim,
                                   const ptrdiff_t *stride,
                                   int              nDims,
                                   int              nBytes,
                                   FILE            *file);

Ics_Error IcsCopyIds(const char *infilename,
                     size_t      inoffset,
                     const char *outfilename);

/* zlib interface functions */
Ics_Error IcsWriteZip(const void *src,
                      size_t      n,
                      FILE       *fp,
                      int         CompLevel);

Ics_Error IcsWriteZipWithStrides(const void      *src,
                                 const size_t    *dim,
                                 const ptrdiff_t *stride,
                                 int              nDims,
                                 int              nBytes,
                                 FILE            *file,
                                 int              level);

Ics_Error IcsOpenZip(Ics_Header *IcsStruct);

Ics_Error IcsCloseZip(Ics_Header *IcsStruct);

Ics_Error IcsReadZipBlock(Ics_Header *IcsStruct,
                          void       *outBuf,
                          size_t      len);

Ics_Error IcsSetZipBlock(Ics_Header *IcsStruct,
                         long        offset,
                         int         whence);

/* Reading COMPRESS-compressed data */
Ics_Error IcsReadCompress(Ics_Header *IcsStruct,
                          void       *outBuf,
                          size_t      len);

#endif
