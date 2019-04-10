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
 * FILE : libics_gzip.c
 *
 * The following internal functions are contained in this file:
 *
 *   IcsWriteZip()
 *   IcsWriteZipWithStrides()
 *   IcsOpenZip()
 *   IcsCloseZip()
 *   IcsReadZipBlock()
 *   IcsSetZipBlock()
 *
 * This is the only file that contains any zlib dependancies.
 *
 * Because of a defect in the zlib interface, the only way of using gzread
 * and gzwrite on streams that are already open is through file handles (which
 * are not ANSI C). The weird thing is that zlib creates a stream from this
 * handle to do its stuff. To avoid using non-ANSI C functions, I copied a
 * large part of gzio.c (simplifying it to do just what I needed it to do).
 * Therefore, most of the code in this file was written by Jean-loup Gailly.
 *    (Copyright (C) 1995-1998 Jean-loup Gailly)
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libics_intern.h"

// Include zlib.h only when available
#ifdef ICS_ZLIB
    #include "zlib.h"
#endif

#define DEF_MEM_LEVEL 8 /* Default value defined in zutil.h */


/* GZIP stuff */
#ifdef ICS_ZLIB
#ifdef WIN32
#define OS_CODE 0x0b
#else
#define OS_CODE 0x03  /* assume Unix */
#endif
static int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */
#endif // ICS_ZLIB

/* Outputs a long in LSB order to the given stream */
#ifdef ICS_ZLIB
static void icsPutLong(FILE *file,
                       unsigned long int x)
{
   int i;
   for (i = 0; i < 4; i++) {
      fputc((int)(x & 0xff), file);
      x >>= 8;
   }
}
#endif


/* Reads a long in LSB order from the given stream. */
#ifdef ICS_ZLIB
static unsigned long int icsGetLong(FILE *file)
{
    unsigned long int x = (unsigned long int)getc(file);
    x += ((unsigned long int)getc(file))<<8;
    x += ((unsigned long int)getc(file))<<16;
    x += ((unsigned long int)getc(file))<<24;
    return x;
}
#endif


/* Write ZIP compressed data. This function mostly does:
     gzFile out;
     char mode[4]; strcpy(mode, "wb0"); mode[2] += level;
     out = gzdopen(dup(fileno(file)), mode);
     if (out == NULL) return IcsErr_FWriteIds);
     if (gzwrite(out, (const voidp)inbuf, n) != (int)n)
     error = IcsErr_CompressionProblem;
     gzclose(out); */
