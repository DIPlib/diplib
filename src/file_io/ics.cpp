/*
 * (c)2017-2025, Cris Luengo.
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

#ifdef DIP_CONFIG_HAS_ICS

#include "diplib/file_io.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/generic_iterators.h"
#include "diplib/library/copy_buffer.h"

#include "file_io_support.h"

#include "libics.h"

namespace dip {

namespace {

constexpr char const* CANNOT_READ_ICS_FILE = "Couldn't read ICS file";
constexpr char const* CANNOT_READ_ICS_METADATA = "Couldn't read ICS metadata";
constexpr char const* CANNOT_READ_ICS_PIXELS = "Couldn't read pixel data from ICS file";
constexpr char const* CANNOT_WRITE_ICS_FILE = "Couldn't write to ICS file";
constexpr char const* CANNOT_WRITE_ICS_METADATA = "Couldn't write metadata to ICS file";
constexpr char const* CANNOT_WRITE_ICS_PIXELS = "Couldn't write data to ICS file";

#define CALL_ICS( function_call, message ) do { Ics_Error error_ = function_call; if( error_ != IcsErr_Ok ) \
{ DIP_THROW_RUNTIME( String( message ) + ": " + IcsGetErrorText( error_ )); }} while(false)

dip::uint FindTensorDimension(
      ICS* ics,
      UnsignedArray const& sizes,
      String& colorSpace
) {
   dip::uint nDims = sizes.size();
   colorSpace = "";
   dip::uint tensorDim = 0;
   for( ; tensorDim < nDims; ++tensorDim ) {
      char const* c_order{};
      CALL_ICS( IcsGetOrderF( ics, static_cast< int >( tensorDim ), &c_order, nullptr ), CANNOT_READ_ICS_FILE );
      String order = c_order;
      ToLowerCase( order );
      // TODO: I don't like having the list of known color spaces here. There must be a better way of doing this.
      if( order == "rgb" ) {
         colorSpace = "RGB";
         break;
      }
      if( order == "srgb" ) {
         colorSpace = "sRGB";
         break;
      }
      if( order == "srgba" ) {
         colorSpace = "sRGBA";
         break;
      }
      if( order == "lab" ) {
         colorSpace = "Lab";
         break;
      }
      if( order == "luv" ) {
         colorSpace = "Luv";
         break;
      }
      if( order == "lch" ) {
         colorSpace = "LCH";
         break;
      }
      if( order == "cmy" ) {
         colorSpace = "CMY";
         break;
      }
      if( order == "cmyk" ) {
         colorSpace = "CMYK";
         break;
      }
      if( order == "xyz" ) {
         colorSpace = "XYZ";
         break;
      }
      if( order == "yxy" ) {
         colorSpace = "Yxy";
         break;
      }
      if( order == "hsi" ) {
         colorSpace = "HSI";
         break;
      }
      if( order == "ich" ) {
         colorSpace = "ICH";
         break;
      }
      if( order == "ish" ) {
         colorSpace = "ISH";
         break;
      }
      if( order == "hcv" ) {
         colorSpace = "HCV";
         break;
      }
      if( order == "hsv" ) {
         colorSpace = "HSV";
         break;
      }
      if( order == "y'pbpr" ) {
         colorSpace = "Y'PbPr";
         break;
      }
      if( order == "y'cbcr" ) {
         colorSpace = "Y'CbCr";
         break;
      }
      if( order == "oklab" ) {
         colorSpace = "Oklab";
         break;
      }
      if( order == "oklch" ) {
         colorSpace = "Oklch";
         break;
      }
      if(( order == "channel" ) || ( order == "channels" ) || ( order == "probe" ) || ( order == "probes" ) || ( order == "tensor" )) {
         break;
      }
   }
   // If the loop above doesn't break, colorDim == nDims, and colorSpace == "".
   /* This is confusing, let's no do this.
   if( tensorDim == nDims ) {
      // No color or tensor dimension recognizable from the names, but maybe there's a dimension with few samples?
      dip::uint tensorSize = 100; // initialize to something > 10
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if(( sizes[ ii ] <= 10 ) && ( sizes[ ii ] < tensorSize )) {
            tensorSize = sizes[ ii ];
            tensorDim = ii;
         }
      }
   }
   */
   return tensorDim;
}

