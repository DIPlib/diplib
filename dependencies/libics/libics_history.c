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
 * FILE : libics_history.c
 *
 * The following library functions are contained in this file:
 *
 *   IcsAddHistoryString()
 *   IcsGetNumHistoryStrings()
 *   IcsNewHistoryIterator()
 *   IcsGetHistoryString()
 *   IcsGetHistoryKeyValue()
 *   IcsGetHistoryStringI()
 *   IcsGetHistoryStringIF()
 *   IcsGetHistoryKeyValueI()
 *   IcsGetHistoryKeyValueIF()
 *   IcsDeleteHistory()
 *   IcsDeleteHistoryStringI()
 *   IcsReplaceHistoryStringI()
 *   IcsFreeHistory()
 *
 * The following internal functions are contained in this file:
 *
 *   IcsInternAddHistory()
 */

/* The void* History in the ICS struct is a pointer to a struct defined in
   libics_intern.h. Only the functions in this file and two others elsewhere
   meddle with this structure [IcsPrintIcs() in libics_test.c and
   WriteIcsHistory() in libics_write.c]. (I guess these two others should start
   using the functions defined here.)

   This struct contains an array of strings. The struct and the array are
   allocated when first adding a string, and the array is reallocated when it
   becomes too small.  The array grows in increments of ICS_HISTARRAY_INCREMENT,
   and it's length is given by the struct element Lenth. Each array element up
   to NStr is either NULL or a pointer to a char[]. These are allocated and
   deallocated when adding or deleting the strings. When deleting a string, the
   array element is set to NULL. It is not possible to move the other array
   elements down because that could invalidate iterators. IcsFreeHistory() frees
   all of these strings, the array and the struct, leaving the History pointer
   in the ICS struct as NULL. */


#include <stdlib.h>
#include <string.h>
#include "libics_intern.h"


/* Add HISTORY line to the ICS file. key can be NULL. */
Ics_Error IcsAddHistoryString(ICS        *ics,
                              const char *key,
                              const char *value)
{
    ICSINIT;
    static char const seps[3] = {ICS_FIELD_SEP,ICS_EOL,'\0'};


    if ((ics == NULL) || (ics->fileMode == IcsFileMode_read))
        return IcsErr_NotValidAction;

    if (key == NULL) {
        key = "";
    }
    error = IcsInternAddHistory(ics, key, value, seps);

    return error;
}


