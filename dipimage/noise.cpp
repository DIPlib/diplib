/*
 * DIPimage 3.0
 * This MEX-file implements the `noise` function
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
#include "diplib/generation.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::String type = "gaussian";
      dip::dfloat param1 = 1.0;
      dip::dfloat param2 = 0.0;
      if( nrhs > 1 ) {
         type = dml::GetString( prhs[ 1 ] );
      }
      if( nrhs > 2 ) {
         param1 = dml::GetFloat( prhs[ 2 ] );
      }
      if( nrhs > 3 ) {
         param2 = dml::GetFloat( prhs[ 3 ] );
      }

      dip::Random random;

      if( type == "gaussian" ) {
         dip::GaussianNoise( in, out, random, param1 * param1 );
      } else if( type == "uniform" ) {
         dip::UniformNoise( in, out, random, param1, param2 );
      } else if( type == "poisson" ) {
         dip::PoissonNoise( in, out, random, param1 );
      } else if( type == "binary" ) {
         dip::BinaryNoise( in, out, random, param1, param2 );
      } else if( type == "saltpepper" ) {
         DIP_THROW( dip::E::NOT_IMPLEMENTED );
      } else if( type == "brownian" ) {
         dip::ColoredNoise( in, out, random, param1 * param1 , -2.0 );
      } else if( type == "pink" ) {
         if( param2 <= 0.0 ) {
            param2 = 1.0;
         }
         dip::ColoredNoise( in, out, random, param1 * param1 , -param2 );
      } else if( type == "blue" ) {
         if( param2 <= 0.0 ) {
            param2 = 1.0;
         }
         dip::ColoredNoise( in, out, random, param1 * param1 , param2 );
      } else if( type == "violet" ) {
         dip::ColoredNoise( in, out, random, param1 * param1 , 2.0 );
      } else {
         DIP_THROW( dip::E::INVALID_FLAG );
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
