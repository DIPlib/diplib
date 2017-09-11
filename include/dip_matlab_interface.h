/*
 * DIPlib 3.0
 * This file contains functionality for the MATLAB interface.
 *
 * (c)2015-2017, Cris Luengo.
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

#ifndef DIP_MATLAB_H
#define DIP_MATLAB_H

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
#include "diplib/file_io.h" // Definition of FileInformation

/*
 * An alternative:
 *
 * We create a `dip::Image` with `new`, and return the pointer to MATLAB in a handle class. The handle class
 * needs a destructor defined, which calls the `dip::Image` destructor in a MEX-file. I think the image
 * needs to be created and destroyed in the same MEX-file for this to work, because the file needs to be locked.
 * If not locked, `clear functions` causes memory to be wiped (apparently) and pointers to dangle.
 *
 * See http://www.mathworks.com/matlabcentral/fileexchange/38964-example-matlab-class-wrapper-for-a-c++-class
 */


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
// Private functions
//

namespace detail {

// Are the strides consistent for how we create them in this interface?
inline bool IsMatlabStrides(
      dip::UnsignedArray const& sizes,
      dip::uint telem,
      dip::IntegerArray const& strides,
      dip::sint tstride
) {
   if( sizes.size() != strides.size() ) {
      //mexPrintf( "IsMatlabStrides: dimensionality test failed\n" );
      return false;
   }
   if(( telem > 1 ) && ( tstride != 1 )) { // tstride is meaningless if there's only one tensor element
      //mexPrintf( "IsMatlabStrides: tensor test failed\n" );
      return false;
   }
   if( sizes.size() == 1 ) {
      if( strides[ 0 ] != static_cast< dip::sint >( telem )) {
         //mexPrintf( "IsMatlabStrides: 1D test failed\n" );
         return false;
      }
   } else if( sizes.size() >= 2 ) {
      dip::sint total = static_cast< dip::sint >( telem );
      if( strides[ 1 ] != total ) {
         //mexPrintf( "IsMatlabStrides: second dimension test failed\n" );
         return false;
      }
      total *= static_cast< dip::sint >( sizes[ 1 ] );
      if( strides[ 0 ] != total ) {
         //mexPrintf( "IsMatlabStrides: first dimension test failed\n" );
         return false;
      }
      total *= static_cast< dip::sint >( sizes[ 0 ] );
      for( dip::uint ii = 2; ii < sizes.size(); ++ii ) {
         if( strides[ ii ] != total ) {
            //mexPrintf( "IsMatlabStrides: higher dimension test failed\n" );
            return false;
         }
         total *= static_cast< dip::sint >( sizes[ ii ] );
      }
   }
   return true;
}

// Do the dip::Image and mxArray dimensions match?
inline bool MatchDimensions(
      dip::UnsignedArray const& sizes,
      dip::uint telem,
      bool complex,
      mwSize const* psizes,
      mwSize ndims
) {
   if(( complex && psizes[ 0 ] != 2 ) || ( !complex && psizes[ 0 ] != 1 )) {
      //mexPrintf( "MatchDimensions: complexity test failed\n" );
      return false;
   }
   if( psizes[ 1 ] != telem ) {
      //mexPrintf( "MatchDimensions: telem test failed\n" );
      return false;
   }
   dip::uint n = ndims - 2;
   if( n > sizes.size() ) { // can be smaller if there are trailing singleton dimensions
      //mexPrintf( "MatchDimensions: dimensionality test failed\n" );
      return false;
   }
   if( n == 1 ) {
      if( psizes[ 2 ] != sizes[ 0 ] ) {
         //mexPrintf( "MatchDimensions: 1D test failed\n" );
         return false;
      }
   } else if( n >= 2 ) {
      if(( psizes[ 2 ] != sizes[ 1 ] ) || ( psizes[ 3 ] != sizes[ 0 ] )) {
         //mexPrintf( "MatchDimensions: first two dimensions test failed\n" );
         return false;
      }
      for( dip::uint ii = 2; ii < n; ++ii ) {
         if( psizes[ 2 + ii ] != sizes[ ii ] ) {
            //mexPrintf( "MatchDimensions: higher dimension test failed\n" );
            return false;
         }
      }
      for( dip::uint ii = n; ii < sizes.size(); ++ii ) {
         if( sizes[ ii ] != 1 ) {
            //mexPrintf( "MatchDimensions: trailing singleton test failed\n" );
            return false;
         }
      }
   }
   return true;
}

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

} // namespace detail

