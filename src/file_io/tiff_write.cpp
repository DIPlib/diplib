/*
 * DIPlib 3.0
 * This file contains definitions for TIFF writing
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef DIP__HAS_TIFF

#include "diplib.h"
#include "diplib/file_io.h"

#include <tiffio.h>

namespace dip {

namespace {

static const char* TIFF_WRITE_TAG = "Error writing tag to TIFF file";

class TiffFile {
   public:
      explicit TiffFile( String filename ) : filename_( std::move( filename )) {
         // Set error and warning handlers, these are library-wide!
         TIFFSetErrorHandler( nullptr ); // TODO: should we set a function here that throws an error? Would that be dangerous when called from within C-code?
         TIFFSetWarningHandler( nullptr );
         // Open the file for reading
         tiff_ = TIFFOpen( filename_.c_str(), "w" );
         DIP_THROW_IF( tiff_ == nullptr, "Could not open the specified file" );
      }
      TiffFile( TiffFile const& ) = delete;
      TiffFile( TiffFile&& ) = delete;
      TiffFile& operator=( TiffFile const& ) = delete;
      TiffFile& operator=( TiffFile&& ) = delete;
      ~TiffFile() {
         if( tiff_ ) {
            TIFFClose( tiff_ );
            tiff_ = nullptr;
         }
      }
      // Implicit cast to TIFF*
      operator TIFF*() { return tiff_; }
      // Retrieve file name
      String const& FileName() const { return filename_; }
   private:
      TIFF* tiff_ = nullptr;
      String filename_;
};

static uint16 CompressionTranslate (String const& compression) {
   if( compression.empty() || ( compression == "deflate" )) {
      return COMPRESSION_DEFLATE;
   } else if( compression == "LZW" ) {
      return COMPRESSION_LZW;
   } else if( compression == "PackBits" ) {
      return COMPRESSION_PACKBITS;
   } else if( compression == "JPEG" ) {
      return COMPRESSION_JPEG;
   } else if( compression == "none" ) {
      return COMPRESSION_NONE;
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
}

#if 0

static void FillBuffer8
(
   void* dest,
   const void* src,
   dip_uint width,
   dip_uint height,        /* Number of pixels to fill */
   dip_IntegerArray stride
)
{
   dip_uint8* _dest;
   const dip_uint8* _src;
   dip_uint ii, jj;

   _dest = (dip_uint8*)dest;
   for (ii=0; ii<height; ii++) {
      _src = (const dip_uint8*)src + ii*stride->array[1];
      for (jj=0; jj<width; jj++) {
         *_dest = *_src;
         _dest++;
         _src += stride->array[0];
      }
   }
}

static void FillBuffer
(
   void* dest,
   const void* src,
   dip_uint width,
   dip_uint height,        /* Number of pixels to fill */
   dip_IntegerArray stride,
   dip_uint size
)
{
   dip_uint8* _dest;
   const dip_uint8* _src;
   dip_uint ii, jj;

   _dest = (dip_uint8*)dest;
   for (ii=0; ii<height; ii++) {
      _src = (const dip_uint8*)src + ii*size*stride->array[1];
      for (jj=0; jj<width; jj++) {
         memcpy (_dest, _src, size);
         _dest += size;
         _src += size*stride->array[0];
      }
   }
}

static void CompactBits8
(
   void* dest,
   const void* src,
   dip_uint width,
   dip_uint height,        /* Number of pixels to fill */
   dip_IntegerArray stride,
   dip_int plane
)
{
   dip_uint8* _dest;
   const dip_uint8* _src;
   dip_uint ii, jj;
   dip_int kk = 7;
   dip_uint8 mask = 1<<plane;
   _dest = (dip_uint8*)dest;
   *_dest = 0;
   for (ii=0; ii<height; ii++) {
      if (kk != 7) {
         kk = 7;
         _dest++;
         *_dest = 0;
      }
      _src = (const dip_uint8*)src + ii*stride->array[1];
      for (jj=0; jj<width; jj++) {
         if (kk < 0) {
            kk = 7;
            _dest++;
            *_dest = 0;
         }
         *_dest |= (dip_uint8)(((*_src)&mask)?1<<kk:0);
         _src += stride->array[0];
         kk--;
      }
   }
}

