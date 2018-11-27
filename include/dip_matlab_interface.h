/*
 * DIPlib 3.0
 * This file contains functionality for the MATLAB interface.
 *
 * (c)2015-2018, Cris Luengo.
 * Based on original DIPlib/MATLAB interface code: (c)1999-2014, Delft University of Technology.
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


#ifndef DIP_MATLAB_INTERFACE_H
#define DIP_MATLAB_INTERFACE_H

#include <map>
#include <utility>
#include <streambuf>
#include <iostream>
#include <cstring>
#ifdef DIP__ENABLE_UNICODE
#include <codecvt>
#include <locale>
#endif

#include <mex.h>

// If compiling MEX-files with the -R2018a flag, the code in this file will not work as intended.
#if MX_HAS_INTERLEAVED_COMPLEX
#error "This DIPlib-MATLAB interface does not support compiling MEX-files with the -R2018a flag"
#endif

// MSVC 2015-2017 has a problem linking std::codecvt_utf8_utf16< char16_t >. Here's a workaround.
// MSVC version numbers from https://sourceforge.net/p/predef/wiki/Compilers/#microsoft-visual-c
#if( _MSC_VER >= 1900 ) && ( _MSC_VER < 1920 )
using char16_type = int16_t;
#else
using char16_type = char16_t;
#endif
static_assert( sizeof( char16_type ) == sizeof( mxChar ), "MATLAB's mxChar is not 16 bits." );

// Undocumented functions in libmx
// See: http://www.advanpix.com/2013/07/19/undocumented-mex-api/
// These functions allow us to get and set object properties without making deep copies, as `mxGetProperty` and
// `mxSetProperty` do. For large images, making a useless copy is too expensive.
extern mxArray* mxGetPropertyShared( mxArray const* pa, mwIndex index, char const* propname );
extern void mxSetPropertyShared( mxArray* pa, mwIndex index, char const* propname, mxArray const* value );
// This is a function that makes a shallow copy of an array (i.e. a new array header pointing at the same data).
extern "C" {
   extern mxArray* mxCreateSharedDataCopy( const mxArray* pr );
}

#include "diplib.h"
#include "diplib/file_io.h"      // Definition of dip::FileInformation
#include "diplib/distribution.h" // Definition of dip::Distribution
#include "diplib/histogram.h"    // Definition of dip::Histogram::Configuration

/// \file
/// \brief This file should be included in each MEX-file. It defines the `#dml` namespace.


/// \brief The `%dml` namespace contains the interface between *MATLAB* and *DIPlib*.
///
/// The functions and classes defined in this namespace are meant to be used in *MATLAB* MEX-files.
namespace dml {


/// \defgroup dip_matlab_interface *DIPlib*--*MATLAB* interface
/// \brief Functions to convert image data, function parameters and other arrays to and from *MATLAB*.
///
/// \{


// These are the names of the properties we get/set in the dip_image class in MATLAB:
constexpr char const* imageClassName = "dip_image";
constexpr char const* arrayPropertyName = "Array"; // Set/get pixel data
constexpr char const* ndimsPropertyName = "NDims"; // Set/get number of dimensions
constexpr char const* tsizePropertyName = "TensorSize"; // Get tensor size: [rows, cols]
constexpr char const* tshapePropertyName = "TensorShape"; // Get tensor shape enum, set tensor shape enum and size
constexpr char const* pxsizePropertyName = "PixelSize"; // Set/get pixel size array
constexpr char const* colspPropertyName = "ColorSpace"; // Set/get color space name
constexpr dip::uint nPxsizeStructFields = 2;
char const* pxsizeStructFields[ nPxsizeStructFields ] = { "magnitude", "units" };

// Make sure that MATLAB stores logical arrays the same way we store binary images
static_assert( sizeof( mxLogical ) == sizeof( dip::bin ), "mxLogical is not one byte!" );


//
// Get input arguments: convert mxArray to various dip:: types
//


#define DML_MIN_ARGS( n ) DIP_THROW_IF( nrhs < ( n ), "Too few input arguments" )
#define DML_MAX_ARGS( n ) DIP_THROW_IF( nrhs > ( n ), "Too many input arguments" )

#define DML_CATCH catch( dip::ParameterError const& e ) { mexErrMsgIdAndTxt( "DIPlib:ParameterError", e.what() ); } \
                  catch( dip::RunTimeError const& e ) { mexErrMsgIdAndTxt( "DIPlib:RunTimeError", e.what() ); } \
                  catch( dip::AssertionError const& e ) { mexErrMsgIdAndTxt( "DIPlib:AssertionError", e.what() ); } \
                  catch( std::exception const& e ) { mexErrMsgIdAndTxt( "DIPlib:StandardException", e.what() ); }


/// \brief True if array is scalar (has single value)
// We define this function because mxIsScalar is too new.
inline bool IsScalar( mxArray const* mx ) {
   return mxGetNumberOfElements( mx ) == 1;
}

/// \brief True if empty or a one-dimensional array
inline bool IsVector( mxArray const* mx ) {
   return ( mxGetNumberOfDimensions( mx ) == 2 ) && (( mxGetM( mx ) <= 1 ) || ( mxGetN( mx ) <= 1 ));
}

/// \brief Convert an unsigned integer from `mxArray` to `dip::uint` by copy.
inline dip::uint GetUnsigned( mxArray const* mx ) {
   if( IsScalar( mx ) && mxIsDouble( mx ) && !mxIsComplex( mx )) {
      double v = *mxGetPr( mx );
      dip::uint out = static_cast< dip::uint >( v );
      if( static_cast< double >( out ) == v ) {
         return out;
      }
   }
   DIP_THROW( "Unsigned integer value expected" );
}

/// \brief Convert a signed integer from `mxArray` to `dip::sint` by copy.
inline dip::sint GetInteger( mxArray const* mx ) {
   if( IsScalar( mx ) && mxIsDouble( mx ) && !mxIsComplex( mx )) {
      double v = *mxGetPr( mx );
      dip::sint out = static_cast< dip::sint >( v );
      if( static_cast< double >( out ) == v ) {
         return out;
      }
   }
   DIP_THROW( "Integer value expected" );
}

/// \brief Convert a floating-point number from `mxArray` to `dip::dfloat` by copy.
inline dip::dfloat GetFloat( mxArray const* mx ) {
   if( IsScalar( mx ) && mxIsDouble( mx ) && !mxIsComplex( mx )) {
      return *mxGetPr( mx );
   }
   DIP_THROW( "Real floating-point value expected" );
}

/// \brief Convert a complex floating-point number from `mxArray` to `dip::dcomplex` by copy.
inline dip::dcomplex GetComplex( mxArray const* mx ) {
   if( IsScalar( mx ) && mxIsDouble( mx )) {
      double* pr = mxGetPr( mx );
      double* pi = mxGetPi( mx );
      dip::dcomplex out{ 0, 0 };
      if( pr ) { out.real( *pr ); }
      if( pi ) { out.imag( *pi ); }
      return out;
   }
   DIP_THROW( "Complex floating-point value expected" );
}

/// \brief Convert a boolean (logical) array from `mxArray` to `dip::BooleanArray` by copy.
inline dip::BooleanArray GetBooleanArray( mxArray const* mx ) {
   if( IsVector( mx )) {
      if( mxIsLogical( mx )) {
         dip::uint n = mxGetNumberOfElements( mx );
         dip::BooleanArray out( n );
         mxLogical* data = mxGetLogicals( mx );
         for( dip::uint ii = 0; ii < n; ++ii ) {
            out[ ii ] = data[ ii ];
         }
         return out;
      } else if( mxIsDouble( mx ) && !mxIsComplex( mx )) {
         dip::uint n = mxGetNumberOfElements( mx );
         dip::BooleanArray out( n );
         double* data = mxGetPr( mx );
         for( dip::uint ii = 0; ii < n; ++ii ) {
            out[ ii ] = data[ ii ] != 0;
         }
         return out;
      }
   }
   DIP_THROW( "Boolean array expected" );
}

/// \brief Convert an unsigned integer array from `mxArray` to `dip::UnsignedArray` by copy.
inline dip::UnsignedArray GetUnsignedArray( mxArray const* mx ) {
   if( mxIsDouble( mx ) && !mxIsComplex( mx ) && IsVector( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      dip::UnsignedArray out( n );
      double* data = mxGetPr( mx );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         double v = data[ ii ];
         out[ ii ] = static_cast< dip::uint >( v );
         DIP_THROW_IF( static_cast< double >( out[ ii ] ) != v, "Array element not an unsigned integer" );
      }
      return out;
   }
   DIP_THROW( "Unsigned integer array expected" );
}

/// \brief Convert a signed integer array from `mxArray` to `dip::IntegerArray` by copy.
inline dip::IntegerArray GetIntegerArray( mxArray const* mx ) {
   if( mxIsDouble( mx ) && !mxIsComplex( mx ) && IsVector( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      dip::IntegerArray out( n );
      double* data = mxGetPr( mx );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         double v = data[ ii ];
         out[ ii ] = static_cast< dip::sint >( v );
         DIP_THROW_IF( static_cast< double >( out[ ii ] ) != v, "Array element not an integer" );
      }
      return out;
   }
   DIP_THROW( "Integer array expected" );
}

/// \brief Convert a floating-point array from `mxArray` to `dip::FloatArray` by copy.
inline dip::FloatArray GetFloatArray( mxArray const* mx ) {
   if( mxIsDouble( mx ) && !mxIsComplex( mx ) && IsVector( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      dip::FloatArray out( n );
      double* data = mxGetPr( mx );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         out[ ii ] = data[ ii ];
      }
      return out;
   }
   DIP_THROW( "Floating-point array expected" );
}

/// \brief Convert a floating-point array for `mxArray` to `std::vector<dip::dfloat>` by copy.
inline std::vector< dip::dfloat > GetStdVectorOfFloats( mxArray const* mx ) {
   if( mxIsDouble( mx ) && !mxIsComplex( mx ) && IsVector( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      std::vector< dip::dfloat > out( n ); // Identical to dml::GetFloatArray except for this line...
      double* data = mxGetPr( mx );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         out[ ii ] = data[ ii ];
      }
      return out;
   }
   DIP_THROW( "Floating-point array expected" );
}

/// \brief Convert an unsigned integer `mxArray` to a `dip::BooleanArray`, where elements of the input are indices
/// where the output array is set. The output array has `nDims` elements. In MATLAB, dimensions start with 1.
/// If `mx` is empty, all dimensions are to be processed.
inline dip::BooleanArray GetProcessArray( mxArray const* mx, dip::uint nDims ) {
   if( !mxIsEmpty( mx )) {
      dip::IntegerArray in;
      try {
         in = GetIntegerArray( mx );
      } catch( dip::Error& ) {
         DIP_THROW( "Process array must be an integer array" );
      }
      dip::BooleanArray out( nDims, false );
      for( auto ii : in ) {
         DIP_THROW_IF( ( ii <= 0 ) || ( ii > static_cast< dip::sint >( nDims )), "Process array contains index out of range" );
         out[ static_cast< dip::uint >( ii - 1 ) ] = true;
      }
      return out;
   } else {
      return dip::BooleanArray( nDims, true );
   }
}

/// \brief Convert a coordinates array from `mxArray` to `dip::CoordinateArray` by copy.
///
/// A coordinates array is either a cell array with arrays of unsigned integers (all of them
/// the same length), or a matrix with a row per coordinate and a column per dimension.
inline dip::CoordinateArray GetCoordinateArray( mxArray const* mx ) {
   if( mxIsDouble( mx ) && !mxIsComplex( mx )) {
      dip::uint n = mxGetM( mx );
      dip::uint ndims = mxGetN( mx );
      dip::CoordinateArray out( n );
      double* data = mxGetPr( mx );
      for( auto& o : out ) {
         o.resize( ndims );
         for( dip::uint ii = 0; ii < ndims; ++ii ) {
            double v = data[ ii * n ];
            o[ ii ] = static_cast< dip::uint >( v );
            DIP_THROW_IF( static_cast< double >( o[ ii ]) != v, "Coordinate value not an unsigned integer" );
         }
         ++data;
      }
      return out;
   } else if( mxIsCell( mx ) && IsVector( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      dip::CoordinateArray out( n );
      dip::uint ndims = 0;
      for( dip::uint ii = 0; ii < n; ++ii ) {
         mxArray const* elem = mxGetCell( mx, ii );
         if( ii == 0 ) {
            ndims = mxGetNumberOfElements( elem );
         } else {
            DIP_THROW_IF( ndims != mxGetNumberOfElements( elem ), "Coordinates in array must have consistent dimensionalities" );
         }
         try {
            out[ ii ] = GetUnsignedArray( elem );
         } catch( dip::Error& ) {
            DIP_THROW( "Coordinates in array must be unsigned integer arrays" );
         }
      }
      return out;
   }
   DIP_THROW( "Coordinate array expected" );
}

/// \brief Convert a coordinates array from `mxArray` to `dip::FloatCoordinateArray` by copy.
///
/// A coordinates array is either a cell array with arrays of double floats (all of them
/// the same length), or a matrix with a row per coordinate and a column per dimension.
inline dip::FloatCoordinateArray GetFloatCoordinateArray( mxArray const* mx ) {
   if( mxIsDouble( mx ) && !mxIsComplex( mx )) {
      dip::uint n = mxGetM( mx );
      dip::uint ndims = mxGetN( mx );
      dip::FloatCoordinateArray out( n );
      double* data = mxGetPr( mx );
      for( auto& o : out ) {
         o.resize( ndims );
         for( dip::uint ii = 0; ii < ndims; ++ii ) {
            o[ ii ] = data[ ii * n ];
         }
         ++data;
      }
      return out;
   } else if( mxIsCell( mx ) && IsVector( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      dip::FloatCoordinateArray out( n );
      dip::uint ndims = 0;
      for( dip::uint ii = 0; ii < n; ++ii ) {
         mxArray const* elem = mxGetCell( mx, ii );
         if( ii == 0 ) {
            ndims = mxGetNumberOfElements( elem );
         } else {
            DIP_THROW_IF( ndims != mxGetNumberOfElements( elem ), "Coordinates in array must have consistent dimensionalities" );
         }
         try {
            out[ ii ] = GetFloatArray( elem );
         } catch( dip::Error& ) {
            DIP_THROW( "Coordinates in array must be numeric arrays" );
         }
      }
      return out;
   }
   DIP_THROW( "Coordinate array expected" );
}

/// \brief Convert a string from `mxArray` to `dip::String` by copy.
inline dip::String GetString( mxArray const* mx ) {
   if( mxIsChar( mx ) && IsVector( mx )) {
      dip::String out( mxGetNumberOfElements( mx ), '\0' );
      mxGetString( mx, &( out[ 0 ] ), out.size() + 1 ); // Why is out.data() a const* ???
      return out;
   }
   DIP_THROW( "String expected" );
}

/// \brief Convert a string from `mxArray` to a UTF-8 encoded `dip::String` by copy.
inline dip::String GetStringUnicode( mxArray const* mx ) {
#ifdef DIP__ENABLE_UNICODE
   if( mxIsChar( mx ) && IsVector( mx )) {
      // We need to copy the UTF16 string in the mxArray because it is not null-terminated, as wstring_convert expects.
      char16_type const* data = reinterpret_cast< char16_type* >( mxGetChars( mx ));
      dip::uint len = mxGetNumberOfElements( mx );
      std::basic_string< char16_type > u16str( len + 1, '\0' ); // one more char for the null terminator.
      std::copy( data, data + len, u16str.begin() ); // here we don't overwrite the last null.
      dip::String out = std::wstring_convert< std::codecvt_utf8_utf16< char16_type >, char16_type >{}.to_bytes( u16str );
      return out;
   }
   DIP_THROW( "String expected" );
#else
   return GetString( mx );
#endif
}

/// \brief Convert a cell array of strings from `mxArray` to `dip::StringArray` by copy.
inline dip::StringArray GetStringArray( mxArray const* mx ) {
   try {
      if( mxIsCell( mx ) && IsVector( mx )) {
         dip::uint n = mxGetNumberOfElements( mx );
         dip::StringArray out( n );
         for( dip::uint ii = 0; ii < n; ++ii ) {
            out[ ii ] = GetString( mxGetCell( mx, ii ));
         }
         return out;
      } else {
         dip::StringArray out( 1 );
         out[ 0 ] = GetString( mx );
         return out;
      }
   } catch( dip::Error& ) {
      DIP_THROW( "String array expected" );
   }
}

/// \brief Convert a cell array of string from `mxArray` to `dip::StringSet` by copy.
inline dip::StringSet GetStringSet( mxArray const* mx ) {
   try {
      if( mxIsCell( mx ) && IsVector( mx )) {
         dip::uint n = mxGetNumberOfElements( mx );
         dip::StringSet out;
         for( dip::uint ii = 0; ii < n; ++ii ) {
            out.insert( GetString( mxGetCell( mx, ii )));
         }
         return out;
      } else {
         dip::StringSet out;
         out.insert( GetString( mx ));
         return out;
      }
   } catch( dip::Error& ) {
      DIP_THROW( "String set expected" );
   }
}

/// \brief Convert a boolean (logical) from `mxArray` to `bool` by copy. Accepts `"yes"` and `"no"` as well.
inline bool GetBoolean( mxArray const* mx ) {
   if( mxIsChar( mx )) {
      dip::String str = GetString( mx );
      if(( str == "yes" ) || ( str == "y" )) {
         return true;
      }
      if(( str == "no" ) || ( str == "n" )) {
         return false;
      }
   }
   if( IsScalar( mx )) {
      if( mxIsLogical( mx )) {
         return *mxGetLogicals( mx );
      }
      if( mxIsDouble( mx ) && !mxIsComplex( mx )) {
         return *mxGetPr( mx ) != 0;
      }
   }
   DIP_THROW( "Boolean value expected" );
}

/// \brief Convert an integer array from `mxArray` to `dip::Range` by copy.
///
/// A range is an integer array with zero to three elements, ordered the same way as in the
/// constructors for `dip::Range`.
inline dip::Range GetRange( mxArray const* mx ) {
   if( mxIsDouble( mx ) && !mxIsComplex( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      if( n <= 3 ) {
         dip::Range out; // default = { 0, -1, 1 } ( == 1:1:end in MATLAB-speak )
         if( n > 0 ) {
            double* data = mxGetPr( mx );
            double start = data[ 0 ];
            out.start = static_cast< dip::sint >( start );
            DIP_THROW_IF( static_cast< double >( out.start ) != start, "Range start value must be an integer" );
            if( n > 1 ) {
               double stop = data[ 1 ];
               out.stop = static_cast< dip::sint >( stop );
               DIP_THROW_IF( static_cast< double >( out.stop ) != stop, "Range start value must be an integer" );
               if( n > 2 ) {
                  double step = data[ 2 ];
                  out.step = static_cast< dip::uint >( step );
                  DIP_THROW_IF( static_cast< double >( out.step ) != step, "Range step value must be a positive integer" );
               }
            } else {
               out.stop = out.start; // with one number, we start and stop at the same value
            }
         }
         return out;
      }
   }
   DIP_THROW( "Range expected" );
}

/// \brief Convert a cell array of integer array from `mxArray` to `dip::RangeArray` by copy.
inline dip::RangeArray GetRangeArray( mxArray const* mx ) {
   if( mxIsCell( mx ) && IsVector( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      dip::RangeArray out( n );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         out[ ii ] = GetRange( mxGetCell( mx, ii ));
      }
      return out;
   } else {
      try {
         dip::RangeArray out( 1 );
         out[ 0 ] = GetRange( mx );
         return out;
      } catch( dip::Error& ) {
         DIP_THROW( "Range array expected" );
      }
   }
}

/// \brief Convert a numeric array from `mxArray` to `dip::Image::Pixel` by copy.
inline dip::Image::Pixel GetPixel( mxArray const* mx ) {
   DIP_THROW_IF( !mxIsDouble( mx ) || !IsVector( mx ), "Pixel value expected" );
   dip::uint n = mxGetNumberOfElements( mx );
   if( mxIsComplex( mx )) {
      dip::Image::Pixel out( dip::DT_DCOMPLEX, n );
      double* pr = mxGetPr( mx );
      double* pi = mxGetPi( mx );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         out[ ii ] = dip::dcomplex( pr[ ii ], pi[ ii ] );
      }
      return out;
   } else {
      dip::Image::Pixel out( dip::DT_DFLOAT, n );
      double* pr = mxGetPr( mx );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         out[ ii ] = pr[ ii ];
      }
      return out;
   }
}

/// \brief Reads a histogram Configuration struct from a cell `mxArray` with key-value pairs.
inline dip::Histogram::Configuration GetHistogramConfiguration( mxArray const* mx ) {
   dip::Histogram::Configuration out;
   out.lowerIsPercentile = true;
   out.upperIsPercentile = true;
   DIP_THROW_IF( !mxIsCell( mx ), "SPECS parameter must be a cell array" );
   dip::uint N = mxGetNumberOfElements( mx );
   dip::uint ii = 0;
   bool hasLower = false;
   bool hasUpper = false;
   bool hasNBins = false;
   bool hasBinSize = false;
   while( ii < N ) {
      dip::String key = dml::GetString( mxGetCell( mx, ii ));
      ++ii;
      if( key == "lower" ) {
         DIP_THROW_IF( ii >= N, "SPECS key requires a value pair" );
         out.lowerBound = dml::GetFloat( mxGetCell( mx, ii ));
         hasLower = true;
         ++ii;
      } else if( key == "upper" ) {
         DIP_THROW_IF( ii >= N, "SPECS key requires a value pair" );
         out.upperBound = dml::GetFloat( mxGetCell( mx, ii ));
         hasUpper = true;
         ++ii;
      } else if( key == "bins" ) {
         DIP_THROW_IF( ii >= N, "SPECS key requires a value pair" );
         out.nBins = dml::GetUnsigned( mxGetCell( mx, ii ));
         hasNBins = true;
         ++ii;
      } else if( key == "binsize" ) {
         DIP_THROW_IF( ii >= N, "SPECS key requires a value pair" );
         out.binSize = dml::GetFloat( mxGetCell( mx, ii ));
         hasBinSize = true;
         ++ii;
      } else if( key == "lower_abs" ) {
         out.lowerIsPercentile = false;
      } else if( key == "upper_abs" ) {
         out.upperIsPercentile = false;
      } else if( key == "exclude_out_of_bounds_values" ) {
         out.excludeOutOfBoundValues = true;
      } else {
         DIP_THROW( "SPECS key not recognized" );
      }
   }
   N = 0;
   if( hasLower ) { ++N; };
   if( hasUpper ) { ++N; };
   if( hasNBins ) { ++N; };
   if( hasBinSize ) { ++N; };
   DIP_THROW_IF( N != 3, "SPECS requires exactly 3 of the 4 core value-pairs to be given" );
   if( !hasLower ) {
      out.mode = dip::Histogram::Configuration::Mode::COMPUTE_LOWER;
   } else if( !hasUpper ) {
      out.mode = dip::Histogram::Configuration::Mode::COMPUTE_UPPER;
   } else if( !hasNBins ) {
      out.mode = dip::Histogram::Configuration::Mode::COMPUTE_BINS;
   } else if( !hasBinSize ) {
      out.mode = dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE;
   }
   return out;
}


//
// Put output values: convert various dip:: types to mxArray
//


/// \brief Create a two-element mxArray and write the two values in it.
inline mxArray* CreateDouble2Vector( dip::dfloat v0, dip::dfloat v1 ) {
   mxArray* out = mxCreateDoubleMatrix( 1, 2, mxREAL );
   double* p = mxGetPr( out );
   p[ 0 ] = v0;
   p[ 1 ] = v1;
   return out;
}

/// \brief Convert an boolean from `bool` to `mxArray` by copy.
inline mxArray* GetArray( bool in ) {
   return mxCreateLogicalScalar( in );
}

/// \brief Convert an unsigned integer from `dip::uint` to `mxArray` by copy.
inline mxArray* GetArray( dip::uint in ) {
   return mxCreateDoubleScalar( static_cast< double >( in ));
}

/// \brief Convert a signed integer from `dip::sint` to `mxArray` by copy.
inline mxArray* GetArray( dip::sint in ) {
   return mxCreateDoubleScalar( static_cast< double >( in ));
}

/// \brief Convert a floating-point number from `dip::dfloat` to `mxArray` by copy.
inline mxArray* GetArray( dip::dfloat in ) {
   return mxCreateDoubleScalar( in );
}

/// \brief Convert a complex floating-point number from `dip::dcomplex` to `mxArray` by copy.
inline mxArray* GetArray( dip::dcomplex in ) {
   mxArray* mx = mxCreateDoubleMatrix( 1, 1, mxCOMPLEX );
   *( mxGetPr( mx )) = in.real();
   *( mxGetPi( mx )) = in.imag();
   return mx;
}

/// \brief Convert a numeric array from `dip::DimensionalArray< T >` to `mxArray` by copy. Works for
/// `dip::UnsignedArray`, `dip::IntegerArray` and `dip::FloatArray`.
template< typename T, typename = std::enable_if_t< std::is_arithmetic< T >::value >>
inline mxArray* GetArray( dip::DimensionArray< T > const& in ) {
   mxArray* mx = mxCreateDoubleMatrix( 1, in.size(), mxREAL );
   double* data = mxGetPr( mx );
   for( dip::uint ii = 0; ii < in.size(); ++ii ) {
      data[ ii ] = static_cast< double >( in[ ii ] );
   }
   return mx;
}

/// \brief Convert a coordinates array from `mxArray` to `dip::CoordinateArray` by copy.
///
/// The output `mxArray` is a matrix with a row per coordinate and a column per dimension.
inline mxArray* GetArray( dip::CoordinateArray const& in ) {
   dip::uint n = in.size();
   if( n == 0 ) {
      return mxCreateDoubleMatrix( 0, 0, mxREAL );
   }
   dip::uint ndims = in[ 0 ].size();
   mxArray* mx = mxCreateDoubleMatrix( n, ndims, mxREAL );
   double* data = mxGetPr( mx );
   for( auto& v : in ) {
      DIP_ASSERT( v.size() == ndims );
      for( dip::uint ii = 0; ii < ndims; ++ii ) {
         data[ ii * n ] = static_cast< double >( v[ ii ] );
      }
      ++data;
   }
   return mx;
}

/// \brief Convert a string from `dip::String` to `mxArray` by copy.
inline mxArray* GetArray( dip::String const& in ) {
   return mxCreateString( in.c_str() );
}

/// \brief Convert a string array from `dip::StringArray` to `mxArray` by copy.
inline mxArray* GetArray( dip::StringArray const& in ) {
   mxArray* mx = mxCreateCellMatrix( 1, in.size() );
   for( dip::uint ii = 0; ii < in.size(); ++ii ) {
      mxSetCell( mx, ii, GetArray( in[ ii ] ));
   }
   return mx;
}

/// \brief Convert a UTF-8 encoded string from `dip::String` to `mxArray` by copy.
inline mxArray* GetArrayUnicode( dip::String const& in ) {
#ifdef DIP__ENABLE_UNICODE
   auto u16str = std::wstring_convert< std::codecvt_utf8_utf16< char16_type >, char16_type >{}.from_bytes( in );
   dip::uint sz[ 2 ] = { 1, u16str.size() };
   mxArray* out = mxCreateCharArray( 2, sz );
   std::copy( u16str.begin(), u16str.end(), reinterpret_cast< char16_type* >( mxGetChars( out )));
   return out;
#else
   return GetArray( in );
#endif
}

/// \brief Convert a sample from `dip::Image::Sample` to `mxArray` by copy.
inline mxArray* GetArray( dip::Image::Sample const& in ) {
   if( in.DataType().IsBinary() ) { // logical array
      return GetArray( dip::detail::CastSample< bool >( in.DataType(), in.Origin() ));
   } else if( in.DataType().IsComplex() ) { // double complex array
      return GetArray( dip::detail::CastSample< dip::dcomplex >( in.DataType(), in.Origin() ));
   } else { // integer or floating-point : double array
      return GetArray( dip::detail::CastSample< dip::dfloat >( in.DataType(), in.Origin() ));
   }
}

/// \brief Convert a set of samples from `dip::Image::Pixel` to `mxArray` by copy.
inline mxArray* GetArray( dip::Image::Pixel const& in ) {
   mxArray* out;
   if( in.DataType().IsBinary() ) { // logical array
      out = mxCreateLogicalMatrix( 1, in.TensorElements() );
      dip::Image::Pixel map( mxGetLogicals( out ), dip::DT_BIN, in.Tensor(), 1 );
      map = in; // copy samples over
   } else if( in.DataType().IsComplex() ) { // double complex array
      out = mxCreateDoubleMatrix( 1, in.TensorElements(), mxCOMPLEX );
      dip::Image::Pixel mapReal( mxGetPr( out ), dip::DT_DFLOAT, in.Tensor(), 1 );
      dip::Image::Pixel mapImag( mxGetPi( out ), dip::DT_DFLOAT, in.Tensor(), 1 );
      mapReal = in.Real(); // copy samples over
      mapImag = in.Imaginary(); // copy samples over
   } else { // integer or floating-point : double array
      out = mxCreateDoubleMatrix( 1, in.TensorElements(), mxREAL );
      dip::Image::Pixel map( mxGetPr( out ), dip::DT_DFLOAT, in.Tensor(), 1 );
      map = in; // copy samples over
   }
   return out;
}

/// \brief Convert a pixel size object `dip::PixelSize` to `mxArray` by copy.
inline mxArray* GetArray( dip::PixelSize const& pixelSize ) {
   mxArray* pxsz = mxCreateStructMatrix( pixelSize.Size(), 1, nPxsizeStructFields, pxsizeStructFields );
   for( dip::uint ii = 0; ii < pixelSize.Size(); ++ii ) {
      mxSetField( pxsz, ii, pxsizeStructFields[ 0 ], dml::GetArray( pixelSize[ ii ].magnitude ));
      mxSetField( pxsz, ii, pxsizeStructFields[ 1 ], dml::GetArrayUnicode( pixelSize[ ii ].units.StringUnicode() ));
   }
   return pxsz;
}

/// \brief Convert a `dip::FileInformation` structure to `mxArray` by copy.
inline mxArray* GetArray( dip::FileInformation const& fileInformation ) {
   constexpr int nFields = 10;
   char const* fieldNames[ nFields ] = {
         "name",
         "fileType",
         "dataType",
         "significantBits",
         "sizes",
         "tensorElements",
         "colorSpace",
         "pixelSize",
         "numberOfImages",
         "history"
   };
   mxArray* out;
   out = mxCreateStructMatrix( 1, 1, nFields, fieldNames );
   mxSetField( out, 0, fieldNames[ 0 ], dml::GetArray( fileInformation.name ));
   mxSetField( out, 0, fieldNames[ 1 ], dml::GetArray( fileInformation.fileType ));
   mxSetField( out, 0, fieldNames[ 2 ], dml::GetArray( dip::String{ fileInformation.dataType.Name() } ));
   mxSetField( out, 0, fieldNames[ 3 ], dml::GetArray( fileInformation.significantBits ));
   mxSetField( out, 0, fieldNames[ 4 ], dml::GetArray( fileInformation.sizes ));
   mxSetField( out, 0, fieldNames[ 5 ], dml::GetArray( fileInformation.tensorElements ));
   mxSetField( out, 0, fieldNames[ 6 ], dml::GetArray( fileInformation.colorSpace ));
   mxSetField( out, 0, fieldNames[ 7 ], dml::GetArray( fileInformation.pixelSize ));
   mxSetField( out, 0, fieldNames[ 8 ], dml::GetArray( fileInformation.numberOfImages ));
   mxSetField( out, 0, fieldNames[ 9 ], dml::GetArray( fileInformation.history ));
   return out;
}

/// \brief Convert a `dip::Distribution` object to `mxArray` by copy.
inline mxArray* GetArray( dip::Distribution const& in ) {
   dip::uint n = in.Size();
   dip::uint m = in.ValuesPerSample();
   mxArray* mx = mxCreateDoubleMatrix( n, m + 1, mxREAL );
   double* data = mxGetPr( mx );
   for( auto& v : in ) {
      double* ptr = data;
      *ptr = v.X();
      for( dip::uint ii = 0; ii < m; ++ii ) {
         ptr += n;
         *ptr = v.Y( ii );
      }
      ++data;
   }
   return mx;
}

//
// Converting mxArray to dip::Image
//


namespace detail {

// Get the dip::Tensor::Shape value from a string mxArray
inline enum dip::Tensor::Shape GetTensorShape( mxArray* mx ) {
   char str[ 25 ];
   if( mxGetString( mx, str, 25 ) == 0 ) {
      try {
         return dip::Tensor::ShapeFromString( str );
      } catch( dip::Error& ) {
         DIP_THROW( dip::String{ "TensorShape string not recognized: " } + dip::String{ str } );
      }
   }
   DIP_THROW( "TensorShape property returned wrong data!" );
}

// Return `img` or `img` cast to a single-float type if it doesn't underflow or overflow.
// Note `img` is always a single sample, of a double-float type.
inline void MaybeCastScalar( dip::Image& img ) {
   if( img.DataType() == dip::DT_DCOMPLEX ) {
      // Will we underflow or overflow?
      dip::dcomplex imgValue = *static_cast< dip::dcomplex* >( img.Origin() );
      dip::dfloat imgValueR = imgValue.real();
      if( std::isfinite( imgValueR ) && ( std::abs( imgValueR ) > static_cast< dip::dfloat >( std::numeric_limits< dip::sfloat >::max() ))) {
         return;
      }
      if( std::isfinite( imgValueR ) && ( imgValueR != 0.0 ) && ( std::abs( imgValueR ) < static_cast< dip::dfloat >( std::numeric_limits< dip::sfloat >::min() ))) {
         return;
      }
      dip::dfloat imgValueI = imgValue.imag();
      if( std::isfinite( imgValueI ) && ( std::abs( imgValueI ) > static_cast< dip::dfloat >( std::numeric_limits< dip::sfloat >::max() ))) {
         return;
      }
      if( std::isfinite( imgValueI ) && ( imgValueI != 0.0 ) && ( std::abs( imgValueI ) < static_cast< dip::dfloat >( std::numeric_limits< dip::sfloat >::min() ))) {
         return;
      }
      img.Convert( dip::DT_SCOMPLEX );
   } else {
      DIP_ASSERT( img.DataType() == dip::DT_DFLOAT );
      dip::dfloat imgValue = *static_cast< dip::dfloat* >( img.Origin() );
      if( std::isfinite( imgValue ) && ( std::abs( imgValue ) > static_cast< dip::dfloat >( std::numeric_limits< dip::sfloat >::max() ))) {
         return;
      }
      if( std::isfinite( imgValue ) && ( imgValue != 0.0 ) && ( std::abs( imgValue ) < static_cast< dip::dfloat >( std::numeric_limits< dip::sfloat >::min() ))) {
         return;
      }
      img.Convert( dip::DT_SFLOAT );
   }
}

} // namespace detail

/// \brief `dml::GetImage` can optionally create a shared copy of the input `mxArray`, which extends its lifetime.
/// This is useful if the MEX-file needs to keep a reference to the object.
enum class GetImageMode {
      REFERENCE,     ///< Reference the `mxArray` in the `dip::Image` object.
      SHARED_COPY    ///< Make a shared copy of the `mxArray` and take ownership of the copy.
};

/// \brief `dml::GetImage` can optionally turn an input numeric array to a tensor image. If the numeric array
/// is a short vector (up to 5 elements) it will be seen as a 0D tensor image.
enum class ArrayConversionMode {
      STANDARD,        ///< All `mxArray`s are scalar images.
      TENSOR_OPERATOR  ///< Turn the last dimension to a tensor dimension if it is short.
};

/// \brief Passing an `mxArray` to *DIPlib*, keeping ownership of the data.
///
/// This function "converts" an `mxArray` with image data to a dip::Image object.
/// The dip::Image object will point to the data in the `mxArray`, unless
/// the array contains complex numbers. Complex data needs to be copied because
/// *MATLAB* represents it internally as two separate data blocks. In that
/// case, the dip::Image object will own its own data block.
///
/// When calling GetImage with a `prhs` argument in `mexFunction()`, use a const
/// modifier for the output argument. This should prevent accidentally modifying
/// an input array, which is supposed to be illegal in `mexFunction()`:
/// ```cpp
///     dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
/// ```
///
/// An empty `mxArray` produces a non-forged image.
inline dip::Image GetImage(
      mxArray const* mx,
      GetImageMode mode = GetImageMode::REFERENCE,
      ArrayConversionMode conversion = ArrayConversionMode::STANDARD ) {
   // Find image properties
   bool complex = false;
   bool needCopy = false;
   bool maybeCast = false;
   bool lastDimToTensor = false;
   dip::Tensor tensor; // scalar by default
   mxClassID type;
   mxArray const* mxdata;
   dip::uint ndims;
   mwSize const* psizes;
   dip::UnsignedArray sizes;
   dip::PixelSize pixelSize;
   dip::String colorSpace;
   if( mxIsClass( mx, imageClassName )) {
      // Data
      mxdata = mxGetPropertyShared( mx, 0, arrayPropertyName );
      if( mxIsEmpty( mxdata )) {
         return {};
      }
      // Sizes
      dip::uint inNDims = mxGetNumberOfDimensions( mxdata );
      psizes = mxGetDimensions( mxdata );
      ndims = GetUnsigned( mxGetPropertyShared( mx, 0, ndimsPropertyName ));
      sizes.resize( ndims, 1 );
      for( dip::uint ii = 2; ii < inNDims; ++ii ) {
         sizes[ ii - 2 ] = psizes[ ii ];
      }
      // Data Type
      type = mxGetClassID( mxdata );
      complex = psizes[ 0 ] > 1;
      // Tensor size and shape
      tensor.SetVector( psizes[ 1 ] );
      if( !tensor.IsScalar() ) {
         dip::UnsignedArray tsize = GetUnsignedArray( mxGetPropertyShared( mx, 0, tsizePropertyName ));
         DIP_THROW_IF( tsize.size() != 2, "Error in tensor size property" );
         enum dip::Tensor::Shape tshape = detail::GetTensorShape( mxGetPropertyShared( mx, 0, tshapePropertyName ));
         tensor.ChangeShape( dip::Tensor( tshape, tsize[ 0 ], tsize[ 1 ] ));
      }
      mxArray* pxsz = mxGetPropertyShared( mx, 0, pxsizePropertyName );
      dip::uint ndim = mxGetNumberOfElements( pxsz );
      dip::PhysicalQuantityArray pq( ndim );
      for(dip::uint ii = 0; ii < ndim; ++ii ) {
         mxArray* magnitude = mxGetField( pxsz, ii, pxsizeStructFields[ 0 ] );
         mxArray* units = mxGetField( pxsz, ii, pxsizeStructFields[ 1 ] );
         if( magnitude && units ) {
            dip::Units u;
            try {
               u = dip::Units( GetStringUnicode( units ));
            } catch( dip::Error const& /*e*/ ) {
               u = dip::Units::Pixel();
            }
            pq[ ii ] = { GetFloat( magnitude ), u };
         }
      }
      pixelSize.Set( pq );
      colorSpace = GetString( mxGetPropertyShared( mx, 0, colspPropertyName ));
   } else {
      // Data
      if( mxIsEmpty( mx )) {
         return {};
      }
      mxdata = mx;
      // Sizes
      ndims = mxGetNumberOfDimensions( mxdata );
      psizes = mxGetDimensions( mxdata );
      if( ndims <= 2 ) {
         if( psizes[ 0 ] == 1 && psizes[ 1 ] == 1 ) {
            ndims = 0;
         } else if( psizes[ 0 ] > 1 && psizes[ 1 ] > 1 ) {
            ndims = 2;
         } else {
            ndims = 1;
         }
      }
      sizes.resize( ndims, 1 );
      if( ndims == 1 ) {
         // for a 1D image, we expect one of the two dimensions to be 1
         sizes[ 0 ] = psizes[ 0 ] * psizes[ 1 ];
      } else if( ndims > 1 ) {
         std::copy( psizes, psizes + ndims, sizes.begin() );
      }
      // Data Type
      type = mxGetClassID( mxdata );
      maybeCast = ( ndims == 0 ) && ( type == mxDOUBLE_CLASS ); // If it's a scalar double, we might want to cast it to single instead.
      complex = mxIsComplex( mxdata );
      if( complex ) {
         // The complex data in an mxArray is stored as two separate memory blocks, and need to be copied to be
         // compatible with DIPlib storage
         needCopy = true;
      }
      if( conversion == ArrayConversionMode::TENSOR_OPERATOR ) {
         // If the last dimension is short, mark it so that we'll turn it into a tensor dimension at the end.
         lastDimToTensor = ( sizes.size() == 1 ) && ( sizes.back() <= 5 );
      }
      // ELSE: It's never a tensor (`tensor` is scalar by default), and color space nor pixel size are defined
   }
   dip::DataType datatype;
   switch( type ) {
      case mxDOUBLE_CLASS:    // dfloat
         if( complex ) { datatype = dip::DT_DCOMPLEX; }
         else { datatype = dip::DT_DFLOAT; }
         break;
      case mxSINGLE_CLASS:    // sfloat
         if( complex ) { datatype = dip::DT_SCOMPLEX; }
         else { datatype = dip::DT_SFLOAT; }
         break;
      case mxINT32_CLASS:     // sint32
         datatype = dip::DT_SINT32;
         break;
      case mxUINT32_CLASS:    // uint32
         datatype = dip::DT_UINT32;
         break;
      case mxINT16_CLASS:     // sint16
         datatype = dip::DT_SINT16;
         break;
      case mxUINT16_CLASS:    // uint16
         datatype = dip::DT_UINT16;
         break;
      case mxINT8_CLASS:      // sint8
         datatype = dip::DT_SINT8;
         break;
      case mxUINT8_CLASS:     // uint8
         datatype = dip::DT_UINT8;
         break;
      case mxLOGICAL_CLASS:   // bin
         datatype = dip::DT_BIN;
         break;
      default:
         DIP_THROW( "Image data is not numeric" );
   }
   DIP_THROW_IF( complex && !datatype.IsComplex(), "MATLAB image data of unsupported type" );
   // Create the stride array
   dip::sint tstride = 1;
   dip::uint s = tensor.Elements();
   dip::IntegerArray strides( ndims );
   for( dip::uint ii = 0; ii < ndims; ii++ ) {
      strides[ ii ] = static_cast< dip::sint >( s );
      s *= sizes[ ii ];
   }
   if( ndims >= 2 ) {
      using std::swap;
      swap( sizes[ 0 ], sizes[ 1 ] );
      swap( strides[ 0 ], strides[ 1 ] );
   }
   if( needCopy ) {
      // Create 2 temporary Image objects for the real and complex component,
      // then copy them over into a new image.
      dip::Image out( sizes, 1, datatype );
      dip::DataType dt = datatype.Real();
      void* p_real = mxGetData( mxdata );
      if( p_real ) {
         dip::Image real( dip::NonOwnedRefToDataSegment( p_real ), p_real, dt, sizes, strides, tensor, tstride );
         out.Real().Copy( real );
      } else {
         out.Real().Fill( 0 );
      }
      void* p_imag = mxGetImagData( mxdata );
      if( p_imag ) {
         dip::Image imag( dip::NonOwnedRefToDataSegment( p_imag ), p_imag, dt, sizes, strides, tensor, tstride );
         out.Imaginary().Copy( imag );
      } else {
         out.Imaginary().Fill( 0 );
      }
      if( maybeCast ) {
         detail::MaybeCastScalar( out );
      }
      //out.SetPixelSize( pixelSize );
      //out.SetColorSpace( colorSpace ); // these are never defined in this case, the input was a plain matrix.
      if( lastDimToTensor ) {
         out.SpatialToTensor();
      }
      return out;
   } else {
      dip::DataSegment dataSegment;
      if( mode == GetImageMode::SHARED_COPY ) {
         //std::cout << "Creating shared copy -- input data pointer = " << mxGetData( mxdata );
         mxArray *copy = mxCreateSharedDataCopy( mxdata );
         //std::cout << " -- copy data pointer = " << mxGetData( copy ) << '\n';
         mexMakeArrayPersistent( copy );
         dataSegment = dip::DataSegment{ copy, []( void* ptr ){ mxDestroyArray( static_cast< mxArray* >( ptr )); }};
      } else {
         //std::cout << "Input data pointer = " << mxGetData( mxdata ) << '\n';
         dataSegment = dip::NonOwnedRefToDataSegment( mxdata );
      }
      void* origin;
      if( datatype.IsBinary() ) {
         origin = mxGetLogicals( mxdata );
      } else {
         // Create Image object
         origin = mxGetData( mxdata );
      }
      // Create Image object
      dip::Image out( dataSegment, origin, datatype, sizes, strides, tensor, tstride );
      if( maybeCast ) {
         detail::MaybeCastScalar( out );
      }
      if( lastDimToTensor ) {
         out.SpatialToTensor();
         // In this case, the input was a plain matrix, so we don't have a pixel size or color space.
      } else {
         out.SetPixelSize( pixelSize );
         out.SetColorSpace( colorSpace );
      }
      return out;
   }
}

