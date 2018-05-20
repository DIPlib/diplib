/*
 * DIPimage 3.0
 * This MEX-file implements the `granulometry` function
 *
 * (c)2018, Cris Luengo.
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

void HandlePolarityAlias( dip::String& polarity ) {
   if( polarity == "dark" ) {
      polarity = dip::S::CLOSING;
   } else if( polarity == "light" ) {
      polarity = dip::S::OPENING;
   }
}

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image mask;
      std::vector< dip::dfloat > scales;
      dip::String type = "isotropic";
      dip::String polarity = dip::S::OPENING;
      dip::StringSet options;

      if(( nrhs > 1) && !mxIsEmpty( prhs[ 1 ] ) && mxIsDouble( prhs[ 1 ] ) && dml::IsVector( prhs[ 1 ] )) {
         // Old-style params: in,scales,minimumFilterSize,maximumFilterSize,minimumZoom,maximumZoom,options,polarity
         DML_MAX_ARGS( 8 );
         scales = dml::GetStdVectorOfFloats( prhs[ 1 ] );
         if( nrhs > 6 ) {
            options = dml::GetStringSet( prhs[ 6 ] ); // this one first, so we can add to it later.
         }
         // Ignore parameters 2 and 3.
         if( nrhs > 4 ) {
            dip::dfloat minimumZoom = dml::GetFloat( prhs[ 4 ] );
            if( minimumZoom != 1 ) {
               options.insert( "subsample" );
            }
         }
         if( nrhs > 5 ) {
            dip::dfloat maximumZoom = dml::GetFloat( prhs[ 5 ] );
            if( maximumZoom != 1 ) {
               options.insert( "interpolate" );
            }
         }
         if( nrhs > 7 ) {
            polarity = dml::GetString( prhs[ 7 ] );
            HandlePolarityAlias( polarity );
         } else {
            polarity = dip::S::CLOSING;
         }
      } else {
         // New-style params: in,mask,scales,type,polarity,options
         DML_MAX_ARGS( 6 );
         if( nrhs > 1 ) {
            mask = dml::GetImage( prhs[ 1 ] );
         }
         if( nrhs > 2 ) {
            scales = dml::GetStdVectorOfFloats( prhs[ 2 ] );
         }
         if( nrhs > 3 ) {
            type = dml::GetString( prhs[ 3 ] );
         }
         if( nrhs > 4 ) {
            polarity = dml::GetString( prhs[ 4 ] );
            HandlePolarityAlias( polarity );
         }
         if( nrhs > 5 ) {
            options = dml::GetStringSet( prhs[ 5 ] );
         }
      }

      dip::Distribution out = dip::Granulometry( in, mask, scales, type, polarity, options );

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