static void CompactBits16
(
   void* dest,
   const void* src,
   dip_uint width,
   dip_uint height,        /* Number of pixels to fill */
   dip_IntegerArray stride,
   dip_int plane
)
{
   dip_uint8* _dest;
   const dip_uint16* _src;
   dip_uint ii, jj;
   dip_int kk = 7;
   dip_uint16 mask = 1<<plane;
   _dest = (dip_uint8*)dest;
   *_dest = 0;
   for (ii=0; ii<height; ii++) {
      if (kk != 7) {
         kk = 7;
         _dest++;
         *_dest = 0;
      }
      _src = (const dip_uint16*)src + ii*stride->array[1];
      for (jj=0; jj<width; jj++) {
         if (kk < 0) {
            kk = 7;
            _dest++;
            *_dest = 0;
         }
         *_dest |= (dip_uint8)(((*_src)&mask)?1<<kk:0);
         _src += stride->array[0];
         kk--;
      }
   }
}

static void CompactBits32
(
   void* dest,
   const void* src,
   dip_uint width,
   dip_uint height,        /* Number of pixels to fill */
   dip_IntegerArray stride,
   dip_int plane
)
{
   dip_uint8* _dest;
   const dip_uint32* _src;
   dip_uint ii, jj;
   dip_int kk = 7;
   dip_uint32 mask = 1<<plane;
   _dest = (dip_uint8*)dest;
   *_dest = 0;
   for (ii=0; ii<height; ii++) {
      if (kk != 7) {
         kk = 7;
         _dest++;
         *_dest = 0;
      }
      _src = (const dip_uint32*)src + ii*stride->array[1];
      for (jj=0; jj<width; jj++) {
         if (kk < 0) {
            kk = 7;
            _dest++;
            *_dest = 0;
         }
         *_dest |= (dip_uint8)(((*_src)&mask)?1<<kk:0);
         _src += stride->array[0];
         kk--;
      }
   }
}


static dip_Error WriteTIFFBinary
(
   dip_Image image,
   TIFF* tiff,
   uint16 compmode
)
{
   uint32 ImageLength, ImageWidth, RowsPerStrip, row, nrow, size;
   dip_int plane;
   dip_DataType datatype;
   dip_IntegerArray dims, stride;
   tstrip_t strip;
   tsize_t scanline;
   tdata_t buf = 0;
   void* vpimagedata;
   char* imagedata;

   /*DIPSJ ("Writing binary images to TIFF is buggy!!!");*/

   /* Get info on image */
   DIPXJ (dip_ImageGetDataType (image, &datatype));
   switch (datatype) {
      case DIP_DT_BIN8:
         size = sizeof (dip_bin8);
         break;
      case DIP_DT_BIN16:
         size = sizeof (dip_bin16);
         break;
      case DIP_DT_BIN32:
         size = sizeof (dip_bin32);
         break;
      default:
         DIPSJ ("Assertion failed");
   }
   DIPXJ (dip_ImageGetDimensions (image, &dims, rg));
   DIPTS (dims->size != 2, DIP_E_DIMENSIONALITY_NOT_SUPPORTED);
   ImageWidth = dims->array[0];
   ImageLength = dims->array[1];
   DIPXJ (dip_ImageGetStride (image, &stride, rg));
   DIPXJ (dip_ImageGetPlane (image, &plane));

   /* Set the tags */
   if (!TIFFSetField (tiff, TIFFTAG_IMAGEWIDTH, ImageWidth)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_IMAGELENGTH, ImageLength)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   /* Is this allowed? Is it required? */
   if (!TIFFSetField (tiff, TIFFTAG_BITSPERSAMPLE, (uint16)1)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_PLANARCONFIG, (uint16)PLANARCONFIG_CONTIG)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_COMPRESSION, compmode)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   RowsPerStrip = TIFFDefaultStripSize (tiff, 0);
   if (!TIFFSetField (tiff, TIFFTAG_ROWSPERSTRIP, RowsPerStrip)) {
      DIPSJ (TIFF_WRITE_TAG);
   }

   /* Get the image data */
   DIPXJ (dip__ImageGetData (image, &vpimagedata));
   imagedata = (char*)vpimagedata;

   /* Write it to the file */
   scanline = TIFFScanlineSize(tiff);
   DIPTS (((uint32)scanline != dipio_IntCeilDiv (ImageWidth, 8)), "Wrong scanline size");
   buf = _TIFFmalloc(TIFFStripSize (tiff));
   strip = 0;
   for (row = 0; row < ImageLength; row += RowsPerStrip) {
      nrow = (row+RowsPerStrip > ImageLength ? ImageLength-row : RowsPerStrip);
      if (size == 4) {
         CompactBits32 (buf, imagedata, ImageWidth, nrow, stride, plane);
      }
      else if (size == 2) {
         CompactBits16 (buf, imagedata, ImageWidth, nrow, stride, plane);
      }
      else {
         CompactBits8 (buf, imagedata, ImageWidth, nrow, stride, plane);
      }
      if (TIFFWriteEncodedStrip (tiff, strip, buf, nrow*scanline) < 0) {
         DIPSJ ("Error writing data");
      }
      imagedata += nrow*size*stride->array[1];
      strip++;
   }

