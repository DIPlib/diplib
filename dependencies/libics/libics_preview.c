/*
 * libics: Image Cytometry Standard file reading and writing.
 *
 * Copyright 2015-2017:
 *   Scientific Volume Imaging Holding B.V.
 *   Laapersveld 63, 1213 VB Hilversum, The Netherlands
 *   https://www.svi.nl
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
 * FILE : libics_preview.c
 *
 * The following library functions are contained in this file:
 *
 *   IcsLoadPreview()
 *   IcsGetPreviewData()
 */


#include <stdlib.h>
#include <math.h>
#include "libics_intern.h"


/* Read a plane out of an ICS file. The buffer is malloc'd, xsize and ysize are
   set to the image size. The data type is always uint8. You need to free() the
   data block when you're done. */
Ics_Error IcsLoadPreview(const char  *filename,
                         size_t       planeNumber,
                         void       **dest,
                         size_t      *xsize,
                         size_t      *ysize)
{
    ICSINIT;
    ICS    *ics;
    size_t  bufSize;
    size_t  xs, ys;
    void *  buf;


    error = IcsOpen (&ics, filename, "r");
    if (error) return error;
    xs = ics->dim[0].size;
    ys = ics->dim[1].size;
    bufSize = xs*ys;
    buf = malloc(bufSize);
    if (buf == NULL) return IcsErr_Alloc;
    error = IcsGetPreviewData(ics, buf, bufSize, planeNumber);
    if (error)
        IcsClose(ics);
    else
        error =IcsClose(ics);

    if (error == IcsErr_Ok) {
        *dest = buf;
        *xsize = xs;
        *ysize = ys;
    }
    else {
        free (buf);
    }
    return error;
}


/* Read a plane of the actual image data from an ICS file, and convert it to
   uint8. */
