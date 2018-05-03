/*
 * DIPimage 3.0
 * This MEX-file implements the `mdhistogrammap` function
 *
 * (c)2018, Cris Luengo.
 * Based on original DIPimage code: (c)2006, Michael van Ginkel.
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

void GetBinConfig(
      dip::FloatArray const& bins,
      dip::dfloat& lowerBound,
      dip::dfloat& upperBound,
      dip::uint const& nBins,
      dip::dfloat& binSize
) {
   DIP_THROW_IF( bins.size() != nBins, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
   if( nBins == 1 ) {
      lowerBound = bins[ 0 ] - 0.5;
      upperBound = bins[ 0 ] + 0.5;
      binSize = 1.0; // There's no way to determine the original bin size here. But I guess it really doesn't matter.
   } else {
      binSize = bins[ 1 ] - bins[ 0 ];
      lowerBound = bins[ 0 ] - binSize / 2.0;
      upperBound = lowerBound + static_cast< dip::dfloat >( nBins ) * binSize;
   }
}

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   try {

      DML_MIN_ARGS( 3 );
      DML_MAX_ARGS( 4 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::uint nDims = in.TensorElements();
      dip::Image histImg = dml::GetImage( prhs[ 1 ] );
      DIP_THROW_IF( !histImg.DataType().IsUnsigned(), dip::E::DATA_TYPE_NOT_SUPPORTED );
      DIP_THROW_IF( nDims != histImg.Dimensionality(), dip::E::NTENSORELEM_DONT_MATCH );

      dip::Histogram::ConfigurationArray conf( nDims );
      for( dip::uint ii = 0; ii < nDims; ii++ ) {
         conf[ ii ].nBins = histImg.Size( ii );
      }
      dip::uint N;
      if( mxIsCell( prhs[ 2 ] )) {
         N = mxGetNumberOfElements( prhs[ 2 ] );
         DIP_THROW_IF( N != nDims, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
         for( dip::uint ii = 0; ii < nDims; ii++ ) {
            dip::FloatArray bins = dml::GetFloatArray( mxGetCell( prhs[ 2 ], ii ) );
            DIP_STACK_TRACE_THIS( GetBinConfig( bins, conf[ ii ].lowerBound, conf[ ii ].upperBound, conf[ ii ].nBins, conf[ ii ].binSize ));
         }
      } else {
         DIP_THROW_IF( 1 != nDims, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
         dip::FloatArray bins = dml::GetFloatArray( prhs[ 2 ] );
         DIP_STACK_TRACE_THIS( GetBinConfig( bins, conf[ 0 ].lowerBound, conf[ 0 ].upperBound, conf[ 0 ].nBins, conf[ 0 ].binSize ));
      }

      dip::BooleanArray excludeOutOfBoundValues = { false };
      if( nrhs > 3 ) {
         excludeOutOfBoundValues = dml::GetBooleanArray( prhs[ 3 ] );
      }

      // Create the histogram object and copy the histogram values into it (there's no other way)
      dip::Histogram hist( conf );
      dip::Image histImgNew = hist.GetImage();
      histImgNew.Protect();
      DIP_STACK_TRACE_THIS( histImgNew.Copy( histImg ));

      // Apply the mapping
      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();
      hist.ReverseLookup( in, out, excludeOutOfBoundValues );
      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