dip_error:
   if (buf) {
      _TIFFfree(buf);
   }
}


static dip_Error WriteTIFFGrayValue
(
   dip_Image image,
   TIFF* tiff,
   uint16 compmode
)
{
   uint16 BitsPerSample, SampleFormat;
   uint32 ImageLength, ImageWidth, RowsPerStrip, row, nrow;
   dip_DataType datatype;
   dip_IntegerArray stride, dims;
   tstrip_t strip;
   tsize_t scanline;
   tdata_t buf = 0;
   void* vpimagedata;
   char* imagedata;

   /* Get info on image */
   DIPXJ (dip_ImageGetDataType (image, &datatype));
   switch (datatype) {
      case DIP_DT_BIN8:
      case DIP_DT_BIN16:
      case DIP_DT_BIN32:
         /* It's binary! Call other function and quit */
         DIPXJ (WriteTIFFBinary (image, tiff, compmode));
         DIPSJ (DIP_OK);
      case DIP_DT_UINT8:
         SampleFormat = SAMPLEFORMAT_UINT;
         BitsPerSample = sizeof(dip_uint8)*8;
         break;
      case DIP_DT_UINT16:
         SampleFormat = SAMPLEFORMAT_UINT;
         BitsPerSample = sizeof(dip_uint16)*8;
         break;
      case DIP_DT_UINT32:
         SampleFormat = SAMPLEFORMAT_UINT;
         BitsPerSample = sizeof(dip_uint32)*8;
         break;
      case DIP_DT_SINT8:
         SampleFormat = SAMPLEFORMAT_INT;
         BitsPerSample = sizeof(dip_sint8)*8;
         break;
      case DIP_DT_SINT16:
         SampleFormat = SAMPLEFORMAT_INT;
         BitsPerSample = sizeof(dip_sint16)*8;
         break;
      case DIP_DT_SINT32:
         SampleFormat = SAMPLEFORMAT_INT;
         BitsPerSample = sizeof(dip_sint32)*8;
         break;
      case DIP_DT_SFLOAT:
         SampleFormat = SAMPLEFORMAT_IEEEFP;
         BitsPerSample = sizeof(dip_sfloat)*8;
         break;
      case DIP_DT_DFLOAT:
         SampleFormat = SAMPLEFORMAT_IEEEFP;
         BitsPerSample = sizeof(dip_dfloat)*8;
         break;
      default:
         DIPSJ ("Data type of image is not compatible with TIFF");
         break;
   }
   DIPXJ (dip_ImageGetDimensions (image, &dims, rg));
   ImageWidth = dims->array[0];
   ImageLength = dims->array[1];
   DIPXJ (dip_ImageGetStride (image, &stride, rg));

   /* Set the tags */
   if (!TIFFSetField (tiff, TIFFTAG_IMAGEWIDTH, ImageWidth)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_IMAGELENGTH, ImageLength)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_BITSPERSAMPLE, BitsPerSample)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_SAMPLEFORMAT, SampleFormat)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_PLANARCONFIG, (uint16)PLANARCONFIG_CONTIG)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_COMPRESSION, compmode)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   RowsPerStrip = TIFFDefaultStripSize (tiff, 0);
   if (!TIFFSetField (tiff, TIFFTAG_ROWSPERSTRIP, RowsPerStrip)) {
      DIPSJ (TIFF_WRITE_TAG);
   }

   /* Get the image data */
   DIPXJ (dip__ImageGetData (image, &vpimagedata));

   /* Write it to the file */
   scanline = TIFFScanlineSize(tiff);
   DIPTS (((uint32)scanline != ImageWidth*(BitsPerSample/8)),
          "Wrong scanline size");
   buf = _TIFFmalloc(TIFFStripSize (tiff));
   strip = 0;
   imagedata = (char*)vpimagedata;
   for (row = 0; row < ImageLength; row += RowsPerStrip) {
      nrow = (row+RowsPerStrip > ImageLength ? ImageLength-row : RowsPerStrip);
      FillBuffer (buf, imagedata, ImageWidth, nrow, stride, BitsPerSample/8);
      if (TIFFWriteEncodedStrip (tiff, strip, buf, nrow*scanline) < 0) {
         DIPSJ ("Error writing data");
      }
      imagedata += nrow*(BitsPerSample/8)*stride->array[1];
      strip++;
   }