/// \brief Convert a cell array of images from `mxArray` to `dip::ImageArray`, using `dml::GetImage` for each
/// element of the cell array.
inline dip::ImageArray GetImageArray( mxArray const* mx ) {
   if( mxIsCell( mx ) && IsVector( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      dip::ImageArray out( n );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         out[ ii ] = GetImage( mxGetCell( mx, ii ));
      }
      return out;
   } else {
      try {
         dip::ImageArray out( 1 );
         out[ 0 ] = GetImage( mx );
         return out;
      } catch( dip::Error& ) {
         DIP_THROW( "Image array expected" );
      }
   }
}


//
// The ExternalInterface for MATLAB: Converting dip::Image to mxArray (sort of)
//


/* How this works:
 *
 * A `dip::Image` object has this class set as its external interface. When the image is forged, the
 * `AllocateData` member function is called to allocate the data segment for the image. This function then
 * creates an `mxArray` of the right sizes and type, and returns its data pointer as the `origin` pointer.
 * The pointer to the `mxArray` is stored in a `dml::MatlabInterface::mxContainer` object, which is owned by
 * the image's `DataSegment` shared pointer. When the image is stripped, this `DataSegment` is reset or
 * replaced. Because it's a shared pointer, when the last copy is reset or replaced, the `mxContainer` object
 * is deleted, causing  the `mxArray` to be destroyed.
 *
 * But, it is possible to set the `mxArray` pointer inside the `mxContainer` object to `nullptr`, thereby
 * "rescuing" the `mxArray`. The shared pointer will no longer own it.
 */

