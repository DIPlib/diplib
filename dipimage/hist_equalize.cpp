/*
 * DIPimage 3.0
 * This MEX-file implements the `hist_equalize` function
 *
 * (c)2018, Cris Luengo.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
 *                                  (c)2013, Patrik Malm & Cris Luengo.
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
#include <diplib/mapping.h>
#include <diplib/histogram.h>
#include <diplib/iterators.h>

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 2 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      if(( nrhs > 1 ) && !mxIsEmpty( prhs[ 1 ] ) && !dml::IsScalar( prhs[ 1 ] )) {
         // Get input as float array, and convert to something we can stick in a histogram
         dip::FloatArray data = dml::GetFloatArray( prhs[ 1 ] );
         dip::dfloat minV = std::numeric_limits< double >::max();
         dip::dfloat maxV = 0;
         for( auto v : data ) {
            if( v < 0 ) {
               v = 0;
            } else if( v > 0 ) {
               minV = std::min( minV, v );
               maxV = std::max( maxV, v );
            }
         }
         dip::dfloat scale = std::min( 1.0 / minV,
                                       static_cast< dip::dfloat >( std::numeric_limits< dip::Histogram::CountType >::max() ) / maxV );
         for( auto& v : data ) {
            v *= scale;
         }
         // Create a histogram of the right dimensions
         dip::Histogram::Configuration config( 0.0, static_cast< int >( data.size() ), 1.0 );
         dip::Histogram example( config );
         // Fill it with the input
         dip::Image img = example.GetImage().QuickCopy();
         DIP_ASSERT( img.NumberOfPixels() == data.size() );
         dip::ImageIterator< dip::Histogram::CountType > imit( img );
         for( auto dait = data.begin(); dait != data.end(); ++dait, ++imit ) {
            *imit = static_cast< dip::Histogram::CountType >( *dait );
         }
         // Call function
         dip::HistogramMatching( in, out, example );
      } else {
         dip::uint nBins = 256;
         if(( nrhs > 1 ) && !mxIsEmpty( prhs[ 1 ] ) ) {
            nBins = dml::GetUnsigned( prhs[ 1 ] );
         }
         dip::HistogramEqualization( in, out, nBins );
      }

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
