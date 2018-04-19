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
 * FILE : libics_binary.c
 *
 * The following library functions are contained in this file:
 *
 *   IcsWriteIds()
 *   IcsCopyIds()
 *   IcsOpenIds()
 *   IcsCloseIds()
 *   IcsReadIdsBlock()
 *   IcsSkipIdsBlock()
 *   IcsSetIdsBlock()
 *   IcsReadIds()
 *
 * The following internal functions are contained in this file:
 *
 *   IcsWritePlainWithStrides()
 *   IcsFillByteOrder()
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libics_intern.h"


/* Write uncompressed data, with strides. */
Ics_Error IcsWritePlainWithStrides(const void      *src,
                                   const size_t    *dim,
                                   const ptrdiff_t *stride,
                                   int              nDims,
                                   int              nBytes,
                                   FILE            *file)
{
    ICSINIT;
    size_t      curpos[ICS_MAXDIM];
    const char *data;
    int         i;
    size_t      j;


    for (i = 0; i < nDims; i++) {
        curpos[i] = 0;
    }
    while (1) {
        data = (char const*)src;
        for (i = 1; i < nDims; i++) {
            data += (ptrdiff_t)curpos[i] * stride[i] * nBytes;
        }
        if (stride[0] == 1) {
            if (fwrite(data, (size_t)nBytes, dim[0], file) != dim[0]) {
                return IcsErr_FWriteIds;
            }
        } else {
            for (j = 0; j < dim[0]; j++) {
                if (fwrite(data, (size_t)nBytes, 1, file) != 1) {
                    return IcsErr_FWriteIds;
                }
                data += stride[0] * nBytes;
         }
        }
        for (i = 1; i < nDims; i++) {
            curpos[i]++;
            if (curpos[i] < dim[i]) {
                break;
            }
            curpos[i] = 0;
        }
        if (i == nDims) {
            break;
        }
    }

    return error;
}


/* Write the data to an IDS file. */
Ics_Error IcsWriteIds(const Ics_Header *icsStruct)
{
    ICSINIT;
    FILE   *fp;
    char    filename[ICS_MAXPATHLEN];
    char    mode[3] = "wb";
    int     i;
    size_t  dim[ICS_MAXDIM];


    if (icsStruct->version == 1) {
        IcsGetIdsName(filename, icsStruct->filename);
    } else {
        if (icsStruct->srcFile[0] != '\0') return IcsErr_Ok;
            /* Do nothing: the data is in another file somewhere */
        IcsStrCpy(filename, icsStruct->filename, ICS_MAXPATHLEN);
        mode[0] = 'a'; /* Open for append */
    }
    if ((icsStruct->data == NULL) || (icsStruct->dataLength == 0))
        return IcsErr_MissingData;

    fp = IcsFOpen(filename, mode);
    if (fp == NULL) return IcsErr_FOpenIds;

    for (i=0; i<icsStruct->dimensions; i++) {
        dim[i] = icsStruct->dim[i].size;
    }
    switch (icsStruct->compression) {
        case IcsCompr_uncompressed:
            if (icsStruct->dataStrides) {
                size_t size = IcsGetDataTypeSize(icsStruct->imel.dataType);
                error = IcsWritePlainWithStrides(icsStruct->data, dim,
                                                 icsStruct->dataStrides,
                                                 icsStruct->dimensions,
                                                 (int)size, fp);
            } else {
                    /* We do the writing in blocks if the data is very large,
                       this avoids a bug in some c library implementations on
                       windows. */
                size_t n = icsStruct->dataLength;
                const size_t nwrite = 1024 * 1024 * 1024;
                char *p = (char*)icsStruct->data;
                while (error == IcsErr_Ok && n > 0) {
                    if (n >= nwrite) {
                        if (fwrite(p, 1, nwrite, fp) != nwrite) {
                            error = IcsErr_FWriteIds;
                        }
                        n -= nwrite;
                        p += nwrite;
                    } else {
                        if (fwrite(p, 1, n, fp) != n) {
                            error = IcsErr_FWriteIds;
                        }
                        n = 0;
                    }
                }
            }
            break;
#ifdef ICS_ZLIB
        case IcsCompr_gzip:
            if (icsStruct->dataStrides) {
                size_t size = IcsGetDataTypeSize(icsStruct->imel.dataType);
                error = IcsWriteZipWithStrides(icsStruct->data, dim,
                                               icsStruct->dataStrides,
                                               icsStruct->dimensions,
                                               (int)size, fp, icsStruct->compLevel);
            } else {
                error = IcsWriteZip(icsStruct->data, icsStruct->dataLength, fp,
                                    icsStruct->compLevel);
            }
            break;
#endif
        default:
            error = IcsErr_UnknownCompression;
    }

    if (fclose (fp) == EOF) {
        if (!error) error = IcsErr_FCloseIds; /* Don't overwrite any previous error. */
    }
    return error;
}


