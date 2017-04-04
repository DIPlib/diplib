/*
 * DIPimage 3.0
 * This MEX-file implements the 'eig' function
 *
 * (c)2017, Cris Luengo.
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

#undef DIP__ENABLE_DOCTEST
#include "dip_matlab_interface.h"
#include "diplib/math.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 1 );

      dml::MatlabInterface mi;

      dip::Image in = dml::GetImage( prhs[ 0 ] );

      if( nlhs == 2 ) {
         dip::Image V = mi.NewImage();
         dip::Image D = mi.NewImage();
         dip::EigenDecomposition( in, D, V );
         plhs[ 0 ] = mi.GetArray( V );
         plhs[ 1 ] = mi.GetArray( D );
      } else if( nlhs <= 1 ) {
         dip::Image E = mi.NewImage();
         dip::Eigenvalues( in, E );
         plhs[ 0 ] = mi.GetArray( E );
      } else {
         DIP_THROW( "EIG needs one or two output arguments" );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
