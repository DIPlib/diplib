/*
 * DIPlib 3.0
 * This file contains definitions for ICS reading and writing
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

#ifdef DIP__HAS_ICS

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/generic_iterators.h"
#include "diplib/library/copy_buffer.h"

#include "libics.h"

// TODO: Use IcsGetErrorText() to get more informative error messages for the user.
// TODO: Reorder dimensions when reading, to match standard x,y,z,t order.
// TODO: When reading, check to see if image's strides match those in the file.
// TODO: Option "fast" to reorder dimensions when reading, to read without strides.
// TODO: Option "fast" to reorder dimensions when writing, to write without strides.

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
      char const* order;
      DIP_THROW_IF( IcsGetOrderF( ics, static_cast< int >( tensorDim ), &order, 0 ) != IcsErr_Ok,
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
   FileInformation fileInformation;
   dip::uint tensorDim;
};

GetICSInfoData GetICSInfo( IcsFile& icsFile ) {
   GetICSInfoData data;

   data.fileInformation.name = static_cast< ICS* >( icsFile )->filename;
   data.fileInformation.fileType = "ICS";
   data.fileInformation.numberOfImages = 1;

   // get layout of image data
   Ics_DataType dt;
   int ndims_;
   size_t icsSizes[ICS_MAXDIM];
   DIP_THROW_IF( IcsGetLayout( icsFile, &dt, &ndims_, icsSizes ) != IcsErr_Ok, "Couldn't read ICS file (IcsGetLayout failed)" );
   dip::uint nDims = static_cast< dip::uint >( ndims_ );
   size_t significantBits;
   DIP_THROW_IF( IcsGetSignificantBits( icsFile, &significantBits ) != IcsErr_Ok, "Couldn't read ICS file (IcsGetSignificantBits failed)" );
   data.fileInformation.significantBits = significantBits;
   data.fileInformation.sizes.resize( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      data.fileInformation.sizes[ ii ] = icsSizes[ ii ];
   }

   // get pixel size
   data.fileInformation.pixelSize.Resize( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      double scale;
      char const* units;
      DIP_THROW_IF( IcsGetPositionF( icsFile, static_cast< int >( ii ), nullptr, &scale, &units ) != IcsErr_Ok,
                    "Couldn't read ICS file (IcsGetPosition failed)" );
      try {
         Units u( units );
         data.fileInformation.pixelSize[ ii ] = { scale, u };
      } catch( ... ) {
         // `Units` failed to parse the string
         data.fileInformation.pixelSize[ ii ] = { scale };
      }
   }

   // is there a color/tensor dimension?
   data.tensorDim = FindTensorDimension( icsFile, data.fileInformation.sizes, data.fileInformation.colorSpace );
   data.fileInformation.tensorElements = 1;
   if( data.tensorDim != nDims ) {
      // color or tensor dimension found
      data.fileInformation.tensorElements = data.fileInformation.sizes[ data.tensorDim ];
      data.fileInformation.sizes.erase( data.tensorDim );
      data.fileInformation.pixelSize.EraseDimension( data.tensorDim );
   }

   // convert ICS data type to DIPlib data type
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
      case Ics_sint8:
         data.fileInformation.dataType = DT_SINT8;
         break;
      case Ics_sint16:
         data.fileInformation.dataType = DT_SINT16;
         break;
      case Ics_sint32:
         data.fileInformation.dataType = DT_SINT32;
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
         DIP_THROW( "Unknown ICS data type" );
   }

   // History tags
   int history_lines;
   DIP_THROW_IF( IcsGetNumHistoryStrings( icsFile, &history_lines ) != IcsErr_Ok, "Couldn't read ICS metadata (IcsGetNumHistoryStrings failed)" );
   data.fileInformation.history.resize( static_cast< dip::uint >( history_lines ));
   if (history_lines>0) {
      Ics_HistoryIterator it;
      DIP_THROW_IF( IcsNewHistoryIterator( icsFile, &it, 0 ) != IcsErr_Ok, "Couldn't read ICS metadata (IcsNewHistoryIterator failed)");
      char const* hist;
      for( dip::uint ii = 0; ii < static_cast< dip::uint >( history_lines ); ++ii ) {
         DIP_THROW_IF( IcsGetHistoryStringIF( icsFile, &it, &hist ) != IcsErr_Ok, "Couldn't read ICS metadata (IcsGetHistoryStringI failed)");
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
      RangeArray roi,
      Range channels
) {
   // open the ICS file
   IcsFile icsFile( filename, "r" );

   // get file information
   GetICSInfoData data;
   DIP_STACK_TRACE_THIS( data = GetICSInfo( icsFile ));
   UnsignedArray sizes = data.fileInformation.sizes;
   dip::uint nDims = sizes.size();

   // check & fix ROI information
   UnsignedArray outSizes( nDims );
   dip::uint outTensor;
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
      channels.Fix( data.fileInformation.tensorElements );
      if( channels.start > channels.stop ) {
         std::swap( channels.start, channels.stop );
         // We don't read the tensor dimension in reverse order
      }
      outTensor = channels.Size();
   DIP_END_STACK_TRACE

   // forge the image
   out.ReForge( outSizes, outTensor, data.fileInformation.dataType );
   if( outTensor == data.fileInformation.tensorElements ) {
      out.SetColorSpace( data.fileInformation.colorSpace );
   }
   out.SetPixelSize( data.fileInformation.pixelSize );

   // get tensor shape if necessary
   if(( outTensor > 1 ) && ( outTensor == data.fileInformation.tensorElements )) {
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
                     } catch ( Error const& ) {
                        // Let this error slip, we don't really care
                     }
                  }
               }
            }
         }
      }
   }

   // make a quick copy and place the tensor dimension back where it was
   Image outRef = out.QuickCopy();
   if( data.fileInformation.tensorElements > 1 ) {
      outRef.TensorToSpatial( data.tensorDim );
      roi.insert( data.tensorDim, channels );
      sizes.insert( data.tensorDim, outTensor );
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
   dip::uint sizeOf = data.fileInformation.dataType.SizeOf();
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
      detail::CopyBuffer( buffer.data(), data.fileInformation.dataType, static_cast< dip::sint >( roi[ 0 ].step ), 1,
                          it.Pointer(), outRef.DataType(), outRef.Stride( 0 ), 1,
                          outRef.Size( 0 ), 1 );
   } while( ++it );

   // we're done
   icsFile.Close();
   return data.fileInformation;
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
   GetICSInfoData data;
   DIP_STACK_TRACE_THIS( data = GetICSInfo( icsFile ));

   // done
   icsFile.Close();
   return data.fileInformation;
}

bool ImageIsICS( String const& filename ) {
   return IcsVersion(filename.c_str(), 1) != 0;
}

void ImageWriteICS(
      Image const& c_image,
      String const& filename,
      StringArray const& history,
      dip::uint significantBits,
      StringSet const& options
) {
   // parse options
   bool oldStyle = false; // true if v1
   bool compress = true;
   for( auto& option : options ) {
      if( option == "v1" ) {
         oldStyle = true;
      } else if( option == "v2" ) {
         oldStyle = false;
      } else if( option == "uncompressed" ) {
         compress = false;
      } else if( option == "gzip" ) {
         compress = true;
      } else {
         DIP_THROW_INVALID_FLAG( option );
      }
   }

   // open the ICS file
   IcsFile icsFile( filename, oldStyle ? "w1" : "w2" );

   // set info on image
   Ics_DataType dt;
   dip::uint maxSignificantBits;
   switch( c_image.DataType()) {
      case DT_BIN:      dt = Ics_uint8;     maxSignificantBits = 1;  break;
      case DT_UINT8:    dt = Ics_uint8;     maxSignificantBits = 8;  break;
      case DT_UINT16:   dt = Ics_uint16;    maxSignificantBits = 16; break;
      case DT_UINT32:   dt = Ics_uint32;    maxSignificantBits = 32; break;
      case DT_SINT8:    dt = Ics_sint8;     maxSignificantBits = 8;  break;
      case DT_SINT16:   dt = Ics_sint16;    maxSignificantBits = 16; break;
      case DT_SINT32:   dt = Ics_sint32;    maxSignificantBits = 32; break;
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
   Image image = c_image.QuickCopy();
   bool isTensor = false;
   if( image.TensorElements() > 1 ) {
      isTensor = true;
      image.TensorToSpatial(); // last dimension
   }
   int nDims = static_cast< int >( image.Dimensionality() );
   DIP_THROW_IF( IcsSetLayout( icsFile, dt, nDims, image.Sizes().data() ) != IcsErr_Ok,
                 "Couldn't write to ICS file (IcsSetLayout failed)" );
   DIP_THROW_IF( IcsSetSignificantBits( icsFile, significantBits ) != IcsErr_Ok, "Couldn't write to ICS file (IcsSetSignificantBits failed)" );
   if( c_image.IsColor() ) {
      DIP_THROW_IF( IcsSetOrder( icsFile, nDims - 1, c_image.ColorSpace().c_str(), 0 ) != IcsErr_Ok,
                    "Couldn't write to ICS file (IcsSetOrder failed)" );
   } else if( isTensor ) {
      DIP_THROW_IF( IcsSetOrder( icsFile, nDims - 1, "tensor", 0 ) != IcsErr_Ok,
                    "Couldn't write to ICS file (IcsSetOrder failed)" );
   }
   if( c_image.HasPixelSize() ) {
      if( isTensor ) { nDims--; }
      for( int ii = 0; ii < nDims; ii++ ) {
         auto pixelSize = c_image.PixelSize( static_cast< dip::uint >( ii ));
         DIP_THROW_IF( IcsSetPosition( icsFile, ii, 0.0, pixelSize.magnitude, pixelSize.units.String().c_str() ) != IcsErr_Ok,
                       "Couldn't write to ICS file (IcsSetPosition failed)" );
      }
      if( isTensor ) {
         DIP_THROW_IF( IcsSetPosition( icsFile, nDims, 0.0, 1.0, nullptr ) != IcsErr_Ok,
                       "Couldn't write to ICS file (IcsSetPosition failed)" );
      }
   }
   if( isTensor ) {
      String tensorShape = c_image.Tensor().TensorShapeAsString() + "\t" +
                           std::to_string( c_image.Tensor().Rows() ) + "\t" +
                           std::to_string( c_image.Tensor().Columns() );
      DIP_THROW_IF( IcsAddHistory( icsFile, "tensor", tensorShape.c_str() ) != IcsErr_Ok,
                    "Couldn't write to ICS file (IcsAddHistory() failed)" );
   }

   // set type of compression
   DIP_THROW_IF( IcsSetCompression( icsFile, compress ? IcsCompr_gzip : IcsCompr_uncompressed, 9 ) != IcsErr_Ok,
                 "Couldn't write to ICS file (IcsSetCompression failed)" );

   // set the image data
   if( image.HasNormalStrides() ) {
      DIP_THROW_IF( IcsSetData( icsFile, image.Origin(), image.NumberOfPixels() * image.DataType().SizeOf() ) != IcsErr_Ok,
                    "Couldn't write to ICS file (IcsSetData failed)" );
   } else {
      DIP_THROW_IF( IcsSetDataWithStrides( icsFile, image.Origin(), 0, image.Strides().data(),
                                           static_cast< int >( image.Dimensionality() )) != IcsErr_Ok,
                    "Couldn't write to ICS file (IcsSetDataWithStrides failed)" );
   }

   // tag the data
   DIP_THROW_IF( IcsAddHistory( icsFile, "software", "DIPlib" ) != IcsErr_Ok,
                 "Couldn't write to ICS file (IcsAddHistory() failed)" );

   // write history lines
   for( auto const& line : history ) {
      auto error = IcsAddHistory( icsFile, 0, line.c_str() );
      if(( error == IcsErr_LineOverflow ) || // history line is too long
         ( error == IcsErr_IllParameter )) { // history line contains illegal characters
         // Ignore these errors, the history line will not be written.
      }
      DIP_THROW_IF( error != IcsErr_Ok, "Couldn't write to ICS file (IcsAddHistory() failed)" );
   }

   // write everything to file by closing it
   icsFile.Close();
}

} // namespace dip

#else // DIP__HAS_ICS

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

FileInformation ImageReadICS( Image&, String const&, RangeArray ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

FileInformation ImageReadICS( Image&, String const&, UnsignedArray const&, UnsignedArray const&, UnsignedArray const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

FileInformation ImageReadICSInfo( String const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

bool ImageIsICS( String const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

void ImageWriteICS( Image const&, String const&, StringArray const&, dip::uint, StringSet const& ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

}

#endif // DIP__HAS_ICS
