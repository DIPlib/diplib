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
 * FILE : libics_util.c
 *
 * The following library functions are contained in this file:
 *
 *   IcsGetLibVersion()
 *   IcsGetIcsName()
 *   IcsGetIdsName()
 *   IcsInit()
 *   IcsGetDataTypeSize()
 *   IcsGetPropsDataType()
 *   IcsGetDataTypeProps()
 *
 * The following internal functions are contained in this file:
 *
 *   IcsStrCpy()
 *   IcsAppendChar()
 *   IcsGetFileName()
 *   IcsExtensionFind()
 *   IcsGetBytesPerSample()
 *   IcsOpenIcs()
 */

#include <stdlib.h>
#include <string.h>
#include "libics_intern.h"

#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

const char ICSEXT[] = ".ics";
const char IDSEXT[] = ".ids";
const char IDSEXT_Z[] = ".ids.Z";
const char IDSEXT_GZ[] = ".ids.gz";


/* This is a wrapper for the fopen function, on UNIX it calls fopen, on Windows
   it uses _wfopen to support UTF-8 filenames. */
FILE* IcsFOpen(const char *path,
               const char *mode)
{
#ifdef _WIN32
    wchar_t *wpath  = NULL, wmode[8];
    int      n      = MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 0);
    FILE    *result = NULL;

    wpath =(wchar_t*)malloc(n * sizeof(wchar_t));
    if (!wpath) return NULL;

    if (!MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, n)) goto exit;
    if (!MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 8)) goto exit;

    result = _wfopen(wpath, wmode);

  exit:
    if (wpath) {
        free(wpath);
    }
    return result;
#else
    return fopen(path, mode);
#endif
}


/* This function can be used to check for the correct library version: if
  (strcmp (ICSLIB_VERSION, IcsGetLibVersion ()) != 0) return ERRORCODE; */
const char *IcsGetLibVersion(void)
{
    return ICSLIB_VERSION;
}


/* Parse a number string and return the value in a size_t. */
size_t IcsStrToSize(const char *str)
{
    unsigned long ulsize;
    size_t        size;


    ulsize = strtoul(str, NULL, 10);
    size =(size_t) ulsize;

    return size;
}


/* A safe strcpy. len is the number of characters in dest. Up to len-1
   characters copied. */
void IcsStrCpy(char       *dest,
               const char *src,
               int         len)
{
    if (dest != src) {
        size_t nchar = strlen(src);
        nchar = (nchar > (size_t)len-1) ? (size_t)len-1 : nchar;
        memcpy(dest, src, nchar);
        dest[nchar] = 0;
    }
}


/* Append a character to a string */
void IcsAppendChar(char *line,
                   char  ch)
{
    size_t len = strlen(line);
    line[len] = ch;
    line[len+1] = '\0';
}


/* Find the start of the filename */
static char *IcsFileNameFind(const char *str)
{
    const char *begin;

#ifdef WIN32
        /* For Windows we check both kinds of path separators */
    begin = strrchr(str, '\\');
    if (begin == NULL) {
        begin = strrchr(str, '/');
    }
#else
    begin = strrchr(str, '/');
#endif
    if (begin == NULL) {
        begin = str;
    }
    else {
        begin++;
    }

    return (char *)begin;
}


/* Find the start of the '.ics' or '.ids' extension.  Also handle filenames
 ending in '.ids.Z' or '.ids.gz'.  All character comparisons must be case
 insensitive.  Return a pointer to the start of the extension or NULL if no
 extension could be found. */
char *IcsExtensionFind(const char *str)
{
    size_t     len;
    const char *ext;


    len = strlen(str);

    ext = str + len - (sizeof(ICSEXT) - 1);
    if (ext >= str && strcasecmp(ext, ICSEXT) == 0) {
        return (char *)ext;
    }

    ext = str + len - (sizeof(IDSEXT) - 1);
    if (ext >= str && strcasecmp(ext, IDSEXT) == 0) {
        return (char *)ext;
    }

    ext = str + len - (sizeof(IDSEXT_Z) - 1);
    if (ext >= str && strcasecmp(ext, IDSEXT_Z) == 0) {
        return (char *)ext;
    }

    ext = str + len - (sizeof(IDSEXT_GZ) - 1);
    if (ext >= str && strcasecmp(ext, IDSEXT_GZ) == 0) {
        return (char *)ext;
    }

    return NULL;
}