Ics_Error IcsGetPreviewData(ICS    *ics,
                            void   *dest,
                            size_t  n,
                            size_t  planeNumber)
{
    ICSINIT;
    void   *buf;
    size_t  bps, i, nPlanes, roiSize;
    int     j, sizeConflict = 0;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_write))
        return IcsErr_NotValidAction;

    if ((n == 0) || (dest == NULL)) return IcsErr_Ok;
    nPlanes = 1;
    for (j = 2; j< ics->dimensions; j++) {
        nPlanes *= ics->dim[j].size;
    }
    if (planeNumber > nPlanes) return IcsErr_IllegalROI;
    if (ics->blockRead != NULL) {
        error = IcsCloseIds(ics);
        if (error) return error;
    }
    error = IcsOpenIds(ics);
    if (error) return error;
    roiSize = ics->dim[0].size * ics->dim[1].size;
    if (n != roiSize) {
        sizeConflict = 1;
        if (n < roiSize) return IcsErr_BufferTooSmall;
    }
    bps = (size_t)IcsGetBytesPerSample(ics);
    if (bps > 1) {
        buf = malloc (roiSize * bps);
        if (buf == NULL) return IcsErr_Alloc;
    }
    else {
        buf = dest;
    }
    if (planeNumber > 0) {
        if (!error) error = IcsSkipIdsBlock (ics, planeNumber * roiSize*bps);
    }
    if (!error) error = IcsReadIdsBlock (ics, buf, roiSize*bps);
    if (!error) error = IcsCloseIds (ics);
    if (error != IcsErr_Ok &&
        error != IcsErr_FSizeConflict &&
        error != IcsErr_OutputNotFilled) {
        if (bps > 1) {
            free (buf);
        }
        return error;
    }
    switch (ics->imel.dataType) {
        case Ics_uint8:
        {
            ics_t_uint8 *in  = buf;
            ics_t_uint8 *out = dest;
            int          offset;
            double       gain;
            int          max = *in, min = *in;

            in++;
            for (i = 1; i < roiSize; i++, in++) {
                if (max < *in) max = *in;
                if (min > *in) min = *in;
            }
            in = buf;
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, in++, out++) {
                *out = (ics_t_uint8)((*in - offset) * gain);
            }
        }
        break;
        case Ics_sint8:
        {
            ics_t_sint8 *in  = buf;
            ics_t_uint8 *out = dest;
            int          offset;
            double       gain;
            int          max = *in, min = *in;

            in++;
            for (i = 1; i < roiSize; i++, in++) {
                if (max < *in) max = *in;
                if (min > *in) min = *in;
            }
            in = buf;
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, in++, out++) {
                *out = (ics_t_uint8)((*in - offset) * gain);
            }
        }
        break;
        case Ics_uint16:
        {
            ics_t_uint16 *in  = buf;
            ics_t_uint8  *out = dest;
            int           offset;
            double        gain;
            int           max = *in, min = *in;

            in++;
            for (i = 1; i < roiSize; i++, in++) {
                if (max < *in) max = *in;
                if (min > *in) min = *in;
            }
            in = buf;
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, in++, out++) {
                *out = (ics_t_uint8)((*in - offset) * gain);
            }
        }
        break;
        case Ics_sint16:
        {
            ics_t_sint16 *in  = buf;
            ics_t_uint8  *out = dest;
            int           offset;
            double        gain;
            int           max = *in, min = *in;

            in++;
            for (i = 1; i < roiSize; i++, in++) {
                if (max < *in) max = *in;
                if (min > *in) min = *in;
            }
            in = buf;
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, in++, out++) {
                *out = (ics_t_uint8)((*in - offset) * gain);
            }
        }
        break;
        case Ics_uint32:
        {
            ics_t_uint32 *in = buf;
            ics_t_uint8 *out = dest;
            ics_t_uint32 offset; double gain;
            ics_t_uint32 max = *in, min = *in;
            in++;
            for (i = 1; i < roiSize; i++, in++) {
                if (max < *in) max = *in;
                if (min > *in) min = *in;
            }
            in = buf;
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, in++, out++) {
                *out = (ics_t_uint8)((*in - offset) * gain);
            }
        }
        break;
        case Ics_sint32:
        {
            ics_t_sint32 *in  = buf;
            ics_t_uint8  *out = dest;
            int           offset;
            double        gain;
            int           max = *in, min = *in;

            in++;
            for (i = 1; i < roiSize; i++, in++) {
                if (max < *in) max = *in;
                if (min > *in) min = *in;
            }
            in = buf;
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, in++, out++) {
                *out = (ics_t_uint8)((*in - offset) * gain);
            }
        }
        break;
        case Ics_real32:
        {
            ics_t_real32 *in  = buf;
            ics_t_uint8  *out = dest;
            double        offset, gain;
            float         max = *in, min = *in;

            in++;
            for (i = 1; i < roiSize; i++, in++) {
                if (max < *in) max = *in;
                if (min > *in) min = *in;
            }
            in = buf;
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, in++, out++) {
                *out = (ics_t_uint8)((*in - offset) * gain);
            }
        }
        break;
        case Ics_real64:
        {
            ics_t_real64 *in  = buf;
            ics_t_uint8  *out = dest;
            double        offset, gain;
            double        max = *in, min = *in;

            in++;
            for (i = 1; i < roiSize; i++, in++) {
                if (max < *in) max = *in;
                if (min > *in) min = *in;
            }
            in = buf;
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, in++, out++) {
                *out = (ics_t_uint8)((*in - offset) * gain);
            }
        }
        break;
        case Ics_complex32:
        {
            ics_t_real32 *in  = buf;
            ics_t_uint8  *out = dest;
            double        offset, gain, mod;
            double        max, min;

            mod = *in * *in;
            in++;
            mod *= *in * *in;
            in++;
            max = mod; min = mod;
            for (i = 1; i < roiSize; i++) {
                mod = *in * *in;
                in++;
                mod *= *in * *in;
                in++;
                if (max < mod) max = mod;
                if (min > mod) min = mod;
            }
            in = buf;
            min = sqrt(min);
            max = sqrt(max);
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, out++) {
                mod = *in * *in;
                in++;
                mod *= *in * *in;
                in++;
                *out = (ics_t_uint8)((mod - offset) * gain);
            }
        }
        break;
        case Ics_complex64:
        {
            ics_t_real64 *in  = buf;
            ics_t_uint8  *out = dest;
            double        offset, gain, mod;
            double        max, min;

            mod = *in * *in;
            in++;
            mod *= *in * *in;
            in++;
            max = mod; min = mod;
            for (i = 1; i < roiSize; i++) {
                mod = *in * *in;
                in++;
                mod *= *in * *in;
                in++;
                if (max < mod) max = mod;
                if (min > mod) min = mod;
            }
            in = buf;
            min = sqrt(min);
            max = sqrt(max);
            offset = min;
            gain = 255.0 / (max - min);
            for (i = 0; i < roiSize; i++, out++) {
                mod = *in * *in;
                in++;
                mod *= *in * *in;
                in++;
                *out = (ics_t_uint8)((mod - offset) * gain);
            }
        }
        break;
        default:
            return IcsErr_UnknownDataType;
    }
    if (bps > 1) {
        free (buf);
    }

    if ((error == IcsErr_Ok) && sizeConflict) {
        error = IcsErr_OutputNotFilled;
    }
    return error;
}
