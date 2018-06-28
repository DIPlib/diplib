/*
 * DIPimage 3.0
 * This MEX-file implements the `mdhistogram` function
 *
 * (c)2017-2018, Cris Luengo.
 * Based on original DIPimage code: (c)2001, Michael van Ginkel.
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
#include "diplib/histogram.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 3 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image const mask = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image{};

      dip::Histogram::ConfigurationArray conf;
      if( nrhs > 2 ) {
         DIP_THROW_IF( !mxIsCell( prhs[ 2 ] ), "SPECS parameter must be a cell array" );
         if( !mxIsCell( mxGetCell( prhs[ 2 ], 0 ))) {
            conf.resize( 1 );
            conf[ 0 ] = dml::GetHistogramConfiguration( prhs[ 2 ] );
         } else {
            dip::uint N = mxGetNumberOfElements( prhs[ 2 ] );
            conf.resize( N );
            for( dip::uint ii = 0; ii < N; ++ii ) {
               conf[ ii ] = dml::GetHistogramConfiguration( mxGetCell( prhs[ 2 ], ii ));
            }
         }
      } else {
         // Default configuration
         conf.resize( 1 );
         conf[ 0 ] = { 0.0, 100.0, 100 };
         conf[ 0 ].lowerIsPercentile = true;
         conf[ 0 ].upperIsPercentile = true;
      }

      dip::Histogram hist( in, mask, conf );

      // Copy the histogram bins to an output array.
      dip::Image const& bins = hist.GetImage();
      dip::uint nDims = bins.Dimensionality();
      plhs[ 0 ] = dml::GetArray( bins );

      // Create the optional 2nd output argument bins
      if( nlhs > 1 ) {
         if( nDims == 1 ) {
            plhs[ 1 ] = dml::GetArray( hist.BinCenters( 0 ));
         } else {
            plhs[ 1 ] = mxCreateCellMatrix( nDims, 1 );
            for( dip::uint ii = 0; ii < nDims; ii++ ) {
               mxSetCell( plhs[ 1 ], ii, dml::GetArray( hist.BinCenters( ii )));
            }
         }
      }

   } DML_CATCH
}
