/*
 * DIPimage 3.0
 * This MEX-file implements the `monogenicsignal` function
 *
 * (c)2018, Cris Luengo.
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
      DML_MAX_ARGS( 9 );

      dip::uint nOut = static_cast< dip::uint >( nlhs );
      if( nOut == 0 ) {
         nOut = 1;
      }

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::FloatArray wavelengths = nrhs > 1 ? dml::GetFloatArray( prhs[ 1 ] ) : dip::FloatArray{ 3.0, 24.0 };
      dip::dfloat bandwidth = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 0.41;
      dip::StringArray outputs = nrhs > 3 ? dml::GetStringArray( prhs[ 3 ] ) : dip::StringArray{};
      dip::dfloat noiseThreshold =  nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : 0.2;
      dip::dfloat frequencySpreadThreshold =  nrhs > 5 ? dml::GetFloat( prhs[ 5 ] ) : 0.5;
      dip::dfloat sigmoidParameter =  nrhs > 6 ? dml::GetFloat( prhs[ 6 ] ) : 10.0;
      dip::dfloat deviationGain =  nrhs > 7 ? dml::GetFloat( prhs[ 7 ] ) : 1.5;
      dip::String polarity =  nrhs > 8 ? dml::GetString( prhs[ 8 ] ) : dip::S::BOTH;

      // Check outputs
      if( outputs.empty() ) {
         DIP_THROW_IF( nOut > 1, "Too many output arguments" );
      } else {
         DIP_THROW_IF( nOut != outputs.size(), "Number of selected output images does not match number of output arguments" );
      }

      DIP_THROW_IF( !outputs.empty() && wavelengths.size() < 2, "nFrequencyScales must be at least 2 to compute phase congruency or symmetry" );

      // Compute monogenic signal
      dip::Image ms = mi.NewImage();
      dip::MonogenicSignal( in, ms, wavelengths, bandwidth, dip::S::SPATIAL, dip::S::SPATIAL);

      if( outputs.empty() ) {

         // If no outputs were requested, just return the structure tensor itself

         plhs[ 0 ] = dml::GetArray( ms );

      } else {

         // Otherwise, compute requested outputs

         dip::ImageArray outar( nOut, mi.NewImage() );
         dip::ImageRefArray out = dip::CreateImageRefArray( outar );

         dip::MonogenicSignalAnalysis( ms, out, outputs, noiseThreshold, frequencySpreadThreshold, sigmoidParameter, deviationGain, polarity );

         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            if( outar[ ii ].IsForged() ) {
               plhs[ ii ] = dml::GetArray( outar[ ii ] );
            }
         }

      }

   } DML_CATCH
}
