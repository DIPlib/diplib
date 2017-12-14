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
 * FILE : libics_read.c
 *
 * The following library functions are contained in this file:
 *
 *   IcsReadIcs()
 *   IcsVersion()
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libics_intern.h"


/* Find the index for "bits", which should be the first parameter. */
static int icsGetBitsParam(char order[ICS_MAXDIM+1][ICS_STRLEN_TOKEN],
                           int  parameters)
{
    int i;


    for (i = 0; i < parameters; i++) {
        if (strcmp(order[i], ICS_ORDER_BITS) == 0) {
            return i;
        }
    }

    return -1;
}


/* Like fgets(), gets a string from a stream. However, does not stop at newline
   character, but at 'sep'. It retains the 'sep' character at the end of the
   string; a null byte is appended. Also, it implements the solution to the
   CR/LF pair problem caused by some windows applications. If 'sep' is LF, it
   might be prepended by a CR. */
static char *icsFGetStr(char *line,
                        int   n,
                        FILE *fi,
                        char  sep)
{
        /* n == ICS_LINE_LENGTH */
    int i = 0;
    int ch;


        /* Imitate fgets() */
    while (i < n - 1) {
        ch = getc(fi);
        if (ch == EOF)
            break;

            /* Skip CR if next char is LF and sep is LF. */
        if (ch == '\r' && sep == '\n') {
            ch = getc(fi);
            if (ch != sep && ch != EOF) {
                ungetc(ch, fi);
                ch = '\r';
            }
        }

        line[i] =(char)ch;
        i++;
        if ((char)ch == sep)
            break;
    }
    line[i] = '\0';
    if (i != 0) {
            /* Read at least a 'sep' character */
        return line;
    } else {
            /* EOF at first getc() call */
        return NULL;
    }
}


/* Read the two ICS separators from file. There is a special case for ICS
   headers which are erroneously written under Windows in text mode causing a
   newline separator to be prepended by a carriage return.  Therefore when the
   second separator is a carriage return and the first separator is not a
   newline then peek at the third character to see if it is a newline.  If so
   then use newline as the second separator. Return IcsErr_FReadIcs on read
   errors and IcsErr_NotIcsFile on premature end-of-file. */
static Ics_Error getIcsSeparators(FILE *fi,
                                  char *seps)
{
    int sep1;
    int sep2;
    int sep3;


    sep1 = fgetc(fi);
    if (sep1 == EOF) {
        return (ferror(fi)) ? IcsErr_FReadIcs : IcsErr_NotIcsFile;
    }
    sep2 = fgetc(fi);
    if (sep2 == EOF) {
        return (ferror(fi)) ? IcsErr_FReadIcs : IcsErr_NotIcsFile;
    }
    if (sep1 == sep2) {
        return IcsErr_NotIcsFile;
    }
    if (sep2 == '\r' && sep1 != '\n') {
        sep3 = fgetc(fi);
        if (sep3 == EOF) {
            return (ferror(fi)) ? IcsErr_FReadIcs : IcsErr_NotIcsFile;
        } else {
            if (sep3 == '\n') {
                sep2 = '\n';
            } else {
                ungetc(sep3, fi);
            }
        }
    }
    seps[0] = (char)sep1;
    seps[1] = (char)sep2;
    seps[2] = '\0';

    return IcsErr_Ok;
}


static Ics_Error getIcsVersion(FILE       *fi,
                               const char *seps,
                               int        *ver)
{
    ICSINIT;
    char *word;
    char  line[ICS_LINE_LENGTH];


    if (icsFGetStr(line, ICS_LINE_LENGTH, fi, seps[1]) == NULL)
        return IcsErr_FReadIcs;
    word = strtok(line, seps);
    if (word == NULL) return IcsErr_NotIcsFile;
    if (strcmp(word, ICS_VERSION) != 0) return IcsErr_NotIcsFile;
    word = strtok(NULL, seps);
    if (word == NULL) return IcsErr_NotIcsFile;
    if (strcmp(word, "1.0") == 0) {
        *ver = 1;
    } else if (strcmp(word, "2.0") == 0) {
        *ver = 2;
    } else {
        error = IcsErr_NotIcsFile;
    }

    return error;
}


