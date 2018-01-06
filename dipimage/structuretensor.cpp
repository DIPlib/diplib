/*
 * DIPimage 3.0
 * This MEX-file implements the `structuretensor` function
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

#include "dip_matlab_interface.h"
#include "diplib/analysis.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 7 );

      dip::uint nOut = static_cast< dip::uint >( nlhs );
      if( nOut == 0 ) {
         nOut = 1;
      }

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::FloatArray gradientSigmas = { 1.0 };
      if( nrhs > 1 ) {
         gradientSigmas = dml::GetFloatArray( prhs[ 1 ] );
      }
      dip::FloatArray tensorSigmas = { 5.0 };
      if( nrhs > 2 ) {
         tensorSigmas = dml::GetFloatArray( prhs[ 2 ] );
      }
      dip::StringArray outputs = {};
      if( nrhs > 3 ) {
         outputs = dml::GetStringArray( prhs[ 3 ] );
      }
      dip::String method = dip::S::BEST;
      if( nrhs > 4 ) {
         method = dml::GetString( prhs[ 4 ] );
      }
      dip::StringArray bc = {};
      if( nrhs > 5 ) {
         bc = dml::GetStringArray( prhs[ 5 ] );
      }
      dip::dfloat truncation = 3;
      if( nrhs > 6 ) {
         truncation = dml::GetFloat( prhs[ 6 ] );
      }

      // Check outputs
      if( outputs.empty() ) {
         DIP_THROW_IF( nOut > 1, "Too many output arguments" );
      } else {
         DIP_THROW_IF( nOut != outputs.size(), "Number of selected output images does not match number of output arguments" );
      }

      // Compute structure tensor
      dip::Image st = mi.NewImage();
      dip::StructureTensor( in, {}, st, gradientSigmas, tensorSigmas, method, bc, truncation );

      if( outputs.empty() ) {

         // If no outputs were requested, just return the structure tensor itself

         plhs[ 0 ] = mi.GetArray( st );

      } else {

         // Otherwise, compute requested outputs

         dip::ImageArray outar( nOut, mi.NewImage() );
         dip::ImageRefArray out;
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            out.emplace_back( outar[ ii ] );
         }

         dip::StructureTensorAnalysis( st, out, outputs );

         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            plhs[ ii ] = mi.GetArray( out[ ii ] );
         }

      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