namespace detail {

// Convert DIPlib data type to MATLAB class ID
inline mxClassID GetMatlabClassID(
      dip::DataType dt
) {
   mxClassID type;
   switch( dt ) {
      case dip::DT_BIN:
         type = mxLOGICAL_CLASS;
         break;
      case dip::DT_UINT8:
         type = mxUINT8_CLASS;
         break;
      case dip::DT_SINT8:
         type = mxINT8_CLASS;
         break;
      case dip::DT_UINT16:
         type = mxUINT16_CLASS;
         break;
      case dip::DT_SINT16:
         type = mxINT16_CLASS;
         break;
      case dip::DT_UINT32:
         type = mxUINT32_CLASS;
         break;
      case dip::DT_SINT32:
         type = mxINT32_CLASS;
         break;
      case dip::DT_SFLOAT:
      case dip::DT_SCOMPLEX:
         type = mxSINGLE_CLASS;
         break;
      case dip::DT_DFLOAT:
      case dip::DT_DCOMPLEX:
         type = mxDOUBLE_CLASS;
         break;
      default:
         DIP_THROW_ASSERTION( "Unhandled DataType" ); // Should not be possible
   }
   return type;
}

// Create mxArray compatible with the given dip::Image properties
inline mxArray* CreateMxArray(
      dip::DataType datatype,
      dip::UnsignedArray const& sizes,
      dip::IntegerArray& strides,
      dip::Tensor const& tensor,
      dip::sint& tstride
) {
   // Find the right MATLAB class
   mxClassID type = detail::GetMatlabClassID( datatype );
   // Copy size array
   dip::UnsignedArray mlsizes = sizes;
   dip::uint n = sizes.size();
   // MATLAB arrays switch y and x axes
   if( n >= 2 ) {
      using std::swap;
      swap( mlsizes[ 0 ], mlsizes[ 1 ] );
   }
   // Create stride array
   tstride = 1;
   dip::uint s = tensor.Elements();
   strides.resize( n );
   for( dip::uint ii = 0; ii < n; ii++ ) {
      strides[ ii ] = static_cast< dip::sint >( s );
      s *= mlsizes[ ii ];
   }
   // Prepend tensor dimension
   mlsizes.insert( 0, tensor.Elements() );
   // Handle complex data
   mlsizes.insert( 0, datatype.IsComplex() ? 2 : 1 );
   // MATLAB arrays switch y and x axes
   if( n >= 2 ) {
      using std::swap;
      swap( strides[ 0 ], strides[ 1 ] );
   }
   // Allocate MATLAB matrix
   if( type == mxLOGICAL_CLASS ) {
      return mxCreateLogicalArray( mlsizes.size(), mlsizes.data() );
   } else {
      return mxCreateNumericArray( mlsizes.size(), mlsizes.data(), type, mxREAL );
   }
}

// Are the strides consistent for how we create them in this interface?
// Note that it is not an issue if singleton dimensions were added or removed, EXCEPT if one of the first
// two dimensions were shifted because of that. Shifting the first two dimensions requires a re-organization
// of the data because we store them swapped: 1, 0, 2, 3, 4, ...
//  - Removing dimension 0 or 1 causes 2 to jump to the front: 2, 1, 3, 4, ...
//  - Adding a dimension 0 causes 1 to jump over 1: 0, x, 1, 2, 3, 4, ...
//  - Adding a dimension 1 causes the same: x, 0, 1, 2, 3, 4, ...
// Thus, here we ignore singleton dimensions only after the first two dimensions.
inline bool IsMatlabStrides(
      dip::Image const& img
) {
   dip::uint nDims = img.Dimensionality();
   dip::uint tElem = img.TensorElements();
   if(( tElem > 1 ) && ( img.TensorStride() != 1 )) {
      //mexPrintf( "IsMatlabStrides: tensor test failed\n" );
      return false;
   }
   // After squeezing, are we dealing with a 1D image?
   dip::uint nSqueezeDims = 0;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( img.Size( ii ) > 1 ) {
         ++nSqueezeDims;
      }
   }
   if( nSqueezeDims == 1 ) {
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if(( img.Size( ii ) > 1 ) && ( img.Stride( ii ) != static_cast< dip::sint >( tElem ))) {
            //mexPrintf( "IsMatlabStrides: 1D test failed\n" );
            return false;
         }
      }
   } else if( nDims >= 2 ) {
      dip::sint total = static_cast< dip::sint >( tElem );
      if( img.Stride( 1 ) != total ) {
         //mexPrintf( "IsMatlabStrides: second dimension test failed\n" );
         return false;
      }
      total *= static_cast< dip::sint >( img.Size( 1 ));
      if( img.Stride( 0 ) != total ) {
         //mexPrintf( "IsMatlabStrides: first dimension test failed\n" );
         return false;
      }
      total *= static_cast< dip::sint >( img.Size( 0 ));
      for( dip::uint ii = 2; ii < nDims; ++ii ) {
         if(( img.Size( ii ) > 1 ) && ( img.Stride( ii ) != total )) {
            //mexPrintf( "IsMatlabStrides: higher dimension test failed\n" );
            return false;
         }
         total *= static_cast< dip::sint >( img.Size( ii ));
      }
   }
   return true;
}