Ics_Error IcsWriteZip(const void *inBuf,
                      size_t      len,
                      FILE       *file,
                      int         level)
{
#ifdef ICS_ZLIB
    z_stream     stream;
    Byte *       outBuf;    /* output buffer */
    int          err, flush;
    size_t       totalCount;
    unsigned int have;
    uLong        crc;


        /* Create an output buffer */
    outBuf = (Byte*)malloc(ICS_BUF_SIZE);
    if (outBuf == Z_NULL) return IcsErr_Alloc;

        /* Initialize the stream for output */
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
    stream.avail_in = 0;
    stream.next_in = NULL;
    stream.next_out = Z_NULL;
    stream.avail_out = 0;

    crc = crc32(0L, Z_NULL, 0);

    err = deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL,
                        Z_DEFAULT_STRATEGY);
        /* windowBits is passed < 0 to suppress zlib header */
    if (err != Z_OK) {
        free(outBuf);
        if (err == Z_VERSION_ERROR) {
            return IcsErr_WrongZlibVersion;
        } else {
            return IcsErr_CompressionProblem;
        }
    }

        /* Write a very simple GZIP header: */
    fprintf(file, "%c%c%c%c%c%c%c%c%c%c", gz_magic[0], gz_magic[1], Z_DEFLATED,
            0,0,0,0,0,0, OS_CODE);

        /* Write the compressed data */
    totalCount = 0;
    do {
        if (len - totalCount < ICS_BUF_SIZE) {
            stream.avail_in = (uInt)(len - totalCount);
        } else {
            stream.avail_in = ICS_BUF_SIZE;
        }
        stream.next_in = (Bytef*)inBuf + totalCount;
        crc = crc32(crc, stream.next_in, stream.avail_in);
        totalCount += stream.avail_in;
        flush = totalCount >= len ? Z_FINISH : Z_NO_FLUSH;
        do {
            stream.avail_out = ICS_BUF_SIZE;
            stream.next_out = outBuf;
            err = deflate(&stream, flush);
            have = ICS_BUF_SIZE - stream.avail_out;
            if (fwrite(outBuf, 1, have, file) != have || ferror(file)) {
                deflateEnd(&stream);
                free(outBuf);
                return IcsErr_FWriteIds;
            }
        } while (stream.avail_out == 0);
    } while (flush != Z_FINISH);

        /* Was all the input processed? */
    if (stream.avail_in != 0) {
        deflateEnd(&stream);
        free(outBuf);
        return IcsErr_CompressionProblem;
    }
        /* Write the CRC and original data length */
    icsPutLong(file, crc);
        /* Data length is written as a 32 bit value, for compatibility we keep
           it like that, even if totalCount is 64 bit. */
    icsPutLong(file, totalCount & 0xFFFFFFFF);
        /* Deallocate stuff */
    err = deflateEnd(&stream);
    free(outBuf);

    return err == Z_OK ? IcsErr_Ok : IcsErr_CompressionProblem;
#else
    (void)inBuf;
    (void)len;
    (void)file;
    (void)level;
    return IcsErr_UnknownCompression;
#endif
}


