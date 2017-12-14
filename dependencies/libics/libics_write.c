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
 * FILE : libics_write.c
 *
 * The following library functions are contained in this file:
 *
 *   IcsWriteIcs()
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "libics_intern.h"

static Ics_Error icsToken2Str(Ics_Token  token,
                              char      *cPtr)
{
    ICSINIT;
    int notFound = 1, i;


        /* Search the globally defined categories for a token match: */
    i = 0;
    while (notFound && i < G_Categories.entries) {
        notFound = token != G_Categories.list[i].token;
        if(!notFound) {
            strcpy(cPtr, G_Categories.list[i].name);
        }
        i++;
    }
    i = 0;
    while (notFound && i < G_SubCategories.entries) {
        notFound = token != G_SubCategories.list[i].token;
        if(!notFound) {
            strcpy(cPtr, G_SubCategories.list[i].name);
        }
        i++;
    }
    i = 0;
    while (notFound && i < G_SubSubCategories.entries) {
        notFound = token != G_SubSubCategories.list[i].token;
        if(!notFound) {
            strcpy(cPtr, G_SubSubCategories.list[i].name);
        }
        i++;
    }
    i = 0;
    while (notFound && i < G_Values.entries) {
        notFound = token != G_Values.list[i].token;
        if (!notFound) {
            strcpy(cPtr, G_Values.list[i].name);
        }
        i++;
    }
    if (notFound) return IcsErr_IllIcsToken;

    return error;
}


static Ics_Error icsFirstToken(char      *line,
                               Ics_Token  token)
{
    ICSINIT;
    char tokenName[ICS_STRLEN_TOKEN];


    error = icsToken2Str(token, tokenName);
    if (error) return error;
    strcpy(line, tokenName);
    IcsAppendChar(line, ICS_FIELD_SEP);

    return error;
}