// Finds out how to reorder dimensions as they are written to the ICS file
//    y,x -> x,y
//    t,x,y -> x,y,t
//    x,q,y -> x,y,q
//    x,y,q,t -> x,y,q,t
//    x,y,t,q -> x,y,t,q
//    dim_3,dim_2,dim_1 -> dim_1,dim_2,dim_3
// - x, y, z are always first 3 dimensions
// - dim_N always goes to dimension N, unless there's a conflict with x, y, z
// - t comes after x, y, z, but otherwise is sorted where it was
// - unknown strings (e.g. q) are sorted where they are, but after x, y, z and also displaced by dim_N
// - dim_0 == x, dim_1 == y, dim_2 == z
UnsignedArray FindDimensionOrder( ICS* ics, dip::uint nDims, dip::uint tensorDim ) {
   struct FileDims {
      dip::uint order = 0;      // where to put the dimension, use only if `known` is true
      bool known = false;       // set if name was recognized
      bool priority = false;    // set if it's one of x, y, z
   };
   // Find recognized labels
   DimensionArray< FileDims > file( nDims ); // This array contains the destination location for each of the input (file) dimensions
   dip::uint maxDim = 2;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( ii == tensorDim ) {
         //std::cout << "dim " << ii << " is tensorDim\n";
         continue;
      }
      char const* order{};
      CALL_ICS( IcsGetOrderF( ics, static_cast< int >( ii ), &order, nullptr ), CANNOT_READ_ICS_FILE );
      //std::cout << "dim " << ii << " is " << order << std::endl;
      if( StringCompareCaseInsensitive( order, "x" )) {
         file[ ii ].order = 0;
         file[ ii ].known = true;
         file[ ii ].priority = true;
      } else if( StringCompareCaseInsensitive( order, "y" )) {
         file[ ii ].order = 1;
         file[ ii ].known = true;
         file[ ii ].priority = true;
      } else if( StringCompareCaseInsensitive( order, "z" )) {
         file[ ii ].order = 2;
         file[ ii ].known = true;
         file[ ii ].priority = true;
      } else if(( std::tolower( order[ 0 ] ) == 'd' ) &&
                ( std::tolower( order[ 1 ] ) == 'i' ) &&
                ( std::tolower( order[ 2 ] ) == 'm' )) {   // "dim_%d", "dim%d"
         order += 3;
         if( *order == '_' ) {
            ++order;
         }
         char* end = nullptr;
         dip::uint dim = std::strtoul( order, &end, 10 ); // Note that std::strtoul() sets `end` removing the `const` qualifier!
         if( end > order ) { // The test on `end` checks to see if std::strtoul() converted some characters.
            file[ ii ].order = dim;
            file[ ii ].known = true;
            file[ ii ].priority = false;
            maxDim = std::max( maxDim, dim );
         }
      }
   }
   // Move tensor dimension to the end
   if( tensorDim < nDims ) {
      maxDim = std::max( maxDim + 1, nDims - 1 );
      file[ tensorDim ].order = maxDim;
      file[ tensorDim ].known = true;
      file[ tensorDim ].priority = false;
   }
   // Create inverse lookup
   std::vector< UnsignedArray > inv( maxDim + 1 ); // This array contains the source location for each of the output dimensions
   UnsignedArray unknown;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( file[ ii ].known ) {
         inv[ file[ ii ].order ].push_back( ii );
      } else {
         unknown.push_back( ii );
      }
   }
   // Create order array
   UnsignedArray order( nDims );
   dip::uint jj = 0;
   // Put all "priority" elements first
   for( auto& list : inv ) {
      for( auto ii : list ) {
         if( file[ ii ].priority ) {
            order[ jj ] = ii;
            ++jj;
         }
      }
   }
   // Next come the non-priority ones
   auto unknownIt = unknown.begin();
   for( auto& list : inv ) {
      for( auto ii : list ) {
         if( !file[ ii ].priority ) { // `file[ii].known` is `true`, otherwise it wouldn't be in the `inv` list.
            dip::uint kk = file[ ii ].order;
            while(( jj < kk ) && ( unknownIt != unknown.end() )) {
               // Put in unknown ones here so that 'dim_6' actually ends up at index 6:
               order[ jj ] = *unknownIt;
               ++unknownIt;
               ++jj;
            }
            order[ jj ] = ii;
            ++jj;
         }
      }
   }
   // Finally take the rest of the unknown ones
   while( unknownIt != unknown.end() ) {
      order[ jj ] = *unknownIt;
      ++unknownIt;
      ++jj;
   }
   //std::cout << "order = " << order << std::endl;
   // Double-check our work
#ifdef DIP_CONFIG_ENABLE_ASSERT
   DIP_ASSERT( jj == nDims );
   UnsignedArray tmp( nDims );
   std::copy( order.begin(), order.end(), tmp.begin() );
   tmp.sort();
   //std::cout << "sorted = " << tmp << std::endl;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_ASSERT( tmp[ ii ] == ii );
   }
#endif
   // Done!
   return order;
}