/* Write ZIP compressed data, with strides. */
Ics_Error IcsWriteZipWithStrides(const void      *src,
                                 const size_t    *dim,
                                 const ptrdiff_t *stride,
                                 int              nDims,
                                 int              nBytes,
                                 FILE            *file,
                                 int              level)
{
#ifdef ICS_ZLIB
    ICSINIT;
    z_stream     stream;
    Byte        *inBuf              = 0; /* input buffer */
    Byte        *inBuf_ptr;
    Byte        *outBuf             = 0; /* output buffer */
    size_t       curPos[ICS_MAXDIM];
    char const  *data;
    int          i, err, done;
    size_t       j;
    size_t       count, totalCount = 0;
    uLong        crc;
    const int    contiguousLine    = stride[0]==1;


        /* Create an output buffer */
    outBuf = (Byte*)malloc(ICS_BUF_SIZE);
    if (outBuf == Z_NULL) return IcsErr_Alloc;
        /* Create an input buffer */
    if (!contiguousLine) {
        inBuf = (Byte*)malloc(dim[0] * (size_t)nBytes);
        if (inBuf == Z_NULL) {
            free(outBuf);
            return IcsErr_Alloc;
        }
    }

        /* Initialize the stream for output */
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
    stream.next_in = (Bytef*)0;
    stream.avail_in = 0;
    stream.next_out = Z_NULL;
    stream.avail_out = 0;
    err = deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS,
                       DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
        /* windowBits is passed < 0 to suppress zlib header */
    if (err != Z_OK) {
        free(outBuf);
        if (!contiguousLine) free(inBuf);
        if (err == Z_VERSION_ERROR) {
            return IcsErr_WrongZlibVersion;
        } else {
            return IcsErr_CompressionProblem;
        }
    }
    stream.next_out = outBuf;
    stream.avail_out = ICS_BUF_SIZE;
    crc = crc32(0L, Z_NULL, 0);

        /* Write a very simple GZIP header: */
    fprintf(file, "%c%c%c%c%c%c%c%c%c%c", gz_magic[0], gz_magic[1], Z_DEFLATED,
            0,0,0,0,0,0, OS_CODE);

        /* Walk over each line in the 1st dimension */
    for (i = 0; i < nDims; i++) {
        curPos[i] = 0;
    }
    while (1) {
        data = (char const*)src;
        for (i = 1; i < nDims; i++) { /* curPos[0]==0 here */
            data += (ptrdiff_t)curPos[i] * stride[i] * nBytes;
        }
            /* Get data line */
        if (contiguousLine) {
            inBuf = (Byte*)data;
        } else {
            inBuf_ptr = inBuf;
            for (j = 0; j < dim[0]; j++) {
                memcpy(inBuf_ptr, data, (size_t)nBytes);
                data += stride[0] * nBytes;
                inBuf_ptr += nBytes;
            }
        }
            /* Write the compressed data */
        stream.next_in = (Bytef*)inBuf;
        stream.avail_in = (uInt)(dim[0] * (size_t)nBytes);
        totalCount += stream.avail_in;
        while (stream.avail_in != 0) {
            if (stream.avail_out == 0) {
                if (fwrite(outBuf, 1, ICS_BUF_SIZE, file) != ICS_BUF_SIZE) {
                    error = IcsErr_FWriteIds;
                    goto error_exit;
                }
                stream.next_out = outBuf;
                stream.avail_out = ICS_BUF_SIZE;
            }
            err = deflate(&stream, Z_NO_FLUSH);
            if (err != Z_OK) {
                break;
            }
        }
            /* Was all the input processed? */
        if (stream.avail_in != 0) {
            error = IcsErr_CompressionProblem;
            goto error_exit;
        }
        crc = crc32(crc, (Bytef*)inBuf, (uInt)(dim[0] * (size_t)nBytes));
            /* This is part of the N-D loop */
        for (i = 1; i < nDims; i++) {
            curPos[i]++;
            if (curPos[i] < dim[i]) {
                break;
            }
            curPos[i] = 0;
        }
        if (i == nDims) {
            break; /* we're done writing */
        }
    }

        /* Flush the stream */
    done = 0;
    for (;;) {
        count = ICS_BUF_SIZE - stream.avail_out;
        if (count != 0) {
            if ((size_t)fwrite(outBuf, 1, count, file) != count) {
                error = IcsErr_FWriteIds;
                goto error_exit;
            }
            stream.next_out = outBuf;
            stream.avail_out = ICS_BUF_SIZE;
        }
        if (done) {
            break;
        }
        err = deflate(&stream, Z_FINISH);
        if ((err != Z_OK) && (err != Z_STREAM_END)) {
            error = IcsErr_CompressionProblem;
            goto error_exit;
        }
        done = (stream.avail_out != 0 || err == Z_STREAM_END);
    }
        /* Write the CRC and original data length */
    icsPutLong(file, crc);
    icsPutLong(file, totalCount & 0xFFFFFFFF);

  error_exit:
        /* Deallocate stuff */
    err = deflateEnd(&stream);
    free(outBuf);
    if (!contiguousLine) free(inBuf);

    if (error) {
        return error;
    } else {
        return err == Z_OK ? IcsErr_Ok : IcsErr_CompressionProblem;
    }
#else
    (void)src;
    (void)dim;
    (void)stride;
    (void)nDims;
    (void)nBytes;
    (void)file;
    (void)level;
    return IcsErr_UnknownCompression;
#endif
}


    /* Start reading ZIP compressed data. This function mostly does:
       br->ZlibStream = gzdopen(dup(fileno(br->DataFilePtr)), "rb"); */