//
// Get input arguments: convert mxArray to various dip:: types
//

#define DML_MIN_ARGS( n ) DIP_THROW_IF( nrhs < n, "Too few input arguments" )
#define DML_MAX_ARGS( n ) DIP_THROW_IF( nrhs > n, "Too many input arguments" )

/// \brief True if empty or a one-dimensional array
inline bool IsVector( mxArray const* mx ) {
   return ( mxGetNumberOfDimensions( mx ) == 2 ) && (( mxGetM( mx ) <= 1 ) || ( mxGetN( mx ) <= 1 ));
}

/// \brief Convert a boolean (logical) from `mxArray` to `bool` by copy.
inline bool GetBoolean( mxArray const* mx ) {
   if( mxIsScalar( mx )) {
      if( mxIsLogical( mx )) {
         return *mxGetLogicals( mx );
      } else if( mxIsDouble( mx ) && !mxIsComplex( mx )) {
         return *mxGetPr( mx ) != 0;
      }
   }
   DIP_THROW( "Boolean value expected" );
}

/// \brief Convert an unsigned integer from `mxArray` to `dip::uint` by copy.
inline dip::uint GetUnsigned( mxArray const* mx ) {
   if( mxIsScalar( mx ) && mxIsDouble( mx ) && !mxIsComplex( mx )) {
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
   if( mxIsScalar( mx ) && mxIsDouble( mx ) && !mxIsComplex( mx )) {
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
   if( mxIsScalar( mx ) && mxIsDouble( mx ) && !mxIsComplex( mx )) {
      return *mxGetPr( mx );
   }
   DIP_THROW( "Real floating-point value expected" );
}

/// \brief Convert a complex floating-point number from `mxArray` to `dip::dcomplex` by copy.
inline dip::dcomplex GetComplex( mxArray const* mx ) {
   if( mxIsScalar( mx ) && mxIsDouble( mx )) {
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
      char* str = mxArrayToUTF8String( mx );
      if( str ) {
         dip::String out( str );
         mxFree( str );
         return out;
      }
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

/// \brief Convert an unsigned integer array from `dip::UnsignedArray` to `mxArray` by copy.
inline mxArray* GetArray( dip::UnsignedArray const& in ) {
   mxArray* mx = mxCreateDoubleMatrix( 1, in.size(), mxREAL );
   double* data = mxGetPr( mx );
   for( dip::uint ii = 0; ii < in.size(); ++ii ) {
      data[ ii ] = static_cast< double >( in[ ii ] );
   }
   return mx;
}

/// \brief Convert a signed integer array from `dip::IntegerArray` to `mxArray` by copy.
inline mxArray* GetArray( dip::IntegerArray const& in ) {
   mxArray* mx = mxCreateDoubleMatrix( 1, in.size(), mxREAL );
   double* data = mxGetPr( mx );
   for( dip::uint ii = 0; ii < in.size(); ++ii ) {
      data[ ii ] = static_cast< double >( in[ ii ] );
   }
   return mx;
}

/// \brief Convert a floating-point array from `dip::FloatArray` to `mxArray` by copy.
inline mxArray* GetArray( dip::FloatArray const& in ) {
   mxArray* mx = mxCreateDoubleMatrix( 1, in.size(), mxREAL );
   double* data = mxGetPr( mx );
   for( dip::uint ii = 0; ii < in.size(); ++ii ) {
      data[ ii ] = in[ ii ];
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
   static_assert( sizeof( char16_t ) == sizeof( mxChar ), "MATLAB's mxChar is not 16 bits." );
   std::u16string u16str = std::wstring_convert< std::codecvt_utf8_utf16< char16_t >, char16_t >{}.from_bytes( in );
   dip::uint sz[ 2 ] = { 1, u16str.size() };
   mxArray* out = mxCreateCharArray( 2, sz );
   std::copy( u16str.begin(), u16str.end(), mxGetChars( out ) );
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

/// \brief Convert a pixel size structure `dip::PixelSize` to `mxArray` by copy.
inline mxArray* GetArray( dip::PixelSize const& pixelSize ) {
   mxArray* pxsz = mxCreateStructMatrix( pixelSize.Size(), 1, nPxsizeStructFields, pxsizeStructFields );
   for( dip::uint ii = 0; ii < pixelSize.Size(); ++ii ) {
      mxSetField( pxsz, ii, pxsizeStructFields[ 0 ], dml::GetArray( pixelSize[ ii ].magnitude ));
      mxSetField( pxsz, ii, pxsizeStructFields[ 1 ], dml::GetArrayUnicode( pixelSize[ ii ].units.String() ));
   }
   return pxsz;
}

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


//
// The ExternalInterface for MATLAB: Converting dip::Image to mxArray (sort of)
//

/* How this works:
 *
 * A `dip::Image` object has this class set as its external interface. When the image is forged, the
 * `AllocateData` member function is called to allocate the data segment for the image. This function then
 * creates an `mxArray` of the right sizes and type, and returns its data pointer as the `origin` pointer.
 * The pointer to the `mxArray` is stored in the image's `DataSegment` shared pointer. When the image is
 * stripped, this `DataSegment` is reset or replaced. Because it's a shared pointer, when the last copy
 * is reset or replaced, the deleter function associated to it is called. This is where the complexity
 * comes in. We need to be able to "rescue" the `mxArray` from being deleted, so we can return it to
 * MATLAB when the MEX-file terminates. So making this deleter function simply call `mxDestroyArray` is
 * not sufficient. Instead, we provide an instance of the `StripHandler` class as the deleter function.
 * Its `operator()` looks up the `mxArray` pointer in the `mla` "database", and only calls `mxDestroyArray`
 * if it can find the pointer. Thus, if we want to "rescue" the `mxArray`, all we need to do is remove
 * its pointer from `mla`.
 *
 * Alternative (NOTE that the description below is not actually implemented!)
 *
 * There are other ways in which we could have accomplished the same thing. This alternative is maybe a
 * little simpler:
 *  - The `DataSegment` pointer points to an object that holds a pointer to the mxArray.
 *  - The object's destructor calls `mxDestroyArray` on this pointer if the pointer is not `nullptr`.
 *  - We can call a function that sets this pointer to `nullptr` (or simply allow directy access to the pointer).
 *  - When we want to "rescue" the `mxArray`, we set its pointer to `nullptr`, the `DataSegment` can then be
 *    deleted without us loosing our `mxArray`.
 * Advantages:
 *  - No `mla` in the external interface. No status at all outside of the `dip::Image` object.
 *  - No special deleter functor in the shared object. The shared object just manages a normal object created
 *    with `new`.
 *  - The `GetArray` function does not need to be a method of the external interface. This makes it easier to
 *    use, as any DIPlib type can be converted to a MATLAB type using the same `dml::GetArray` interface.
 * Disadvantages:
 *  - The object needs to be allocated on the heap. With `std::make_shared` this allocation is combined with
 *    that for the shared pointer's control structure, but we then need to `std::static_pointer_cast` to a
 *    void shared pointer, which (maybe?) requires one more allocation?
 *  - The external interface needs to continue existing for as long as the `dip::Image` that references it
 *    might be forged or reforged, even though there is no need to access it directly otherwise (i.e. it is
 *    not clear from the calling code that the external interface needs to continue existing).
 * Example use would be:
 * ```cpp
 *     dml::MatlabInterface mi;
 *     dip::Image img_out0 = mi.NewImage();
 *     ...
 *     plhs[ 0 ] = dml::GetArray( img_out0 );
 * ```
 * The class would look like this:
 * ```cpp
 *     class MatlabInterface : public dip::ExternalInterface {
 *        public:
 *           struct mxContainer {
 *              mxArray* array = nullptr;
 *              ~mxContainer() { if( array ) { mxDestroyArray( array ); } }
 *           };
 *           virtual dip::DataSegment AllocateData( ... ) override {
 *              ...
 *              auto tmp = std::make_shared< mxContainer >();
 *              tmp->array = mxCreateNumericArray( ... );
 *              origin = mxGetData( tmp->array );
 *              return std::static_pointer_cast< void >( tmp );
 *           }
 *           dip::Image NewImage() {
 *              dip::Image out;
 *              out.SetExternalInterface( this );
 *              return out;
 *           }
 *     };
 *     mxArray* GetArray( dip::Image const& img ) {
 *        DIP_THROW_IF( !img.IsForged(), dip::E::IMAGE_NOT_FORGED );
 *        auto tmp = static_cast< MatlabInterface::mxContainer* >( img.Data()->get() );
 *        mxArray* out = nullptr;
 *        if( tmp && tmp->array ) {
 *           out = tmp->array;
 *           tmp->array = nullptr;
 *        }
 *        if( out ) {
 *           ... // test, set `out = nullptr` if we need a fresh copy
 *        }
 *        if( out ) {
 *           ... // create a fresh copy of `img`
 *        }
 *        ... // create dip_image object holding the mxArray
 *        return out;
 *     }
 * ```
 */

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
/// To return those images back to *MATLAB*, use the GetArray() method, which returns
/// the `mxArray` created when the image was forged:
/// ```cpp
///     plhs[ 0 ] = mi.GetArray( img_out0 );
///     plhs[ 1 ] = mi.GetArray( img_out1 );
/// ```
///
/// If you don't use the GetArray() method, the `mxArray` that contains
/// the pixel data will be destroyed when the dip::Image object goes out
/// of scope.
///
/// Remember to not assign a result into the images created with `NewImage`,
/// as the pixel data will be copied in the assignment into a *MATLAB* array.
/// Instead, use the *DIPlib* functions that take output images as function
/// arguments:
/// ```cpp
///     img_out0 = in1 + in2;                                                                    // Bad!
///     dip::Add( in1, in2, out, DataType::SuggestArithmetic( in1.DataType(), in1.DataType() )); // Correct
/// ```
/// In the first case, `in1 + in2` is computed into a temporary image, whose
/// pixels are then copied into the `mxArray` created for `img_out0`. In the
/// second case, the result of the operation is directly written into the
/// `mxArray`, no copies are necessary.
///
/// This interface handler doesn't own any image data.
class MatlabInterface : public dip::ExternalInterface {
   private:
      // Here we store the mxArray pointers that the interface owns.
      std::set< mxArray* > mla;
      // This is the deleter functor we'll associate to the dip::DataSegment.
      class StripHandler {
         private:
            MatlabInterface& interface;
         public:
            StripHandler( MatlabInterface& mi ) : interface{ mi } {};
            void operator()( void* p ) {
               mxArray* m = static_cast< mxArray* >( p );
               auto it = interface.mla.find( m );
               if( it != interface.mla.end() ) {
                  mxDestroyArray( *it );
                  interface.mla.erase( it );
               }
            };
      };

   public:
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
         mxArray* m;
         if( type == mxLOGICAL_CLASS ) {
            m = mxCreateLogicalArray( mlsizes.size(), mlsizes.data() );
            origin = mxGetLogicals( m );

         } else {
            m = mxCreateNumericArray( mlsizes.size(), mlsizes.data(), type, mxREAL );
            origin = mxGetData( m );
         }
         //mexPrintf( "   Created mxArray as dip::Image data block. Data pointer = %p.\n", p );
         mla.insert( m );
         return dip::DataSegment{ m, StripHandler( *this ) };
      }

      /// \brief Find the `mxArray` that holds the data for the dip::Image `img`,
      /// and create a MATLAB dip_image object around it.
      mxArray* GetArray( dip::Image const& img ) {
         //std::cout << "GetArray for image: " << img;
         DIP_THROW_IF( !img.IsForged(), dip::E::IMAGE_NOT_FORGED );
         mxArray* mat = static_cast< mxArray* >( img.Data() );
         auto it = mla.find( mat );
         if( it == mla.end() ) {
            mat = nullptr;
         }
         if( !mat ) { mexPrintf( "   ...that image was not forged through the MATLAB interface\n" ); } // TODO: temporary warning, to be removed.
         void* mptr = mat ? ( img.DataType().IsBinary() ? mxGetLogicals( mat ) : mxGetData( mat ) ) : nullptr;
         // Does the image point to a modified view of the mxArray or to a non-MATLAB array?
         if( !mat || ( mptr != img.Origin() ) ||
             !detail::IsMatlabStrides( img.Sizes(), img.TensorElements(),
                               img.Strides(), img.TensorStride() ) ||
             !detail::MatchDimensions( img.Sizes(), img.TensorElements(), img.DataType().IsComplex(),
                               mxGetDimensions( mat ), mxGetNumberOfDimensions( mat )) ||
             ( mxGetClassID( mat ) != detail::GetMatlabClassID( img.DataType() ))
             // TODO: added or removed singleton dimensions should not trigger a data copy, but a modification of the mxArray.
         ) {
            // Yes, it does. We need to make a copy of the image into a new MATLAB array.
            mexPrintf( "   Copying data from dip::Image to mxArray\n" ); // TODO: temporary warning, to be removed.
            dip::Image tmp = NewImage();
            tmp.Copy( img );
            //std::cout << "   New image: " << tmp;
            mat = static_cast< mxArray* >( tmp.Data() );
            mla.erase( mat );
         } else {
            // No, it doesn't. Directly return the mxArray.
            mla.erase( it );
            //mexPrintf( "   Retrieving mxArray out of output dip::Image object\n" );
         }

         // Create a MATLAB dip_image object with the mxArray inside.
         // We create an empty object, then set the Array property, because calling the constructor
         // with the mxArray for some reason causes a deep copy of the mxArray.
         mxArray* out;
         mexCallMATLAB( 1, &out, 0, nullptr, imageClassName );
         mxSetPropertyShared( out, 0, arrayPropertyName, mat );
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


//
// Converting mxArray to dip::Image
//


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
inline dip::Image GetImage( mxArray const* mx ) {
   // Find image properties
   bool complex = false;
   bool needCopy = false;
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
            } catch( dip::Error const& ) {
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
         // for a 1D image, we expect one of the two dimensions to be 1. This also handles the case that one of them is 0.
         sizes[ 0 ] = psizes[ 0 ] * psizes[ 1 ];
      } else if( ndims > 1 ) {
         for( dip::uint ii = 0; ii < ndims; ii++ ) {
            sizes[ ii ] = psizes[ ii ];
         }
      }
      // Data Type
      type = mxGetClassID( mxdata );
      complex = mxIsComplex( mxdata );
      if( complex ) {
         // The complex data in an mxArray is stored as two separate memory blocks, and need to be copied to be
         // compatible with DIPlib storage
         needCopy = true;
      }
      // It's never a tensor (`tensor` is scalar by default), and color space nor pixel size are defined
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
         dip::Image real( nullptr, p_real, dt, sizes, strides, tensor, tstride );
         out.Real().Copy( real );
      } else {
         out.Real().Fill( 0 );
      }
      void* p_imag = mxGetImagData( mxdata );
      if( p_imag ) {
         dip::Image imag( nullptr, p_imag, dt, sizes, strides, tensor, tstride );
         out.Imaginary().Copy( imag );
      } else {
         out.Imaginary().Fill( 0 );
      }
      //out.SetPixelSize( pixelSize );
      //out.SetColorSpace( colorSpace ); // these are never defined in this case, the input was a plain matrix.
      return out;
   } else if( datatype.IsBinary() ) {
      // Create Image object
      dip::Image out( dip::NonOwnedRefToDataSegment( mxdata ), mxGetLogicals( mxdata ), datatype, sizes, strides, tensor, tstride );
      out.SetPixelSize( pixelSize );
      out.SetColorSpace( colorSpace );
      return out;
   } else {
      // Create Image object
      dip::Image out( dip::NonOwnedRefToDataSegment( mxdata ), mxGetData( mxdata ), datatype, sizes, strides, tensor, tstride );
      out.SetPixelSize( pixelSize );
      out.SetColorSpace( colorSpace );
      return out;
   }
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
      ~streambuf() {
         std::cout.rdbuf( stdoutbuf );
      }
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
void ToUpper( dip::String& str ) {
   for( auto& c : str ) {
      c = static_cast< char >( std::toupper( static_cast< unsigned char >( c )));
   }
}

/// \brief Convert a string to all lower-case letters, using current locale. Will not work with Unicode strings.
void ToLower( dip::String& str ) {
   for( auto& c : str ) {
      c = static_cast< char >( std::tolower( static_cast< unsigned char >( c )));
   }
}

/// \}

} // namespace dml

#endif
