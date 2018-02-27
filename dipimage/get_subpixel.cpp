/*
 * DIPimage 3.0
 * This MEX-file implements the `get_subpixel` function
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/geometry.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 3 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::FloatCoordinateArray coords = dml::GetFloatCoordinateArray( prhs[ 1 ] );

      dip::String mode = "linear";
      if( nrhs > 2 ) {
         mode = dml::GetString( prhs[ 2 ] );
         if( mode == "spline" ) {
            mode = "cubic";
         }
      }

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();
      out.SetDataType( in.DataType().IsComplex() ? dip::DT_DCOMPLEX : dip::DT_DFLOAT );
      out.SetSizes( { in.TensorElements(), coords.size() } ); // Creates 1x1xNxT matrix
      out.Forge();
      out.SpatialToTensor( 0 ); // Out is 1D image with right number of tensor elements
      out.Protect();

      dip::ResampleAt( in, out, coords, mode );

      out.TensorToSpatial( 0 ); // Return to original shape
      plhs[ 0 ] = mi.GetArrayAsArray( out );
      // the image has 2, 3 or 4 dimensions, we want to get rid of the first two singleton dimension
      mwSize nDims = mxGetNumberOfDimensions( plhs[ 0 ] );
      const mwSize* dims = mxGetDimensions( plhs[ 0 ] );
      DIP_ASSERT( dims[ 0 ] == 1 );
      DIP_ASSERT( dims[ 1 ] == 1 );
      mwSize newDims[ 2 ] = { nDims > 2 ? dims[ 2 ] : 1, nDims > 3 ? dims[ 3 ] : 1 };
      mxSetDimensions( plhs[ 0 ], newDims, 2 );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