static Ics_Error icsAddToken(char      *line,
                             Ics_Token  token)
{
    ICSINIT;
    char tokenName[ICS_STRLEN_TOKEN];


    error = icsToken2Str(token, tokenName);
    if (error) return error;
    if (strlen(line) + strlen(tokenName) + 2 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
    strcat(line, tokenName);
    IcsAppendChar(line, ICS_FIELD_SEP);

    return error;
}


static Ics_Error icsAddLastToken(char      *line,
                                 Ics_Token  token)
{
    ICSINIT;
    char tokenName[ICS_STRLEN_TOKEN];


    error = icsToken2Str(token, tokenName);
    if (error) return error;
    if (strlen(line) + strlen(tokenName) + 2 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
    strcat(line, tokenName);
    IcsAppendChar(line, ICS_EOL);

    return error;
}


static Ics_Error icsAddTokenWithIndex(char        *line,
                                      Ics_Token   token,
                                      const char *index)
{
    ICSINIT;
    char tokenName[ICS_STRLEN_TOKEN];


    error = icsToken2Str(token, tokenName);
    if (error) return error;
    if (strlen(line) + strlen(tokenName) + strlen(index) + 4 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
    strcat(line, tokenName);
    strcat(line, "[");
    strcat(line, index);
    strcat(line, "]");
    IcsAppendChar(line, ICS_FIELD_SEP);

    return error;
}


static Ics_Error icsFirstText(char *line,
                              char *text)
{
    ICSINIT;


    if (text[0] == '\0') return IcsErr_EmptyField;
    if (strlen(text) + 2 > ICS_LINE_LENGTH) return IcsErr_LineOverflow;
    strcpy(line, text);
    IcsAppendChar(line, ICS_FIELD_SEP);

    return error;
}


static Ics_Error icsAddText(char *line,
                            char *text)
{
    ICSINIT;


    if (text[0] == '\0') return IcsErr_EmptyField;
    if (strlen(line) + strlen(text) + 2 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
    strcat(line, text);
    IcsAppendChar(line, ICS_FIELD_SEP);

    return error;
}


static Ics_Error icsAddLastText(char *line,
                                char *text)
{
    ICSINIT;


    if (text[0] == '\0') return IcsErr_EmptyField;
    if (strlen(line) + strlen(text) + 2 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
    strcat(line, text);
    IcsAppendChar(line, ICS_EOL);

    return error;
}


static Ics_Error icsAddInt(char     *line,
                           long int  i)
{
    ICSINIT;
    char intStr[ICS_STRLEN_OTHER];


    sprintf(intStr, "%ld%c", i, ICS_FIELD_SEP);
    if (strlen(line) + strlen(intStr) + 1 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
    strcat(line, intStr);

    return error;
}


static Ics_Error icsAddLastInt(char     *line,
                               long int  i)
{
    ICSINIT;
    char intStr[ICS_STRLEN_OTHER];


    sprintf(intStr, "%ld%c", i, ICS_EOL);
    if (strlen(line) + strlen(intStr) + 1 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
    strcat(line, intStr);

    return error;
}


static Ics_Error icsAddDouble(char   *line,
                              double  d)
{
    ICSINIT;
    char dStr[ICS_STRLEN_OTHER];


    if (d == 0 ||(fabs(d) < ICS_MAX_DOUBLE && fabs(d) >= ICS_MIN_DOUBLE)) {
        sprintf(dStr, "%f%c", d, ICS_FIELD_SEP);
    } else {
        sprintf(dStr, "%e%c", d, ICS_FIELD_SEP);
    }
    if (strlen(line) + strlen(dStr) + 1 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
    strcat(line, dStr);

    return error;
}


static Ics_Error icsAddLastDouble(char   *line,
                                  double  d)
{
    ICSINIT;
    char dStr[ICS_STRLEN_OTHER];


    if (d == 0 || (fabs(d) < ICS_MAX_DOUBLE && fabs(d) >= ICS_MIN_DOUBLE)) {
        sprintf(dStr, "%f%c", d, ICS_EOL);
    } else {
        sprintf(dStr, "%e%c", d, ICS_EOL);
    }
    if (strlen(line) + strlen(dStr) + 1 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
    strcat(line, dStr);

    return error;
}


static Ics_Error icsAddSensorState(char            *line,
                                   Ics_SensorState  state)
{
    ICSINIT;


    switch (state) {
        case IcsSensorState_default:
            error = icsAddToken(line, ICSTOK_STATE_DEFAULT);
            break;
        case IcsSensorState_estimated:
            error = icsAddToken(line, ICSTOK_STATE_ESTIMATED);
            break;
        case IcsSensorState_reported:
            error = icsAddToken(line, ICSTOK_STATE_REPORTED);
            break;
        case IcsSensorState_verified:
            error = icsAddToken(line, ICSTOK_STATE_VERIFIED);
            break;
        default:
            error = IcsErr_UnknownSensorState;
            break;
    }

    return error;
}


static Ics_Error icsAddLastSensorState(char            *line,
                                       Ics_SensorState  state)
{
    ICSINIT;


    switch (state) {
        case IcsSensorState_default:
            error = icsAddLastToken(line, ICSTOK_STATE_DEFAULT);
            break;
        case IcsSensorState_estimated:
            error = icsAddLastToken(line, ICSTOK_STATE_ESTIMATED);
            break;
        case IcsSensorState_reported:
            error = icsAddLastToken(line, ICSTOK_STATE_REPORTED);
            break;
        case IcsSensorState_verified:
            error = icsAddLastToken(line, ICSTOK_STATE_VERIFIED);
            break;
        default:
            error = IcsErr_UnknownSensorState;
            break;
    }

    return error;
}


static Ics_Error icsAddLine(char *line,
                            FILE *fp)
{
    ICSINIT;


    if (fputs(line, fp) == EOF) return IcsErr_FWriteIcs;

    return error;
}


static Ics_Error writeIcsSource(Ics_Header *icsStruct,
                                FILE       *fp)
{
    ICSINIT;
    unsigned int  problem;
    char line[ICS_LINE_LENGTH];


    if ((icsStruct->version >= 2) &&(icsStruct->srcFile[0] != '\0')) {
            /* Write the source filename to the file */
        problem = icsFirstToken(line, ICSTOK_SOURCE);
        problem |= icsAddToken(line, ICSTOK_FILE);
        problem |= icsAddLastText(line, icsStruct->srcFile);
        if (problem) return IcsErr_FailWriteLine;
        error = icsAddLine(line, fp);
        if (error) return error;

            /* Now write the source file offset to the file */
        problem = icsFirstToken(line, ICSTOK_SOURCE);
        problem |= icsAddToken(line, ICSTOK_OFFSET);
        problem |= icsAddLastInt(line,(long int)icsStruct->srcOffset);
        if (problem) return IcsErr_FailWriteLine;
        error = icsAddLine(line, fp);
        if (error) return error;
    }

    return error;
}


static Ics_Error writeIcsLayout(Ics_Header *icsStruct,
                                FILE       *fp)
{
    ICSINIT;
    unsigned int    problem;
    int    i;
    char   line[ICS_LINE_LENGTH];
    size_t size;


        /* Write the number of parameters to the buffer: */
    if (icsStruct->dimensions < 1) return IcsErr_NoLayout;
    if (icsStruct->dimensions > ICS_MAXDIM) return IcsErr_TooManyDims;
    problem = icsFirstToken(line, ICSTOK_LAYOUT);
    problem |= icsAddToken(line, ICSTOK_PARAMS);
    problem |= icsAddLastInt(line, icsStruct->dimensions + 1);
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

        /* Now write the order identifiers to the buffer: */
    problem = icsFirstToken(line, ICSTOK_LAYOUT);
    problem |= icsAddToken(line, ICSTOK_ORDER);
    problem |= icsAddText(line, ICS_ORDER_BITS);
    for (i = 0; i < icsStruct->dimensions-1; i++) {
        if (*(icsStruct->dim[i].order) == '\0') return IcsErr_NoLayout;
        problem |= icsAddText(line, icsStruct->dim[i].order);
    }
    if (*(icsStruct->dim[i].order) == '\0') return IcsErr_NoLayout;
    problem |= icsAddLastText(line, icsStruct->dim[i].order);
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

        /* Write the sizes: */
    problem = icsFirstToken(line, ICSTOK_LAYOUT);
    problem |= icsAddToken(line, ICSTOK_SIZES);
    size = IcsGetDataTypeSize(icsStruct->imel.dataType);
    problem |= icsAddInt(line,(long int)size * 8);
    for (i = 0; i < icsStruct->dimensions-1; i++) {
        if (icsStruct->dim[i].size == 0) return IcsErr_NoLayout;
        problem |= icsAddInt(line,(long int) icsStruct->dim[i].size);
    }
    if (icsStruct->dim[i].size == 0) return IcsErr_NoLayout;
    problem |= icsAddLastInt(line,(long int) icsStruct->dim[i].size);
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

        /* Coordinates class. Video(default) means 0,0 corresponds with
           top-left. */
    if (*(icsStruct->coord) == '\0') {
        strcpy(icsStruct->coord, ICS_COORD_VIDEO);
    }
    problem = icsFirstToken(line, ICSTOK_LAYOUT);
    problem |= icsAddToken(line, ICSTOK_COORD);
    problem |= icsAddLastText(line, icsStruct->coord);
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

        /* Number of significant bits, default is the number of bits/sample: */
    if (icsStruct->imel.sigBits == 0) {
        size = IcsGetDataTypeSize(icsStruct->imel.dataType);
        icsStruct->imel.sigBits = size * 8;
    }
    problem = icsFirstToken(line, ICSTOK_LAYOUT);
    problem |= icsAddToken(line, ICSTOK_SIGBIT);
    problem |= icsAddLastInt(line,(long int)icsStruct->imel.sigBits);
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

    return error;
}


static Ics_Error writeIcsRep(Ics_Header *icsStruct,
                             FILE       *fp)
{
    ICSINIT;
    unsigned int    problem;
    int        empty, i;
    char       line[ICS_LINE_LENGTH];
    Ics_Format format;
    int        sign;
    size_t     bits;


    IcsGetPropsDataType(icsStruct->imel.dataType, &format, &sign, &bits);

        /* Write basic format, i.e. integer, float or complex, default is
           integer: */
    problem = icsFirstToken(line, ICSTOK_REPRES);
    problem |= icsAddToken(line, ICSTOK_FORMAT);
    switch(format) {
        case IcsForm_integer:
            problem |= icsAddLastToken(line, ICSTOK_FORMAT_INTEGER);
            break;
        case IcsForm_real:
            problem |= icsAddLastToken(line, ICSTOK_FORMAT_REAL);
            break;
        case IcsForm_complex:
            problem |= icsAddLastToken(line, ICSTOK_FORMAT_COMPLEX);
            break;
        default:
            return IcsErr_UnknownDataType;
    }
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

        /* Signal whether the 'basic format' is signed or unsigned. Rubbish for
           float or complex, but this seems to be the definition. */
    problem = icsFirstToken(line, ICSTOK_REPRES);
    problem |= icsAddToken(line, ICSTOK_SIGN);
    if (sign == 1) {
        problem |= icsAddLastToken(line, ICSTOK_SIGN_SIGNED);
    } else {
        problem |= icsAddLastToken(line, ICSTOK_SIGN_UNSIGNED);
    }
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

        /* Signal whether the entire data array is compressed and if so by what
           compression technique: */
    problem = icsFirstToken(line, ICSTOK_REPRES);
    problem |= icsAddToken(line, ICSTOK_COMPR);
    switch(icsStruct->compression) {
        case IcsCompr_uncompressed:
            problem |= icsAddLastToken(line, ICSTOK_COMPR_UNCOMPRESSED);
            break;
        case IcsCompr_compress:
            problem |= icsAddLastToken(line, ICSTOK_COMPR_COMPRESS);
            break;
        case IcsCompr_gzip:
            problem |= icsAddLastToken(line, ICSTOK_COMPR_GZIP);
            break;
        default:
            return IcsErr_UnknownCompression;
    }
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

        /* Define the byteorder. This is supposed to resolve little/big endian
           problems. If the calling function put something here, we'll keep
           it. Otherwise we fill in the machine's byte order. */
    empty = 0;
    for (i = 0; i <(int)IcsGetDataTypeSize(icsStruct->imel.dataType); i++) {
        empty |= !(icsStruct->byteOrder[i]);
    }
    if (empty) {
        IcsFillByteOrder((int)IcsGetDataTypeSize(icsStruct->imel.dataType),
                         icsStruct->byteOrder);
    }
    problem = icsFirstToken(line, ICSTOK_REPRES);
    problem |= icsAddToken(line, ICSTOK_BYTEO);
    for (i = 0; i <(int)IcsGetDataTypeSize(icsStruct->imel.dataType) - 1; i++) {
        problem |= icsAddInt(line, icsStruct->byteOrder[i]);
    }
    problem |= icsAddLastInt(line, icsStruct->byteOrder[i]);
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

        /* SCIL_Image compatability stuff: SCIL_TYPE */
    if (icsStruct->scilType[0] != '\0') {
        problem = icsFirstToken(line, ICSTOK_REPRES);
        problem |= icsAddToken(line, ICSTOK_SCILT);
        problem |= icsAddLastText(line, icsStruct->scilType);
        if (problem) return IcsErr_FailWriteLine;
        error = icsAddLine(line, fp);
    }    if (error) return error;


    return error;
}


static Ics_Error writeIcsParam(Ics_Header *icsStruct,
                               FILE       *fp)
{
    ICSINIT;
    unsigned int    problem;
    int  i;
    char line[ICS_LINE_LENGTH];


        /* Define the origin, scaling factors and the units */
    problem = icsFirstToken(line, ICSTOK_PARAM);
    problem |= icsAddToken(line, ICSTOK_ORIGIN);
    problem |= icsAddDouble(line, icsStruct->imel.origin);
    for (i = 0; i < icsStruct->dimensions-1; i++) {
        problem |= icsAddDouble(line, icsStruct->dim[i].origin);
    }
    problem |= icsAddLastDouble(line, icsStruct->dim[i].origin);
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

    problem = icsFirstToken(line, ICSTOK_PARAM);
    problem |= icsAddToken(line, ICSTOK_SCALE);
    problem |= icsAddDouble(line, icsStruct->imel.scale);
    for (i = 0; i < icsStruct->dimensions-1; i++) {
        problem |= icsAddDouble(line, icsStruct->dim[i].scale);
    }
    problem |= icsAddLastDouble(line, icsStruct->dim[i].scale);
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

    problem = icsFirstToken(line, ICSTOK_PARAM);
    problem |= icsAddToken(line, ICSTOK_UNITS);
    if (icsStruct->imel.unit[0] == '\0') {
        problem |= icsAddText(line, ICS_UNITS_RELATIVE);
    } else {
        problem |= icsAddText(line, icsStruct->imel.unit);
    }
    for (i = 0; i < icsStruct->dimensions-1; i++) {
        if (icsStruct->dim[i].unit[0] == '\0') {
            problem |= icsAddText(line, ICS_UNITS_UNDEFINED);
        } else {
            problem |= icsAddText(line, icsStruct->dim[i].unit);
        }
    }
    if (icsStruct->dim[i].unit[0] == '\0') {
        problem |= icsAddLastText(line, ICS_UNITS_UNDEFINED);
    } else {
        problem |= icsAddLastText(line, icsStruct->dim[i].unit);
    }
    if (problem) return IcsErr_FailWriteLine;
    error = icsAddLine(line, fp);
    if (error) return error;

        /* Write labels associated with the dimensions to the .ics file, if
           any: */
    problem = 0;
    for (i = 0; i <(int)icsStruct->dimensions; i++) {
        problem |= *(icsStruct->dim[i].label) == '\0';
    }
    if (!problem) {
        problem = icsFirstToken(line, ICSTOK_PARAM);
        problem |= icsAddToken(line, ICSTOK_LABELS);
        problem |= icsAddText(line, ICS_LABEL_BITS);
        for (i = 0; i < icsStruct->dimensions-1; i++) {
            problem |= icsAddText(line, icsStruct->dim[i].label);
        }
        problem |= icsAddLastText(line, icsStruct->dim[i].label);
        if (problem) return IcsErr_FailWriteLine;
        error = icsAddLine(line, fp);
        if (error) return error;
    }

    return error;
}


#define ICS_ADD_SENSOR_DOUBLE(TOKEN, FIELD)                         \
do {                                                                \
    problem = icsFirstToken(line, ICSTOK_SENSOR);                   \
    problem |= icsAddToken(line, ICSTOK_SPARAMS);                   \
    problem |= icsAddToken(line, TOKEN);                            \
    for (i = 0; i < chans - 1; i++) {                               \
        problem |= icsAddDouble(line, icsStruct->FIELD[i]);         \
    }                                                               \
    problem |= icsAddLastDouble(line, icsStruct->FIELD[chans - 1]); \
    if (!problem) {                                                 \
        error = icsAddLine(line, fp);                               \
        if (error) return error;                                    \
    }                                                               \
} while (0)


#define ICS_ADD_SENSOR_DOUBLE_ONE(TOKEN, FIELD)             \
do {                                                        \
    problem = icsFirstToken(line, ICSTOK_SENSOR);           \
    problem |= icsAddToken(line, ICSTOK_SPARAMS);           \
    problem |= icsAddToken(line, TOKEN);                    \
    problem |= icsAddLastDouble(line, icsStruct->FIELD);    \
    if (!problem) {                                         \
        error = icsAddLine(line, fp);                       \
        if (error) return error;                            \
    }                                                       \
} while (0)


#define ICS_ADD_SENSOR_DOUBLE_INDEX(TOKEN, FIELD, TAG, IDX)             \
do {                                                                    \
    problem = icsFirstToken(line, ICSTOK_SENSOR);                       \
    problem |= icsAddToken(line, ICSTOK_SPARAMS);                       \
    problem |= icsAddTokenWithIndex(line, TOKEN, TAG);                  \
    for (i = 0; i < chans - 1; i++) {                                   \
        problem |= icsAddDouble(line, icsStruct->FIELD[i][IDX]);        \
    }                                                                   \
    problem |= icsAddLastDouble(line, icsStruct->FIELD[chans - 1][IDX]); \
    if (!problem) {                                                     \
        error = icsAddLine(line, fp);                                   \
        if (error) return error;                                        \
    }                                                                   \
} while (0)


#define ICS_ADD_SENSOR_INT(TOKEN, FIELD)                            \
do {                                                                \
    problem = icsFirstToken(line, ICSTOK_SENSOR);                   \
    problem |= icsAddToken(line, ICSTOK_SPARAMS);                   \
    problem |= icsAddToken(line, TOKEN);                            \
    for (i = 0; i < chans - 1; i++) {                               \
        problem |= icsAddInt(line, icsStruct->FIELD[i]);            \
    }                                                               \
    problem |= icsAddLastInt(line, icsStruct->FIELD[chans - 1]);    \
    if (!problem) {                                                 \
        error = icsAddLine(line, fp);                               \
        if (error) return error;                                    \
    }                                                               \
} while (0)


#define ICS_ADD_SENSOR_STRING(TOKEN, FIELD)                 \
do {                                                        \
    problem = icsFirstToken(line, ICSTOK_SENSOR);           \
    problem |= icsAddToken(line, ICSTOK_SPARAMS);           \
    problem |= icsAddToken(line, TOKEN);                    \
    for (i = 0; i < chans - 1; i++) {                       \
        problem |= icsAddText(line, icsStruct->FIELD[i]);   \
    }                                                       \
    problem |= icsAddLastText(line, icsStruct->FIELD[i]);   \
    if (!problem) {                                         \
        error = icsAddLine(line, fp);                       \
        if (error) return error;                            \
    }                                                       \
} while (0)


static Ics_Error writeIcsSensorData(Ics_Header *icsStruct,
                                    FILE       *fp)
{
    ICSINIT;
    unsigned int    problem;
    int  i, chans;
    char line[ICS_LINE_LENGTH];


    if (icsStruct->writeSensor) {

        chans = icsStruct->sensorChannels;
        if (chans > ICS_MAX_LAMBDA) return IcsErr_TooManyChans;

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_TYPE);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddText(line, icsStruct->type[i]);
        }
        problem |= icsAddLastText(line, icsStruct->type[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_MODEL);
        problem |= icsAddLastText(line, icsStruct->model);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_CHANS);
        problem |= icsAddLastInt(line, chans);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        ICS_ADD_SENSOR_STRING(ICSTOK_IMDIR, imagingDirection);
        ICS_ADD_SENSOR_DOUBLE_ONE(ICSTOK_NUMAPER, numAperture);
        ICS_ADD_SENSOR_INT(ICSTOK_OBJQ, objectiveQuality);
        ICS_ADD_SENSOR_DOUBLE_ONE(ICSTOK_REFRIME, refrInxMedium);
        ICS_ADD_SENSOR_DOUBLE_ONE(ICSTOK_REFRILM, refrInxLensMedium);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_PINHRAD, pinholeRadius);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_ILLPINHRAD, illPinholeRadius);
        ICS_ADD_SENSOR_DOUBLE_ONE(ICSTOK_PINHSPA, pinholeSpacing);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_EXBFILL, excitationBeamFill);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_LAMBDEX, lambdaEx);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_LAMBDEM, lambdaEm);
        ICS_ADD_SENSOR_INT(ICSTOK_PHOTCNT, exPhotonCnt);
        ICS_ADD_SENSOR_DOUBLE_ONE(ICSTOK_IFACE1, interfacePrimary);
        ICS_ADD_SENSOR_DOUBLE_ONE(ICSTOK_IFACE2, interfaceSecondary);
        
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_DETMAG, detectorMagn);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_DETPPU, detectorPPU);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_DETBASELINE, detectorBaseline);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_DETLNAVGCNT, detectorLineAvgCnt);

        ICS_ADD_SENSOR_STRING(ICSTOK_STEDDEPLMODE, stedDepletionMode);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_STEDLAMBDA, stedLambda);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_STEDSATFACTOR, stedSatFactor);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_STEDIMMFRACTION, stedImmFraction);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_STEDVPPM, stedVPPM);
        
        ICS_ADD_SENSOR_STRING(ICSTOK_SPIMEXCTYPE, spimExcType);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_SPIMPLANENA, spimPlaneNA);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_SPIMFILLFACTOR, spimFillFactor);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_SPIMPLANEGAUSSWIDTH, spimPlaneGaussWidth);
        ICS_ADD_SENSOR_DOUBLE_INDEX(ICSTOK_SPIMPLANEPROPDIR, spimPlanePropDir,
                                    "X", 0);
        ICS_ADD_SENSOR_DOUBLE_INDEX(ICSTOK_SPIMPLANEPROPDIR, spimPlanePropDir,
                                    "Y", 1);
        ICS_ADD_SENSOR_DOUBLE_INDEX(ICSTOK_SPIMPLANEPROPDIR, spimPlanePropDir,
                                    "Z", 2);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_SPIMPLANECENTEROFF, spimPlaneCenterOff);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_SPIMPLANEFOCUSOF, spimPlaneFocusOff);

        ICS_ADD_SENSOR_STRING(ICSTOK_SCATTERMODEL, scatterModel);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_SCATTERFREEPATH, scatterFreePath);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_SCATTERRELCONTRIB, scatterRelContrib);
        ICS_ADD_SENSOR_DOUBLE(ICSTOK_SCATTERBLURRING, scatterBlurring);
    }

    return error;
}


