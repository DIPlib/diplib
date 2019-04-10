#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libics.h"

int main(int argc, const char* argv[]) {
   ICS*         ip;
   Ics_DataType dt;
   int          ndims;
   size_t       dims[ICS_MAXDIM];
   size_t       imelsize;
   size_t       bufsize;
   ptrdiff_t    strides[3];
   void*        buf1;
   void*        buf2;
   void*        buf3;
   Ics_Error    retval;


   if (argc != 3) {
      fprintf(stderr, "Two file names required: in out\n");
      exit(-1);
   }

   /* Read image */
   retval = IcsOpen(&ip, argv[1], "r");
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not open input file: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   IcsGetLayout(ip, &dt, &ndims, dims);
   strides[0] = -1;
   strides[1] = -(ptrdiff_t)(dims[0]*dims[2]);
   strides[2] = -(ptrdiff_t)(dims[0]);
   imelsize = IcsGetImelSize(ip);
   bufsize = IcsGetDataSize(ip);
   buf1 = malloc(bufsize);
   if (buf1 == NULL) {
      fprintf(stderr, "Could not allocate memory.\n");
      exit(-1);
   }
   retval = IcsGetData(ip, buf1, bufsize);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not read input image data: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   buf3 = malloc(bufsize);
   if (buf3 == NULL) {
      fprintf(stderr, "Could not allocate memory.\n");
      exit(-1);
   }
   retval = IcsGetDataWithStrides(ip, (char*)buf3 + bufsize - imelsize, 0, strides, 3);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not read input image data using strides: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   retval = IcsClose(ip);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not close input file: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }

   /* Write image */
   retval = IcsOpen(&ip, argv[2], "w2");
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not open output file: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   IcsSetLayout(ip, dt, ndims, dims);
   IcsSetDataWithStrides(ip, (char*)buf3 + bufsize - imelsize, bufsize, strides, 3);
#ifdef ICS_ZLIB
   IcsSetCompression(ip, IcsCompr_gzip, 6);
#endif
   retval = IcsClose(ip);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not write output file: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }

   /* Read image */
   retval = IcsOpen(&ip, argv[2], "r");
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not open output file for reading: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   if (bufsize != IcsGetDataSize(ip)) {
      fprintf(stderr, "Data in output file not same size as written.\n");
      exit(-1);
   }
   buf2 = malloc(bufsize);
   if (buf2 == NULL) {
      fprintf(stderr, "Could not allocate memory.\n");
      exit(-1);
   }
   retval = IcsGetData(ip, buf2, bufsize);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not read output image data: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   retval = IcsClose(ip);
   if (retval != IcsErr_Ok) {
      fprintf(stderr, "Could not close output file: %s\n",
              IcsGetErrorText(retval));
      exit(-1);
   }
   if (memcmp(buf1, buf2, bufsize) != 0) {
      fprintf(stderr, "Data in output file does not match data in input.\n");
      exit(-1);
   }

   free(buf1);
   free(buf2);
   exit(0);
}