static Ics_Error getIcsFileName(FILE       *fi,
                                const char *seps)
{
    ICSINIT;
    char *word;
    char  line[ICS_LINE_LENGTH];


    if (icsFGetStr(line, ICS_LINE_LENGTH, fi, seps[1]) == NULL)
        return IcsErr_FReadIcs;
    word = strtok(line, seps);
    if (word == NULL) return IcsErr_NotIcsFile;
    if (strcmp(word, ICS_FILENAME) != 0) return IcsErr_NotIcsFile;

    return error;
}


static Ics_Token getIcsToken(char           *str,
                             Ics_SymbolList *listSpec)
{
    int i;
    Ics_Token token = ICSTOK_NONE;


    if (str != NULL) {
        for (i = 0; i < listSpec->entries; i++) {
            if (strcmp(listSpec->list[i].name, str) == 0) {
                token = listSpec->list[i].token;
            }
        }
    }

    return token;
}


static Ics_Error getIcsCat(char        *str,
                           const char  *seps,
                           Ics_Token   *cat,
                           Ics_Token   *subCat,
                           Ics_Token   *subSubCat,
                           const char **index)
{
    ICSINIT;
    char *token, buffer[ICS_LINE_LENGTH], *idx;


    *subCat = *subSubCat = ICSTOK_NONE;
    *index = NULL;

    IcsStrCpy(buffer, str, ICS_LINE_LENGTH);
    token = strtok(buffer, seps);
    *cat = getIcsToken(token, &G_Categories);
    if (*cat == ICSTOK_NONE) return IcsErr_MissCat;
    if ((*cat != ICSTOK_HISTORY) &&(*cat != ICSTOK_END)) {
        token = strtok(NULL, seps);
        *subCat = getIcsToken(token, &G_SubCategories);
        if (*subCat == ICSTOK_NONE) return IcsErr_MissSubCat;
        if (*subCat == ICSTOK_SPARAMS || *subCat == ICSTOK_SSTATES) {
            token = strtok(NULL, seps);
            if (token[strlen(token) - 1] == ']') {
                idx = strchr(token, '[');
                if (idx) {
                    token[strlen(token) - 1] = '\0';
                    *idx = '\0';
                    *index = idx + 1;
                }
            }
            *subSubCat = getIcsToken(token, &G_SubSubCategories);
            if (*subSubCat == ICSTOK_NONE) return IcsErr_MissSensorSubSubCat;
        }
    }

        /* Copy the remaining stuff into 'str' */
    if ((token = strtok(NULL, seps)) != NULL) {
        strcpy(str, token);
    }
    while ((token = strtok(NULL, seps)) != NULL) {
        IcsAppendChar(str, seps[0]);
        strcat(str, token);
    }

    return error;
}


static Ics_Error getIcsSensorState(char            *str,
                                   Ics_SensorState *state)
{
    ICSINIT;


    switch (getIcsToken(str, &G_Values)) {
        case ICSTOK_STATE_DEFAULT:
            *state = IcsSensorState_default;
            break;
        case ICSTOK_STATE_ESTIMATED:
            *state = IcsSensorState_estimated;
            break;
        case ICSTOK_STATE_REPORTED:
            *state = IcsSensorState_reported;
            break;
        case ICSTOK_STATE_VERIFIED:
            *state = IcsSensorState_verified;
            break;
        default:
             error = IcsErr_UnknownSensorState;
             break;
    }
    return error;
}


#define ICS_SET_SENSOR_STRING(FIELD)            \
do {                                            \
    while (ptr != NULL && i < ICS_MAX_LAMBDA) { \
        IcsStrCpy(icsStruct->FIELD[i++],        \
                  ptr, ICS_STRLEN_TOKEN);       \
        ptr = strtok(NULL, seps);               \
    }                                           \
} while (0)


#define ICS_SET_SENSOR_DOUBLE_ONE(FIELD)        \
do {                                            \
    if (ptr != NULL) {                          \
        icsStruct->FIELD = atof(ptr);           \
    }                                           \
} while (0)