dip_error:
   if (buf) {
      _TIFFfree(buf);
   }
}


static dip_Error WriteTIFFFullColour
(
   dip_Image image,
   TIFF* tiff,
   uint16 compmode
)
{
   uint16 SamplesPerPixel;
   uint32 ImageLength, ImageWidth, RowsPerStrip, row, nrow;
   dip_Image im_uint8;
   dip_IntegerArray stride, dims;
   tdata_t buf = 0;
   tstrip_t strip;
   tsize_t scanline;
   dip_int s;
   void* vpimagedata;
   char* imagedata;

   /* Get info on image */
   DIPXJ (dip_ImageGetDimensions (image, &dims, rg));
   if (dims->size != 3) {
      DIPSJ ("Assertion failed");
   }
   ImageWidth = dims->array[0];
   ImageLength = dims->array[1];
   SamplesPerPixel = dims->array[2];

   /* convert the input to uint8 for further processing */
   DIPXJ( dip_ImageNew( &im_uint8, rg ));
   DIPXJ( dip_ConvertDataType( image, im_uint8, DIP_DT_UINT8 ));
   DIPXJ( dip_ImageGetStride( im_uint8, &stride, rg ));

   /* Set the tags */
   if (!TIFFSetField (tiff, TIFFTAG_IMAGEWIDTH, ImageWidth)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_IMAGELENGTH, ImageLength)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_BITSPERSAMPLE, (uint16)8)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_SAMPLESPERPIXEL, SamplesPerPixel)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   /* How can we set the panar configuration to match the strides? */
   if (!TIFFSetField (tiff, TIFFTAG_PLANARCONFIG, (uint16)PLANARCONFIG_SEPARATE)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   if (!TIFFSetField (tiff, TIFFTAG_COMPRESSION, compmode)) {
      DIPSJ (TIFF_WRITE_TAG);
   }
   RowsPerStrip = TIFFDefaultStripSize (tiff, 0);
   if (!TIFFSetField (tiff, TIFFTAG_ROWSPERSTRIP, RowsPerStrip)) {
      DIPSJ (TIFF_WRITE_TAG);
   }

   /* Get the image data */
   DIPXJ (dip__ImageGetData (im_uint8, &vpimagedata));

   /* Write it to the file */
   scanline = TIFFScanlineSize(tiff);
   DIPTS (((uint32)scanline != ImageWidth), "Wrong scanline size");
   buf = _TIFFmalloc(TIFFStripSize (tiff));
   strip = 0;
   for (s = 0; s < SamplesPerPixel; s++) {
      imagedata = (char*)vpimagedata + s*stride->array[2];
      for (row = 0; row < ImageLength; row += RowsPerStrip) {
         nrow = (row+RowsPerStrip > ImageLength ? ImageLength-row : RowsPerStrip);
         /*strip = TIFFComputeStrip (tiff, row, s);*/
         FillBuffer8 (buf, imagedata, ImageWidth, nrow, stride);
         if (TIFFWriteEncodedStrip (tiff, strip, buf, nrow*scanline) < 0) {
            DIPSJ ("Error writing data");
         }
         imagedata += nrow*stride->array[1];
         strip++;
      }
   }

dip_error:
   if (buf) {
      _TIFFfree(buf);
   }
}

