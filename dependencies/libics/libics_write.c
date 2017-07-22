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
    int  problem;
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
    int    problem, i;
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
    int        problem, empty, i;
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
        IcsFillByteOrder(IcsGetDataTypeSize(icsStruct->imel.dataType),
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
    int  problem, i;
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


static Ics_Error writeIcsSensorData(Ics_Header *icsStruct,
                                    FILE       *fp)
{
    ICSINIT;
    int  problem, i, chans;
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

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_PINHRAD);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->pinholeRadius[i]);
        }
        problem |= icsAddLastDouble(line, icsStruct->pinholeRadius[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_LAMBDEX);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->lambdaEx[i]);
        }
        problem |= icsAddLastDouble(line, icsStruct->lambdaEx[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_LAMBDEM);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->lambdaEm[i]);
        }
        problem |= icsAddLastDouble(line, icsStruct->lambdaEm[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_PHOTCNT);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddInt(line, icsStruct->exPhotonCnt[i]);
        }
        problem |= icsAddLastInt(line, icsStruct->exPhotonCnt[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_REFRIME);
        problem |= icsAddLastDouble(line, icsStruct->refrInxMedium);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_NUMAPER);
        problem |= icsAddLastDouble(line, icsStruct->numAperture);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_REFRILM);
        problem |= icsAddLastDouble(line, icsStruct->refrInxLensMedium);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_PINHSPA);
        problem |= icsAddLastDouble(line, icsStruct->pinholeSpacing);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

            /* Add STED parameters */
        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_STEDDEPLMODE);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddText(line, icsStruct->stedDepletionMode[i]);
        }
        problem |= icsAddLastText(line, icsStruct->stedDepletionMode[i]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_STEDLAMBDA);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->stedLambda[i]);
        }
        problem |= icsAddLastDouble(line, icsStruct->stedLambda[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_STEDSATFACTOR);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->stedSatFactor[i]);
        }
        problem |= icsAddLastDouble(line, icsStruct->stedSatFactor[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_STEDIMMFRACTION);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->stedImmFraction[i]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->stedImmFraction[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_STEDVPPM);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->stedVPPM[i]);
        }
        problem |= icsAddLastDouble(line, icsStruct->stedVPPM[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

            /* Add  SPIM parameters */
        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SPIMEXCTYPE);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddText(line, icsStruct->spimExcType[i]);
        }
        problem |= icsAddLastText(line, icsStruct->spimExcType[i]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SPIMPLANENA);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->spimPlaneNA[i]);
        }
        problem |= icsAddLastDouble(line, icsStruct->spimPlaneNA[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SPIMFILLFACTOR);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->spimFillFactor[i]);
        }
        problem |= icsAddLastDouble(line, icsStruct->spimFillFactor[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SPIMPLANEGAUSSWIDTH);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->spimPlaneGaussWidth[i]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->spimPlaneGaussWidth[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddTokenWithIndex(line, ICSTOK_SPIMPLANEPROPDIR, "X");
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->spimPlanePropDir[i][0]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->spimPlanePropDir[chans - 1][0]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddTokenWithIndex(line, ICSTOK_SPIMPLANEPROPDIR, "Y");
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->spimPlanePropDir[i][1]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->spimPlanePropDir[chans - 1][1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddTokenWithIndex(line, ICSTOK_SPIMPLANEPROPDIR, "Z");
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->spimPlanePropDir[i][2]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->spimPlanePropDir[chans - 1][2]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SPIMPLANECENTEROFF);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->spimPlaneCenterOff[i]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->spimPlaneCenterOff[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SPIMPLANEFOCUSOF);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->spimPlaneFocusOff[i]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->spimPlaneFocusOff[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

            /* Add  Scatter parameters */
        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SCATTERMODEL);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddText(line, icsStruct->scatterModel[i]);
        }
        problem |= icsAddLastText(line, icsStruct->scatterModel[i]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SCATTERFREEPATH);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->scatterFreePath[i]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->scatterFreePath[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SCATTERRELCONTRIB);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->scatterRelContrib[i]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->scatterRelContrib[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_SCATTERBLURRING);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->scatterBlurring[i]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->scatterBlurring[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

            /* Add detector parameters. */
        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_DETPPU);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->detectorPPU[i]);
        }
        problem |= icsAddLastDouble(line, icsStruct->detectorPPU[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_DETBASELINE);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->detectorBaseline[i]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->detectorBaseline[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SPARAMS);
        problem |= icsAddToken(line, ICSTOK_DETLNAVGCNT);
        for (i = 0; i < chans - 1; i++) {
            problem |= icsAddDouble(line, icsStruct->detectorLineAvgCnt[i]);
        }
        problem |= icsAddLastDouble(line,
                                    icsStruct->detectorLineAvgCnt[chans - 1]);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }


    }

    return error;
}