/* Add HISTORY lines to the ICS file (key can be "", value shouldn't). */
Ics_Error IcsInternAddHistory(Ics_Header *ics,
                              const char *key,
                              const char *value,
                              const char *seps)
{
    ICSINIT;
    size_t       len;
    char        *line;
    Ics_History *hist;

        /* Checks */
    len = strlen(key) + strlen(value) + 2;
        /* Length of { key + '\t' + value + '\0' } */
    if (strlen(ICS_HISTORY) + len + 2 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
        /* Length of { "history" + '\t' + key + '\t' + value + '\n' + '\0' } */
    if (strchr(key, ICS_FIELD_SEP) != NULL) return IcsErr_IllParameter;
    if (strchr(key, seps[0]) != NULL) return IcsErr_IllParameter;
    if (strchr(key, seps[1]) != NULL) return IcsErr_IllParameter;
    if (strchr(key, ICS_EOL) != NULL) return IcsErr_IllParameter;
    if (strchr(key, '\n') != NULL) return IcsErr_IllParameter;
    if (strchr(key, '\r') != NULL) return IcsErr_IllParameter;
    if (strchr(value, seps[1]) != NULL) return IcsErr_IllParameter;
    if (strchr(value, ICS_EOL) != NULL) return IcsErr_IllParameter;
    if (strchr(value, '\n') != NULL) return IcsErr_IllParameter;
    if (strchr(value, '\r') != NULL) return IcsErr_IllParameter;

        /* Allocate array if necessary */
    if (ics->history == NULL) {
        ics->history = malloc(sizeof(Ics_History));
        if (ics->history == NULL) return IcsErr_Alloc;
        hist = (Ics_History*)ics->history;
        hist->strings = (char**)malloc(ICS_HISTARRAY_INCREMENT * sizeof(char*));
        if (hist->strings == NULL) {
            free(ics->history);
            ics->history = NULL;
            return IcsErr_Alloc;
        }
        hist->length = ICS_HISTARRAY_INCREMENT;
        hist->nStr = 0;
    } else {
        hist = (Ics_History*)ics->history;
    }
        /* Reallocate if array is not large enough */
    if ((size_t)hist->nStr >= hist->length) {
        size_t n = hist->length + ICS_HISTARRAY_INCREMENT;
        char** tmp = (char**)realloc(hist->strings, n * sizeof(char*));
        if (tmp == NULL) return IcsErr_Alloc;
        hist->strings = tmp;
        hist->length = n;
    }

        /* Create line */
    line = (char*)malloc(len * sizeof(char));
    if (line == NULL) return IcsErr_Alloc;
    if (key[0] != '\0') {
        strcpy(line, key); /* already tested length */
        IcsAppendChar(line, ICS_FIELD_SEP);
    } else {
        line[0] = '\0';
    }
    strcat(line, value);
        /* Convert seps[0] into ICS_FIELD_SEP */
    if (seps[0] != ICS_FIELD_SEP) {
        char *s;
        while ((s = strchr(line, seps[0])) != NULL) {
            s[0] = ICS_FIELD_SEP;
        }
    }

        /* Put line into array */
    hist->strings[hist->nStr] = line;
    hist->nStr++;

    return error;
}

/* Get the number of HISTORY lines from the ICS file. */
Ics_Error IcsGetNumHistoryStrings(ICS *ics,
                                  int *num)
{
    ICSINIT;
    int          i, count = 0;
    Ics_History *hist;

    if (ics == NULL) return IcsErr_NotValidAction;

    hist = (Ics_History*)ics->history;

    *num = 0;
    if (hist == NULL) return IcsErr_Ok;
    for (i = 0; i < hist->nStr; i++) {
        if (hist->strings[i] != NULL) {
            count++;
        }
    }
    *num = count;

    return error;
}

/* Finds next matching string in history. */
static void IcsIteratorNext(Ics_History         *hist,
                            Ics_HistoryIterator *it)
{
    size_t nchar = strlen(it->key);
    it->previous = it->next;
    it->next++;
    if (nchar > 0) {
        for (; it->next < hist->nStr; it->next++) {
            if ((hist->strings[it->next] != NULL) &&
                (strncmp(it->key, hist->strings[it->next], nchar) == 0)) {
                break;
            }
        }
    }
    if (it->next >= hist->nStr) {
        it->next = -1;
    }
}

/* Initializes history iterator. key can be NULL. */
Ics_Error IcsNewHistoryIterator(ICS                 *ics,
                                Ics_HistoryIterator *it,
                                char const          *key)
{
    ICSINIT;
    Ics_History *hist;

    if (ics == NULL) return IcsErr_NotValidAction;

    hist = (Ics_History*)ics->history;

    it->next = -1;
    it->previous = -1;
    if ((key == NULL) ||(key[0] == '\0')) {
        it->key[0] = '\0';
    } else {
        size_t n;
        IcsStrCpy(it->key, key, ICS_STRLEN_TOKEN);
            /* Append a \t, so that the search for the key finds whole words */
        n = strlen(it->key);
        it->key[n] = ICS_FIELD_SEP;
        it->key[n + 1] = '\0';
    }

    if (hist == NULL) return IcsErr_EndOfHistory;

    IcsIteratorNext(hist, it);
    if (it->next < 0) return IcsErr_EndOfHistory;

    return error;
}


static Ics_HistoryIterator intern_iter = {-1,-1,{'\0'}};


/* Get HISTORY lines from the ICS file. history must have at least
   ICS_LINE_LENGTH characters allocated. */
Ics_Error IcsGetHistoryString(ICS              *ics,
                              char             *string,
                              Ics_HistoryWhich  which)
{
    ICSINIT;

    if (ics == NULL) return IcsErr_NotValidAction;

    if (which == IcsWhich_First) {
        error = IcsNewHistoryIterator(ics, &intern_iter, NULL);
        if (error) return error;
    }
    error = IcsGetHistoryStringI(ics, &intern_iter, string);
    if (error) return error;

    return error;
}

/* Get history line from the ICS file as key/value pair. key must have
   ICS_STRLEN_TOKEN characters allocated, and value ICS_LINE_LENGTH. key can be
   null, token will be discarded. */
Ics_Error IcsGetHistoryKeyValue(ICS              *ics,
                                char             *key,
                                char             *value,
                                Ics_HistoryWhich  which)
{
    ICSINIT;

    if (ics == NULL) return IcsErr_NotValidAction;

    if (which == IcsWhich_First) {
        error = IcsNewHistoryIterator(ics, &intern_iter, NULL);
        if (error) return error;
    }
    error = IcsGetHistoryKeyValueI(ics, &intern_iter, key, value);
    if (error) return error;

    return error;
}


/* Get history line from the ICS file using iterator. string must have at least
   ICS_LINE_LENGTH characters allocated. */
Ics_Error IcsGetHistoryStringI(ICS                 *ics,
                               Ics_HistoryIterator *it,
                               char                *string)
{
    ICSINIT;
    const char *ptr;

    error = IcsGetHistoryStringIF(ics, it, &ptr);
    if (!error) {
        IcsStrCpy(string, ptr, ICS_LINE_LENGTH);
    }

    return error;
}

/* Idem, but without copying the string. Output pointer `string` set to internal
   buffer, which will be valid until IcsClose or IcsFreeHistory is called. */
Ics_Error IcsGetHistoryStringIF(ICS                 *ics,
                                Ics_HistoryIterator *it,
                                const char         **string)
{
    ICSINIT;
    Ics_History *hist;

    if (ics == NULL) return IcsErr_NotValidAction;

    hist = (Ics_History*)ics->history;

    if (hist == NULL) return IcsErr_EndOfHistory;
    if ((it->next >= 0) &&(hist->strings[it->next] == NULL)) {
            /* The string pointed to has been deleted.
             * Find the next string, but don't change prev! */
        int prev = it->previous;
        IcsIteratorNext(hist, it);
        it->previous = prev;
    }
    if (it->next < 0) return IcsErr_EndOfHistory;
    *string = hist->strings[it->next];
    IcsIteratorNext(hist, it);

    return error;
}

/* Get history line from the ICS file as key/value pair using iterator. key must
   have ICS_STRLEN_TOKEN characters allocated, and value ICS_LINE_LENGTH. key
   can be null, token will be discarded. */
Ics_Error IcsGetHistoryKeyValueI(ICS                 *ics,
                                 Ics_HistoryIterator *it,
                                 char                *key,
                                 char                *value)
{
    ICSINIT;
    const char *ptr;

    error = IcsGetHistoryKeyValueIF(ics, it, key, &ptr);
    if (!error) {
        IcsStrCpy(value, ptr, ICS_LINE_LENGTH);
    }

    return error;
}

/* Idem, but without copying the string. Output pointer `value` set to internal
   buffer, which will be valid until IcsClose or IcsFreeHistory is called. */
Ics_Error IcsGetHistoryKeyValueIF(ICS                 *ics,
                                  Ics_HistoryIterator *it,
                                  char                *key,
                                  const char         **value)
{
    ICSINIT;
    size_t      length;
    const char *buf;
    char       *ptr;


    error = IcsGetHistoryStringIF(ics, it, &buf);
    if (error) return error;

    ptr = strchr(buf, ICS_FIELD_SEP);
    length = (size_t)(ptr-buf);
    if ((ptr != NULL) &&(length > 0) &&(length < ICS_STRLEN_TOKEN)) {
        if (key != NULL) {
            memcpy(key, buf, length);
            key[length] = '\0';
        }
        *value = ptr + 1;
    } else {
        if (key != NULL) {
            key[0] = '\0';
        }
        *value = buf;
    }

    return error;
}

/* Delete all history lines with key from ICS file. key can be NULL, deletes
   all. */
Ics_Error IcsDeleteHistory(ICS        *ics,
                           const char *key)
{
    ICSINIT;
    Ics_History *hist;

    if (ics == NULL) return IcsErr_NotValidAction;

    hist = (Ics_History*)ics->history;

    if (hist == NULL) return IcsErr_Ok;
    if (hist->nStr == 0) return IcsErr_Ok;

    if ((key == NULL) ||(key[0] == '\0')) {
        int i;
        for (i = 0; i < hist->nStr; i++) {
            if (hist->strings[i] != NULL) {
                free(hist->strings[i]);
                hist->strings[i] = NULL;
            }
        }
        hist->nStr = 0;
    } else {
        Ics_HistoryIterator it;
        IcsNewHistoryIterator(ics, &it, key);
        if (it.next >= 0) {
            IcsIteratorNext(hist, &it);
        }
        while (it.previous >= 0) {
            free(hist->strings[it.previous]);
            hist->strings[it.previous] = NULL;
            IcsIteratorNext(hist, &it);
        }
            /* If we deleted strings at the end, recover those spots. */
        hist->nStr--;
        while ((hist->nStr >= 0) &&(hist->strings[hist->nStr] == NULL)) {
            hist->nStr--;
        }
        hist->nStr++;
    }

    return error;
}

/* Delete last retrieved history line(iterator still points to the same
   string). */
Ics_Error IcsDeleteHistoryStringI(ICS                 *ics,
                                  Ics_HistoryIterator *it)
{
    ICSINIT;
    Ics_History *hist;

    if (ics == NULL) return IcsErr_NotValidAction;

    hist = (Ics_History*)ics->history;

    if (hist == NULL) return IcsErr_Ok;      /* give error message? */
    if (it->previous < 0) return IcsErr_Ok;
    if (hist->strings[it->previous] == NULL) return IcsErr_Ok;

    free(hist->strings[it->previous]);
    hist->strings[it->previous] = NULL;
    if (it->previous == hist->nStr-1) {
            /* We just deleted the last string. Let's recover that spot. */
        hist->nStr--;
    }
    it->previous = -1;

    return error;
}

/* Replace last retrieved history line (iterator still points to the same
   string). Contains code duplicated from IcsInternAddHistory(). */
Ics_Error IcsReplaceHistoryStringI(ICS                 *ics,
                                   Ics_HistoryIterator *it,
                                   const char          *key,
                                   const char          *value)
{
    ICSINIT;
    size_t       len;
    char        *line;
    Ics_History *hist;

    if (ics == NULL) return IcsErr_NotValidAction;

    hist = (Ics_History*)ics->history;

    if (hist == NULL) return IcsErr_Ok;      /* give error message? */
    if (it->previous < 0) return IcsErr_Ok;
    if (hist->strings[it->previous] == NULL) return IcsErr_Ok;

        /* Checks */
    len = strlen(key) + strlen(value) + 2;
        /* Length of { key + '\t' + value + '\0' } */
    if (strlen(ICS_HISTORY) + len + 2 > ICS_LINE_LENGTH)
        return IcsErr_LineOverflow;
        /* Length of { "history" + '\t' + key + '\t' + value + '\n' + '\0' } */
    if (strchr(key, ICS_FIELD_SEP) != NULL) return IcsErr_IllParameter;
    if (strchr(key, ICS_EOL) != NULL) return IcsErr_IllParameter;
    if (strchr(key, '\n') != NULL) return IcsErr_IllParameter;
    if (strchr(key, '\r') != NULL) return IcsErr_IllParameter;
    if (strchr(value, ICS_EOL) != NULL) return IcsErr_IllParameter;
    if (strchr(value, '\n') != NULL) return IcsErr_IllParameter;
    if (strchr(value, '\r') != NULL) return IcsErr_IllParameter;

        /* Create line */
    line = (char*)realloc(hist->strings[it->previous], len * sizeof(char));
    if (line == NULL) return IcsErr_Alloc;
    hist->strings[it->previous] = line;
    if (key[0] != '\0') {
        strcpy(line, key); /* already tested length */
        IcsAppendChar(line, ICS_FIELD_SEP);
    }
    strcat(line, value);

    return error;
}

/* Free the memory allocated for history. */
void IcsFreeHistory(Ics_Header *ics)
{
    int          i;
    Ics_History *hist = (Ics_History*)ics->history;


    if (hist != NULL) {
        for (i = 0; i < hist->nStr; i++) {
            if (hist->strings[i] != NULL) {
                free(hist->strings[i]);
            }
        }
        free(hist->strings);
        free(ics->history);
        ics->history = NULL;
    }
}
