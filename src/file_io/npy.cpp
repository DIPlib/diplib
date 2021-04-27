/*
 * DIPlib 3.0
 * This file contains definitions for NumPy NPY file reading and writing
 *
 * (c)2021, Cris Luengo.
 * Based on cnpy: (c)2011  Carl Rogers (MIT License)
 * Based on libnpy: (c)2017 Leon Merten Lohse (MIT License)
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

// See https://numpy.org/devdocs/reference/generated/numpy.lib.format.html for format specs.
// See https://github.com/rogersce/cnpy for cnpy
// See https://github.com/llohse/libnpy/ for libnpy

#include <regex>
#include <fstream>

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/generic_iterators.h"

namespace dip {

namespace {

constexpr dip::uint magicStringLength = 8;
constexpr char const* magicString = "\x93NUMPY\x01\x00"; // version number is hard-coded here.

void WriteMagic( std::ostream& ostream ) {
   ostream.write( magicString, magicStringLength );
}

bool ReadMagic( std::istream& istream ) {
   char buf[ magicStringLength ];
   istream.read( buf, magicStringLength );
   if( !istream ) {
      return false;
   }
   if( std::memcmp( buf, magicString, magicStringLength ) != 0 ) {
      return false;
   }
   return true;
}

constexpr char littleEndianChar = '<';
constexpr char bigEndianChar = '>';
constexpr char noEndianChar = '|';

bool LittleEndianTest() {
   int x = 1;
   return reinterpret_cast< char* >( &x )[ 0 ] == 1;
}

char SystemEndianChar() {
   return LittleEndianTest() ? littleEndianChar : bigEndianChar;
}

char TypeChar( DataType dt ) {
   switch( dt ) {
      case DT_BIN:
         return 'b';
      case DT_UINT8:
      case DT_UINT16:
      case DT_UINT32:
      case DT_UINT64:
         return 'u';
      case DT_SINT8:
      case DT_SINT16:
      case DT_SINT32:
      case DT_SINT64:
         return 'i';
      case DT_SFLOAT:
      case DT_DFLOAT:
         return 'f';
      case DT_SCOMPLEX:
      case DT_DCOMPLEX:
         return 'c';
   }
   DIP_THROW( "Unknown data type" ); // This should never happen, but GCC complains.
}

void ReverseArray( UnsignedArray& array ) {
   dip::uint ndim = array.size();
   for( dip::uint ii = 0; ii < ndim / 2; ++ii ) {
      std::swap( array[ ii ], array[ ndim - ii - 1 ] );
   }
}

std::string CreateHeaderDict( DataType dataType, UnsignedArray const& sizes, bool fortranOrder ) {
   std::ostringstream out;
   out << "{'descr': '" << SystemEndianChar();
   out << TypeChar( dataType );
   out << dataType.SizeOf();
   out << "', 'fortran_order': " << ( fortranOrder ? "True" : "False" ) << ", 'shape': (";
   for( dip::uint s : sizes ) {
      out << s << ", ";
   }
   out << "), }\n";
   return out.str();
}

void WriteHeader( std::ostream& ostream, DataType dataType, UnsignedArray const& sizes, bool fortranOrder ) {
   WriteMagic( ostream );
   std::string headerDict = CreateHeaderDict( dataType, sizes, fortranOrder );
   dip::uint length = magicStringLength + 2 + headerDict.length();
   dip::uint padding = 64 - length % 64;
   headerDict += std::string( padding, ' ' );
   length = headerDict.length();
   ostream.put( static_cast< char >( length & 0xffu ));
   ostream.put( static_cast< char >(( length >> 8u ) & 0xffu ));
   ostream.write( headerDict.data(), static_cast< dip::sint >( length ));
}

void ReadHeader( std::istream& istream, DataType& dataType, UnsignedArray& sizes, bool& fortranOrder, bool& swapEndianness ) {
   DIP_THROW_IF( !ReadMagic( istream ), "File is not NPY version 1.0" );
   char buf[ 2 ];
   istream.read( buf, 2 );
   DIP_THROW_IF( !istream, "Could not read NPY file header" );
   dip::uint length = static_cast< dip::uint >( buf[ 0 ] ) + ( static_cast< dip::uint >( buf[ 1 ] ) << 8u );
   std::string headerDict( length, '\0' );
   istream.read( &headerDict[ 0 ], static_cast< dip::sint >( length ));
   //std::cout << "Header length: " << length << '\n';
   //std::cout << "Header contents: >>" << headerDict << "<<\n";
   DIP_THROW_IF( !istream, "Could not read NPY file header" );

   // 'fortran_order'
   std::smatch res;
   std::regex regex_fortran_order( "'fortran_order': *(True|False)" );
   std::regex_search( headerDict, res, regex_fortran_order );
   DIP_THROW_IF ( res.size() != 2, "Failed to parse NYP header keyword 'fortran_order'" );
   fortranOrder = res.str( 1 ) == "True";

   // 'shape'
   sizes.clear();
   std::regex regex_shape( "'shape': *\\(([^)]*)\\)" );
   std::regex_search( headerDict, res, regex_shape );
   DIP_THROW_IF ( res.size() != 2, "Failed to parse NYP header keyword 'shape'" );
   //std::cout << "shape: " << res.str() << '\n' << "     : ";
   std::string shapeStr = res.str( 1 );
   std::regex regex_num( "[0-9]+" );
   for( auto it = std::sregex_iterator( shapeStr.begin(), shapeStr.end(), regex_num ); it != std::sregex_iterator{}; ++it ) {
      //std::cout << it->str() << ", ";
      sizes.push_back( std::stoul( it->str()));
   }
   ReverseArray( sizes );
   //std::cout << '\n';

   // 'descr'
   std::regex regex_descr( "'descr': *'([^']+)'" );
   std::regex_search( headerDict, res, regex_descr );
   DIP_THROW_IF ( res.size() != 2, "Failed to parse NYP header keyword 'descr'" );
   //std::cout << "descr: " << res.str() << '\n';
   // ABxxxx
   //   A is the endianness char, one of littleEndianChar, bigEndianChar or noEndianChar
   //   B is the data type char
   //   xxx is the number of bytes for the data type
   std::string descr = res.str( 1 );
   swapEndianness = !(( descr[ 0 ] == SystemEndianChar() ) || ( descr[ 0 ] == noEndianChar )); // if unknown endianness, just read in as-is
   dip::uint bytes = std::stoul( descr.substr( 2 ));
   if( bytes == 1 ) {
      swapEndianness = false; // for one-byte numbers we don't need to worry about the byte order.
   }
   switch( descr[ 1 ] ) {
      case 'b':
         dataType = DT_BIN;
         DIP_THROW_IF( bytes != 1, "Failed to parse NYP header keyword 'descr': unacceptable bit depth" );
         break;
      case 'u':
         switch( bytes ) {
            case 1:
               dataType = DT_UINT8;
               break;
            case 2:
               dataType = DT_UINT16;
               break;
            case 4:
               dataType = DT_UINT32;
               break;
            case 8:
               dataType = DT_UINT64;
               break;
            default:
               DIP_THROW( "Failed to parse NYP header keyword 'descr': unacceptable bit depth" );
         }
         break;
      case 'i':
         switch( bytes ) {
            case 1:
               dataType = DT_SINT8;
               break;
            case 2:
               dataType = DT_SINT16;
               break;
            case 4:
               dataType = DT_SINT32;
               break;
            case 8:
               dataType = DT_SINT64;
               break;
            default:
               DIP_THROW( "Failed to parse NYP header keyword 'descr': unacceptable bit depth" );
         }
         break;
      case 'f':
         switch( bytes ) {
            case 4:
               dataType = DT_SFLOAT;
               break;
            case 8:
               dataType = DT_DFLOAT;
               break;
            default:
               DIP_THROW( "Failed to parse NYP header keyword 'descr': unacceptable bit depth" );
         }
         break;
      case 'c':
         switch( bytes ) {
            case 8:
               dataType = DT_SCOMPLEX;
               break;
            case 16:
               dataType = DT_DCOMPLEX;
               break;
            default:
               DIP_THROW( "Failed to parse NYP header keyword 'descr': unacceptable bit depth" );
         }
         break;
      default:
         DIP_THROW( "Failed to parse NYP header keyword 'descr': unrecognized type character" );
   }
}

IntegerArray MakeFortranOrderStrides( UnsignedArray const& sizes) {
   dip::uint nDims = sizes.size();
   IntegerArray strides( nDims );
   dip::sint stride = strides.back() = 1;
   for( dip::uint ii = nDims - 1; ii > 0; --ii ) {
      stride *= static_cast< dip::sint >( sizes[ ii ] );
      strides[ ii - 1 ] = stride;
   }
   return strides;
}

std::ifstream OpenNPYForReading( String filename, FileInformation& fileInformation, bool& fortranOrder, bool& swapEndianness ) {
   fileInformation.name = std::move( filename );
   std::ifstream istream( fileInformation.name, std::ifstream::binary );
   if( !istream ) {
      if( !FileHasExtension( fileInformation.name )) {
         fileInformation.name = FileAddExtension( fileInformation.name, "npy" );
         istream.open( fileInformation.name, std::ifstream::binary );
      }
      if( !istream ) {
         DIP_THROW_RUNTIME( "Could not open the specified NPY file" );
      }
   }
   ReadHeader( istream, fileInformation.dataType, fileInformation.sizes, fortranOrder, swapEndianness );
   fileInformation.fileType = "NYP";
   fileInformation.significantBits = fileInformation.dataType.SizeOf() * 8;
   fileInformation.tensorElements = 1;
   fileInformation.numberOfImages = 1;
   return istream;
}

} // namespace

FileInformation ImageReadNPY(
      Image& out,
      String const& filename
) {
   FileInformation fileInformation;
   bool fortranOrder = false;
   bool swapEndianness = false;
   std::ifstream istream;
   DIP_STACK_TRACE_THIS( istream = OpenNPYForReading( filename, fileInformation, fortranOrder, swapEndianness ));
   DIP_STACK_TRACE_THIS( out.ReForge( fileInformation.sizes, 1, fileInformation.dataType, Option::AcceptDataTypeChange::DONT_ALLOW ));
   dip::uint nDims = fileInformation.sizes.size();
   bool matchingStrides = true;
   if( nDims > 0 ) {
      if( fortranOrder ) {
         // In Fortran order, the y-axis changes faster than the x-axis. We make it so that it is the last dimension that has a stride of 1
         IntegerArray strides = MakeFortranOrderStrides( fileInformation.sizes );
         matchingStrides = strides == out.Strides();
         if( !matchingStrides && !out.IsProtected() && out.HasContiguousData() ) {
            // We can make the strides match because the data is contiguous and the image was not protected (which
            // would indicate the caller didn't want us to mess with it).
            out.SetStridesUnsafe( std::move( strides ));
            matchingStrides = true;
         }
      } else { // "C" order
         // In C order, we have normal strides
         matchingStrides = out.HasNormalStrides();
      }
   }
   if( matchingStrides ) {
      // Read data in one go
      istream.read( static_cast< char* >( out.Origin() ), static_cast< dip::sint >( out.NumberOfPixels() * out.DataType().SizeOf() ));
   } else {
      // Read data sample per sample, and write in the right locations in the image
      auto sampleSize = static_cast< dip::sint >( out.DataType().SizeOf() );
      Image tmp = out.QuickCopy();
      if( fortranOrder ) {
         UnsignedArray order( tmp.Dimensionality() );
         std::iota( order.rbegin(), order.rend(), 0 );
         tmp.PermuteDimensions( order );
      }
      GenericImageIterator< dip::uint8 > it( tmp );
      do {
         istream.read( static_cast< char* >( it.Pointer() ), sampleSize );
      } while( ++it );
   }
   DIP_THROW_IF( !istream, "Error reading pixel data from NPY file" );
   if( swapEndianness ) {
      out.SwapBytesInSample();
   }
   return fileInformation;
}

FileInformation ImageReadNPYInfo( String const& filename ) {
   FileInformation fileInformation;
   bool fortranOrder = false;
   bool swapEndianness = false;
   DIP_STACK_TRACE_THIS( OpenNPYForReading( filename, fileInformation, fortranOrder, swapEndianness ));
   return fileInformation;
}

bool ImageIsNPY( String const& filename ) {
   try {
      FileInformation fileInformation;
      bool fortranOrder = false;
      bool swapEndianness = false;
      OpenNPYForReading( filename, fileInformation, fortranOrder, swapEndianness );
   } catch( ... ) {
      return false;
   }
   return true;
}

void ImageWriteNPY(
      Image const& image,
      String const& filename
) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !image.IsScalar(), E::IMAGE_NOT_SCALAR );

   std::ofstream ostream;
   if( FileHasExtension( filename )) {
      ostream.open( filename, std::ofstream::binary );
   } else {
      ostream.open( FileAddExtension( filename, "npy" ), std::ofstream::binary );
   }
   if( !ostream ) {
      DIP_THROW_RUNTIME( "Could not open specified NPY file for writing" );
   }
   bool fortranOrder = false;
   bool matchingStrides = true;
   if( !image.HasNormalStrides() ) {
      if( image.Strides() == MakeFortranOrderStrides( image.Sizes() )) {
         fortranOrder = true;
      } else {
         matchingStrides = false;
      }
   }
   UnsignedArray sizes = image.Sizes();
   ReverseArray( sizes );
   DIP_STACK_TRACE_THIS( WriteHeader( ostream, image.DataType(), sizes, fortranOrder ));
   if( matchingStrides ) {
      // Write data in one go
      ostream.write( static_cast< char* >( image.Origin() ), static_cast< dip::sint >( image.NumberOfPixels() * image.DataType().SizeOf() ));
   } else {
      // Write data sample per sample
      auto sampleSize = static_cast< dip::sint >( image.DataType().SizeOf() );
      GenericImageIterator< dip::uint8 > it( image );
      do {
         ostream.write( static_cast< char* >( it.Pointer() ), sampleSize );
      } while( ++it );
   }
   DIP_THROW_IF( !ostream, "Error writing pixel data to NPY file" );
}

} // namespace dip

// NOTE! This is tested in /pydip/test/npy_test.md