#define ICS_SET_SENSOR_INT(FIELD)               \
do {                                            \
    while (ptr != NULL && i < ICS_MAX_LAMBDA) { \
        icsStruct->FIELD[i++] = atoi(ptr);      \
        ptr = strtok(NULL, seps);               \
    }                                           \
} while (0)


#define ICS_SET_SENSOR_DOUBLE(FIELD)            \
do {                                            \
    while (ptr != NULL && i < ICS_MAX_LAMBDA) { \
        icsStruct->FIELD[i++] = atof(ptr);      \
        ptr = strtok(NULL, seps);               \
    }                                           \
} while (0)


#define ICS_SET_SENSOR_STATE(FIELD)             \
do {                                            \
    while (ptr != NULL && i < ICS_MAX_LAMBDA) { \
        error = getIcsSensorState(ptr, &state); \
        icsStruct->FIELD ## State[i++] = state; \
        ptr = strtok(NULL, seps);               \
    }                                           \
} while(0)

    
#define ICS_SET_SENSOR_STATE_ONE(FIELD)         \
do {                                            \
    if (ptr != NULL) {                          \
        error = getIcsSensorState(ptr, &state); \
        icsStruct->FIELD ## State = state;      \
    }                                           \
} while(0)


Ics_Error IcsReadIcs(Ics_Header *icsStruct,
                     const char *filename,
                     int         forceName,
                     int         forceLocale)
{
    ICSINIT;
    ICS_INIT_LOCALE;
    FILE            *fp;
    int              end        = 0, si, sj;
    size_t           i, j;
    char             seps[3], *ptr, *data;
    char             line[ICS_LINE_LENGTH];
    Ics_Token        cat, subCat, subSubCat;
    const char      *idx;
        /* These are temporary buffers to hold the data read until it is copied
           to the Ics_Header structure. This is needed because the Ics_Header
           structure is made to look more like we like to see images, compared
           to the way the data is written in the ICS file. */
    Ics_Format       format     = IcsForm_unknown;
    int              sign       = 1;
    int              parameters = 0;
    char             order[ICS_MAXDIM+1][ICS_STRLEN_TOKEN];
    size_t           sizes[ICS_MAXDIM+1];
    double           origin[ICS_MAXDIM+1];
    double           scale[ICS_MAXDIM+1];
    char             label[ICS_MAXDIM+1][ICS_STRLEN_TOKEN];
    char             unit[ICS_MAXDIM+1][ICS_STRLEN_TOKEN];
    Ics_SensorState  state      = IcsSensorState_default;


    for (i = 0; i < ICS_MAXDIM+1; i++) {
        sizes[i] = 1;
        origin[i] = 0.0;
        scale[i] = 1.0;
        order[i][0] = '\0';
        label[i][0] = '\0';
        unit[i][0] = '\0';
    }

    IcsInit(icsStruct);
    icsStruct->fileMode = IcsFileMode_read;

    IcsStrCpy(icsStruct->filename, filename, ICS_MAXPATHLEN);
    error = IcsOpenIcs(&fp, icsStruct->filename, forceName);
    if (error) return error;

    if (forceLocale) {
        ICS_SET_LOCALE;
    }

    if (!error) error = getIcsSeparators(fp, seps);

    if (!error) error = getIcsVersion(fp, seps, &(icsStruct->version));
    if (!error) error = getIcsFileName(fp, seps);

    while (!end && !error
           && (icsFGetStr(line, ICS_LINE_LENGTH, fp, seps[1]) != NULL)) {
        if (getIcsCat(line, seps, &cat, &subCat, &subSubCat, &idx) != IcsErr_Ok)
            continue;
        ptr = strtok(line, seps);
        i = 0;
        switch (cat) {
            case ICSTOK_END:
                end = 1;
                if (icsStruct->srcFile[0] == '\0') {
                    icsStruct->srcOffset =(size_t) ftell(fp);
                    IcsStrCpy(icsStruct->srcFile, icsStruct->filename,
                              ICS_MAXPATHLEN);
                }
                break;
            case ICSTOK_SOURCE:
                switch (subCat) {
                    case ICSTOK_FILE:
                        if (ptr != NULL) {
                            IcsStrCpy(icsStruct->srcFile, ptr, ICS_MAXPATHLEN);
                        }
                        break;
                    case ICSTOK_OFFSET:
                        if (ptr != NULL) {
                            icsStruct->srcOffset = IcsStrToSize(ptr);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case ICSTOK_LAYOUT:
                switch (subCat) {
                    case ICSTOK_PARAMS:
                        if (ptr != NULL) {
                            parameters = atoi(ptr);
                            if (parameters > ICS_MAXDIM+1) {
                                error = IcsErr_TooManyDims;
                            }
                        }
                        break;
                    case ICSTOK_ORDER:
                        while (ptr!= NULL && i < ICS_MAXDIM+1) {
                            IcsStrCpy(order[i++], ptr, ICS_STRLEN_TOKEN);
                            ptr = strtok(NULL, seps);
                        }
                        break;
                    case ICSTOK_SIZES:
                        while (ptr!= NULL && i < ICS_MAXDIM+1) {
                            sizes[i++] = IcsStrToSize(ptr);
                            ptr = strtok(NULL, seps);
                        }
                        break;
                    case ICSTOK_COORD:
                        if (ptr != NULL) {
                            IcsStrCpy(icsStruct->coord, ptr, ICS_STRLEN_TOKEN);
                        }
                        break;
                    case ICSTOK_SIGBIT:
                        if (ptr != NULL) {
                            icsStruct->imel.sigBits = IcsStrToSize(ptr);
                        }
                        break;
                    default:
                        error = IcsErr_MissLayoutSubCat;
                }
                break;
            case ICSTOK_REPRES:
                switch (subCat) {
                    case ICSTOK_FORMAT:
                        switch (getIcsToken(ptr, &G_Values)) {
                            case ICSTOK_FORMAT_INTEGER:
                                format = IcsForm_integer;
                                break;
                            case ICSTOK_FORMAT_REAL:
                                format = IcsForm_real;
                                break;
                            case ICSTOK_FORMAT_COMPLEX:
                                format = IcsForm_complex;
                                break;
                            default:
                                format = IcsForm_unknown;
                        }
                        break;
                    case ICSTOK_SIGN:
                    {
                        Ics_Token tok = getIcsToken(ptr, &G_Values);
                        if (tok == ICSTOK_SIGN_UNSIGNED) {
                            sign = 0;
                        } else {
                            sign = 1;
                        }
                        break;
                    }
                    case ICSTOK_SCILT:
                        if (ptr!= NULL) {
                            IcsStrCpy(icsStruct->scilType, ptr,
                                      ICS_STRLEN_TOKEN);
                        }
                        break;
                    case ICSTOK_COMPR:
                        switch (getIcsToken(ptr, &G_Values)) {
                            case ICSTOK_COMPR_UNCOMPRESSED:
                                icsStruct->compression = IcsCompr_uncompressed;
                                break;
                            case ICSTOK_COMPR_COMPRESS:
                                if (icsStruct->version == 1) {
                                    icsStruct->compression = IcsCompr_compress;
                                } else { /* A version 2.0 file never uses
                                            COMPRESS, maybe it means GZIP? */
                                    icsStruct->compression = IcsCompr_gzip;
                                }
                                break;
                            case ICSTOK_COMPR_GZIP:
                                icsStruct->compression = IcsCompr_gzip;
                                break;
                            default:
                                error = IcsErr_UnknownCompression;
                        }
                        break;
                    case ICSTOK_BYTEO:
                        while (ptr!= NULL && i < ICS_MAX_IMEL_SIZE) {
                            icsStruct->byteOrder[i++] = atoi(ptr);
                            ptr = strtok(NULL, seps);
                        }
                        break;
                    default:
                        error = IcsErr_MissRepresSubCat;
                        break;
                }
                break;
            case ICSTOK_PARAM:
                switch (subCat) {
                    case ICSTOK_ORIGIN:
                        while (ptr!= NULL && i < ICS_MAXDIM+1) {
                            origin[i++] = atof(ptr);
                            ptr = strtok(NULL, seps);
                        }
                        break;
                    case ICSTOK_SCALE:
                        while (ptr!= NULL && i < ICS_MAXDIM+1) {
                            scale[i++] = atof(ptr);
                            ptr = strtok(NULL, seps);
                        }
                        break;
                    case ICSTOK_UNITS:
                        while (ptr!= NULL && i < ICS_MAXDIM+1) {
                            IcsStrCpy(unit[i++], ptr, ICS_STRLEN_TOKEN);
                            ptr = strtok(NULL, seps);
                        }
                        break;
                    case ICSTOK_LABELS:
                        while (ptr!= NULL && i < ICS_MAXDIM+1) {
                            IcsStrCpy(label[i++], ptr, ICS_STRLEN_TOKEN);
                            ptr = strtok(NULL, seps);
                        }
                        break;
                    default:
                        error = IcsErr_MissParamSubCat;
                }
                break;
            case ICSTOK_HISTORY:
                if (ptr != NULL) {
                    data = strtok(NULL, seps+1); /* This will get the rest of
                                                    the line */
                    if (data == NULL) { /* data is not allowed to be "", but ptr
                                           is */
                        data = ptr;
                        ptr = "";
                    }
                        /* The next portion is to avoid having
                           IcsInternAddHistory return IcsErr_LineOverflow. */
                    i = strlen(ptr);
                    if (i+1 > ICS_STRLEN_TOKEN) {
                        ptr[ICS_STRLEN_TOKEN-1] = '\0';
                        i = ICS_STRLEN_TOKEN-1;
                    }
                    j = strlen(ICS_HISTORY);
                    if ((strlen(data) + i + j + 4) > ICS_LINE_LENGTH) {
                        data[ICS_LINE_LENGTH - i - j - 4] = '\0';
                    }
                    error = IcsInternAddHistory(icsStruct, ptr, data, seps);
                }
                break;
            case ICSTOK_SENSOR:
                switch (subCat) {
                    case ICSTOK_TYPE:
                        while (ptr != NULL && i < ICS_MAX_LAMBDA) {
                            IcsStrCpy(icsStruct->type[i++], ptr,
                                      ICS_STRLEN_TOKEN);
                            ptr = strtok(NULL, seps);
                        }
                        break;
                    case ICSTOK_MODEL:
                        if (ptr != NULL) {
                            IcsStrCpy(icsStruct->model, ptr, ICS_STRLEN_OTHER);
                        }
                        break;
                    case ICSTOK_SPARAMS:
                        switch (subSubCat) {
                            case ICSTOK_CHANS:
                                if (ptr != NULL) {
                                    int v = atoi(ptr);
                                    icsStruct->sensorChannels = v;
                                    if (v > ICS_MAX_LAMBDA) {
                                        error = IcsErr_TooManyChans;
                                    }
                                }
                                break;
                            case ICSTOK_IMDIR:
                                ICS_SET_SENSOR_STRING(imagingDirection);
                                break;
                            case ICSTOK_NUMAPER:
                                ICS_SET_SENSOR_DOUBLE_ONE(numAperture);
                                break;
                            case ICSTOK_OBJQ:
                                ICS_SET_SENSOR_INT(objectiveQuality);
                                break;
                            case ICSTOK_REFRIME:
                                ICS_SET_SENSOR_DOUBLE_ONE(refrInxMedium);
                                break;
                            case ICSTOK_REFRILM:
                                ICS_SET_SENSOR_DOUBLE_ONE(refrInxLensMedium);
                                break;
                            case ICSTOK_PINHRAD:
                                ICS_SET_SENSOR_DOUBLE(pinholeRadius);
                                break;
                            case ICSTOK_ILLPINHRAD:
                                ICS_SET_SENSOR_DOUBLE(illPinholeRadius);
                                break;
                            case ICSTOK_PINHSPA:
                                ICS_SET_SENSOR_DOUBLE_ONE(pinholeSpacing);
                                break;
                            case ICSTOK_EXBFILL:
                                ICS_SET_SENSOR_DOUBLE(excitationBeamFill);
                                break;
                            case ICSTOK_LAMBDEX:
                                ICS_SET_SENSOR_DOUBLE(lambdaEx);
                                break;
                            case ICSTOK_LAMBDEM:
                                ICS_SET_SENSOR_DOUBLE(lambdaEm);
                                break;
                            case ICSTOK_PHOTCNT:
                                ICS_SET_SENSOR_INT(exPhotonCnt);
                                break;
                            case ICSTOK_IFACE1:
                                ICS_SET_SENSOR_DOUBLE_ONE(interfacePrimary);
                                break;
                            case ICSTOK_IFACE2:
                                ICS_SET_SENSOR_DOUBLE_ONE(interfaceSecondary);
                                break;
                            case ICSTOK_DETMAG:
                                ICS_SET_SENSOR_DOUBLE(detectorMagn);
                                break;
                            case ICSTOK_DETPPU:
                                ICS_SET_SENSOR_DOUBLE(detectorPPU);
                                break;
                            case ICSTOK_DETBASELINE:
                                ICS_SET_SENSOR_DOUBLE(detectorBaseline);
                                break;
                            case ICSTOK_DETLNAVGCNT:
                                ICS_SET_SENSOR_DOUBLE(detectorLineAvgCnt);
                                break;
                            case ICSTOK_STEDDEPLMODE:
                                ICS_SET_SENSOR_STRING(stedDepletionMode);
                                break;
                            case ICSTOK_STEDLAMBDA:
                                ICS_SET_SENSOR_DOUBLE(stedLambda);
                                break;
                            case ICSTOK_STEDSATFACTOR:
                                ICS_SET_SENSOR_DOUBLE(stedSatFactor);
                                break;
                            case ICSTOK_STEDIMMFRACTION:
                                ICS_SET_SENSOR_DOUBLE(stedImmFraction);
                                break;
                            case ICSTOK_STEDVPPM:
                                ICS_SET_SENSOR_DOUBLE(stedVPPM);
                                break;
                            case ICSTOK_SPIMEXCTYPE:
                                ICS_SET_SENSOR_STRING(spimExcType);
                                break;
                            case ICSTOK_SPIMFILLFACTOR:
                                ICS_SET_SENSOR_DOUBLE(spimFillFactor);
                                break;
                            case ICSTOK_SPIMPLANENA:
                                ICS_SET_SENSOR_DOUBLE(spimPlaneNA);
                                break;
                            case ICSTOK_SPIMPLANEGAUSSWIDTH:
                                ICS_SET_SENSOR_DOUBLE(spimPlaneGaussWidth);
                                break;
                            case ICSTOK_SPIMPLANEPROPDIR:
                                while (ptr != NULL && i < ICS_MAX_LAMBDA) {
                                    switch (idx[0]) {
                                        case  'X':
                                            icsStruct->spimPlanePropDir[i++][0]
                                                = atof(ptr);
                                            break;
                                        case  'Y':
                                            icsStruct->spimPlanePropDir[i++][1]
                                                = atof(ptr);
                                            break;
                                        case  'Z':
                                            icsStruct->spimPlanePropDir[i++][2]
                                                = atof(ptr);
                                            break;
                                        default:
                                            break;
                                    }
                                    ptr = strtok(NULL, seps);
                                }
                                break;
                            case ICSTOK_SPIMPLANECENTEROFF:
                                ICS_SET_SENSOR_DOUBLE(spimPlaneCenterOff);
                                break;
                            case ICSTOK_SPIMPLANEFOCUSOF:
                                ICS_SET_SENSOR_DOUBLE(spimPlaneFocusOff);
                                break;
                            case ICSTOK_SCATTERMODEL:
                                ICS_SET_SENSOR_STRING(scatterModel);
                                break;
                            case ICSTOK_SCATTERFREEPATH:
                                ICS_SET_SENSOR_DOUBLE(scatterFreePath);
                                break;
                            case ICSTOK_SCATTERRELCONTRIB:
                                ICS_SET_SENSOR_DOUBLE(scatterRelContrib);
                                break;
                            case ICSTOK_SCATTERBLURRING:
                                ICS_SET_SENSOR_DOUBLE(scatterBlurring);
                                break;
                            default:
                                error = IcsErr_MissSensorSubSubCat;
                        }
                        break;
                    case ICSTOK_SSTATES:
                        switch (subSubCat) {
                            case ICSTOK_IMDIR:
                                ICS_SET_SENSOR_STATE(imagingDirection);
                                break;
                            case ICSTOK_NUMAPER:
                                ICS_SET_SENSOR_STATE_ONE(numAperture);
                                break;
                            case ICSTOK_OBJQ:
                                ICS_SET_SENSOR_STATE(objectiveQuality);
                                break;
                            case ICSTOK_REFRIME:
                                ICS_SET_SENSOR_STATE_ONE(refrInxMedium);
                                break;
                            case ICSTOK_REFRILM:
                                ICS_SET_SENSOR_STATE_ONE(refrInxLensMedium);
                                break;
                            case ICSTOK_PINHRAD:
                                ICS_SET_SENSOR_STATE(pinholeRadius);
                                break;
                            case ICSTOK_ILLPINHRAD:
                                ICS_SET_SENSOR_STATE(illPinholeRadius);
                                break;
                            case ICSTOK_PINHSPA:
                                ICS_SET_SENSOR_STATE_ONE(pinholeSpacing);
                                break;
                            case ICSTOK_EXBFILL:
                                ICS_SET_SENSOR_STATE(excitationBeamFill);
                                break;
                            case ICSTOK_LAMBDEX:
                                ICS_SET_SENSOR_STATE(lambdaEx);
                                break;
                            case ICSTOK_LAMBDEM:
                                ICS_SET_SENSOR_STATE(lambdaEm);
                                break;
                            case ICSTOK_PHOTCNT:
                                ICS_SET_SENSOR_STATE(exPhotonCnt);
                                break;
                            case ICSTOK_IFACE1:
                                ICS_SET_SENSOR_STATE_ONE(interfacePrimary);
                                break;
                            case ICSTOK_IFACE2:
                                ICS_SET_SENSOR_STATE_ONE(interfaceSecondary);
                                break;
                            case ICSTOK_DETMAG:
                                ICS_SET_SENSOR_STATE(detectorMagn);
                                break;
                            case ICSTOK_DETPPU:
                                ICS_SET_SENSOR_STATE(detectorPPU);
                                break;
                            case ICSTOK_DETBASELINE:
                                ICS_SET_SENSOR_STATE(detectorBaseline);
                                break;
                            case ICSTOK_DETLNAVGCNT:
                                ICS_SET_SENSOR_STATE(detectorLineAvgCnt);
                                break;
                            case ICSTOK_STEDDEPLMODE:
                                ICS_SET_SENSOR_STATE(stedDepletionMode);
                                break;
                            case ICSTOK_STEDLAMBDA:
                                ICS_SET_SENSOR_STATE(stedLambda);
                                break;
                            case ICSTOK_STEDSATFACTOR:
                                ICS_SET_SENSOR_STATE(stedSatFactor);
                                break;
                            case ICSTOK_STEDIMMFRACTION:
                                ICS_SET_SENSOR_STATE(stedImmFraction);
                                break;
                            case ICSTOK_STEDVPPM:
                                ICS_SET_SENSOR_STATE(stedVPPM);
                                break;
                            case ICSTOK_SPIMEXCTYPE:
                                ICS_SET_SENSOR_STATE(spimExcType);
                                break;
                            case ICSTOK_SPIMFILLFACTOR:
                                ICS_SET_SENSOR_STATE(spimFillFactor);
                                break;
                            case ICSTOK_SPIMPLANENA:
                                ICS_SET_SENSOR_STATE(spimPlaneNA);
                                break;
                            case ICSTOK_SPIMPLANEGAUSSWIDTH:
                                ICS_SET_SENSOR_STATE(spimPlaneGaussWidth);
                                break;
                            case ICSTOK_SPIMPLANEPROPDIR:
                                ICS_SET_SENSOR_STATE(spimPlanePropDir);
                                break;
                            case ICSTOK_SPIMPLANECENTEROFF:
                                ICS_SET_SENSOR_STATE(spimPlaneCenterOff);
                                break;
                            case ICSTOK_SPIMPLANEFOCUSOF:
                                ICS_SET_SENSOR_STATE(spimPlaneFocusOff);
                                break;
                            case ICSTOK_SCATTERMODEL:
                                ICS_SET_SENSOR_STATE(scatterModel);
                                break;
                            case ICSTOK_SCATTERFREEPATH:
                                ICS_SET_SENSOR_STATE(scatterFreePath);
                                break;
                            case ICSTOK_SCATTERRELCONTRIB:
                                ICS_SET_SENSOR_STATE(scatterRelContrib);
                                break;
                            case ICSTOK_SCATTERBLURRING:
                                ICS_SET_SENSOR_STATE(scatterBlurring);
                                break;
                            default:
                                error = IcsErr_MissSensorSubSubCat;
                        }
                        break;
                    default:
                        error = IcsErr_MissSensorSubCat;
                }
                break;
            default:
                error = IcsErr_MissCat;
        }
    }

        /* In newer libics versions(> 1.5.2) a microscope type is specified per
           sensor channel. For files from previous libics versions a single
           microscope type is stored. To allow compatibility. when reading older
           files in which a single microscope type is defined and multiple
           sensor channels, the microscope type will be duplicated to all sensor
           channels. */
    for (sj = 1; sj < icsStruct->sensorChannels; sj++) {
        if (strlen(icsStruct->type[sj]) == 0) {
            IcsStrCpy(icsStruct->type[sj], icsStruct->type[0],
                       ICS_STRLEN_TOKEN);
        }
    }

    if (!error) {
        int bits = icsGetBitsParam(order, parameters);
        if (bits < 0) {
            error = IcsErr_MissBits;
        } else {
            IcsGetDataTypeProps(&(icsStruct->imel.dataType), format, sign,
                                sizes[bits]);
            for (sj = 0, si = 0; si < parameters; si++) {
                if (si == bits) {
                    icsStruct->imel.origin = origin[si];
                    icsStruct->imel.scale = scale[i];
                    strcpy(icsStruct->imel.unit, unit[si]);
                } else {
                    icsStruct->dim[sj].size = sizes[si];
                    icsStruct->dim[sj].origin = origin[si];
                    icsStruct->dim[sj].scale = scale[si];
                    strcpy(icsStruct->dim[sj].order, order[si]);
                    strcpy(icsStruct->dim[sj].label, label[si]);
                    strcpy(icsStruct->dim[sj].unit, unit[si]);
                    sj++;
                }
            }
            icsStruct->dimensions = parameters - 1;
        }
    }

    if (forceLocale) {
        ICS_REVERT_LOCALE;
    }

    if (fclose(fp) == EOF) {
        if (!error) error = IcsErr_FCloseIcs; /* Don't overwrite any previous
                                                 error. */
    }
    return error;
}


/* Read the first 3 lines of an ICS file to see which version it is. It returns
   0 if it is not an ICS file, or the version number if it is. */
int IcsVersion(const char *filename,
               int         forceName)
{
    ICSINIT;
    ICS_INIT_LOCALE;
    int   version;
    FILE *fp;
    char  FileName[ICS_MAXPATHLEN];
    char  seps[3];


    IcsStrCpy(FileName, filename, ICS_MAXPATHLEN);
    error = IcsOpenIcs(&fp, FileName, forceName);
    if (error) return 0;
    version = 0;
    ICS_SET_LOCALE;
    if (!error) error = getIcsSeparators(fp, seps);
    if (!error) error = getIcsVersion(fp, seps, &version);
    if (!error) error = getIcsFileName(fp, seps);
    ICS_REVERT_LOCALE;
    if (fclose(fp) == EOF) {
        return 0;
    }
    return error ? 0 : version;
}