/* Strip the path from everything but the file name(including extension). */
void IcsGetFileName(char       *dest,
                    const char *src)
{
    const char *begin;
    char       *end;


    begin = IcsFileNameFind(src);
    IcsStrCpy(dest, begin, ICS_MAXPATHLEN);
    end = IcsExtensionFind(dest);
    if (end != NULL) {
        *end = '\0';
    }
}


/* Make a filename ending in '.ics' from the given filename.  If the filename
  ends in '.IDS' then make this '.ICS'.  Also accept filenames ending in
  '.ids.Z' and '.ids.gz', but strip the compression extension. */
char *IcsGetIcsName(char       *dest,
                    const char *src,
                    int         forceName)
{
    char *end;


    IcsStrCpy(dest, src, ICS_MAXPATHLEN);
    end = IcsExtensionFind(dest);
    if (end != NULL) {
        if (strcasecmp(end, ICSEXT) == 0) {
            return dest;
        } else {
                /* Keep same case. */
            if (end[2] == 'd') {
                end[2] = 'c';
                end[4] = '\0';
                return dest;
            } else if (end[2] == 'D') {
                end[2] = 'C';
                end[4] = '\0';
                return dest;
            } else { /* does not happen! */
                if (!forceName) {
                    *end = '\0';
                }
            }
        }
    }
    if (!forceName && strlen(dest) + strlen(ICSEXT) + 1 < ICS_MAXPATHLEN) {
        strcat(dest, ICSEXT);
    }

    return dest;
}


/* Make a filename ending in '.ids' from the given filename.  If the filename
  ends in '.ICS' then make this '.IDS'.  Also accept filenames ending in
  '.ids.Z' and '.ids.gz', but strip the compression extension. */
char *IcsGetIdsName(char       *dest,
                    const char *src)
{
    char *end;

    IcsStrCpy(dest, src, ICS_MAXPATHLEN);
    end = IcsExtensionFind(dest);
    if (end != NULL) {
        if (strcasecmp(end, ICSEXT) == 0) {
                /* Keep same case. */
            if (end[2] == 'c') {
                end[2] = 'd';
                return dest;
            } else if (end[2] == 'C') {
                end[2] = 'D';
                return dest;
            } else {
                *end = '\0';
            }
        } else {
            end[4] = '\0';
            return dest;
        }
    }
    if (strlen(dest) + strlen(IDSEXT) + 1 < ICS_MAXPATHLEN) {
        strcat(dest, IDSEXT);
    }

    return dest;
}


/* Open an .ics file, even if the name given end in .ids. */
Ics_Error IcsOpenIcs(FILE **fpp,
                     char  *filename,
                     int    forceName)
{
    ICSINIT;
    FILE* fp;
    char FileName[ICS_MAXPATHLEN];

    IcsGetIcsName(FileName, filename, forceName);
    fp = IcsFOpen(FileName, "rb");
    if (fp == NULL) return IcsErr_FOpenIcs;

    *fpp = fp;
    strcpy(filename, FileName);

    return error;
}


