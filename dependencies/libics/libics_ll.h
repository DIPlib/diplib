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
 * FILE : libics_ll.h
 *
 * This file defines the low-level interface functions. Include it in your
 * source code only if that is the way you want to go.
 */


#ifndef LIBICS_LL_H
#define LIBICS_LL_H


#include "libics.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef ICS Ics_Header;


/* These are the known data formats for imels. */
typedef enum {
   IcsForm_unknown = 0,
   IcsForm_integer,
   IcsForm_real,
   IcsForm_complex
} Ics_Format;


/* Definitions of separators in the .ics file for writing: */
#define ICS_FIELD_SEP '\t'
#define ICS_EOL       '\n'


/* Reads a .ics file into an Ics_Header structure. */
ICSEXPORT Ics_Error IcsReadIcs(Ics_Header *icsStruct,
                               const char *filename,
                               int         forcename,
                               int         forcelocale);

/* Writes an Ics_Header structure into a .ics file. */
ICSEXPORT Ics_Error IcsWriteIcs(Ics_Header *icsStruct,
                                const char *filename);

/* Initializes image data reading. */
ICSEXPORT Ics_Error IcsOpenIds(Ics_Header *icsStruct);

/* Ends image data reading. */
ICSEXPORT Ics_Error IcsCloseIds(Ics_Header *icsStruct);

/* Reads image data block from disk. */
ICSEXPORT Ics_Error IcsReadIdsBlock(Ics_Header *icsStruct,
                                    void       *outbuf,
                                    size_t      len);

/* Skips image data block from disk (fseek forward). */
ICSEXPORT Ics_Error IcsSkipIdsBlock(Ics_Header *icsStruct,
                                    size_t      len);

/* Sets the file pointer into the image data on disk (fseek anywhere). */
ICSEXPORT Ics_Error IcsSetIdsBlock(Ics_Header *icsStruct,
                                   long        offset,
                                   int         whence);

/* Reads image data from disk. */
ICSEXPORT Ics_Error IcsReadIds(Ics_Header *icsStruct,
                               void       *dest,
                               size_t      n);

/* Writes image data to disk. */
ICSEXPORT Ics_Error IcsWriteIds(const Ics_Header *icsStruct);

/* Initializes the Ics_Header structure to default values and zeros. */
ICSEXPORT void IcsInit(Ics_Header *icsStruct);

/* Appends ".ics" to the filename (removing ".ids" if present) If (forcename) we
  do change ".ids" into ".ics", but do not add anything. */
ICSEXPORT char *IcsGetIcsName(char       *dest,
                              const char *src,
                              int         forcename);

/* Appends ".ids" to the filename (removing ".ics" if present). */
ICSEXPORT char *IcsGetIdsName(char       *dest,
                              const char *src);

/* Return pointer to .ics or .ids extension or NULL if not found. */
ICSEXPORT char *IcsExtensionFind(const char *str);

/* Returns the size in bytes of the data type. */
ICSEXPORT size_t IcsGetDataTypeSize(Ics_DataType DataType);

/* Fills in format, sign and bits according to the data type. */
ICSEXPORT void IcsGetPropsDataType(Ics_DataType  DataType,
                                   Ics_Format   *format,
                                   int          *sign,
                                   size_t       *bits);

/* Sets the data type according to format, sign and bits. */
ICSEXPORT void IcsGetDataTypeProps(Ics_DataType *DataType,
                                   Ics_Format    format,
                                   int           sign,
                                   size_t        bits);

/* Free the memory allocated for history. */
ICSEXPORT void IcsFreeHistory(Ics_Header *ics);


#ifdef __cplusplus
}
#endif


#endif