/* Append image data from infilename at inoffset to outfilename. If outfilename
   is a .ics file it must end with the END keyword. */
Ics_Error IcsCopyIds(const char *infilename,
                     size_t      inoffset,
                     const char *outfilename)
{
    ICSINIT;
    FILE   *in     = NULL;
    FILE   *out    = NULL;
    char   *buffer = NULL;
    int     done   = 0;
    size_t  n;


        /* Open files */
    in = IcsFOpen(infilename, "rb");
    if (in == NULL) {
        error = IcsErr_FCopyIds;
        goto exit;
    }
    if (fseek(in, (long)inoffset, SEEK_SET) != 0) {
        error = IcsErr_FCopyIds;
        goto exit;
    }
    out = IcsFOpen(outfilename, "ab");
    if (out == NULL) {
        error = IcsErr_FCopyIds;
        goto exit;
    }
        /* Create an output buffer */
    buffer = (char*)malloc(ICS_BUF_SIZE);
    if (buffer == NULL) {
        error = IcsErr_Alloc;
        goto exit;
    }
    while (!done) {
        n = fread(buffer, 1, ICS_BUF_SIZE, in);
        if (feof (in)) {
            done = 1;
        } else if (n != ICS_BUF_SIZE) {
            error = IcsErr_FCopyIds;
            goto exit;
        }
        if (fwrite(buffer, 1, n, out) != n) {
            error = IcsErr_FCopyIds;
            goto exit;
        }
    }

  exit:
    if (buffer) free(buffer);
    if (in) fclose(in);
    if (out) fclose(out);
    return error;
}


/* Check if a file exist. */
static int IcsExistFile(const char *filename)
{
    FILE *fp;


    if ((fp = IcsFOpen(filename, "rb")) != NULL) {
        fclose (fp);
        return 1;
    } else {
        return 0;
    }
}


/* Find out if we are running on a little endian machine (Intel) or on a big
   endian machine. On Intel CPUs the least significant byte is stored first in
   memory. Returns: 1 if little endian; 0 big endian (e.g. MIPS). */
static int IcsIsLittleEndianMachine(void)
{
    int i = 1;
    char *cptr = (char*)&i;
    return (*cptr == 1);
}


/* Fill the byte order array with the machine's byte order. */
void IcsFillByteOrder(Ics_DataType dataType,
                      int          bytes,
                      int          machineByteOrder[ICS_MAX_IMEL_SIZE])
{
    int i, hbytes;


    if (bytes > ICS_MAX_IMEL_SIZE) {
            /* This will cause problems if undetected, */
            /* but shouldn't happen anyway */
        bytes = ICS_MAX_IMEL_SIZE;
    }

    if (IcsIsLittleEndianMachine()) {
            /* Fill byte order for a little endian machine. */
        for (i = 0; i < bytes; i++) {
            machineByteOrder[i] = 1 + i;
        }
    } else {
        if (dataType == Ics_complex32 || dataType == Ics_complex64) {
            hbytes = bytes/2;
            for (i = 0; i < hbytes; i++) {
                machineByteOrder[i]        = hbytes - i;
                machineByteOrder[i+hbytes] = bytes - i;
            }
        } else {
            for (i = 0; i < bytes; i++) {
                machineByteOrder[i] = bytes - i;
            }
        }
    }
}