Ics_Error IcsOpenZip(Ics_Header *icsStruct)
{
#ifdef ICS_ZLIB
    Ics_BlockRead * br   = (Ics_BlockRead*)icsStruct->blockRead;
    FILE           *file = br->dataFilePtr;
    z_stream*       stream;
    void           *inBuf;
    int             err;
    int             method, flags; /* hold data from the GZIP header */


        /* check the GZIP header */
    if ((getc(file) != gz_magic[0]) || (getc(file) != gz_magic[1]))
        return IcsErr_CorruptedStream;
    method = getc(file);
    flags = getc(file);
    if ((method != Z_DEFLATED) || ((flags & RESERVED) != 0))
        return IcsErr_CorruptedStream;
    fseek(file, 6, SEEK_CUR);          /* Discard time, xflags and OS code: */
    if ((flags & EXTRA_FIELD) != 0) {  /* skip the extra field */
        size_t len;
        len  =  (uInt)getc(file);
        len += ((uInt)getc(file)) << 8;
        if (feof (file)) return IcsErr_CorruptedStream;
        fseek(file, (long)len, SEEK_CUR);
    }
    if ((flags & ORIG_NAME) != 0) {   /* skip the original file name */
        int c;
        while (((c = getc(file)) != 0) && (c != EOF));
    }
    if ((flags & COMMENT) != 0) {     /* skip the .gz file comment */
        int c;
        while (((c = getc(file)) != 0) && (c != EOF));
    }
    if ((flags & HEAD_CRC) != 0) {    /* skip the header crc */
        fseek(file, 2, SEEK_CUR);
    }
    if (feof(file) || ferror(file)) return IcsErr_CorruptedStream;

        /* Create an input buffer */
    inBuf = malloc(ICS_BUF_SIZE);
    if (inBuf == NULL) return IcsErr_Alloc;

        /* Initialize the stream for input */
    stream = (z_stream*)malloc(sizeof (z_stream));
    if (stream == NULL) return IcsErr_Alloc;
    stream->zalloc = NULL;
    stream->zfree = NULL;
    stream->opaque = NULL;
    stream->next_in = NULL;
    stream->avail_in = 0;
    stream->next_out = NULL;
    stream->avail_out = 0;
    stream->next_in = (Byte*)inBuf;
    err = inflateInit2(stream, -MAX_WBITS);
        /* windowBits is passed < 0 to tell that there is no zlib header.  Note
           that in this case inflate *requires* an extra "dummy" byte after the
           compressed stream in order to complete decompression and return
           Z_STREAM_END. Here the gzip CRC32 ensures that 4 bytes are present
           after the compressed stream. */
    if (err != Z_OK) {
        if (err != Z_VERSION_ERROR) {
            inflateEnd(stream);
        }
        free(inBuf);
        if (err == Z_VERSION_ERROR) {
            return IcsErr_WrongZlibVersion;
        } else {
            return IcsErr_DecompressionProblem;
        }
    }

    br->zlibStream = stream;
    br->zlibInputBuffer = inBuf;
    br->zlibCRC = crc32(0L, Z_NULL, 0);
    return IcsErr_Ok;
#else
    (void)icsStruct;
    return IcsErr_UnknownCompression;
#endif
}


/* Close ZIP compressed data stream. This function mostly does:
     gzclose((gzFile)br->ZlibStream); */
Ics_Error IcsCloseZip(Ics_Header *icsStruct)
{
#ifdef ICS_ZLIB
    Ics_BlockRead *br     = (Ics_BlockRead*)icsStruct->blockRead;
    z_stream*      stream = (z_stream*)br->zlibStream;
    int            err;

    err = inflateEnd(stream);
    free(stream);
    br->zlibStream = NULL;
    free(br->zlibInputBuffer);
    br->zlibInputBuffer = NULL;

    if (err != Z_OK) {
        return IcsErr_DecompressionProblem;
    }
    return IcsErr_Ok;
#else
    (void)icsStruct;
    return IcsErr_UnknownCompression;
#endif
}


/* Read ZIP compressed data block. This function mostly does:
     gzread((gzFile)br->ZlibStream, outBuf, len); */