static Ics_Error writeIcsSensorStates(Ics_Header *icsStruct,
                                      FILE       *fp)
{
    ICSINIT;
    int             problem, i, chans;
    char            line[ICS_LINE_LENGTH];
    Ics_SensorState state;

    if (icsStruct->writeSensorStates) {

        chans = icsStruct->sensorChannels;
        if (chans > ICS_MAX_LAMBDA) return IcsErr_TooManyChans;

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_PINHRAD);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->pinholeRadiusState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->pinholeRadiusState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_LAMBDEX);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->lambdaExState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->lambdaExState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_LAMBDEM);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->lambdaEmState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->lambdaEmState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_PHOTCNT);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->exPhotonCntState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->exPhotonCntState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_REFRIME);
        problem |= icsAddLastSensorState(line, icsStruct->refrInxMediumState);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_NUMAPER);
        problem |= icsAddLastSensorState(line, icsStruct->numApertureState);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_REFRILM);
        problem |= icsAddLastSensorState(line,
                                         icsStruct->refrInxLensMediumState);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_PINHSPA);
        problem |= icsAddLastSensorState(line, icsStruct->pinholeSpacingState);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

            /* Add STED parameters */
        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_STEDDEPLMODE);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->stedDepletionModeState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->stedDepletionModeState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_STEDLAMBDA);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->stedLambdaState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->stedLambdaState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_STEDSATFACTOR);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->stedSatFactorState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->stedSatFactorState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_STEDIMMFRACTION);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->stedImmFractionState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->stedImmFractionState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_STEDVPPM);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->stedVPPMState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->stedVPPMState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

            /* Add SPIM parameters */
        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SPIMEXCTYPE);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->spimExcTypeState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->spimExcTypeState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SPIMPLANENA);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->spimPlaneNAState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->spimPlaneNAState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SPIMFILLFACTOR);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->spimFillFactorState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->spimFillFactorState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SPIMPLANEGAUSSWIDTH);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->spimPlaneGaussWidthState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->spimPlaneGaussWidthState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SPIMPLANEPROPDIR);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->spimPlanePropDirState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->spimPlanePropDirState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SPIMPLANECENTEROFF);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->spimPlaneCenterOffState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->spimPlaneCenterOffState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SPIMPLANEFOCUSOF);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->spimPlaneFocusOffState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->spimPlaneFocusOffState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

            /* Add Scatter parameters */
        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SCATTERMODEL);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->scatterModelState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->scatterModelState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SCATTERRELCONTRIB);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->scatterRelContribState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->scatterRelContribState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_SCATTERBLURRING);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->scatterBlurring[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->scatterBlurring[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

            /* Add detector parameters. */
        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_DETPPU);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->detectorPPUState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->detectorPPUState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_DETBASELINE);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->detectorBaselineState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->detectorBaselineState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

        problem = icsFirstToken(line, ICSTOK_SENSOR);
        problem |= icsAddToken(line, ICSTOK_SSTATES);
        problem |= icsAddToken(line, ICSTOK_DETLNAVGCNT);
        for (i = 0; i < chans - 1; i++) {
            state = icsStruct->detectorLineAvgCntState[i];
            problem |= icsAddSensorState(line, state);
        }
        state = icsStruct->detectorLineAvgCntState[chans - 1];
        problem |= icsAddLastSensorState(line, state);
        if (!problem) {
            error = icsAddLine(line, fp);
            if (error) return error;
        }

    }

    return error;
}


static Ics_Error writeIcsHistory(Ics_Header *icsStruct,
                                 FILE       *fp)
{
    ICSINIT;
    int          problem, i;
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