// Do the dip::Image and mxArray sizes match?
// NOTE: this function will add or remove singletons from `mat` if it can make the sizes match.
// As in the function above, added singletons in the first two dimensions will require a data copy,
// because we store them swapped. So we only add or remove singletons in dimensions 2 and above.
// The exception is if there is only one non-singleton dimension -- we can add and remove singletons
// as we please in this case without affecting storage order.
inline bool MatchSizes(
      dip::Image const& img,
      mxArray* mat
) {
   mwSize const* mexSizes = mxGetDimensions( mat );
   bool complex = img.DataType().IsComplex();
   if(( complex && mexSizes[ 0 ] != 2 ) || ( !complex && mexSizes[ 0 ] != 1 )) {
      //mexPrintf( "MatchSizes: complexity test failed\n" );
      return false;
   }
   if( mexSizes[ 1 ] != img.TensorElements() ) {
      //mexPrintf( "MatchSizes: TensorElements test failed\n" );
      return false;
   }
   mwSize nDimsMex = mxGetNumberOfDimensions( mat );
   dip::uint nDimsDip = img.Dimensionality();
   dip::UnsignedArray const& dipSizes = img.Sizes();
   // Check number of pixels
   dip::uint totalDip = std::accumulate( dipSizes.begin(), dipSizes.end(), static_cast< dip::uint >(1), std::multiplies< dip::uint >() );
   dip::uint totalMex = std::accumulate( mexSizes + 2, mexSizes + nDimsMex, static_cast< dip::uint >(1), std::multiplies< dip::uint >() );
   if( totalDip != totalMex ) {
      //mexPrintf( "MatchSizes: number of pixels test failed\n" );
      return false;
   }
   // After squeezing, are we dealing with a 1D image?
   bool dipIs1D = nDimsDip < 1;
   for( dip::uint ii = 0; ii < nDimsDip; ++ii ) {
      if( dipSizes[ ii ] == totalDip ) {
         dipIs1D = true;
      }
   }
   bool mexIs1D = nDimsMex < 3;
   for( dip::uint ii = 2; ii < nDimsMex; ++ii ) {
      if( mexSizes[ ii ] == totalMex ) {
         mexIs1D = true;
      }
   }
   if( dipIs1D != mexIs1D ) {
      //mexPrintf( "MatchSizes: 1D test failed\n" );
      return false;
   }
   bool needAdjustment = false;
   if( mexIs1D ) {
      // Both are 1D images (possibly with singleton dimensions)
      needAdjustment = true; // not sure, but it won't hurt
   } else {
      // Both have at least 2 non-singleton dimensions.
      if(( mexSizes[ 2 ] != dipSizes[ 1 ] ) || ( mexSizes[ 3 ] != dipSizes[ 0 ] )) {
         //mexPrintf( "MatchDimensions: first two dimensions test failed\n" );
         return false;
      }
      dip::uint iiMex = 4;
      dip::uint iiDip = 2;
      while( iiDip < nDimsDip ) {
         dip::uint sz = ( iiMex < nDimsMex ) ? mexSizes[ iiMex ] : 1;
         if( sz != dipSizes[ iiDip ] ) {
            if( sz == 1 ) {
               needAdjustment = true;
               ++iiMex;
               continue;
            }
            if( dipSizes[ iiDip ] == 1 ) {
               needAdjustment = true;
               ++iiDip;
               continue;
            }
            //mexPrintf( "MatchSizes: size test failed\n" );
            return false;
         }
         ++iiMex;
         ++iiDip;
      }
   }
   if( needAdjustment ) {
      //mexPrintf( "MatchSizes: adjusting mxArray singletons\n" );
      dip::UnsignedArray newSizes( nDimsDip + 2 );
      newSizes[ 0 ] = mexSizes[ 0 ];
      newSizes[ 1 ] = mexSizes[ 1 ];
      std::copy( dipSizes.begin(), dipSizes.end(), newSizes.begin() + 2 );
      if( nDimsDip > 1 ) {
         std::swap( newSizes[ 2 ], newSizes[ 3 ] );
      }
      mxSetDimensions( mat, newSizes.data(), newSizes.size() );
   }
   return true;
}

} // namespace detail

