/*
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

/*
 * Interface:
 *
 * out = colorspacemanager(in,col)
 *    in = input image
 *    col = color space name
 *
 * num = colorspacemanager(col)
 *    col = color space name
 *    num = number of channels for color space
 */

#include "dip_matlab_interface.h"
#include "diplib/color.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {

   static dip::ColorSpaceManager csm;

   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 2 );

      if( nrhs == 1 ) {

         dip::String col = dml::GetString( prhs[ 0 ] );
         dip::uint n = csm.NumberOfChannels( col );
         plhs[ 0 ] = dml::GetArray( n );

      } else {

         dip::Image in = dml::GetImage( prhs[ 0 ] );
         dip::String col = dml::GetString( prhs[ 1 ] );

         if( !in.IsColor() ) {
            if( csm.NumberOfChannels( col ) == in.TensorElements() ) {
               // Set the color space, if correct number of tensor elements
               plhs[ 0 ] = mxDuplicateArray( prhs[ 0 ] );
               col = csm.CanonicalName( col );
               if( col == dip::S::GREY ) {
                  col = "";
               }
               mxSetPropertyShared( plhs[ 0 ], 0, dml::colspPropertyName, dml::GetArray( col ));
               return;
            }
            DIP_THROW_IF( in.TensorElements() > 1, dip::E::INCONSISTENT_COLORSPACE );
         }

         // Convert the color space -- we get here if `in` has a ColorSpace string or if it's scalar.
         dml::MatlabInterface mi;
         dip::Image out = mi.NewImage();
         csm.Convert( in, out, col );
         plhs[ 0 ] = dml::GetArray( out );

      }

   } DML_CATCH
}