class IcsFile {
   public:
      // Constructor. `mode` should start with `r` or `w`.
      // When `mode` starts with `r`, don't give any other options.
      IcsFile( String const& filename, char const* mode ) {
         // Open the file. When reading, try with the exact given name first.
         if( !(( mode[ 0 ] == 'r' ) && ( IcsOpen( &ics_, filename.c_str(), "rf" ) == IcsErr_Ok ))) {
            CALL_ICS( IcsOpen( &ics_, filename.c_str(), mode ), "Couldn't open ICS file" );
         }
      }
      IcsFile( IcsFile const& ) = delete;
      IcsFile( IcsFile&& ) = delete;
      IcsFile& operator=( IcsFile const& ) = delete;
      IcsFile& operator=( IcsFile&& ) = delete;
      ~IcsFile() {
         if( ics_ ) {
            IcsClose( ics_ ); // Don't check for failures, we cannot throw here anyway.
         }
      }
      // Always call Close(), don't let the destructor close the file if all is OK -- it won't throw if there's an error.
      void Close() {
         if( ics_ ) {
            // Not using CALL_ICS here, we need to set `ics_` to NULL before throwing, otherwise the destructor is going to try to close again!
            Ics_Error error = IcsClose( ics_ );
            ics_ = nullptr;
            if( error != IcsErr_Ok ) {
               DIP_THROW_RUNTIME( String( "Couldn't close ICS file: " ) + IcsGetErrorText( error ) );
            }
         }
      }
      // Implicit cast to ICS*
      operator ICS*() { return ics_; }
   private:
      ICS* ics_ = nullptr;
};

struct GetICSInfoData {
   FileInformation fileInformation;
   UnsignedArray fileSizes;   // Sizes in the order they appear in the file (including the tensor dimension).
   UnsignedArray order;       // How to reorder the dimensions: image dimension ii is file dimension order[ii];
                              // if there is a tensor dimension, then order.back() is tensorDim
};

GetICSInfoData GetICSInfo( IcsFile& icsFile ) {
   GetICSInfoData data;

   data.fileInformation.name = static_cast< ICS* >( icsFile )->filename;
   data.fileInformation.fileType = "ICS";
   data.fileInformation.numberOfImages = 1;

   // get layout of image data
   Ics_DataType dt{};
   int ndims_{};
   std::size_t icsSizes[ICS_MAXDIM];
   CALL_ICS( IcsGetLayout( icsFile, &dt, &ndims_, icsSizes ), CANNOT_READ_ICS_FILE );
   dip::uint nDims = static_cast< dip::uint >( ndims_ );
   std::size_t significantBits{};
   CALL_ICS( IcsGetSignificantBits( icsFile, &significantBits ), CANNOT_READ_ICS_FILE );
   data.fileInformation.significantBits = significantBits;
   // convert ICS data type to DIP data type
   switch( dt ) {
      case Ics_uint8:
         data.fileInformation.dataType = significantBits == 1 ? DT_BIN : DT_UINT8;
         break;
      case Ics_uint16:
         data.fileInformation.dataType = DT_UINT16;
         break;
      case Ics_uint32:
         data.fileInformation.dataType = DT_UINT32;
         break;
      case Ics_uint64:
         data.fileInformation.dataType = DT_UINT64;
         break;
      case Ics_sint8:
         data.fileInformation.dataType = DT_SINT8;
         break;
      case Ics_sint16:
         data.fileInformation.dataType = DT_SINT16;
         break;
      case Ics_sint32:
         data.fileInformation.dataType = DT_SINT32;
         break;
      case Ics_sint64:
         data.fileInformation.dataType = DT_SINT64;
         break;
      case Ics_real32:
         data.fileInformation.dataType = DT_SFLOAT;
         break;
      case Ics_real64:
         data.fileInformation.dataType = DT_DFLOAT;
         break;
      case Ics_complex32:
         data.fileInformation.dataType = DT_SCOMPLEX;
         break;
      case Ics_complex64:
         data.fileInformation.dataType = DT_DCOMPLEX;
         break;
      default:
         DIP_THROW_RUNTIME( "Unknown ICS data type" );
   }
   data.fileSizes.resize( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      data.fileSizes[ ii ] = static_cast< dip::uint >( icsSizes[ ii ] );
   }

   // get pixel size and origin
   PixelSize pixelSize;
   PhysicalQuantityArray origin( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      double scale{}, offset{};
      char const* units{};
      CALL_ICS( IcsGetPositionF( icsFile, static_cast< int >( ii ), &offset, &scale, &units ), CANNOT_READ_ICS_FILE );
      if( StringCompareCaseInsensitive( units, "undefined" )) {
         pixelSize.Set( ii, PhysicalQuantity::Pixel() );
         origin[ ii ] = offset * PhysicalQuantity::Pixel();
      } else {
         Units u;
         try {
            u.FromString( units );
         } catch( Error const& ) {
            u = Units::Pixel();
         }
         PhysicalQuantity ps{ scale, u };
         ps.Normalize();
         pixelSize.Set( ii, ps );
         PhysicalQuantity o{ offset, u };
         o.Normalize();
         origin[ ii ] = o;
      }
   }

   // is there a color/tensor dimension?
   dip::uint tensorDim = FindTensorDimension( icsFile, data.fileSizes, data.fileInformation.colorSpace );
   data.fileInformation.tensorElements = ( tensorDim < nDims ) ? data.fileSizes[ tensorDim ] : 1;

   // re-order dimensions
   data.order = FindDimensionOrder( icsFile, nDims, tensorDim );
   data.fileInformation.sizes.resize( nDims );
   data.fileInformation.origin.resize( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      data.fileInformation.sizes[ ii ] = data.fileSizes[ data.order[ ii ]];
      data.fileInformation.pixelSize.Set( ii, pixelSize[ data.order[ ii ]] );
      data.fileInformation.origin[ ii ] = origin[ data.order[ ii ]];
   }
   if( data.fileInformation.tensorElements > 1 ) {
      data.fileInformation.sizes.pop_back();
      //data.fileInformation.pixelSize.EraseDimension( nDims - 1 ); // doesn't do anything
      data.fileInformation.origin.pop_back();
   }

   // History tags
   int history_lines{};
   CALL_ICS( IcsGetNumHistoryStrings( icsFile, &history_lines ), CANNOT_READ_ICS_METADATA );
   data.fileInformation.history.resize( static_cast< dip::uint >( history_lines ));
   if( history_lines > 0 ) {
      Ics_HistoryIterator it;
      CALL_ICS( IcsNewHistoryIterator( icsFile, &it, nullptr ), CANNOT_READ_ICS_METADATA );
      char const* hist{};
      for( dip::uint ii = 0; ii < static_cast< dip::uint >( history_lines ); ++ii ) {
         CALL_ICS( IcsGetHistoryStringIF( icsFile, &it, &hist ), CANNOT_READ_ICS_METADATA );
         data.fileInformation.history[ ii ] = hist;
      }
   }

   // done
   return data;
}

} // namespace