/// \brief This class is the dip::ExternalInterface for the *MATLAB* interface.
///
/// In a MEX-file, use the following code when declaring images to be
/// used as the output to a function:
/// ```cpp
///     dml::MatlabInterface mi;
///     dip::Image img_out0 = mi.NewImage();
///     dip::Image img_out1 = mi.NewImage();
/// ```
/// This configures the images `img_out0` and `img_out1` such that, when they are
/// forged later on, an `mxArray` structure will be created to hold the pixel data.
/// `mxArray` is *MATLAB*'s representation of arrays.
/// To return those images back to *MATLAB*, use `dml::GetArray`, which returns
/// the `mxArray` created when the image was forged:
/// ```cpp
///     plhs[ 0 ] = dm::GetArray( img_out0 );
///     plhs[ 1 ] = dm::GetArray( img_out1 );
/// ```
///
/// If you don't use `dml::GetArray`, the `mxArray` that contains
/// the pixel data will be destroyed when the dip::Image object goes out
/// of scope.
///
/// Note that the `%dml::MatlabInterface` object needs to persist for the duration
/// of the lifetime of the images returned by the `NewImage` method, since these
/// images hold a pointer to it.
///
/// Remember to not assign a result into the images created with `NewImage`,
/// as the pixel data will be copied in the assignment into a *MATLAB* array.
/// Instead, use the *DIPlib* functions that take output images as function
/// arguments:
/// ```cpp
///     img_out0 = in1 + in2;           // Bad! Incurs an unnecessary copy
///     dip::Add( in1, in2, img_out0 ); // Correct, the operation writes directly in the output data segment
/// ```
/// In the first case, `in1 + in2` is computed into a temporary image, whose
/// pixels are then copied into the `mxArray` created for `img_out0`. In the
/// second case, the result of the operation is directly written into the
/// `mxArray`, no copies are necessary.
///
/// This interface handler doesn't own any image data.
class MatlabInterface : public dip::ExternalInterface {
   public:

      // This holds the pointer to the `mxArray`, but it can also hold a `nullptr`.
      // Inside a `std::shared_ptr`, this object's mxArray can be extracted without
      // it being deleted when the container goes out of scope.
      struct mxContainer {
         mxArray* array = nullptr;
         explicit mxContainer( mxArray* a ) : array( a ) {}
         ~mxContainer() {
            if( array ) {
               mxDestroyArray( array );
            }
         }
      };

      /// This function overrides dip::ExternalInterface::AllocateData().
      /// It is called when an image with this `ExternalInterface` is forged.
      /// It allocates a *MATLAB* `mxArray` and returns a `dip::DataSegment`
      /// containing a pointer to the  `mxArray` data pointer, with a custom
      /// deleter functor. It also adjusts strides to match the `mxArray` storage.
      ///
      /// A user will never call this function directly.
      virtual dip::DataSegment AllocateData(
            void*& origin,
            dip::DataType datatype,
            dip::UnsignedArray const& sizes,
            dip::IntegerArray& strides,
            dip::Tensor const& tensor,
            dip::sint& tstride
      ) override {
         mxArray* m = detail::CreateMxArray( datatype, sizes, strides, tensor, tstride );
         if( mxIsLogical( m )) {
            origin = mxGetLogicals( m );
         } else {
            origin = mxGetData( m );
         }
         auto tmp = std::make_shared< mxContainer >( m );
         return std::static_pointer_cast< void >( tmp );
      }