Ics_Error IcsReadZipBlock(Ics_Header *icsStruct,
                          void       *outBuf,
                          size_t      len)
{
#ifdef ICS_ZLIB
    Ics_BlockRead *br      = (Ics_BlockRead*)icsStruct->blockRead;
    FILE          *file    = br->dataFilePtr;
    z_stream*      stream  = (z_stream*)br->zlibStream;
    void          *inBuf   = br->zlibInputBuffer;
    int            err;
    size_t         prevout = stream->total_out, todo = len;
    unsigned int   bufsize, done;
    Bytef         *prevbuf;

        /* Read the compressed data */
    do {
        stream->avail_in = (uInt)fread(inBuf, 1, ICS_BUF_SIZE, file);
        if (ferror(file)) {
            return IcsErr_FReadIds;
        }
        if (stream->avail_in == 0 && todo > 0) {
            err = Z_STREAM_ERROR;
            break;
        }
        stream->next_in = inBuf;
        do {
            if (todo == 0) {
                err = Z_OK;
                break;
            }
            bufsize = (unsigned int)(todo < ICS_BUF_SIZE ? todo : ICS_BUF_SIZE);
            stream->avail_out = bufsize;
            prevbuf = stream->next_out = (Bytef*)outBuf + len - todo;;
            err = inflate(stream, Z_NO_FLUSH);
            if (!(err == Z_OK || err == Z_STREAM_END || err == Z_BUF_ERROR)) {
                return IcsErr_FReadIds;
            }
            done = bufsize - stream->avail_out;
            todo -= done;
            br->zlibCRC = crc32(br->zlibCRC, prevbuf, done);
        } while (stream->avail_out == 0);
    } while (err != Z_STREAM_END && todo > 0);

        /* Set the file pointer back so that unused input can be read again. */
    fseek(file, -(int)stream->avail_in, SEEK_CUR);

    if (err == Z_STREAM_END) {
            /* All the data has been decompressed: Check CRC and original data
               size */
        if (icsGetLong(file) != br->zlibCRC) {
            err = Z_STREAM_ERROR;
        } else {
            if (icsGetLong(file) != stream->total_out) {
                err = Z_STREAM_ERROR;
            }
        }
    }

        /* Report errors */
    if (err == Z_STREAM_ERROR) return IcsErr_CorruptedStream;
    if (err == Z_STREAM_END) {
        if (len != stream->total_out - prevout) return IcsErr_EndOfStream;
        return IcsErr_Ok;
    }
    if (err == Z_OK) return IcsErr_Ok;
    return IcsErr_DecompressionProblem;
#else
    (void)icsStruct;
    (void)outBuf;
    (void)len;
    return IcsErr_UnknownCompression;
#endif
}


/* Skip ZIP compressed data block. This function mostly does:
     gzseek((gzFile)br->ZlibStream, (z_off_t)offset, whence); */
Ics_Error IcsSetZipBlock(Ics_Header *icsStruct,
                         long        offset,
                         int         whence)
{
#ifdef ICS_ZLIB
    ICSINIT;
    size_t         n, bufsize;
    void          *buf;
    Ics_BlockRead *br     = (Ics_BlockRead*)icsStruct->blockRead;
    z_stream*      stream = (z_stream*)br->zlibStream;

    if ((whence == SEEK_CUR) && (offset<0)) {
        offset += (long)stream->total_out;
        whence = SEEK_SET;
    }
    if (whence == SEEK_SET) {
        if (offset < 0) return IcsErr_IllParameter;
        error = IcsCloseIds(icsStruct);
        if (error) return error;
        error = IcsOpenIds(icsStruct);
        if (error) return error;
        if (offset==0) return IcsErr_Ok;
    }

    bufsize = (unsigned int)(offset < ICS_BUF_SIZE ? offset : ICS_BUF_SIZE);
    buf = malloc(bufsize);
    if (buf == NULL) return IcsErr_Alloc;

    n = (size_t)offset;
    while (n > 0) {
        if (n > bufsize) {
            error = IcsReadZipBlock(icsStruct, buf, bufsize);
            n -= bufsize;
        } else {
            error = IcsReadZipBlock(icsStruct, buf, n);
            break;
        }
        if (error) {
            break;
        }
    }

    free(buf);

    return error;
#else
    (void)icsStruct;
    (void)offset;
    (void)whence;
    return IcsErr_UnknownCompression;
#endif
}