FileInformation ImageReadICS(
      Image& out,
      String const& filename,
      RangeArray const& roi,
      Range const& channels,
      String const& mode
) {
   bool fast{};
   DIP_STACK_TRACE_THIS( fast = BooleanFromString( mode, "fast", "" ));

   // open the ICS file
   IcsFile icsFile( filename, "r" );

   // get file information
   GetICSInfoData data;
   DIP_STACK_TRACE_THIS( data = GetICSInfo( icsFile ));

   //std::cout << "[ImageReadICS] fileInformation.sizes = " << data.fileInformation.sizes << std::endl;
   //std::cout << "[ImageReadICS] fileInformation.tensorElements = " << data.fileInformation.tensorElements << std::endl;
   //std::cout << "[ImageReadICS] fileSizes = " << data.fileSizes << std::endl;
   //std::cout << "[ImageReadICS] order = " << data.order << std::endl;

   UnsignedArray sizes = data.fileInformation.sizes;
   UnsignedArray order = data.order;
   dip::uint nDims = sizes.size();

   // check & fix ROI information
   RoiSpec roiSpec;
   DIP_STACK_TRACE_THIS( roiSpec = CheckAndConvertRoi( roi, channels, data.fileInformation, nDims ));
   if( !roiSpec.isFullImage || !roiSpec.isAllChannels ) {
      fast = false;
   }

   // prepare the strides of the image on file (including tensor dimension)
   UnsignedArray tmp( data.fileSizes.size() );
   tmp[ 0 ] = 1;
   for( dip::uint ii = 1; ii < tmp.size(); ++ii ) {
      tmp[ ii ] = tmp[ ii - 1 ] * data.fileSizes[ ii - 1 ];
   }
   IntegerArray strides( tmp.size() );
   for( dip::uint ii = 0; ii < tmp.size(); ++ii ) {
      strides[ ii ] = static_cast< dip::sint >( tmp[ data.order[ ii ]] );
   }
   // if there's a tensor dimension, it's sorted last in `strides`.
   //std::cout << "[ImageReadICS] strides = " << strides << std::endl;

   // if "fast", try to match strides with those in the file
   if( fast ) {
      IntegerArray reqStrides( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         reqStrides[ ii ] = strides[ ii ];
      }
      dip::sint reqTensorStride = roiSpec.tensorElements > 1 ? strides.back() : 1;
      if(   ( out.Strides() != reqStrides )
         || ( out.TensorStride() != reqTensorStride )
         || ( out.Sizes() != roiSpec.sizes )
         || ( out.TensorElements() != roiSpec.tensorElements )
         || ( out.DataType() != data.fileInformation.dataType )) {
         out.Strip();
      }
      if( !out.IsForged() ) {
         out.SetStrides( std::move( reqStrides ));
         out.SetTensorStride( reqTensorStride );
      }
   }

   // forge the image
   out.ReForge( roiSpec.sizes, roiSpec.tensorElements, data.fileInformation.dataType );
   if( roiSpec.tensorElements == data.fileInformation.tensorElements ) {
      out.SetColorSpace( data.fileInformation.colorSpace );
   }
   out.SetPixelSize( data.fileInformation.pixelSize );

   // get tensor shape if necessary
   if(( roiSpec.tensorElements > 1 ) && ( roiSpec.tensorElements == data.fileInformation.tensorElements )) {
      Ics_HistoryIterator it;
      Ics_Error e = IcsNewHistoryIterator( icsFile, &it, "tensor" );
      if( e == IcsErr_Ok ) {
         char line[ ICS_LINE_LENGTH ];
         e = IcsGetHistoryKeyValueI( icsFile, &it, nullptr, line );
         if( e == IcsErr_Ok ) {
            // parse `value`
            char* ptr = std::strtok( line, "\t" );
            if( ptr != nullptr ) {
               char* shape = ptr;
               ptr = std::strtok( nullptr, "\t" );
               if( ptr != nullptr ) {
                  dip::uint rows = std::stoul( ptr );
                  ptr = std::strtok( nullptr, "\t" );
                  if( ptr != nullptr ) {
                     dip::uint columns = std::stoul( ptr );
                     try {
                        out.ReshapeTensor( Tensor{ shape, rows, columns } );
                     } catch ( Error const& ) { // NOLINT(*-empty-catch)
                        // Let this error slip, we don't really care
                     }
                  }
               }
            }
         }
      }
   }
   //std::cout << "[ImageReadICS] out = " << out << std::endl;

   // make a quick copy and place the tensor dimension at the back
   Image outRef = out.QuickCopy();
   if( data.fileInformation.tensorElements > 1 ) {
      outRef.TensorToSpatial();
      roiSpec.roi.push_back( roiSpec.channels );
      sizes.push_back( roiSpec.tensorElements );
      ++nDims;
   }
   //std::cout << "[ImageReadICS] outRef = " << outRef << std::endl;

   if( strides == out.Strides() ) {
      // Fast reading!
      //std::cout << "[ImageReadICS] fast reading!\n";

      CALL_ICS( IcsGetData( icsFile, outRef.Origin(), outRef.NumberOfPixels() * outRef.DataType().SizeOf() ), CANNOT_READ_ICS_PIXELS );

   } else {
      // Reading using strides
      //std::cout << "[ImageReadICS] reading with strides\n";

      // remove any singleton dimensions (in the input file, not the roi)
      // this should improve reading speed, especially if the first dimension is singleton
      for( dip::uint ii = nDims; ii > 0; ) { // loop backwards, so we don't skip a dimension when erasing
         --ii;
         if( sizes[ ii ] == 1 ) {
            sizes.erase( ii );
            roiSpec.roi.erase( ii );
            order.erase( ii );
            strides.erase( ii );
            outRef.Squeeze( ii );
         }
      }
      nDims = outRef.Dimensionality();

      // re-order dimensions according to strides, so that we only go forward in the file
      auto sort = strides.sorted_indices();
      outRef.PermuteDimensions( sort );
      sizes = sizes.permute( sort );
      roiSpec.roi = roiSpec.roi.permute( sort );
      order = order.permute( sort );
      strides = strides.permute( sort );

      // what is the processing dimension?
      dip::uint procDim = 0;
      for( dip::uint ii = 1; ii < order.size(); ++ii ) {
         if( order[ ii ] < order[ procDim ] ) {
            procDim = ii;
         }
      }

      // prepare the buffer
      dip::uint sizeOf = data.fileInformation.dataType.SizeOf();
      dip::uint bufSize = sizeOf * (( outRef.Size( procDim ) - 1 ) * roiSpec.roi[ procDim ].step + 1 );
      std::vector< uint8 > buffer( bufSize );

      // read the data
      dip::uint cur_loc = 0;
      GenericImageIterator<> it( outRef, procDim );
      do {
         // find location in file to read at
         UnsignedArray const& curipos = it.Coordinates();
         dip::uint new_loc = sizeOf * roiSpec.roi[ procDim ].Offset();
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != procDim ) {
               dip::uint curfpos = curipos[ ii ] * roiSpec.roi[ ii ].step + roiSpec.roi[ ii ].Offset();
               new_loc += sizeOf * curfpos * static_cast< dip::uint >( strides[ ii ] );
            }
         }
         // read line portion into buffer
         DIP_ASSERT( new_loc >= cur_loc ); // we cannot move backwards!
         if( new_loc > cur_loc ) {
            IcsSkipDataBlock( icsFile, new_loc - cur_loc );
            cur_loc = new_loc;
         }
         CALL_ICS( IcsGetDataBlock( icsFile, buffer.data(), bufSize ), CANNOT_READ_ICS_PIXELS );
         cur_loc += bufSize;
         // copy buffer to image
         detail::CopyBuffer( buffer.data(), data.fileInformation.dataType, static_cast< dip::sint >( roiSpec.roi[ procDim ].step ), 1,
                             it.Pointer(), outRef.DataType(), outRef.Stride( procDim ), 1,
                             outRef.Size( procDim ), 1 );
      } while( ++it );

   }

   // apply the mirroring to the output image
   out.Mirror( roiSpec.mirror );

   // we're done
   icsFile.Close();
   return data.fileInformation;
}

