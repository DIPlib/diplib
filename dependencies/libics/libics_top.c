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
 * Library General Public License for more details.`
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * FILE : libics_top.c
 *
 * The following library functions are contained in this file:
 *
 *   IcsOpen()
 *   IcsClose()
 *   IcsGetLayout()
 *   IcsSetLayout()
 *   IcsGetDataSize()
 *   IcsGetImelSize()
 *   IcsGetImageSize()
 *   IcsGetData()
 *   IcsGetDataBlock()
 *   IcsSkipDataBlock()
 *   IcsGetROIData()
 *   IcsGetDataWithStrides()
 *   IcsSetData()
 *   IcsSetDataWithStrides()
 *   IcsSetSource()
 *   IcsSetCompression()
 *   IcsGetPosition()
 *   IcsGetPositionF()
 *   IcsSetPosition()
 *   IcsGetOrder()
 *   IcsGetOrderF()
 *   IcsSetOrder()
 *   IcsGetCoordinateSystem()
 *   IcsSetCoordinateSystem()
 *   IcsGetSignificantBits()
 *   IcsSetSignificantBits()
 *   IcsGetImelUnits()
 *   IcsGetImelUnitsF()
 *   IcsSetImelUnits()
 *   IcsGetScilType()
 *   IcsSetScilType()
 *   IcsGuessScilType()
 *   IcsGetErrorText()
 */


#include <stdlib.h>
#include <string.h>
#include "libics_intern.h"


/* Default Order and Label strings: */
char const* ICSKEY_ORDER[] = {"x", "y", "z", "t", "probe"};
const char * ICSKEY_LABEL[] = {"x-position", "y-position", "z-position",
                               "time", "probe"};
#define ICSKEY_ORDER_LENGTH 5 /* Number of elements in ICSKEY_ORDER and
                                 ICSKEY_LABEL arrays. */


/* Create an ICS structure, and read the stuff from file if reading. */
Ics_Error IcsOpen(ICS        **ics,
                  const char  *filename,
                  const char  *mode)
{
    ICSINIT;
    int    version = 0, forceName = 0, forceLocale = 1, reading = 0;
    int    writing = 0;
    size_t i;


        /* the mode string is one of: "r", "w", "rw", with "f" and/or "l"
           appended for reading and "1" or "2" appended for writing */
    for (i = 0; i<strlen(mode); i++) {
        switch (mode[i]) {
            case 'r':
                if (reading) return IcsErr_IllParameter;
                reading = 1;
                break;
            case 'w':
                if (writing) return IcsErr_IllParameter;
                writing = 1;
                break;
            case 'f':
                if (forceName) return IcsErr_IllParameter;
                forceName = 1;
                break;
            case 'l':
                if (forceLocale) return IcsErr_IllParameter;
                forceLocale = 0;
                break;
            case '1':
                if (version!=0) return IcsErr_IllParameter;
                version = 1;
                break;
            case '2':
                if (version!=0) return IcsErr_IllParameter;
                version = 2;
                break;
            default:
                return IcsErr_IllParameter;
        }
    }
    *ics =(ICS*)malloc(sizeof(ICS));
    if (*ics == NULL) return IcsErr_Alloc;
    if (reading) {
            /* We're reading or updating */
        error = IcsReadIcs(*ics, filename, forceName, forceLocale);
        if (error) {
            free(*ics);
            *ics = NULL;
        } else {
            if (writing) {
                    /* We're updating */
                (*ics)->fileMode = IcsFileMode_update;
            } else {
                    /* We're just reading */
                (*ics)->fileMode = IcsFileMode_read;
            }
        }
    } else if (writing) {
            /* We're writing */
        IcsInit(*ics);
        (*ics)->fileMode = IcsFileMode_write;
        if (version) {
            (*ics)->version = version;
        }
        IcsStrCpy((*ics)->filename, filename, ICS_MAXPATHLEN);
    } else {
            /* Missing an "r" or "w" mode character */
        return IcsErr_IllParameter;
    }

    return error;
}


/* Free the ICS structure, and write the stuff to file if writing. */
Ics_Error IcsClose(ICS *ics)
{
    ICSINIT;
    char filename[ICS_MAXPATHLEN+4];


    if (ics == NULL) return IcsErr_NotValidAction;
    if (ics->fileMode == IcsFileMode_read) {
            /* We're reading */
        if (ics->blockRead != NULL) {
            error = IcsCloseIds(ics);
        }
    } else if (ics->fileMode == IcsFileMode_write) {
            /* We're writing */
        error = IcsWriteIcs(ics, NULL);
        if (!error) error = IcsWriteIds(ics);
    } else {
            /* We're updating */
        int needcopy = 0;
        if (ics->blockRead != NULL) {
            error = IcsCloseIds(ics);
        }
        if (ics->version == 2 && !strcmp(ics->srcFile, ics->filename)) {
                /* The ICS file contains the data */
            needcopy = 1;
            ics->srcFile[0] = '\0'; /* needed to get the END keyword in the
                                       header */
                /* Rename the original file */
            strcpy(filename, ics->filename);
            strcat(filename, ".tmp");
            if (rename(ics->filename, filename)) {
                error = IcsErr_FTempMoveIcs;
            }
        }
        if (!error) error = IcsWriteIcs(ics, NULL);
        if (!error && needcopy) {
                /* Copy the data over from the original file */
            error = IcsCopyIds(filename, ics->srcOffset, ics->filename);
                /* Delete original file */
            if (!error) {
                remove(filename);
            }
        }
        if (error) {
                /* Let's try copying the old file back */
            remove(ics->filename);
            rename(filename, ics->filename);
        }
    }
    IcsFreeHistory(ics);
    free(ics);

    return error;
}


/* Get the layout parameters from the ICS structure. */
Ics_Error IcsGetLayout(const ICS    *ics,
                       Ics_DataType *dt,
                       int          *nDims,
                       size_t       *dims)
{
    ICSINIT;
    int i;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_write))
        return IcsErr_NotValidAction;

    *nDims = ics->dimensions;
    *dt = ics->imel.dataType;
        /* Get the image sizes. Ignore the orders */
    for (i = 0; i < *nDims; i++) {
        dims[i] = ics->dim[i].size;
    }

    return error;
}


/* Put the layout parameters in the ICS structure. */
Ics_Error IcsSetLayout(ICS          *ics,
                       Ics_DataType  dt,
                       int           nDims,
                       const size_t *dims)
{
    ICSINIT;
    int i;


    if ((ics == NULL) || (ics->fileMode != IcsFileMode_write))
        return IcsErr_NotValidAction;

    if (nDims > ICS_MAXDIM) return IcsErr_TooManyDims;
        /* Set the pixel parameters */
    ics->imel.dataType = dt;
        /* Set the image sizes */
    for (i=0; i<nDims; i++) {
        ics->dim[i].size = dims[i];
        if (i < ICSKEY_ORDER_LENGTH) {
            strcpy(ics->dim[i].order, ICSKEY_ORDER[i]);
            strcpy(ics->dim[i].label, ICSKEY_LABEL[i]);
        } else {
                /* Could overflow: */
            snprintf(ics->dim[i].order, ICS_STRLEN_TOKEN, "dim_%d", i);
            snprintf(ics->dim[i].label, ICS_STRLEN_TOKEN, "dim_%d", i);
        }
    }
    ics->dimensions = nDims;

    return error;
}


/* Get the image size in bytes. */
size_t IcsGetDataSize(const ICS *ics)
{
    if (ics == NULL) return 0;
    if (ics->dimensions == 0) return 0;
    return IcsGetImageSize(ics) * IcsGetBytesPerSample(ics);
}


/* Get the pixel size in bytes. */
size_t IcsGetImelSize(const ICS *ics)
{
    if (ics != NULL) {
        return IcsGetBytesPerSample(ics);
    } else {
        return 0;
    }
}


/* Get the image size in pixels. */
size_t IcsGetImageSize(const ICS *ics)
{
    int    i;
    size_t size = 1;


    if (ics == NULL) return 0;
    if (ics->dimensions == 0) return 0;
    for (i = 0; i < ics->dimensions; i++) {
        size *= ics->dim[i].size;
    }

    return size;
}


/* Get the image data. It is read from the file right here. */
Ics_Error IcsGetData(ICS    *ics,
                     void   *dest,
                     size_t  n)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_write))
        return IcsErr_NotValidAction;

    if ((n != 0) &&(dest != NULL)) {
        error = IcsReadIds(ics, dest, n);
    }

    return error;
}


/* Read a portion of the image data from an ICS file. */
Ics_Error IcsGetDataBlock(ICS    *ics,
                          void   *dest,
                          size_t  n)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_write))
        return IcsErr_NotValidAction;

    if ((n != 0) &&(dest != NULL)) {
        if (ics->blockRead == NULL) {
            error = IcsOpenIds(ics);
        }
        if (!error) error = IcsReadIdsBlock(ics, dest, n);
    }

    return error;
}


/* Skip a portion of the image from an ICS file. */
Ics_Error IcsSkipDataBlock(ICS    *ics,
                           size_t  n)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_write))
        return IcsErr_NotValidAction;

    if (n != 0) {
        if (ics->blockRead == NULL) {
            error = IcsOpenIds(ics);
        }
        if (!error) error = IcsSkipIdsBlock(ics, n);
    }

    return error;
}


/* Read a square region of the image from an ICS file. */
Ics_Error IcsGetROIData(ICS          *ics,
                        const size_t *offsetPtr,
                        const size_t *sizePtr,
                        const size_t *samplingPtr,
                        void         *destPtr,
                        size_t        n)
{
    ICSINIT;
    int           i, sizeConflict = 0, p;
    size_t        j;
    size_t        imelSize, roiSize, curLoc, newLoc, bufSize;
    size_t        curPos[ICS_MAXDIM];
    size_t        stride[ICS_MAXDIM];
    size_t        bOffset[ICS_MAXDIM];
    size_t        bSize[ICS_MAXDIM];
    size_t        bSampling[ICS_MAXDIM];
    const size_t *offset, *size, *sampling;
    char         *buf;
    char         *dest            = (char*)destPtr;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_write))
        return IcsErr_NotValidAction;

    if ((n == 0) ||(dest == NULL)) return IcsErr_Ok;
    p = ics->dimensions;
    if (offsetPtr != NULL) {
        offset = offsetPtr;
    } else {
        for (i = 0; i < p; i++) {
            bOffset[i] = 0;
        }
        offset = bOffset;
    }
    if (sizePtr != NULL) {
        size = sizePtr;
    } else {
        for (i = 0; i < p; i++) {
            bSize[i] = ics->dim[i].size - offset[i];
        }
        size = bSize;
    }
    if (samplingPtr != NULL) {
        sampling = samplingPtr;
    } else {
        for (i = 0; i < p; i++) {
            bSampling[i] = 1;
        }
        sampling = bSampling;
    }
    for (i = 0; i < p; i++) {
        if (sampling[i] < 1 || offset[i] + size[i] > ics->dim[i].size)
            return IcsErr_IllegalROI;
    }
    imelSize = IcsGetBytesPerSample(ics);
    roiSize = imelSize;
    for (i = 0; i < p; i++) {
        roiSize *= (size[i] + sampling[i] - 1) / sampling[i];
    }
    if (n != roiSize) {
        sizeConflict = 1;
        if (n < roiSize) return IcsErr_BufferTooSmall;
    }
        /* The stride array tells us how many imels to skip to go the next pixel
           in each dimension */
    stride[0] = 1;
    for (i = 1; i < p; i++) {
        stride[i] = stride[i - 1] * ics->dim[i - 1].size;
    }
    error = IcsOpenIds(ics);
    if (error) return error;
    bufSize = imelSize*size[0];
    if (sampling[0] > 1) {
            /* We read a line in a buffer, and then copy the needed imels to
               dest */
        buf =(char*)malloc(bufSize);
        if (buf == NULL) return IcsErr_Alloc;
        curLoc = 0;
        for (i = 0; i < p; i++) {
            curPos[i] = offset[i];
        }
        while (1) {
            newLoc = 0;
            for (i = 0; i < p; i++) {
                newLoc += curPos[i] * stride[i];
            }
            newLoc *= imelSize;
            if (curLoc < newLoc) {
                error = IcsSkipIdsBlock(ics, newLoc - curLoc);
                curLoc = newLoc;
            }
            if (!error) error = IcsReadIdsBlock(ics, buf, bufSize);
            if (error != IcsErr_Ok) {
                break; /* stop reading on error */
            }
            curLoc += bufSize;
            for (j=0; j < size[0]; j += sampling[0]) {
                memcpy(dest, buf + i * imelSize, imelSize);
                dest += imelSize;
            }
            for (i = 1; i < p; i++) {
                curPos[i] += sampling[i];
                if (curPos[i] < offset[i] + size[i]) {
                    break;
                }
                curPos[i] = offset[i];
            }
            if (i==p) {
                break; /* we're done reading */
            }
        }
        free(buf);
    } else {
            /* No subsampling in dim[0] required: read directly into dest */
        curLoc = 0;
        for (i = 0; i < p; i++) {
            curPos[i] = offset[i];
        }
        while (1) {
            newLoc = 0;
            for (i = 0; i < p; i++) {
                newLoc += curPos[i] * stride[i];
            }
            newLoc *= imelSize;
            if (curLoc < newLoc) {
                error = IcsSkipIdsBlock(ics, newLoc - curLoc);
                curLoc = newLoc;
            }
            if (!error) error = IcsReadIdsBlock(ics, dest, bufSize);
            if (error != IcsErr_Ok) {
                break; /* stop reading on error */
            }
            curLoc += bufSize;
            dest += bufSize;
            for (i = 1; i < p; i++) {
                curPos[i] += sampling[i];
                if (curPos[i] < offset[i] + size[i]) {
                    break;
                }
                curPos[i] = offset[i];
            }
            if (i==p) {
                break; /* we're done reading */
            }
        }
    }
    if (error)
        IcsCloseIds(ics);
    else
        error = IcsCloseIds(ics);

    if ((error == IcsErr_Ok) && sizeConflict) {
        error = IcsErr_OutputNotFilled;
    }
    return error;
}


/* Read the image data into a region of your buffer. */
Ics_Error IcsGetDataWithStrides(ICS          *ics,
                                void         *destPtr,
                                size_t        n,
                                const size_t *stridePtr,
                                int           nDims)
{
    ICSINIT;
    int           i, p;
    size_t        j;
    size_t        imelSize, lastpixel, bufSize;
    size_t        curPos[ICS_MAXDIM];
    size_t        b_stride[ICS_MAXDIM];
    size_t const *stride;
    char         *buf;
    char         *dest = (char*)destPtr;
    char         *out;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_write))
        return IcsErr_NotValidAction;

    if ((n == 0) ||(dest == NULL)) return IcsErr_Ok;
    p = ics->dimensions;
    if (nDims != p) return IcsErr_IllParameter;
    if (stridePtr != NULL) {
        stride = stridePtr;
    } else {
        b_stride[0] = 1;
        for (i = 1; i < p; i++) {
            b_stride[i] = b_stride[i - 1] * ics->dim[i - 1].size;
        }
        stride = b_stride;
    }
    imelSize = IcsGetBytesPerSample(ics);
    lastpixel = 0;
    for (i = 0; i < p; i++) {
        lastpixel +=(ics->dim[i].size - 1) * stride[i];
    }
    if (lastpixel * imelSize > n) return IcsErr_IllParameter;

    error = IcsOpenIds(ics);
    if (error) return error;
    bufSize = imelSize*ics->dim[0].size;
    if (stride[0] > 1) {
            /* We read a line in a buffer, and then copy the imels to dest */
        buf =(char*)malloc(bufSize);
        if (buf == NULL) return IcsErr_Alloc;
        for (i = 0; i < p; i++) {
            curPos[i] = 0;
        }
        while (1) {
            out = dest;
            for (i = 1; i < p; i++) {
                out += curPos[i] * stride[i] * imelSize;
            }
            if (!error) error = IcsReadIdsBlock(ics, buf, bufSize);
            if (error != IcsErr_Ok) {
                break; /* stop reading on error */
            }
            for (j = 0; j < ics->dim[0].size; j++) {
                memcpy(out, buf + j * imelSize, imelSize);
                out += stride[0]*imelSize;
            }
            for (i = 1; i < p; i++) {
                curPos[i]++;
                if (curPos[i] < ics->dim[i].size) {
                    break;
                }
                curPos[i] = 0;
            }
            if (i==p) {
                break; /* we're done reading */
            }
        }
        free(buf);
    } else {
            /* No subsampling in dim[0] required: read directly into dest */
        for (i = 0; i < p; i++) {
            curPos[i] = 0;
        }
        while (1) {
            out = dest;
            for (i = 1; i < p; i++) {
                out += curPos[i] * stride[i] * imelSize;
            }
            if (!error) error = IcsReadIdsBlock(ics, out, bufSize);
            if (error != IcsErr_Ok) {
                break; /* stop reading on error */
            }
            for (i = 1; i < p; i++) {
                curPos[i]++;
                if (curPos[i] < ics->dim[i].size) {
                    break;
                }
                curPos[i] = 0;
            }
            if (i==p) {
                break; /* we're done reading */
            }
        }
    }
    if (error)
        IcsCloseIds(ics);
    else
        error = IcsCloseIds(ics);

    return error;
}


/* Set the image data. The pointer must be valid until IcsClose() is called. */
Ics_Error IcsSetData(ICS        *ics,
                     const void *src,
                     size_t      n)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode != IcsFileMode_write))
        return IcsErr_NotValidAction;

    if (ics->srcFile[0] != '\0') return IcsErr_DuplicateData;
    if (ics->data != NULL) return IcsErr_DuplicateData;
    if (ics->dimensions == 0) return IcsErr_NoLayout;
    if (n != IcsGetDataSize(ics)) {
        error = IcsErr_FSizeConflict;
    }
    ics->data = src;
    ics->dataLength = n;
    ics->dataStrides = NULL;

    return error;
}


/* Set the image data. The pointers must be valid until IcsClose() is
   called. The strides indicate how to go to the next neighbor along each
   dimension. Use this is your image data is not in one contiguous block or you
   want to swap some dimensions in the file. nDims is the length of the strides
   array and should match the dimensionality previously given. */
Ics_Error IcsSetDataWithStrides(ICS          *ics,
                                const void   *src,
                                size_t        n,
                                const size_t *strides,
                                int           nDims)
{
    ICSINIT;
    size_t lastpixel;
    int    i;


    if ((ics == NULL) || (ics->fileMode != IcsFileMode_write))
        return IcsErr_NotValidAction;

    if (ics->srcFile[0] != '\0') return IcsErr_DuplicateData;
    if (ics->data != NULL) return IcsErr_DuplicateData;
    if (ics->dimensions == 0) return IcsErr_NoLayout;
    if (nDims != ics->dimensions) return IcsErr_IllParameter;
    lastpixel = 0;
    for (i = 0; i < nDims; i++) {
        lastpixel +=(ics->dim[i].size-1) * strides[i];
    }
    if (lastpixel * IcsGetDataTypeSize(ics->imel.dataType) > n)
        return IcsErr_IllParameter;
    if (n != IcsGetDataSize(ics)) {
        error = IcsErr_FSizeConflict;
    }
    ics->data = src;
    ics->dataLength = n;
    ics->dataStrides = strides;

    return error;
}


/* Set the image data source file. */
Ics_Error IcsSetSource(ICS        *ics,
                       const char *fname,
                       size_t      offset)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode != IcsFileMode_write))
        return IcsErr_NotValidAction;

    if (ics->version == 1) return IcsErr_NotValidAction;
    if (ics->srcFile[0] != '\0') return IcsErr_DuplicateData;
    if (ics->data != NULL) return IcsErr_DuplicateData;
    IcsStrCpy(ics->srcFile, fname, ICS_MAXPATHLEN);
    ics->srcOffset = offset;

    return error;
}


/* Set the compression method and compression parameter. */
Ics_Error IcsSetCompression(ICS             *ics,
                            Ics_Compression  compression,
                            int              level)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode != IcsFileMode_write))
        return IcsErr_NotValidAction;

    if (compression == IcsCompr_compress)
        compression = IcsCompr_gzip; /* don't try writing 'compress' compressed
                                        data. */
    ics->compression = compression;
    ics->compLevel = level;

    return error;
}


/* Get the position of the image in the real world: the origin of the first
   pixel, the distances between pixels and the units in which to measure. If you
   are not interested in one of the parameters, set the pointer to
   NULL. Dimensions start at 0. */
Ics_Error IcsGetPosition(const ICS *ics,
                         int        dimension,
                         double    *origin,
                         double    *scale,
                         char      *units)
{
    ICSINIT;
    const char* ptr;

    error = IcsGetPositionF(ics, dimension, origin, scale, &ptr);
    if (!error) {
        if (units) {
            strcpy(units, ptr);
        }
    }

    return error;
}

/* Idem, but without copying the strings. Output pointer `units` set to internal
   buffer, which will be valid until IcsClose is called. */
Ics_Error IcsGetPositionF(const ICS   *ics,
                          int          dimension,
                          double      *origin,
                          double      *scale,
                          const char **units)
{
    ICSINIT;


    if (ics == NULL) return IcsErr_NotValidAction;

    if (dimension >= ics->dimensions) return IcsErr_NotValidAction;

    if (origin) {
        *origin = ics->dim[dimension].origin;
    }
    if (scale) {
        *scale = ics->dim[dimension].scale;
    }
    if (units) {
        if (ics->dim[dimension].unit[0] != '\0') {
            *units = ics->dim[dimension].unit;
        } else {
            *units = ICS_UNITS_UNDEFINED;
        }
    }

    return error;
}

/* Set the position of the image in the real world: the origin of the first
   pixel, the distances between pixels and the units in which to measure. If
   units is NULL or empty, it is set to the default value of
   "undefined". Dimensions start at 0. */
Ics_Error IcsSetPosition(ICS        *ics,
                         int         dimension,
                         double      origin,
                         double      scale,
                         const char *units)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (dimension >= ics->dimensions) return IcsErr_NotValidAction;
    ics->dim[dimension].origin = origin;
    ics->dim[dimension].scale = scale;
    if (units &&(units[0] != '\0')) {
        IcsStrCpy(ics->dim[dimension].unit, units, ICS_STRLEN_TOKEN);
    } else {
        strcpy(ics->dim[dimension].unit, ICS_UNITS_UNDEFINED);
    }

    return error;
}


/* Get the ordering of the dimensions in the image. The ordering is defined by
   names and labels for each dimension. The defaults are x, y, z, t(time) and
   probe. Dimensions start at 0. */
Ics_Error IcsGetOrder(const ICS *ics,
                      int        dimension,
                      char      *order,
                      char      *label)
{
    ICSINIT;
    const char *order_ptr;
    const char *label_ptr;

    error = IcsGetOrderF(ics, dimension, &order_ptr, &label_ptr);
    if (!error) {
        if (order) {
            strcpy(order, order_ptr);
        }
        if (label) {
            strcpy(label, label_ptr);
        }
    }

    return error;
}

/* Idem, but without copying the strings. Output pointers `order` and `label` set
   to internal buffer, which will be valid until IcsClose is called. */
Ics_Error IcsGetOrderF(const ICS   *ics,
                       int          dimension,
                       const char **order,
                       const char **label)
{
    ICSINIT;


    if (ics == NULL) return IcsErr_NotValidAction;

    if (dimension >= ics->dimensions) return IcsErr_NotValidAction;
    if (order) {
        *order = ics->dim[dimension].order;
    }
    if (label) {
        *label = ics->dim[dimension].label;
    }

    return error;
}


/* Set the ordering of the dimensions in the image. The ordering is defined by
   providing names and labels for each dimension. The defaults are x, y, z, t
  (time) and probe. Dimensions start at 0. */
Ics_Error IcsSetOrder(ICS        *ics,
                      int         dimension,
                      const char *order,
                      const char *label)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (dimension >= ics->dimensions) return IcsErr_NotValidAction;
    if (order &&(order[0] != '\0')) {
        IcsStrCpy(ics->dim[dimension].order, order, ICS_STRLEN_TOKEN);
        if (label &&(label[0] != '\0')) {
            IcsStrCpy(ics->dim[dimension].label, label, ICS_STRLEN_TOKEN);
        } else {
            IcsStrCpy(ics->dim[dimension].label, order, ICS_STRLEN_TOKEN);
        }
    } else {
        if (label &&(label[0] != '\0')) {
            IcsStrCpy(ics->dim[dimension].label, label, ICS_STRLEN_TOKEN);
        } else {
            error = IcsErr_NotValidAction;
        }
    }

    return error;
}


/* Get the coordinate system used in the positioning of the pixels. Related to
   IcsGetPosition(). The default is "video". */
Ics_Error IcsGetCoordinateSystem(const ICS *ics,
                                 char      *coord)
{
    ICSINIT;


    if (ics == NULL) return IcsErr_NotValidAction;

    if (coord == NULL) return IcsErr_NotValidAction;
    if (ics->coord[0] != '\0') {
        strcpy(coord, ics->coord);
    } else {
        strcpy(coord, ICS_COORD_VIDEO);
    }

    return error;
}


/* Set the coordinate system used in the positioning of the pixels. Related to
   IcsSetPosition(). The default is "video". */
Ics_Error IcsSetCoordinateSystem(ICS        *ics,
                                 const char *coord)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (coord &&(coord[0] != '\0')) {
        IcsStrCpy(ics->coord, coord, ICS_STRLEN_TOKEN);
    } else {
        strcpy(ics->coord, ICS_COORD_VIDEO);
    }

    return error;
}


/* Get the number of significant bits. */
Ics_Error IcsGetSignificantBits(const ICS *ics,
                                size_t    *nbits)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_write))
        return IcsErr_NotValidAction;

    if (nbits == NULL) return IcsErr_NotValidAction;
    *nbits = ics->imel.sigBits;

    return error;
}


/* Set the number of significant bits. */
Ics_Error IcsSetSignificantBits(ICS    *ics,
                                size_t  nbits)
{
    ICSINIT;
    size_t maxbits = IcsGetDataTypeSize(ics->imel.dataType) * 8;

    if ((ics == NULL) || (ics->fileMode != IcsFileMode_write))
        return IcsErr_NotValidAction;

    if (ics->dimensions == 0) return IcsErr_NoLayout;
    if (nbits > maxbits) {
        nbits = maxbits;
    }
    ics->imel.sigBits = nbits;

    return error;
}


/* Set the position of the pixel values: the offset and scaling, and the units
   in which to measure. If you are not interested in one of the parameters, set
   the pointer to NULL. */
Ics_Error IcsGetImelUnits(const ICS *ics,
                          double    *origin,
                          double    *scale,
                          char      *units)
{
    ICSINIT;
    const char* ptr;

    error = IcsGetImelUnitsF(ics, origin, scale, &ptr);
    if (!error) {
        if (units) {
            strcpy(units, ptr);
        }
    }

    return error;
}

/* Idem, but without copying the strings. Output pointer `units` set to internal
   buffer, which will be valid until IcsClose is called. */
Ics_Error IcsGetImelUnitsF(const ICS   *ics,
                           double      *origin,
                           double      *scale,
                           const char **units)
{
    ICSINIT;


    if (ics == NULL) return IcsErr_NotValidAction;

    if (origin) {
        *origin = ics->imel.origin;
    }
    if (scale) {
        *scale = ics->imel.scale;
    }
    if (units) {
        if (ics->imel.unit[0] != '\0') {
            *units = ics->imel.unit;
        } else {
            *units = ICS_UNITS_RELATIVE;
        }
    }

    return error;
}


/* Set the position of the pixel values: the offset and scaling, and the units
   in which to measure. If units is NULL or empty, it is set to the default
   value of "relative". */
Ics_Error IcsSetImelUnits(ICS        *ics,
                          double      origin,
                          double      scale,
                          const char *units)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    ics->imel.origin = origin;
    ics->imel.scale = scale;
    if (units &&(units[0] != '\0')) {
        IcsStrCpy(ics->imel.unit, units, ICS_STRLEN_TOKEN);
    } else {
        strcpy(ics->imel.unit, ICS_UNITS_RELATIVE);
    }

    return error;
}


/* Get the string for the SCIL_TYPE parameter. This string is used only by
   SCIL_Image. */
Ics_Error IcsGetScilType(const ICS *ics,
                         char      *sciltype)
{
    ICSINIT;


    if (ics == NULL) return IcsErr_NotValidAction;

    if (sciltype == NULL) return IcsErr_NotValidAction;
    strcpy(sciltype, ics->scilType);

    return error;
}


/* Set the string for the SCIL_TYPE parameter. This string is used only by
   SCIL_Image. It is required if you want to read the image using SCIL_Image. */
Ics_Error IcsSetScilType(ICS        *ics,
                         const char *sciltype)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    IcsStrCpy(ics->scilType, sciltype, ICS_STRLEN_TOKEN);

    return error;
}


/* As IcsSetScilType, but creates a string according to the DataType in the ICS
   structure. It can create a string for g2d, g3d, f2d, f3d, c2d and c3d. */
Ics_Error IcsGuessScilType(ICS *ics)
{
    ICSINIT;


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    switch (ics->imel.dataType) {
        case Ics_uint8:
        case Ics_sint8:
        case Ics_uint16:
        case Ics_sint16:
            ics->scilType[0] = 'g';
            break;
        case Ics_real32:
            ics->scilType[0] = 'f';
            break;
        case Ics_complex32:
            ics->scilType[0] = 'c';
            break;
        case Ics_uint32:
        case Ics_sint32:
        case Ics_real64:
        case Ics_complex64:
            return IcsErr_NoScilType;
        case Ics_unknown:
        default:
            ics->scilType[0] = '\0';
            return IcsErr_NotValidAction;
    }
    if (ics->dimensions == 3) {
        ics->scilType[1] = '3';
    } else if (ics->dimensions > 3) {
        ics->scilType[0] = '\0';
        error = IcsErr_NoScilType;
    } else {
        ics->scilType[1] = '2';
    }
    ics->scilType[2] = 'd';
    ics->scilType[3] = '\0';

    return error;
}


/* Returns a textual description of the error code. */
const char *IcsGetErrorText(Ics_Error error)
{
    const char *msg;

    switch (error) {
        case IcsErr_Ok:
            msg = "A-OK";
            break;
        case IcsErr_FSizeConflict:
            msg = "Non fatal error: unexpected data size";
            break;
        case IcsErr_OutputNotFilled:
            msg = "Non fatal error: the output buffer could not be completely "
                "filled";
            break;
        case IcsErr_Alloc:
            msg = "Memory allocation error";
            break;
        case IcsErr_BitsVsSizeConfl:
            msg = "Image size conflicts with bits per element";
            break;
        case IcsErr_BlockNotAllowed:
            msg = "It is not possible to read COMPRESS-compressed data in "
                "blocks";
            break;
        case IcsErr_BufferTooSmall:
            msg = "The buffer was too small to hold the given ROI";
            break;
        case IcsErr_CompressionProblem:
            msg = "Some error occurred during compression";
            break;
        case IcsErr_CorruptedStream:
            msg = "The compressed input stream is corrupted";
            break;
        case IcsErr_DecompressionProblem:
            msg = "Some error occurred during decompression";
            break;
        case IcsErr_DuplicateData:
            msg = "The ICS data structure already contains incompatible stuff";
            break;
        case IcsErr_EmptyField:
            msg = "Empty field";
            break;
        case IcsErr_EndOfHistory:
            msg = "All history lines have already been returned";
            break;
        case IcsErr_EndOfStream:
            msg = "Unexpected end of stream";
            break;
        case IcsErr_FailWriteLine:
            msg = "Failed to write a line in .ics file";
            break;
        case IcsErr_FCloseIcs:
            msg = "File close error on .ics file";
            break;
        case IcsErr_FCloseIds:
            msg = "File close error on .ids file";
            break;
        case IcsErr_FCopyIds:
            msg = "Failed to copy image data from temporary file on .ics file "
                "opened for updating";
            break;
        case IcsErr_FOpenIcs:
            msg = "File open error on .ics file";
            break;
        case IcsErr_FOpenIds:
            msg = "File open error on .ids file";
            break;
        case IcsErr_FReadIcs:
            msg = "File read error on .ics file";
            break;
        case IcsErr_FReadIds:
            msg = "File read error on .ids file";
            break;
        case IcsErr_FTempMoveIcs:
            msg = "Failed to rename .ics file opened for updating";
            break;
        case IcsErr_FWriteIcs:
            msg = "File write error on .ics file";
            break;
        case IcsErr_FWriteIds:
            msg = "File write error on .ids file";
            break;
        case IcsErr_IllegalROI:
            msg = "The given ROI extends outside the image";
            break;
        case IcsErr_IllIcsToken:
            msg = "Illegal ICS token detected";
            break;
        case IcsErr_IllParameter:
            msg = "A function parameter has a value that is not legal or does "
                "not match with a value previously given";
            break;
        case IcsErr_LineOverflow:
            msg = "Line overflow in .ics file";
            break;
        case IcsErr_MissBits:
            msg = "Missing \"bits\" element in .ics file";
            break;
        case IcsErr_MissCat:
            msg = "Missing main category";
            break;
        case IcsErr_MissingData:
            msg = "There is no Data defined";
            break;
        case IcsErr_MissLayoutSubCat:
            msg = "Missing layout subcategory";
            break;
        case IcsErr_MissParamSubCat:
            msg = "Missing parameter subcategory";
            break;
        case IcsErr_MissRepresSubCat:
            msg = "Missing representation subcategory";
            break;
        case IcsErr_MissSensorSubCat:
            msg = "Missing sensor subcategory";
            break;
        case IcsErr_MissSensorSubSubCat:
            msg = "Missing sensor subsubcategory";
            break;
        case IcsErr_MissSubCat:
            msg = "Missing sub category";
            break;
        case IcsErr_NoLayout:
            msg = "Layout parameters missing or not defined";
            break;
        case IcsErr_NoScilType:
            msg = "There doesn't exist a SCIL_TYPE value for this image";
            break;
        case IcsErr_NotIcsFile:
            msg = "Not an ICS file";
            break;
        case IcsErr_NotValidAction:
            msg = "The function won't work on the ICS given";
            break;
        case IcsErr_TooManyChans:
            msg = "Too many channels specified";
            break;
        case IcsErr_TooManyDims:
            msg = "Data has too many dimensions";
            break;
        case IcsErr_UnknownCompression:
            msg = "Unknown compression type";
            break;
        case IcsErr_UnknownDataType:
            msg = "The data type is not recognized";
            break;
        case IcsErr_UnknownSensorState:
            msg = "The state is not recognized";
            break;
        case IcsErr_WrongZlibVersion:
            msg = "libics is linking to a different version of zlib than used "
                "during compilation";
            break;
        default:
            msg = "Some error occurred I know nothing about.";
    }
    return msg;
}
