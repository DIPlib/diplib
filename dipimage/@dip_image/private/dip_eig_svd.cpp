/*
 * DIPimage 3.0
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

#include "diplib/math.h"

namespace {

void eig( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   DML_MAX_ARGS( 2 );
   dml::MatlabInterface mi;
   dip::Image in = dml::GetImage( prhs[ 0 ] );

   if( nrhs == 2 ) {
      // V1 = EIG(A,'largest') or VN = EIG(A,'smallest')

      dip::String mode = dml::GetString( prhs[ 1 ] );
      dip::Image V = mi.NewImage();
      if( mode == "largest" ) {
         dip::LargestEigenVector( in, V );
      } else if( mode == "smallest" ) {
         dip::SmallestEigenVector( in, V );
      } else {
         DIP_THROW_INVALID_FLAG( mode );
      }
      plhs[ 0 ] = dml::GetArray( V );

   } else if( nlhs > 1 ) {
      // [V,D] = EIG(A)

      dip::Image V = mi.NewImage();
      dip::Image D = mi.NewImage();
      dip::EigenDecomposition( in, D, V );
      plhs[ 0 ] = dml::GetArray( V );
      plhs[ 1 ] = dml::GetArray( D );

   } else {
      // E = EIG(A)

      dip::Image E = mi.NewImage();
      dip::Eigenvalues( in, E );
      plhs[ 0 ] = dml::GetArray( E );
   }
}

void svd( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   DML_MAX_ARGS( 1 );
   dml::MatlabInterface mi;
   dip::Image in = dml::GetImage( prhs[ 0 ] );
   if( nlhs == 3 ) {
      dip::Image S = mi.NewImage();
      dip::Image U = mi.NewImage();
      dip::Image V = mi.NewImage();
      dip::SingularValueDecomposition( in, U, S, V );
      plhs[ 0 ] = dml::GetArray( U );
      plhs[ 1 ] = dml::GetArray( S );
      plhs[ 2 ] = dml::GetArray( V );
   } else if( nlhs <= 1 ) {
      dip::Image S = mi.NewImage();
      dip::SingularValues( in, S );
      plhs[ 0 ] = dml::GetArray( S );
   } else {
      DIP_THROW( "SVD needs one or three output arguments" );
   }
}

} // namespace

// Gateway function

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 2 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      prhs += 1;
      nrhs -= 1;

      if( function == "eig" ) {
         eig( nlhs, plhs, nrhs, prhs );
      } else if( function == "svd" ) {
         svd( nlhs, plhs, nrhs, prhs );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

   } DML_CATCH
}
