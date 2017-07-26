/*
 * libics: Image Cytometry Standard file reading and writing.
 *
 * Copyright (C) 2000-2013 Cris Luengo and others
 * Copyright 2015, 2017:
 *   Scientific Volume Imaging Holding B.V.
 *   Laapersveld 63, 1213 VB Hilversum, The Netherlands
 *   https://www.svi.nl
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
 * FILE : libics_conf.h
 *
 * This file allows you to configure libics. Only needed to build the
 * library
 *
 */


#ifndef LIBICS_CONF_H
#define LIBICS_CONF_H


/* Doubles larger than ICS_MAX_DOUBLE or smaller than ICS_MIN_DOUBLE are written
   in scientific notation. */
#define ICS_MAX_DOUBLE 10000000
#define ICS_MIN_DOUBLE 0.001


/* ICS_HISTARRAY_INCREMENT sets the increment for the array allocated to contain
   the history strings. */
#define ICS_HISTARRAY_INCREMENT 1024


/* ICS_BUF_SIZE is the size of the buffer allocated to:
   - Do the compression. This is independent from the memory allocated by zlib
     for the dictionary.
   - Decompress stuff into when skipping a data block (IcsSetIdsBlock() for
     compressed files).
   - Copy data over from one IDS file to another when the ICS file is opened for
     updating. */
#define ICS_BUF_SIZE 16384


/* These are used internally when the precise length of a variable is needed.
   We also use size_t for a variable that is as wide as a pointer
   (i.e. can hold the size of any data block).
 */
#include <stdint.h>
typedef uint8_t  ics_t_uint8;
typedef int8_t   ics_t_sint8;
typedef uint16_t ics_t_uint16;
typedef int16_t  ics_t_sint16;
typedef uint32_t ics_t_uint32;
typedef int32_t  ics_t_sint32;
typedef float ics_t_real32;
typedef double ics_t_real64;


#undef ICS_USING_CONFIGURE
#if !defined(ICS_USING_CONFIGURE)

/*********************************************************************
 *** If we are not using the autoconf configure script:
 *********************************************************************/

/* If ICS_FORCE_C_LOCALE is set, the locale is set to "C" before each read or
   write operation. This ensures that the ICS header file is formatted
   properly. If your program does not modify the locale, you can safely comment
   out is line: the "C" locale is the default. If this constant is not defined
   and your program changes the locale, the resulting ICS header file might not
   be readable by other applications: it will only be readable if the reading
   application is set to the same locale as the writing application.  The ICS
   standard calls for the locale to be set to "C". */
#define ICS_FORCE_C_LOCALE


/* If ICS_DO_GZEXT is defined, the code will search for IDS files which have the
   .ids.gz or .ids.Z filename extension. */
#define ICS_DO_GZEXT


/* If ICS_ZLIB is defined, the zlib dependency is included, and the library will
   be able to read GZIP compressed files.  This variable is set by the makefile
   -- enable ZLIB support there. */
/*#define ICS_ZLIB*/


#else

/*********************************************************************
 *** If we are using the autoconf configure script:
 *********************************************************************/

/* If we should force the c locale. */
#undef ICS_FORCE_C_LOCALE

/* Whether to search for IDS files with .ids.gz or .ids.Z extension. */
#undef ICS_DO_GZEXT

/* Whether to use zlib compression. */
#undef ICS_ZLIB

#endif

#endif
