/*
 * DIPimage 3.0
 * This MEX-file implements the `convolve` function
 *
 * (c)2017-2018, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

#include "dip_matlab_interface.h"
#include "diplib/linear.h"

namespace {

// Convert a real or complex floating-point array for `mxArray` to `std::vector<dip::dfloat>` by copy.
// If the array was complex, the output contains real and imaginary elements interleaved.
inline std::vector< dip::dfloat > GetRealOrComplexArray( mxArray const* mx ) {
   if( mxIsDouble( mx ) && dml::IsVector( mx )) {
      bool isComplex = mxIsComplex( mx );
      dip::uint k = isComplex ? 2 : 1;
      dip::uint n = mxGetNumberOfElements( mx );
      std::vector< dip::dfloat > out( k * n );
      double* data = mxGetPr( mx );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         out[ k * ii ] = data[ ii ];
      }
      if( isComplex ) {
         data = mxGetPi( mx );
         for( dip::uint ii = 0; ii < n; ++ii ) {
            out[ k * ii + 1 ] = data[ ii ];
         }
      }
      return out;
   }
   DIP_THROW( "Real- or complex-valued floating-point array expected" );
}

dip::OneDimensionalFilter GetFilter(
      mxArray const* mxFilter, // A struct array
      dip::uint ii
) {
   dip::OneDimensionalFilter out;
   mxArray const* elem = mxGetField( mxFilter, ii, "filter" );
   DIP_THROW_IF( !elem, "" );
   out.filter = GetRealOrComplexArray( elem );
   out.isComplex = mxIsComplex( elem );

   elem = mxGetField( mxFilter, ii, "origin" );
   if( elem ) {
      out.origin = dml::GetInteger( elem );
   }

   elem = mxGetField( mxFilter, ii, "flags" );
   if( elem ) {
      out.symmetry = dml::GetString( elem );
   }

   return out;
}

} // namespace

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   char const* wrongFilter = "Wrong filter definition";

   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::StringArray bc;
      if( nrhs > 2 ) {
         bc = dml::GetStringArray( prhs[ 2 ] );
      }

      dip::OneDimensionalFilterArray filterArray;
      mxArray const* mxFilter = prhs[ 1 ];
      if( mxIsNumeric( mxFilter ) || mxIsClass( mxFilter, "dip_image" )) {

         dip::Image const filter = dml::GetImage( mxFilter );
         filterArray = dip::SeparateFilter( filter );
         if( filterArray.empty() ) {

            if( filter.NumberOfPixels() > 7 * 7 ) { // note that this is an arbitrary threshold, and should probably depend also on log2(image size)
               dip::ConvolveFT( in, filter, out );
            } else {
               dip::GeneralConvolution( in, filter, out, bc );
            }

         } else {

            dip::SeparableConvolution( in, out, filterArray, bc );

         }

      } else {

         if( mxIsCell( mxFilter )) {
            DIP_THROW_IF( !dml::IsVector( mxFilter ), wrongFilter );
            dip::uint n = mxGetNumberOfElements( mxFilter );
            filterArray.resize( n );
            for( dip::uint ii = 0; ii < n; ++ii ) {
               mxArray const* elem = mxGetCell( mxFilter, ii );
               try {
                  filterArray[ ii ].filter = GetRealOrComplexArray( elem );
                  filterArray[ ii ].isComplex = mxIsComplex( elem );
               } catch( dip::Error& ) {
                  DIP_THROW( wrongFilter );
               }
            }
         } else if( mxIsStruct( mxFilter )) {
            dip::uint n = mxGetNumberOfElements( mxFilter );
            filterArray.resize( n );
            for( dip::uint ii = 0; ii < n; ++ii ) {
               try {
                  filterArray[ ii ] = GetFilter( mxFilter, ii );
               } catch( dip::Error& ) {
                  DIP_THROW( wrongFilter );
               }
            }
         }

         dip::SeparableConvolution( in, out, filterArray, bc );

      }

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