/* Initialize the Ics_Header structure. */
void IcsInit(Ics_Header *icsStruct)
{
    int i;

    icsStruct->version = 2; /* We write an ICS v.2.0 as default */
    icsStruct->fileMode = IcsFileMode_write;
    icsStruct->data = NULL;
    icsStruct->dataLength = 0;
    icsStruct->dataStrides = NULL;
    icsStruct->filename[0] = '\0';
    icsStruct->dimensions = 0;
    for (i = 0; i < ICS_MAXDIM; i++) {
        icsStruct->dim[i].size = 0;
        icsStruct->dim[i].origin = 0.0;
        icsStruct->dim[i].scale = 1.0;
        icsStruct->dim[i].order[0] = '\0';
        icsStruct->dim[i].label[0] = '\0';
        icsStruct->dim[i].unit[0] = '\0';
    }
    icsStruct->imel.dataType = Ics_unknown;
    icsStruct->imel.sigBits = 0;
    icsStruct->imel.origin = 0.0;
    icsStruct->imel.scale = 1.0;
    icsStruct->imel.unit[0] = '\0';
    icsStruct->coord[0] = '\0';
    icsStruct->compression = IcsCompr_uncompressed;
    icsStruct->compLevel = 0;
    icsStruct->history = NULL;
    icsStruct->blockRead = NULL;
    icsStruct->srcFile[0] = '\0';
    icsStruct->srcOffset = 0;
    for (i = 0; i < ICS_MAX_IMEL_SIZE; i++) {
        icsStruct->byteOrder[i] = 0;
    }
    icsStruct->writeSensor = 0;
    icsStruct->writeSensorStates = 0;
    icsStruct->model[0]= '\0';
    icsStruct->numAperture = 0.0;
    icsStruct->numApertureState = IcsSensorState_default;
    icsStruct->refrInxMedium = 0.0;
    icsStruct->refrInxMediumState = IcsSensorState_default;
    icsStruct->refrInxLensMedium = 0.0;
    icsStruct->refrInxLensMediumState = IcsSensorState_default;
    icsStruct->pinholeSpacing = 0.0;
    icsStruct->pinholeSpacingState = IcsSensorState_default;
    icsStruct->interfacePrimary = 0.0;
    icsStruct->interfacePrimaryState = IcsSensorState_default;
    icsStruct->interfaceSecondary = 0.0;
    icsStruct->interfaceSecondaryState = IcsSensorState_default;
    icsStruct->sensorChannels = 0;
    for (i = 0; i < ICS_MAX_LAMBDA; i++) {
        icsStruct->type[i][0] = '\0';
        icsStruct->imagingDirection[i][0] = '\0';
        icsStruct->imagingDirectionState[i] = IcsSensorState_default;
        icsStruct->objectiveQuality[i] = 0;
        icsStruct->objectiveQualityState[i] = IcsSensorState_default;
        icsStruct->pinholeRadius[i] = 0.0;
        icsStruct->pinholeRadiusState[i] = IcsSensorState_default;
        icsStruct->illPinholeRadius[i] = 0.0;
        icsStruct->illPinholeRadiusState[i] = IcsSensorState_default;
        icsStruct->excitationBeamFill[i] = 0.0;
        icsStruct->excitationBeamFillState[i] = IcsSensorState_default;
        icsStruct->lambdaEx[i] = 0.0;
        icsStruct->lambdaExState[i] = IcsSensorState_default;
        icsStruct->lambdaEm[i] = 0.0;
        icsStruct->lambdaEmState[i] = IcsSensorState_default;
        icsStruct->exPhotonCnt[i] = 1;
        icsStruct->exPhotonCntState[i] = IcsSensorState_default;
        icsStruct->detectorMagn[i] = 1.0;
        icsStruct->detectorMagnState[i] = IcsSensorState_default;
        icsStruct->detectorPPU[i] = 1.0;
        icsStruct->detectorPPUState[i] = IcsSensorState_default;
        icsStruct->detectorBaseline[i] = 0.0;
        icsStruct->detectorBaselineState[i] = IcsSensorState_default;
        icsStruct->detectorLineAvgCnt[i] = 1.0;
        icsStruct->detectorLineAvgCntState[i] = IcsSensorState_default;
        icsStruct->stedDepletionMode[i][0] = '\0';
        icsStruct->stedDepletionModeState[i] = IcsSensorState_default;
        icsStruct->stedLambda[i] = 0.0;
        icsStruct->stedLambdaState[i] = IcsSensorState_default;
        icsStruct->stedSatFactor[i] = 0.0;
        icsStruct->stedSatFactorState[i] = IcsSensorState_default;
        icsStruct->stedImmFraction[i] = 0.0;
        icsStruct->stedImmFractionState[i] = IcsSensorState_default;
        icsStruct->stedVPPM[i] = 0.0;
        icsStruct->stedVPPMState[i] = IcsSensorState_default;
        icsStruct->spimExcType[i][0] = '\0';
        icsStruct->spimExcTypeState[i] = IcsSensorState_default;
        icsStruct->spimPlaneNA[i] = 0.0;
        icsStruct->spimPlaneNAState[i] = IcsSensorState_default;
        icsStruct->spimFillFactor[i] = 0.0;
        icsStruct->spimFillFactorState[i] = IcsSensorState_default;
        icsStruct->spimPlaneGaussWidth[i] = 0.0;
        icsStruct->spimPlaneGaussWidthState[i] = IcsSensorState_default;
        icsStruct->spimPlanePropDir[i][0] = 0.0;
        icsStruct->spimPlanePropDir[i][1] = 0.0;
        icsStruct->spimPlanePropDir[i][2] = 0.0;
        icsStruct->spimPlanePropDirState[i] = IcsSensorState_default;
        icsStruct->spimPlaneCenterOff[i] = 0.0;
        icsStruct->spimPlaneCenterOffState[i] = IcsSensorState_default;
        icsStruct->spimPlaneFocusOff[i] = 0.0;
        icsStruct->spimPlaneFocusOffState[i] = IcsSensorState_default;
        icsStruct->scatterModel[i][0] = '\0';
        icsStruct->scatterModelState[i] = IcsSensorState_default;
        icsStruct->scatterFreePath[i] = 0.0;
        icsStruct->scatterFreePathState[i] = IcsSensorState_default;
        icsStruct->scatterRelContrib[i] = 0.0;
        icsStruct->scatterRelContribState[i] = IcsSensorState_default;
        icsStruct->scatterBlurring[i] = 0.0;
        icsStruct->scatterBlurringState[i] = IcsSensorState_default;
    }
    icsStruct->scilType[0] = '\0';
}