#define ICS_ADD_SENSOR_STATE(TOKEN, FIELD)          \
do {                                                \
    problem = icsFirstToken(line, ICSTOK_SENSOR);   \
    problem |= icsAddToken(line, ICSTOK_SSTATES);   \
    problem |= icsAddToken(line, TOKEN);            \
    for (i = 0; i < chans - 1; i++) {               \
        state = icsStruct->FIELD ## State[i];       \
        problem |= icsAddSensorState(line, state);  \
    }                                               \
    state = icsStruct->FIELD ## State[chans - 1];   \
    problem |= icsAddLastSensorState(line, state);  \
    if (!problem) {                                 \
        error = icsAddLine(line, fp);               \
        if (error) return error;                    \
    }                                               \
} while (0)


#define ICS_ADD_SENSOR_STATE_ONE(TOKEN, FIELD)                          \
do {                                                                    \
    problem = icsFirstToken(line, ICSTOK_SENSOR);                       \
    problem |= icsAddToken(line, ICSTOK_SSTATES);                       \
    problem |= icsAddToken(line, TOKEN);                                \
    problem |= icsAddLastSensorState(line, icsStruct->FIELD ## State);  \
    if (!problem) {                                                     \
        error = icsAddLine(line, fp);                                   \
        if (error) return error;                                        \
    }                                                                   \
} while (0)


static Ics_Error writeIcsSensorStates(Ics_Header *icsStruct,
                                      FILE       *fp)
{
    ICSINIT;
    unsigned int    problem;
    int             i, chans;
    char            line[ICS_LINE_LENGTH];
    Ics_SensorState state;

    if (icsStruct->writeSensorStates) {

        chans = icsStruct->sensorChannels;
        if (chans > ICS_MAX_LAMBDA) return IcsErr_TooManyChans;

        ICS_ADD_SENSOR_STATE(ICSTOK_IMDIR, imagingDirection);
        ICS_ADD_SENSOR_STATE_ONE(ICSTOK_NUMAPER, numAperture);
        ICS_ADD_SENSOR_STATE(ICSTOK_OBJQ, objectiveQuality);
        ICS_ADD_SENSOR_STATE_ONE(ICSTOK_REFRIME, refrInxMedium);
        ICS_ADD_SENSOR_STATE_ONE(ICSTOK_REFRILM, refrInxLensMedium);
        ICS_ADD_SENSOR_STATE(ICSTOK_PINHRAD, pinholeRadius);
        ICS_ADD_SENSOR_STATE(ICSTOK_ILLPINHRAD, illPinholeRadius);
        ICS_ADD_SENSOR_STATE_ONE(ICSTOK_PINHSPA, pinholeSpacing);
        ICS_ADD_SENSOR_STATE(ICSTOK_EXBFILL, excitationBeamFill);
        ICS_ADD_SENSOR_STATE(ICSTOK_LAMBDEX, lambdaEx);
        ICS_ADD_SENSOR_STATE(ICSTOK_LAMBDEM, lambdaEm);
        ICS_ADD_SENSOR_STATE(ICSTOK_PHOTCNT, exPhotonCnt);
        ICS_ADD_SENSOR_STATE_ONE(ICSTOK_IFACE1, interfacePrimary);
        ICS_ADD_SENSOR_STATE_ONE(ICSTOK_IFACE2, interfaceSecondary);
        
        ICS_ADD_SENSOR_STATE(ICSTOK_DETMAG, detectorMagn);
        ICS_ADD_SENSOR_STATE(ICSTOK_DETPPU, detectorPPU);
        ICS_ADD_SENSOR_STATE(ICSTOK_DETBASELINE, detectorBaseline);
        ICS_ADD_SENSOR_STATE(ICSTOK_DETLNAVGCNT, detectorLineAvgCnt);

        ICS_ADD_SENSOR_STATE(ICSTOK_STEDDEPLMODE, stedDepletionMode);
        ICS_ADD_SENSOR_STATE(ICSTOK_STEDLAMBDA, stedLambda);
        ICS_ADD_SENSOR_STATE(ICSTOK_STEDSATFACTOR, stedSatFactor);
        ICS_ADD_SENSOR_STATE(ICSTOK_STEDIMMFRACTION, stedImmFraction);
        ICS_ADD_SENSOR_STATE(ICSTOK_STEDVPPM, stedVPPM);
        
        ICS_ADD_SENSOR_STATE(ICSTOK_SPIMEXCTYPE, spimExcType);
        ICS_ADD_SENSOR_STATE(ICSTOK_SPIMPLANENA, spimPlaneNA);
        ICS_ADD_SENSOR_STATE(ICSTOK_SPIMFILLFACTOR, spimFillFactor);
        ICS_ADD_SENSOR_STATE(ICSTOK_SPIMPLANEGAUSSWIDTH, spimPlaneGaussWidth);
        ICS_ADD_SENSOR_STATE(ICSTOK_SPIMPLANEPROPDIR, spimPlanePropDir);
        ICS_ADD_SENSOR_STATE(ICSTOK_SPIMPLANECENTEROFF, spimPlaneCenterOff);
        ICS_ADD_SENSOR_STATE(ICSTOK_SPIMPLANEFOCUSOF, spimPlaneFocusOff);

        ICS_ADD_SENSOR_STATE(ICSTOK_SCATTERMODEL, scatterModel);
        ICS_ADD_SENSOR_STATE(ICSTOK_SCATTERFREEPATH, scatterFreePath);
        ICS_ADD_SENSOR_STATE(ICSTOK_SCATTERRELCONTRIB, scatterRelContrib);
        ICS_ADD_SENSOR_STATE(ICSTOK_SCATTERBLURRING, scatterBlurring);

    }

    return error;
}


static Ics_Error writeIcsHistory(Ics_Header *icsStruct,
                                 FILE       *fp)
{
    ICSINIT;
    unsigned int problem;
    int          i;
    char         line[ICS_LINE_LENGTH];
    Ics_History* hist = (Ics_History*)icsStruct->history;


    if (hist != NULL) {
        for (i = 0; i < hist->nStr; i++) {
            if (hist->strings[i] != NULL) {
                problem = icsFirstToken(line, ICSTOK_HISTORY);
                problem |= icsAddLastText(line, hist->strings[i]);
                if (!problem) {
                    error = icsAddLine(line, fp);
                    if (error) return error;
                }
            }
        }
    }

    return error;
}


static Ics_Error markEndOfFile(Ics_Header *icsStruct,
                               FILE       *fp)
{
    ICSINIT;
    char line[ICS_LINE_LENGTH];


    if ((icsStruct->version != 1) &&(icsStruct->srcFile[0] == '\0')) {
        error = icsFirstToken(line, ICSTOK_END);
        if (error) return IcsErr_FailWriteLine;
        IcsAppendChar(line, ICS_EOL);
        error = icsAddLine(line, fp);
        if (error) return error;
    }

    return error;
}


Ics_Error IcsWriteIcs(Ics_Header *icsStruct,
                      const char *filename)
{
    ICSINIT;
    ICS_INIT_LOCALE;
    char  line[ICS_LINE_LENGTH];
    char  buf[ICS_MAXPATHLEN];
    FILE *fp;


    if ((filename != NULL) &&(filename[0] != '\0')) {
        IcsGetIcsName(icsStruct->filename, filename, 0);
    } else if (icsStruct->filename[0] != '\0') {
        IcsStrCpy(buf, icsStruct->filename, ICS_MAXPATHLEN);
        IcsGetIcsName(icsStruct->filename, buf, 0);
    } else {
        return IcsErr_FOpenIcs;
    }

    fp = IcsFOpen(icsStruct->filename, "wb");
    if (fp == NULL) return IcsErr_FOpenIcs;

    ICS_SET_LOCALE;

    line[0] = ICS_FIELD_SEP;
    line[1] = ICS_EOL;
    line[2] = '\0';
    error = icsAddLine(line, fp);

        /* Which ICS version is this file? */
    if (!error) {
        icsFirstText(line, ICS_VERSION);
        if (icsStruct->version == 1) {
            icsAddLastText(line, "1.0");
        } else {
            icsAddLastText(line, "2.0");
        }
        if (!error) error = icsAddLine(line, fp);
    }

        /* Write the root of the filename: */
    if (!error) {
        IcsGetFileName(buf, icsStruct->filename);
        icsFirstText(line, ICS_FILENAME);
        icsAddLastText(line, buf);
        if (!error) error = icsAddLine(line, fp);
    }

        /* Write all image descriptors: */
    if (!error) error = writeIcsSource(icsStruct, fp);
    if (!error) error = writeIcsLayout(icsStruct, fp);
    if (!error) error = writeIcsRep(icsStruct, fp);
    if (!error) error = writeIcsParam(icsStruct, fp);
    if (!error) error = writeIcsSensorData(icsStruct, fp);
    if (!error) error = writeIcsSensorStates(icsStruct, fp);
    if (!error) error = writeIcsHistory(icsStruct, fp);
    if (!error) error = markEndOfFile(icsStruct, fp);

    ICS_REVERT_LOCALE;

    if (fclose(fp) == EOF) {
        if (!error) error = IcsErr_FCloseIcs; /* Don't overwrite any previous
                                                 error. */
    }
    return error;
}