      /// \brief Constructs a dip::Image object with the external interface set so that,
      /// when forged, a *MATLAB* `mxArray` will be allocated to hold the samples.
      ///
      /// Use dml::MatlabInterface::GetArray to obtain the `mxArray` and assign
      /// it as a `lhs` argument to your MEX-file.
      dip::Image NewImage() {
         dip::Image out;
         out.SetExternalInterface( this );
         return out;
      }
};

/// \brief Find the `mxArray` that holds the data for the dip::Image `img`.
// The *UNDOCUMENTED* `doNotSetToTrue` flag tells this function that the data shared pointer in the image
// is an `mxArray`. This is true only if `img` was created by `dml::GetImage`. You should not need this
// functionality, it is used in `dipimage/private/imagedisplay.cpp`.
inline mxArray* GetArrayAsArray( dip::Image const& img, bool doNotSetToTrue = false ) {
   DIP_THROW_IF( !img.IsForged(), dip::E::IMAGE_NOT_FORGED );
   mxArray* mat = nullptr;
   // Make sure that `img` has external data and the correct external interface set
   if( img.IsExternalData() ) {
      if( doNotSetToTrue ) {
         mat = static_cast< mxArray* >( img.Data() );
      } else if( dynamic_cast< MatlabInterface* >( img.ExternalInterface() )) {
         auto tmp = static_cast< MatlabInterface::mxContainer* >( img.Data() );
         if( tmp ) {
            // Get the `mxArray` that contains the data for `img`
            mat = tmp->array;
            // By setting this to a `nullptr`, we ensure it will not be destroyed later on
            tmp->array = nullptr;
         }
      }
   }
   // Get the data pointer inside the `mxArray` (if we have one)
   void* mptr = mat ? ( img.DataType().IsBinary() ? mxGetLogicals( mat ) : mxGetData( mat )) : nullptr;
   // If the mxArray data pointer is not equal to the image origin, we either have a mirrored dimension or
   // shifted origin, or the image data is not stored in the mxArray at all -- either way we need a copy
   bool needCopy = !mat || ( mptr != img.Origin() );
   if( !needCopy ) {
      // If we fudged the data type, we need a copy.
      needCopy = mxGetClassID( mat ) != detail::GetMatlabClassID( img.DataType() );
   }
   if( !needCopy ) {
      // If the strides are not as expected, we've permuted dimensions and need a copy
      needCopy = !detail::IsMatlabStrides( img );
   }
   if( !needCopy ) {
      // Compare also dimensions, adding or removing singleton dimensions from `mat` if that helps
      // If needCopy is false, `mat` can have been modified (shape only, not data of course).
      needCopy = !detail::MatchSizes( img, mat );
   }
   // If the image points to a modified view, or a non-MATLAB array, make a copy
   if( needCopy ) {
#ifdef DIP__ENABLE_ASSERT // Or use NDEBUG instead?
      mexPrintf( "GetArrayAsArray: Copying data from dip::Image to mxArray\n" );
#endif
      dip::IntegerArray strides;
      dip::sint tStride = 1;
      mxArray* newmat = detail::CreateMxArray( img.DataType(), img.Sizes(), strides, img.Tensor(), tStride );
      dip::Image tmp = GetImage( newmat );
      // Note that `newmat` is designed to be stuck inside a `dip_image` object, so is slightly different than
      // what `GetImage` expects: the tensor dimension is spatial dimension 0, and spatial dimension 1 is the complex
      // "dimension". GetImage will also swap the first two dimensions. We need to fix `tmp` to be like `img`.
      tmp.ExpandDimensionality( img.Dimensionality() + 2 );
      if( img.DataType().IsComplex() ) {
         tmp.MergeComplex( 1 );
      } else {
         tmp.Squeeze( 1 );
      }
      tmp.SpatialToTensor( 0 );
      if( tmp.Dimensionality() > 1 ) {
         tmp.SwapDimensions( 0, 1 );
      }
      tmp.Protect(); // Shouldn't be necessary, but consider this an assert.
      tmp.Copy( img );
      mxDestroyArray( mat );
      mat = newmat;
   }
   return mat;
}

