/*
 * DIPlib 3.0
 * This file contains definitions for coordinate image generation
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

#include <libics.h>
#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/generic_iterators.h"
#include "diplib/library/copy_buffer.h"

#include "libics.h"

namespace dip {

namespace {

dip::uint FindTensorDimension(
      ICS* ics,
      UnsignedArray const& sizes,
      String& colorSpace
) {
   dip::uint nDims = sizes.size();
   colorSpace = "";
   dip::uint tensorDim;
   for( tensorDim = 0; tensorDim < nDims; ++tensorDim ) {
      char order[ ICS_STRLEN_TOKEN ];
      DIP_THROW_IF( IcsGetOrder( ics, static_cast< int >( tensorDim ), order, 0 ) != IcsErr_Ok,
                    "IcsGetOrder() failed: couldn't read ICS file" );
      if( strcasecmp( order, "RGB" ) == 0 ) {
         colorSpace = "RGB";
         break;
      } else if( strcasecmp( order, "sRGB" ) == 0 ) {
         colorSpace = "sRGB";
         break;
      } else if( strcasecmp( order, "Lab" ) == 0 ) {
         colorSpace = "Lab";
         break;
      } else if( strcasecmp( order, "Luv" ) == 0 ) {
         colorSpace = "Luv";
         break;
      } else if( strcasecmp( order, "LCH" ) == 0 ) {
         colorSpace = "LCH";
         break;
      } else if( strcasecmp( order, "CMY" ) == 0 ) {
         colorSpace = "CMY";
         break;
      } else if( strcasecmp( order, "CMYK" ) == 0 ) {
         colorSpace = "CMYK";
         break;
      } else if( strcasecmp( order, "XYZ" ) == 0 ) {
         colorSpace = "XYZ";
         break;
      } else if( strcasecmp( order, "Yxy" ) == 0 ) {
         colorSpace = "Yxy";
         break;
      } else if( strcasecmp( order, "HSI" ) == 0 ) {
         colorSpace = "HSI";
         break;
      } else if( strcasecmp( order, "ICH" ) == 0 ) {
         colorSpace = "ICH";
         break;
      } else if( strcasecmp( order, "ISH" ) == 0 ) {
         colorSpace = "ISH";
         break;
      } else if( strcasecmp( order, "HCV" ) == 0 ) {
         colorSpace = "HCV";
         break;
      } else if( strcasecmp( order, "HSV" ) == 0 ) {
         colorSpace = "HSV";
         break;
      } else if( strcasecmp( order, "channel" ) == 0 || strcasecmp( order, "channels" ) == 0 ||
                 strcasecmp( order, "probe" ) == 0 || strcasecmp( order, "probes" ) == 0 ||
                 strcasecmp( order, "tensor" ) == 0 ) {
         break;
      }
   }
   // If the loop above doesn't break, colorDim == nDims, and colorSpace == "".
   if( tensorDim == nDims ) {
      // no color or tensor dimension recognizable from the names, but maybe there's a dimension with few samples?
      dip::uint tensorSize = 100; // initialize to something > 10
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if(( sizes[ ii ] <= 10 ) && ( sizes[ ii ] < tensorSize )) {
            tensorSize = sizes[ ii ];
            tensorDim = ii;
         }
      }
   }
   return tensorDim;
}

class IcsFile {
   public:
      // Constructor. `mode` should start with `r` or `w`.
      // When `mode` starts with `r`, don't give any other options.
      IcsFile( String const& filename, char const* mode ) {
         // Open the file. When reading, try with the exact given name first.
         if( !(( mode[ 0 ] == 'r' ) && ( IcsOpen( &ics_, filename.c_str(), "rf" ) == IcsErr_Ok ))) {
            DIP_THROW_IF( IcsOpen( &ics_, filename.c_str(), mode ) != IcsErr_Ok,
                          "Couldn't open ICS file (IcsOpen failed)" );
         }
      }
      IcsFile( IcsFile const& ) = delete;
      IcsFile( IcsFile&& ) = delete;
      IcsFile& operator=( IcsFile const& ) = delete;
      IcsFile& operator=( IcsFile&& ) = delete;
      ~IcsFile() {
         if( ics_ ) {
            IcsClose( ics_ ); // Don't check for failures, we cannot throw here anyway.
            ics_ = nullptr;
         }
      }
      // Always call Close(), don't let the destructor close the file if all is OK -- it won't throw if there's an error.
      void Close() {
         if( ics_ ) {
            DIP_THROW_IF( IcsClose( ics_ ) != IcsErr_Ok, "Couldn't write to ICS file (IcsClose failed)" );
            ics_ = nullptr;
         }
      }
      // Implicit cast to ICS*
      operator ICS*() { return ics_; }
   private:
      ICS* ics_ = nullptr;
};

struct GetICSInfoData {
   dip::uint tensorDim;
};

GetICSInfoData GetICSInfo( IcsFile& icsFile, FileInformation& fileInformation ) {
   fileInformation.name = static_cast< ICS* >( icsFile )->filename;
   fileInformation.fileType = "ICS";
   fileInformation.numberOfImages = 1;

   GetICSInfoData data;

   // get layout of image data
   Ics_DataType dt;
   int ndims_;
   size_t icsSizes[ICS_MAXDIM];
   DIP_THROW_IF( IcsGetLayout( icsFile, &dt, &ndims_, icsSizes ) != IcsErr_Ok, "Couldn't read ICS file (IcsGetLayout failed)" );
   dip::uint nDims = static_cast< dip::uint >( ndims_ );
   size_t significantBits;
   DIP_THROW_IF( IcsGetSignificantBits( icsFile, &significantBits ) != IcsErr_Ok, "Couldn't read ICS file (IcsGetSignificantBits failed)" );
   fileInformation.significantBits = significantBits;
   fileInformation.sizes.resize( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      fileInformation.sizes[ ii ] = icsSizes[ ii ];
   } // TODO: reorder based on "order" strings

   // get pixel size
   fileInformation.pixelSize.Resize( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      double scale;
      char units[1024];
      DIP_THROW_IF( IcsGetPosition( icsFile, static_cast< int >( ii ), nullptr, &scale, units ) != IcsErr_Ok,
                    "Couldn't read ICS file (IcsGetPosition failed)" );
      try {
         Units u( units );
         fileInformation.pixelSize[ ii ] = { scale, u };
      } catch( ... ) {
         // `Units` failed to parse the string
         fileInformation.pixelSize[ ii ] = { scale };
      }
   }

   // is there a color/tensor dimension?
   data.tensorDim = FindTensorDimension( icsFile, fileInformation.sizes, fileInformation.colorSpace );
   fileInformation.tensorElements = 1;
   if( data.tensorDim != nDims ) {
      // color or tensor dimension found
      fileInformation.tensorElements = fileInformation.sizes[ data.tensorDim ];
      fileInformation.sizes.erase( data.tensorDim );
      fileInformation.pixelSize.EraseDimension( data.tensorDim );
   }

   // convert ICS data type to DIPlib data type
   switch( dt ) {
      case Ics_uint8:
         fileInformation.dataType = significantBits == 1 ? DT_BIN : DT_UINT8;
         break;
      case Ics_uint16:
         fileInformation.dataType = DT_UINT16;
         break;
      case Ics_uint32:
         fileInformation.dataType = DT_UINT32;
         break;
      case Ics_sint8:
         fileInformation.dataType = DT_SINT8;
         break;
      case Ics_sint16:
         fileInformation.dataType = DT_SINT16;
         break;
      case Ics_sint32:
         fileInformation.dataType = DT_SINT32;
         break;
      case Ics_real32:
         fileInformation.dataType = DT_SFLOAT;
         break;
      case Ics_real64:
         fileInformation.dataType = DT_DFLOAT;
         break;
      case Ics_complex32:
         fileInformation.dataType = DT_SCOMPLEX;
         break;
      case Ics_complex64:
         fileInformation.dataType = DT_DCOMPLEX;
         break;
      default:
         DIP_THROW( "Unknown ICS data type" );
   }

   // History tags
   int history_lines;
   DIP_THROW_IF( IcsGetNumHistoryStrings( icsFile, &history_lines ) != IcsErr_Ok, "Couldn't read ICS metadata (IcsGetNumHistoryStrings failed)" );
   fileInformation.history.resize( static_cast< dip::uint >( history_lines ));
   if (history_lines>0) {
      Ics_HistoryIterator it;
      DIP_THROW_IF( IcsNewHistoryIterator( icsFile, &it, 0 ) != IcsErr_Ok, "Couldn't read ICS metadata (IcsNewHistoryIterator failed)");
      char hist[ICS_LINE_LENGTH];
      for( dip::uint ii = 0; ii < history_lines; ++ii ) {
         DIP_THROW_IF( IcsGetHistoryStringI( icsFile, &it, hist ) != IcsErr_Ok, "Couldn't read ICS metadata (IcsGetHistoryStringI failed)");
         fileInformation.history[ ii ] = hist;
      }
   }

   // done
   return data;
}

} // namespace

FileInformation ImageReadICS(
      Image& out,
      String const& filename,
      RangeArray roi
) {
   // open the ICS file
   IcsFile icsFile( filename, "r" );

   // get file information
   FileInformation fileInformation;
   GetICSInfoData data;
   DIP_STACK_TRACE_THIS( data = GetICSInfo( icsFile, fileInformation ));
   UnsignedArray sizes = fileInformation.sizes;
   dip::uint nDims = sizes.size();

   // check & fix ROI information
   UnsignedArray outSizes( nDims );
   BooleanArray mirror( nDims, false );
   DIP_START_STACK_TRACE
      ArrayUseParameter( roi, nDims, Range{} );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         roi[ ii ].Fix( sizes[ ii ] );
         if( roi[ ii ].start > roi[ ii ].stop ) {
            std::swap( roi[ ii ].start, roi[ ii ].stop );
            mirror[ ii ] = true;
         }
         outSizes[ ii ] = roi[ ii ].Size();
      }
   DIP_END_STACK_TRACE

   // forge the image
   out.ReForge( outSizes, fileInformation.tensorElements, fileInformation.dataType );
   out.SetColorSpace( fileInformation.colorSpace );
   out.SetPixelSize( fileInformation.pixelSize );

   // make a quick copy and place the tensor dimension back where it was
   Image outRef = out.QuickCopy();
   if( fileInformation.tensorElements > 1 ) {
      outRef.TensorToSpatial( data.tensorDim );
      Range tensorRange{};
      tensorRange.Fix( fileInformation.tensorElements );
      roi.insert( data.tensorDim, tensorRange );
      sizes.insert( data.tensorDim, fileInformation.tensorElements );
      ++nDims;
   }
   
   // apply the mirroring to the output image
   out.Mirror( mirror );

   // remove any singleton dimensions (in the input file, not the roi)
   // this should improve reading speed, especially if the first dimension is singleton
   for( dip::uint ii = nDims; ii > 0; ) { // loop backwards, so we don't skip a dimension when erasing
      --ii;
      if( sizes[ ii ] == 1 ) {
         sizes.erase( ii );
         roi.erase( ii );
         outRef.Squeeze( ii );
      }
   }
   nDims = outRef.Dimensionality();

   // prepare the buffer
   dip::uint sizeOf = fileInformation.dataType.SizeOf();
   dip::uint bufSize = sizeOf * (( outRef.Size( 0 ) - 1 ) * roi[ 0 ].step + 1 );
   std::vector< uint8 > buffer( bufSize );

   // prepare the strides of the image on file
   UnsignedArray strides( nDims );
   strides[ 0 ] = 1;
   for( dip::uint ii = 1; ii < nDims; ++ii ) {
      strides[ ii ] = strides[ ii - 1 ] * sizes[ ii - 1 ];
   }

   // read the data
   dip::uint cur_loc = 0;
   GenericImageIterator<> it( outRef, 0 );
   do {
      // find location in file to read at
      UnsignedArray const& curipos = it.Coordinates();
      dip::uint new_loc = 0;
      new_loc += sizeOf * roi[ 0 ].Offset();
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         dip::uint curfpos = curipos[ ii ] * roi[ ii ].step + roi[ ii ].Offset();
         new_loc += sizeOf * curfpos * strides[ ii ];
      }
      // read line portion into buffer
      if( new_loc > cur_loc ) {
         IcsSkipDataBlock( icsFile, new_loc - cur_loc );
         cur_loc = new_loc;
      }
      DIP_THROW_IF( IcsGetDataBlock( icsFile, buffer.data(), bufSize ) != IcsErr_Ok,
                    "Couldn't read pixel data from ICS file (IcsGetDataBlock failed)" );
      cur_loc += bufSize;
      // copy buffer to image
      detail::CopyBuffer( buffer.data(), fileInformation.dataType, static_cast< dip::sint >( roi[ 0 ].step ), 1,
                          it.Pointer(), outRef.DataType(), outRef.Stride( 0 ), 1,
                          outRef.Size( 0 ), 1 );
   } while( ++it );

   // we're done
   icsFile.Close();
   return fileInformation;
}

FileInformation ImageReadICS(
      Image& image,
      String const& filename,
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      UnsignedArray const& spacing
) {
   dip::uint n = origin.size();
   n = std::max( n, sizes.size() );
   n = std::max( n, spacing.size() );
   if( n > 1 ) {
      DIP_THROW_IF(( origin.size() > 1 ) && ( origin.size() != n ), E::ARRAY_SIZES_DONT_MATCH );
      DIP_THROW_IF(( sizes.size() > 1 ) && ( sizes.size() != n ), E::ARRAY_SIZES_DONT_MATCH );
      DIP_THROW_IF(( spacing.size() > 1 ) && ( spacing.size() != n ), E::ARRAY_SIZES_DONT_MATCH );
   }
   RangeArray roi( n );
   if( origin.size() == 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].start = static_cast< dip::sint >( origin[ 0 ] );
      }
   } else if( origin.size() > 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].start = static_cast< dip::sint >( origin[ ii ] );
      }
   }
   if( sizes.size() == 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].stop = roi[ ii ].start + static_cast< dip::sint >( sizes[ 0 ] ) - 1;
      }
   } else if( sizes.size() > 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].stop = roi[ ii ].start + static_cast< dip::sint >( sizes[ ii ] ) - 1;
      }
   }
   if( spacing.size() == 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].step = spacing[ 0 ];
      }
   } else if( spacing.size() > 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].step = spacing[ ii ];
      }
   }
   return ImageReadICS( image, filename, roi );
}

FileInformation ImageReadICSInfo( String const& filename ) {
   // open the ICS file
   IcsFile icsFile( filename, "r" );

   // get file information
   FileInformation fileInformation;
   DIP_STACK_TRACE_THIS( GetICSInfo( icsFile, fileInformation ));

   // done
   icsFile.Close();
   return fileInformation;
}

bool ImageIsICS( String const& filename ) {
   return IcsVersion(filename.c_str(), 1) != 0;
}

void ImageWriteICS(
      Image const& image,
      String const& filename,
      StringArray const& history,
      dip::uint significantBits,
      StringSet const& options
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

} // namespace dip
