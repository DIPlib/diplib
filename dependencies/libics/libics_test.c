/*
 * libics: Image Cytometry Standard file reading and writing.
 *
 * Copyright 2015-2017:
 *   Scientific Volume Imaging Holding B.V.
 *   Laapersveld 63, 1213 VB Hilversum, The Netherlands
 *   https://www.svi.nl
 *
 * Copyright (C) 2000-2013, 2016 Cris Luengo and others
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
 * FILE : libics_test.c
 *
 * The following library functions are contained in this file:
 *
 *   IcsPrintIcs()
 *   IcsPrintError()
 */

#include <stdlib.h>
#include <stdio.h>
#include "libics_intern.h"
#include "libics_test.h"

void IcsPrintIcs (ICS const* ics)
{
   int p, ii;
   Ics_Format Format;
   int Sign;
   size_t Bits;
   char* s;

   IcsGetPropsDataType (ics->imel.dataType, &Format, &Sign, &Bits);
   p = ics->dimensions;
   printf ("Version: %d\n", ics->version);
   printf ("FileMode: %d\n", ics->fileMode);
   printf ("Filename: %s\n", ics->filename);
   printf ("SrcFile: %s\n", ics->srcFile);
   printf ("SrcOffset: %ld\n", (long int)ics->srcOffset);
   printf ("Data: %p\n", ics->data);
   printf ("DataLength: %ld\n", (long int)ics->dataLength);
   printf ("Parameters: %d\n", ics->dimensions+1);
   printf ("Order: bits ");
   for (ii=0; ii<p; ii++)
      printf ("%s ", ics->dim[ii].order);
   printf ("\n");
   printf ("Sizes: %d ", (int)Bits);
   for (ii=0; ii<p; ii++)
      printf ("%lu ", (unsigned long) ics->dim[ii].size);
   printf ("\n");
   printf ("Sigbits: %d\n", (int)ics->imel.sigBits);
   printf ("Origin: %f ", ics->imel.origin);
   for (ii=0; ii<p; ii++)
      printf ("%f ", ics->dim[ii].origin);
   printf ("\n");
   printf ("Scale: %f ", ics->imel.scale);
   for (ii=0; ii<p; ii++)
      printf ("%f ", ics->dim[ii].scale);
   printf ("\n");
   printf ("Labels: intensity ");
   for (ii=0; ii<p; ii++)
      printf ("%s ", ics->dim[ii].label);
   printf ("\n");
   printf ("Units: %s ", ics->imel.unit);
   for (ii=0; ii<p; ii++)
      printf ("%s ", ics->dim[ii].unit);
   printf ("\n");
   switch (Format) {
      case IcsForm_real:
         s = "real";
         break;
      case IcsForm_complex:
         s = "complex";
         break;
      default:
         s = "integer";
   }
   printf ("Format: %s\n", s);
   printf ("Sign: %s\n", Sign?"signed":"unsigned");
   printf ("SCIL_TYPE: %s\n", ics->scilType);
   printf ("Coordinates: %s\n", ics->coord);
   switch (ics->compression) {
      case IcsCompr_uncompressed:
         s = "uncompressed";
         break;
      case IcsCompr_compress:
         s = "compress";
         break;
      case IcsCompr_gzip:
         s = "gzip";
         break;
      default:
         s = "unknown";
   }
   printf ("Compression: %s (level %d)\n", s, ics->compLevel);
   printf ("Byteorder: ");
   for (ii=0; ii<ICS_MAX_IMEL_SIZE; ii++)
      if (ics->byteOrder[ii] != 0)
         printf ("%d ", ics->byteOrder[ii]);
      else
         break;
   printf ("\n");
   printf ("BlockRead: %p\n", ics->blockRead);
   if (ics->blockRead != NULL) {
      Ics_BlockRead* br = (Ics_BlockRead*)ics->blockRead;
      printf ("   DataFilePtr: %p\n", (void*)br->dataFilePtr);
#ifdef ICS_ZLIB
      printf ("   ZlibStream: %p\n", br->zlibStream);
      printf ("   ZlibInputBuffer: %p\n", br->zlibInputBuffer);
#endif
   }
   printf ("Sensor data: \n");
   printf ("   Sensor type:");
   for (ii=0; ii< ics->sensorChannels; ii++)
       printf(" %s", ics->type[ii]);
   printf("\n");
   printf ("   Sensor model: %s\n", ics->model);
   printf ("   SensorChannels: %d\n", ics->sensorChannels);
   printf ("   RefrInxMedium: %f\n", ics->refrInxMedium);
   printf ("   NumAperture: %f\n", ics->numAperture);
   printf ("   RefrInxLensMedium: %f\n", ics->refrInxLensMedium);
   printf ("   PinholeSpacing: %f\n", ics->pinholeSpacing);
   printf ("   PinholeRadius: ");
   for (ii = 0; ii < ICS_MAX_LAMBDA && ii < ics->sensorChannels; ++ii) {
      printf ("%f ", ics->pinholeRadius[ii]);
   }
   printf ("\n");
   printf ("   LambdaEx: ");
   for (ii = 0; ii < ICS_MAX_LAMBDA && ii < ics->sensorChannels; ++ii) {
      printf ("%f ", ics->lambdaEx[ii]);
   }
   printf ("\n");
   printf ("   LambdaEm: ");
   for (ii = 0; ii < ICS_MAX_LAMBDA && ii < ics->sensorChannels; ++ii) {
      printf ("%f ", ics->lambdaEm[ii]);
   }
   printf ("\n");
   printf ("   ExPhotonCnt: ");
   for (ii = 0; ii < ICS_MAX_LAMBDA && ii < ics->sensorChannels; ++ii) {
      printf ("%d ", ics->exPhotonCnt[ii]);
   }
   printf ("\n");
   printf ("History Lines:\n");
   if (ics->history != NULL) {
      Ics_History* hist = (Ics_History*)ics->history;
      for (ii = 0; ii < hist->nStr; ii++) {
         if (hist->strings[ii] != NULL) {
            printf ("   %s\n", hist->strings[ii]);
         }
      }
   }
}

void IcsPrintError (Ics_Error error)
{
    char const* msg;

    msg = IcsGetErrorText (error);
    printf ("libics error: %s.\n", msg);
}