#endif

} // namespace

#if 0

void ImageWriteTIFF(
      Image const& image,
      String const& filename,
      String const& compression = "",
      dip::uint jpegLevel = 80
) {
   TIFF* tiff = NULL;
   dip_int dimensionality;
   dip_dfloat xden, yden;
   uint16 compmode;
   dip_Boolean verdict;

   DIPXJ (dip_IsScalar (image, 0));
   DIPXJ (dip_ImageGetDimensionality (image, &dimensionality));
   if (photometric == DIPIO_PHM_GREYVALUE) {
      DIPTS (dimensionality != 2, DIP_E_DIMENSIONALITY_NOT_SUPPORTED);
   }
   else {
      DIPTS (dimensionality != 3, DIP_E_DIMENSIONALITY_NOT_SUPPORTED);
   }

   /* Open TIFF file */
   DIPXJ (dipio_FileCompareExtension (filename, "tiff", &verdict));
   if (!verdict) {
      DIPXJ (dipio_FileAddExtension (filename, &filename, "tif", rg));
   }
   tiff = TIFFOpen (filename->string, "w");
   if (tiff == NULL) {
      DIPSJ ("Could not open the specified file");
   }

   compmode = CompressionTranslate (compression.method);

   switch (photometric) {
      case DIPIO_PHM_GREYVALUE:
         if (!TIFFSetField (tiff, TIFFTAG_PHOTOMETRIC, (uint16)PHOTOMETRIC_MINISBLACK)) {
            DIPSJ (TIFF_WRITE_TAG);
         }
         DIPXJ (WriteTIFFGrayValue (image, tiff, compmode));
         break;
      case DIPIO_PHM_RGB:
         if (!TIFFSetField (tiff, TIFFTAG_PHOTOMETRIC, (uint16)PHOTOMETRIC_RGB)) {
            DIPSJ (TIFF_WRITE_TAG);
         }
         DIPXJ (WriteTIFFFullColour (image, tiff, compmode));
         break;
      case DIPIO_PHM_CIELAB:
         if (!TIFFSetField (tiff, TIFFTAG_PHOTOMETRIC, (uint16)PHOTOMETRIC_CIELAB)) {
            DIPSJ (TIFF_WRITE_TAG);
         }
         DIPXJ (WriteTIFFFullColour (image, tiff, compmode));
         break;
      default:
         /*printf ("Warning: Photometric interpretation not supported in writing TIFF files. Writing as \"Separated\".");*/
      case DIPIO_PHM_CMYK:
         if (!TIFFSetField (tiff, TIFFTAG_PHOTOMETRIC, (uint16)PHOTOMETRIC_SEPARATED)) {
            DIPSJ (TIFF_WRITE_TAG);
         }
         DIPXJ (WriteTIFFFullColour (image, tiff, compmode));
         break;
   }
   TIFFSetField (tiff, TIFFTAG_SOFTWARE, "DIPlib with dipIO");
   DIPXJ( dipio_PhysDimsToDPI (physDims, &xden, &yden) );
   TIFFSetField (tiff, TIFFTAG_XRESOLUTION, (float)xden);
   TIFFSetField (tiff, TIFFTAG_YRESOLUTION, (float)yden);
   TIFFSetField (tiff, TIFFTAG_RESOLUTIONUNIT, (uint16)RESUNIT_INCH);

dip_error:
   if (tiff) {
      TIFFClose (tiff);
   }
}

#endif

} // namespace dip

#else // DIP__HAS_TIFF

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

void ImageWriteTIFF( Image const&, String const&, String const&, dip::uint ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

}

#endif // DIP__HAS_TIFF