/* Reorder the bytes in the images as specified in the ByteOrder array. */
static Ics_Error IcsReorderIds(char        *buf,
                               size_t       length,
                               Ics_DataType dataType,
                               int          srcByteOrder[ICS_MAX_IMEL_SIZE],
                               int          bytes)
{
    ICSINIT;
    int  i;
    size_t j, imels;
    int  dstByteOrder[ICS_MAX_IMEL_SIZE];
    char imel[ICS_MAX_IMEL_SIZE];
    int  different = 0, empty = 0;


    imels = length / (size_t)bytes;
    if (length % (size_t)bytes != 0) return IcsErr_BitsVsSizeConfl;

        /* Create destination byte order: */
    IcsFillByteOrder(dataType, bytes, dstByteOrder);

        /* Localize byte order array: */
    for (i = 0; i < bytes; i++){
        different |= (srcByteOrder[i] != dstByteOrder[i]);
        empty |= !(srcByteOrder[i]);
    }
    if (!different || empty) return IcsErr_Ok;

    for (j = 0; j < imels; j++){
        for (i = 0; i < bytes; i++){
            imel[srcByteOrder[i]-1] = buf[i];
        }
        for (i = 0; i < bytes; i++){
            buf[i] = imel[dstByteOrder[i]-1];
        }
        buf += bytes;
    }

    return error;
}


/* Open an IDS file for reading. */
Ics_Error IcsOpenIds(Ics_Header *icsStruct)
{
    ICSINIT;
    Ics_BlockRead *br;
    char           filename[ICS_MAXPATHLEN];
    size_t         offset = 0;


    if (icsStruct->blockRead != NULL) {
        error = IcsCloseIds(icsStruct);
        if (error) return error;
    }
    if (icsStruct->version == 1) {          /* Version 1.0 */
        IcsGetIdsName(filename, icsStruct->filename);
#ifdef ICS_DO_GZEXT
            /* If the .ids file does not exist then maybe the .ids.gz or .ids.Z
             * file exists. */
        if (!IcsExistFile(filename)) {
            if (strlen(filename) < ICS_MAXPATHLEN - 4) {
                strcat(filename, ".gz");
                if (IcsExistFile(filename)) {
                    icsStruct->compression = IcsCompr_gzip;
                } else {
                    strcpy(filename + strlen(filename) - 3, ".Z");
                    if (IcsExistFile(filename)) {
                        icsStruct->compression = IcsCompr_compress;
                    } else {
                        return IcsErr_FOpenIds;
                    }
                }
            }
        }
#endif
    } else {                                  /* Version 2.0 */
        if (icsStruct->srcFile[0] == '\0') return IcsErr_MissingData;
        IcsStrCpy(filename, icsStruct->srcFile, ICS_MAXPATHLEN);
        offset = icsStruct->srcOffset;
    }

    br = (Ics_BlockRead*)malloc(sizeof (Ics_BlockRead));
    if (br == NULL) return IcsErr_Alloc;

    br->dataFilePtr = IcsFOpen(filename, "rb");
    if (br->dataFilePtr == NULL) return IcsErr_FOpenIds;
    if (fseek(br->dataFilePtr, (long)offset, SEEK_SET) != 0) {
        fclose(br->dataFilePtr);
        free(br);
        return IcsErr_FReadIds;
    }
#ifdef ICS_ZLIB
    br->zlibStream = NULL;
    br->zlibInputBuffer = NULL;
#endif
    br->compressRead = 0;
    icsStruct->blockRead = br;

#ifdef ICS_ZLIB
    if (icsStruct->compression == IcsCompr_gzip) {
        error = IcsOpenZip(icsStruct);
        if (error) {
            fclose (br->dataFilePtr);
            free(icsStruct->blockRead);
            icsStruct->blockRead = NULL;
            return error;
        }
    }
#endif

    return error;
}


