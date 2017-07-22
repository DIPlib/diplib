#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libics.h"

int main(int argc, const char* argv[]) {
   ICS* ip;
   Ics_Error retval;
   int nstr;
   char buffer[ICS_LINE_LENGTH];
   char token[ICS_STRLEN_TOKEN];
   Ics_HistoryIterator it;
   const char token1[] = "sequence1";
   const char token2[] = "sequence2";
   const char stuff1[] = "this is some data";
   const char stuff2[] = "this is some more data";
   const char stuff3[] = "this is some other stuff";

   if (argc != 2) {
      fprintf(stderr, "One file name required\n");
      exit(-1);
   }

   /* Open image for update */
   retval = IcsOpen(&ip, argv[1], "rw");
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not open file for update: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }

   /* Remove history lines */
   retval = IcsDeleteHistory(ip, "test");
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not delete history lines: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }

   /* Add history lines */
   retval = IcsAddHistory(ip, token1, stuff1);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not add history line: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   retval = IcsAddHistory(ip, token1, stuff2);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not add history line: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   retval = IcsAddHistory(ip, token2, stuff3);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not add history line: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }

   /* Check */
   retval = IcsGetNumHistoryStrings(ip, &nstr);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not get number of history lines: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   if (nstr != 3) {
      fprintf(stderr, "Number of history lines not correct.\n");
      exit(-1);
   }

   /* Read history lines and compare */
   retval = IcsNewHistoryIterator(ip, &it, "");
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not make new history iterator: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   retval = IcsGetHistoryKeyValueI(ip, &it, token, buffer);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not read 1st history string: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   if (strcmp(token,token1)!=0 || strcmp(buffer,stuff1)!=0) {
      fprintf(stderr, "1st history string does not match: \"%s\" vs \"%s\"\n",
              token, token1);
      exit(-1);
   }
   retval = IcsGetHistoryKeyValueI(ip, &it, token, buffer);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not read 2nd history string: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   if (strcmp(token,token1)!=0 || strcmp(buffer,stuff2)!=0) {
      fprintf(stderr, "2nd history string does not match.\n");
      exit(-1);
   }
   retval = IcsGetHistoryKeyValueI(ip, &it, token, buffer);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not read 3rd history string: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   if (strcmp(token,token2)!=0 || strcmp(buffer,stuff3)!=0) {
      fprintf(stderr, "3rd history string does not match.\n");
      exit(-1);
   }

   /* Check earlier deleted line */
   retval = IcsNewHistoryIterator(ip, &it, "test");
   if (retval != IcsErr_EndOfHistory) {
      fprintf(stderr, "Did not properly delete original 'test' line.\n");
      exit(-1);
   }

   /* Read token2 line */
   retval = IcsNewHistoryIterator(ip, &it, token2);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not make new history iterator: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   retval = IcsGetHistoryKeyValueI(ip, &it, 0, buffer);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not read history string: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   if (strcmp(buffer,stuff3)!=0) {
      fprintf(stderr, "history string does not match.\n");
      exit(-1);
   }

   /* Commit changes */
   retval = IcsClose(ip);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not close file: %s\n", IcsGetErrorText(retval));
      exit(-1);
   }

   exit(0);
}