/* Find the number of bytes per sample. */
int IcsGetBytesPerSample(const Ics_Header *icsStruct)
{
    return (int)IcsGetDataTypeSize(icsStruct->imel.dataType);
}


/* Get the size of the Ics_DataType in bytes. */
size_t IcsGetDataTypeSize(Ics_DataType dataType)
{
    size_t bytes;


    switch (dataType) {
        case Ics_uint8:
        case Ics_sint8:
            bytes = 1;
            break;
        case Ics_uint16:
        case Ics_sint16:
            bytes = 2;
            break;
        case Ics_uint32:
        case Ics_sint32:
        case Ics_real32:
            bytes = 4;
            break;
        case Ics_uint64:
        case Ics_sint64:
        case Ics_real64:
        case Ics_complex32:
            bytes = 8;
            break;
        case Ics_complex64:
            bytes = 16;
            break;
        default:
            bytes = 0;
    }

    return bytes;
}


/* Get the properties of the Ics_DataType */
void IcsGetPropsDataType(Ics_DataType  dataType,
                         Ics_Format   *format,
                         int          *sign,
                         size_t       *bits)
{
    *bits = IcsGetDataTypeSize(dataType) * 8;
    *sign = 1;
    switch (dataType) {
        case Ics_uint8:
        case Ics_uint16:
        case Ics_uint32:
        case Ics_uint64:
            *sign = 0;
            /* fallthrough */
        case Ics_sint8:
        case Ics_sint16:
        case Ics_sint32:
        case Ics_sint64:
            *format = IcsForm_integer;
            break;
        case Ics_real32:
        case Ics_real64:
            *format = IcsForm_real;
            break;
        case Ics_complex32:
        case Ics_complex64:
            *format = IcsForm_complex;
            break;
        default:
            *format = IcsForm_unknown;
    }
}


/* Get the Ics_DataType belonging to the given properties */
void IcsGetDataTypeProps(Ics_DataType* dataType,
                         Ics_Format    format,
                         int           sign,
                         size_t        bits)
{
    switch (format) {
        case IcsForm_integer:
            switch (bits) {
                case 8:
                    *dataType = sign ? Ics_sint8 : Ics_uint8;
                    break;
                case 16:
                    *dataType = sign ? Ics_sint16 : Ics_uint16;
                    break;
                case 32:
                    *dataType = sign ? Ics_sint32 : Ics_uint32;
                    break;
                case 64:
                    *dataType = sign ? Ics_sint64 : Ics_uint64;
                    break;
                default:
                    *dataType = Ics_unknown;
            }
            break;
        case IcsForm_real:
            switch (bits) {
                case 32:
                    *dataType = Ics_real32;
                    break;
                case 64:
                    *dataType = Ics_real64;
                    break;
                default:
                    *dataType = Ics_unknown;
            }
            break;
        case IcsForm_complex:
            switch (bits) {
                case 2*32:
                    *dataType = Ics_complex32;
                    break;
                case 2*64:
                    *dataType = Ics_complex64;
                    break;
                default:
                    *dataType = Ics_unknown;
            }
            break;
        default:
            *dataType = Ics_unknown;
    }
}