/* Close an IDS file for reading. */
Ics_Error IcsCloseIds(Ics_Header *icsStruct)
{
    ICSINIT;
    Ics_BlockRead* br = (Ics_BlockRead*)icsStruct->blockRead;


    if (br->dataFilePtr && fclose(br->dataFilePtr) == EOF) {
        error = IcsErr_FCloseIds;
    }
#ifdef ICS_ZLIB
    if (br->zlibStream != NULL) {
        if (!error)
            error = IcsCloseZip(icsStruct);
        else
            IcsCloseZip(icsStruct);
    }
#endif
    free(br);
    icsStruct->blockRead = NULL;

    return error;
}


/* Read a data block from an IDS file. */
Ics_Error IcsReadIdsBlock(Ics_Header *icsStruct,
                          void       *dest,
                          size_t      n)
{
    ICSINIT;
    Ics_BlockRead* br = (Ics_BlockRead*)icsStruct->blockRead;


    switch (icsStruct->compression) {
        case IcsCompr_uncompressed:
            if ((fread(dest, 1, n, br->dataFilePtr)) != n) {
                if (ferror(br->dataFilePtr)) {
                    error = IcsErr_FReadIds;
                } else {
                    error = IcsErr_EndOfStream;
                }
            }
            break;
#ifdef ICS_ZLIB
        case IcsCompr_gzip:
            error = IcsReadZipBlock(icsStruct, dest, n);
            break;
#endif
        case IcsCompr_compress:
            if (br->compressRead) {
                error = IcsErr_BlockNotAllowed;
            } else {
                error = IcsReadCompress(icsStruct, dest, n);
                br->compressRead = 1;
            }
            break;
        default:
            error = IcsErr_UnknownCompression;
    }

    if (!error) error = IcsReorderIds((char*)dest, n, icsStruct->imel.dataType,
                                      icsStruct->byteOrder,
                                      IcsGetBytesPerSample(icsStruct));

    return error;
}


/* Skip a data block from an IDS file. */
Ics_Error IcsSkipIdsBlock(Ics_Header *icsStruct,
                          size_t      n)
{
    return IcsSetIdsBlock (icsStruct, (long)n, SEEK_CUR);
}


/* Sets the file pointer into the IDS file. */
Ics_Error IcsSetIdsBlock(Ics_Header *icsStruct,
                         long        offset,
                         int         whence)
{
    ICSINIT;
    Ics_BlockRead* br = (Ics_BlockRead*)icsStruct->blockRead;


    switch (icsStruct->compression) {
        case IcsCompr_uncompressed:
            switch (whence) {
                case SEEK_SET:
                case SEEK_CUR:
                    if (fseek(br->dataFilePtr, (long)offset, whence) != 0) {
                        if (ferror(br->dataFilePtr)) {
                            error = IcsErr_FReadIds;
                        } else {
                            error = IcsErr_EndOfStream;
                        }
                    }
                    break;
                default:
                    error = IcsErr_IllParameter;
            }
            break;
#ifdef ICS_ZLIB
        case IcsCompr_gzip:
            switch (whence) {
                case SEEK_SET:
                case SEEK_CUR:
                    error = IcsSetZipBlock(icsStruct, offset, whence);
                    break;
                default:
                    error = IcsErr_IllParameter;
            }
            break;
#endif
        case IcsCompr_compress:
            error = IcsErr_BlockNotAllowed;
            break;
        default:
            error = IcsErr_UnknownCompression;
    }

    return error;
}


/* Read the data from an IDS file. */
Ics_Error IcsReadIds(Ics_Header *icsStruct,
                     void       *dest,
                     size_t      n)
{
    ICSINIT;

    error = IcsOpenIds(icsStruct);
    if (error) return error;
    error = IcsReadIdsBlock(icsStruct, dest, n);
    if (!error)
        error = IcsCloseIds(icsStruct);
    else
        IcsCloseIds(icsStruct);

    return error;
}