FileInformation ImageReadICS(
      Image& image,
      String const& filename,
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      UnsignedArray const& spacing,
      Range const& channels,
      String const& mode
) {
   RangeArray roi;
   DIP_STACK_TRACE_THIS( roi = ConvertRoiSpec( origin, sizes, spacing ));
   return ImageReadICS( image, filename, roi, channels, mode );
}

FileInformation ImageReadICSInfo( String const& filename ) {
   // open the ICS file
   IcsFile icsFile( filename, "r" );

   // get file information
   GetICSInfoData data;
   DIP_STACK_TRACE_THIS( data = GetICSInfo( icsFile ));

   // done
   icsFile.Close();
   return data.fileInformation;
}

bool ImageIsICS( String const& filename ) {
   try {
      IcsFile icsFile( filename, "r" );
   } catch( ... ) {
      return false;
   }
   return true;
}

namespace {

inline bool StridesArePositive( IntegerArray const& strides ) {
   for( auto s : strides ) {
      if( s < 1 ) {
         return false;
      }
   }
   return true;
}

} // namespace

void ImageWriteICS(
      Image const& c_image,
      String const& filename,
      StringArray const& history,
      dip::uint significantBits,
      StringSet const& options
) {
   DIP_THROW_IF( !c_image.IsForged(), E::IMAGE_NOT_FORGED );
   // parse options
   bool oldStyle = false; // true if v1
   bool compress = true;
   bool fast = false;
   for( auto& option : options ) {
      if( option == "v1" ) {
         oldStyle = true;
      } else if( option == "v2" ) {
         oldStyle = false;
      } else if( option == "uncompressed" ) {
         compress = false;
      } else if( option == "gzip" ) {
         compress = true;
      } else if( option == "fast" ) {
         fast = true;
      } else {
         DIP_THROW_INVALID_FLAG( option );
      }
   }

   // should we reorder dimensions?
   if( fast ) {
      if( !c_image.HasContiguousData() || !StridesArePositive( c_image.Strides() )) {
         fast = false;
      }
   }

   // find info on image
   Ics_DataType dt{};
   dip::uint maxSignificantBits{};
   switch( c_image.DataType() ) {
      case DT_BIN:      dt = Ics_uint8;     maxSignificantBits = 1;  break;
      case DT_UINT8:    dt = Ics_uint8;     maxSignificantBits = 8;  break;
      case DT_UINT16:   dt = Ics_uint16;    maxSignificantBits = 16; break;
      case DT_UINT32:   dt = Ics_uint32;    maxSignificantBits = 32; break;
      case DT_UINT64:   dt = Ics_uint64;    maxSignificantBits = 64; break;
      case DT_SINT8:    dt = Ics_sint8;     maxSignificantBits = 8;  break;
      case DT_SINT16:   dt = Ics_sint16;    maxSignificantBits = 16; break;
      case DT_SINT32:   dt = Ics_sint32;    maxSignificantBits = 32; break;
      case DT_SINT64:   dt = Ics_sint64;    maxSignificantBits = 64; break;
      case DT_SFLOAT:   dt = Ics_real32;    maxSignificantBits = 32; break;
      case DT_DFLOAT:   dt = Ics_real64;    maxSignificantBits = 64; break;
      case DT_SCOMPLEX: dt = Ics_complex32; maxSignificantBits = 32; break;
      case DT_DCOMPLEX: dt = Ics_complex64; maxSignificantBits = 64; break;
      default:
         DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED ); // Should not happen
   }
   if( significantBits == 0 ) {
      significantBits = maxSignificantBits;
   } else {
      significantBits = std::min( significantBits, maxSignificantBits );
   }

   // Quick copy of the image, with tensor dimension moved to the end
   Image image = c_image.QuickCopy();
   bool isTensor = false;
   if( image.TensorElements() > 1 ) {
      isTensor = true;
      image.TensorToSpatial(); // last dimension
   }

   // open the ICS file
   IcsFile icsFile( filename, oldStyle ? "w1" : "w2" );

   // set info on image
   int nDims = static_cast< int >( image.Dimensionality() );
   CALL_ICS( IcsSetLayout( icsFile, dt, nDims, image.Sizes().data() ), CANNOT_WRITE_ICS_FILE );
   if( nDims >= 5 ) {
      // By default, 5th dimension is called "probe", but this is turned into a tensor dimension...
      CALL_ICS( IcsSetOrder( icsFile, 4, "dim_4", nullptr ), CANNOT_WRITE_ICS_FILE );
   }
   CALL_ICS( IcsSetSignificantBits( icsFile, significantBits ), CANNOT_WRITE_ICS_FILE );
   if( c_image.IsColor() ) {
      CALL_ICS( IcsSetOrder( icsFile, nDims - 1, c_image.ColorSpace().c_str(), nullptr ), CANNOT_WRITE_ICS_FILE );
   } else if( isTensor ) {
      CALL_ICS( IcsSetOrder( icsFile, nDims - 1, "tensor", nullptr ), CANNOT_WRITE_ICS_FILE );
   }
   if( c_image.HasPixelSize() ) {
      if( isTensor ) { nDims--; }
      for( int ii = 0; ii < nDims; ii++ ) {
         auto pixelSize = c_image.PixelSize( static_cast< dip::uint >( ii ));
         CALL_ICS( IcsSetPosition( icsFile, ii, 0.0, pixelSize.magnitude, pixelSize.units.String().c_str() ), CANNOT_WRITE_ICS_FILE );
      }
      if( isTensor ) {
         CALL_ICS( IcsSetPosition( icsFile, nDims, 0.0, 1.0, nullptr ), CANNOT_WRITE_ICS_FILE );
      }
   }
   if( isTensor ) {
      String tensorShape = c_image.Tensor().TensorShapeAsString() + "\t" +
                           std::to_string( c_image.Tensor().Rows() ) + "\t" +
                           std::to_string( c_image.Tensor().Columns() );
      CALL_ICS( IcsAddHistory( icsFile, "tensor", tensorShape.c_str() ), CANNOT_WRITE_ICS_METADATA );
   }

   // set type of compression
   CALL_ICS( IcsSetCompression( icsFile, compress ? IcsCompr_gzip : IcsCompr_uncompressed, 9 ), CANNOT_WRITE_ICS_FILE );

   // set the image data
   if( fast ) {
      UnsignedArray order = image.Strides().sorted_indices();
      image.PermuteDimensions( order ); // This is the same as `image.StandardizeStrides()`, but with a lot of redundant checking
      DIP_ASSERT( image.HasNormalStrides() ); // Otherwise things go bad...
      ICS* ics = icsFile;
      Ics_DataRepresentation dim[ ICS_MAXDIM ];
      dip::uint nd = order.size();
      for( dip::uint ii = 0; ii < image.Dimensionality(); ++ii ) {
         std::memcpy( &( dim[ ii ] ), &( ics->dim[ order[ ii ]] ), sizeof( Ics_DataRepresentation ));
      }
      std::memcpy( ics->dim, dim, sizeof( Ics_DataRepresentation ) * nd ); // Copy only the dimensions we've set.
   }
   if( image.HasNormalStrides() ) {
      CALL_ICS( IcsSetData( icsFile, image.Origin(), image.NumberOfPixels() * image.DataType().SizeOf() ), CANNOT_WRITE_ICS_PIXELS );
   } else {
      CALL_ICS( IcsSetDataWithStrides( icsFile, image.Origin(), image.NumberOfPixels() * image.DataType().SizeOf(),
                                       image.Strides().data(), static_cast< int >( image.Dimensionality() )), CANNOT_WRITE_ICS_PIXELS );
   }

   // tag the data
   CALL_ICS( IcsAddHistory( icsFile, "software", "DIPlib " DIP_VERSION_STRING ), CANNOT_WRITE_ICS_METADATA );

   // write history lines
   for( auto const& line : history ) {
      auto error = IcsAddHistory( icsFile, nullptr, line.c_str() );
      if(( error == IcsErr_LineOverflow ) || // history line is too long
         ( error == IcsErr_IllParameter )) { // history line contains illegal characters
         // Ignore these errors, the history line will not be written.
      }
      CALL_ICS( error, CANNOT_WRITE_ICS_METADATA );
   }

   // write everything to file by closing it
   icsFile.Close();
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE( "[DIPlib] testing ICS file reading and writing" ) {
   dip::Image image = dip::ImageReadICS( DIP_EXAMPLES_DIR "/chromo3d.ics" );
   image.SetPixelSize( dip::PhysicalQuantityArray{ 6 * dip::Units::Micrometer(), 300 * dip::Units::Nanometer() } );

   dip::ImageWriteICS( image, "test1.ics", { "line1", "line2 is good" }, 7, { "v1", "uncompressed" } );
   dip::Image result = dip::ImageReadICS( "test1", dip::RangeArray{}, {} );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::FULL ));

   dip::ImageWriteICS( image, "test1f.ics", { "line1", "line2 is good" }, 7, { "v1", "uncompressed", "fast" } );
   result = dip::ImageReadICS( "test1f", dip::RangeArray{}, {}, "fast" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::FULL ));

   result = dip::ImageReadICS( "test1f", dip::RangeArray{}, {} );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::FULL ));

   result = dip::ImageReadICS( "test1", dip::RangeArray{}, {}, "fast" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::FULL ));

   // Turn it on its side so the image to write has non-standard strides
   image.SwapDimensions( 0, 2 );

   dip::ImageWriteICS( image, "test2.ics", { "key\tvalue" }, 7, { "v1", "uncompressed" } );
   result = dip::ImageReadICS( "test2", dip::RangeArray{}, {} );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::FULL ));

   dip::ImageWriteICS( image, "test2f.ics", { "key\tvalue" }, 7, { "v1", "uncompressed", "fast" } );
   result = dip::ImageReadICS( "test2f", dip::RangeArray{}, {}, "fast" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::FULL ));

   result = dip::ImageReadICS( "test2f", dip::RangeArray{}, {} );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::FULL ));

   result = dip::ImageReadICS( "test2", dip::RangeArray{}, {}, "fast" );
   DOCTEST_CHECK( dip::testing::CompareImages( image, result, dip::Option::CompareImagesMode::FULL ));

   // Test writing a 64-bit integer image
   image = dip::Image( { 32, 24 }, 1, dip::DT_SINT64 );
   image.Fill( 1234567890ll );
   image.At( 0 ) = 0;
   image.At( 1 ) = 9876543210ll;
   image.At( 10 ) = 0;
   DOCTEST_REQUIRE( image.DataType() == dip::DT_SINT64 );
   dip::ImageWriteICS( image, "test3.ics" );
   result = dip::ImageReadICS( "test3.ics" );
   DOCTEST_CHECK( result.DataType() == dip::DT_SINT64 );
   DOCTEST_CHECK( result.At( 0 ).As< dip::sint64 >() == 0 );
   DOCTEST_CHECK( result.At( 1 ).As< dip::sint64 >() == 9876543210ll );
   DOCTEST_CHECK( result.At( 2 ).As< dip::sint64 >() == 1234567890ll );
   DOCTEST_CHECK( result.At( 9 ).As< dip::sint64 >() == 1234567890ll );
   DOCTEST_CHECK( result.At( 10 ).As< dip::sint64 >() == 0 );
   DOCTEST_CHECK( result.At( 11 ).As< dip::sint64 >() == 1234567890ll );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST

#else // DIP_CONFIG_HAS_ICS

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

constexpr char const* NOT_AVAILABLE = "DIPlib was compiled without ICS support.";

FileInformation ImageReadICS(
      Image& /*out*/,
      String const& /*filename*/,
      RangeArray const& /*roi*/,
      Range const& /*channels*/,
      String const& /*mode*/
) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadICS(
      Image& /*image*/,
      String const& /*filename*/,
      UnsignedArray const& /*origin*/,
      UnsignedArray const& /*sizes*/,
      UnsignedArray const& /*spacing*/,
      Range const& /*channels*/,
      String const& /*mode*/
) {
   DIP_THROW( NOT_AVAILABLE );
}

FileInformation ImageReadICSInfo( String const& /*filename*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

bool ImageIsICS( String const& /*filename*/ ) {
   DIP_THROW( NOT_AVAILABLE );
}

void ImageWriteICS(
      Image const& /*image*/,
      String const& /*filename*/,
      StringArray const& /*history*/,
      dip::uint /*significantBits*/,
      StringSet const& /*options*/
) {
   DIP_THROW( NOT_AVAILABLE );
}

}

#endif // DIP_CONFIG_HAS_ICS