/// \brief Find the `mxArray` that holds the data for the dip::Image `img`,
/// and create a MATLAB dip_image object around it.
// See `dml::GetArrayAsArray` above for the meaning of `doNotSetToTrue`.
inline mxArray* GetArray( dip::Image const& img, bool doNotSetToTrue = false ) {
   mxArray* mat;
   DIP_STACK_TRACE_THIS( mat = GetArrayAsArray( img, doNotSetToTrue ));
   // Create a MATLAB `dip_image` object with the `mxArray` inside.
   // We create an empty object, then set the Array property, because calling the constructor
   // with the `mxArray` for some reason causes a deep copy of the `mxArray`.
   //std::cout << "Output data pointer = " << mxGetData( mat );
   mxArray* out;
   mexCallMATLAB( 1, &out, 0, nullptr, imageClassName );
   mxSetPropertyShared( out, 0, arrayPropertyName, mat );
   //mxArray* tmp = mxGetPropertyShared( out, 0, arrayPropertyName );
   //std::cout << " -- output data pointer = " << mxGetData( tmp ) << '\n';
   // Set NDims property
   mxArray* ndims = mxCreateDoubleScalar( static_cast< double >( img.Dimensionality() ));
   mxSetPropertyShared( out, 0, ndimsPropertyName, ndims );
   // Set TensorShape property
   if( img.TensorElements() > 1 ) {
      mxArray* tshape;
      switch( img.TensorShape() ) {
         default:
            //case dip::Tensor::Shape::COL_VECTOR:
            tshape = CreateDouble2Vector( static_cast< dip::dfloat >( img.TensorElements() ), 1 );
            break;
         case dip::Tensor::Shape::ROW_VECTOR:
            tshape = CreateDouble2Vector( 1, static_cast< dip::dfloat >( img.TensorElements() ));
            break;
         case dip::Tensor::Shape::COL_MAJOR_MATRIX:
            tshape = CreateDouble2Vector( static_cast< dip::dfloat >( img.TensorRows() ), static_cast< dip::dfloat >( img.TensorColumns() ));
            break;
         case dip::Tensor::Shape::ROW_MAJOR_MATRIX:
            // requires property to be set twice
            tshape = mxCreateString( img.Tensor().TensorShapeAsString().c_str() );
            mxSetPropertyShared( out, 0, tshapePropertyName, tshape );
            tshape = CreateDouble2Vector( static_cast< dip::dfloat >( img.TensorRows() ), static_cast< dip::dfloat >( img.TensorColumns() ));
            break;
         case dip::Tensor::Shape::DIAGONAL_MATRIX:
         case dip::Tensor::Shape::SYMMETRIC_MATRIX:
         case dip::Tensor::Shape::UPPTRIANG_MATRIX:
         case dip::Tensor::Shape::LOWTRIANG_MATRIX:
            tshape = mxCreateString( img.Tensor().TensorShapeAsString().c_str() );
            break;
      }
      mxSetPropertyShared( out, 0, tshapePropertyName, tshape );
   }
   // Set PixelSize property
   if( img.HasPixelSize() ) {
      dip::PixelSize const& pixelSize = img.PixelSize();
      mxArray* pxsz = dml::GetArray( pixelSize );
      mxSetPropertyShared( out, 0, pxsizePropertyName, pxsz );
   }
   // Set ColorSpace property
   if( img.IsColor() ) {
      mxSetPropertyShared( out, 0, colspPropertyName, dml::GetArray( img.ColorSpace() ));
   }
   return out;
}


//
// Some other common code for many functions
//


/// \brief Gets a structuring element or kernel from the input argument(s) at `index`, and `index+1`. `index` is
/// updated to point to the next unused input argument.
template< typename K >
inline K GetKernel( int nrhs, const mxArray* prhs[], int& index, dip::uint nDims ) {
   K k;
   if( nrhs > index ) {
      if( mxIsNumeric( prhs[ index ] ) && ( mxGetNumberOfElements( prhs[ index ] ) <= nDims )) {
         // This looks like a sizes vector
         auto filterParam = GetFloatArray( prhs[ index ] );
         ++index;
         if( nrhs > index ) {
            auto filterShape = GetString( prhs[ index ] );
            ++index;
            k = K( filterParam, filterShape );
         } else {
            k = K( filterParam );
         }
      } else {
         // Assume it's an image?
         k = K( GetImage( prhs[ index ] ) );
         ++index;
      }
   }
   return k;
}


//
// Utility
//


/// \brief An output stream buffer for MEX-files.
///
/// Creating an object of this class replaces the stream buffer in `std::cout` with the newly
/// created object. This buffer will be used as long as the object exists. When the object
/// is destroyed (which happens automatically when it goes out of scope), the original
/// stream buffer is replaced.
///
/// Create an object of this class at the beginning of any MEX-file that uses `std::cout` to
/// print information to the *MATLAB* terminal. *DIPlib* defines several classes with a stream
/// insertion operator that would be cumbersome to use with a `std::stringstream` and `mexPrintf`.
/// This class simplifies their use.
class streambuf : public std::streambuf {
   public:
      streambuf() {
         stdoutbuf = std::cout.rdbuf( this );
      }
      ~streambuf() override {
         std::cout.rdbuf( stdoutbuf );
      }
      // Cannot be copied or moved:
      streambuf( streambuf const& ) = delete;
      streambuf( streambuf&& ) = delete;
      streambuf& operator=( streambuf const& ) = delete;
      streambuf& operator=( streambuf&& ) = delete;
   protected:
      virtual std::streamsize xsputn( const char* s, std::streamsize n ) override {
         mexPrintf( "%.*s", n, s );
         return n;
      }
      virtual int overflow( int c = EOF ) override {
         if( c != EOF ) {
            mexPrintf( "%.1s", &c );
         }
         return 1;
      }
   private:
      std::streambuf* stdoutbuf;
};

/// \brief Convert a string to all upper-case letters, using current locale. Will not work with Unicode strings.
inline void ToUpper( dip::String& str ) {
   for( auto& c : str ) {
      c = static_cast< char >( std::toupper( static_cast< unsigned char >( c )));
   }
}

/// \brief Convert a string to all lower-case letters, using current locale. Will not work with Unicode strings.
inline void ToLower( dip::String& str ) {
   for( auto& c : str ) {
      c = static_cast< char >( std::tolower( static_cast< unsigned char >( c )));
   }
}


/// \}

} // namespace dml

#endif // DIP_MATLAB_INTERFACE_H
