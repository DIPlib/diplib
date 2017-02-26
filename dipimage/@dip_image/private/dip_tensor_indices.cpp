/*
 * DIPimage 3.0
 * This MEX-file that gives a look up table for the linear index of image's tensor components
 *
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
 * out = dip_tensor_indices(image)
 *    out is a look-up table for image's tensor. For a tensor with M rows, element at (i,j) is out(i+j*M).
 *
 * out = dip_tensor_indices(images,[i,j])
 *    out is the linear index into the tensor for element (i,j) (i.e. the value of element i+j*M in the look-up table
 *    that would be generaged in the call of the first form.
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include "dip_matlab_interface.h"


void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {
      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 2 );

      // Get tensor information from dip_image object
      DIP_THROW_IF( !mxIsClass( prhs[ 0 ], dml::imageClassName ), "First input argument must be a dip_image object" );
      dip::UnsignedArray tsize = dml::GetUnsignedArray( mxGetPropertyShared( prhs[ 0 ], 0, dml::tsizePropertyName ));
      DIP_THROW_IF( tsize.size() != 2, "Error in tensor size property" );
      enum dip::Tensor::Shape tshape = dml::GetTensorShape( mxGetPropertyShared( prhs[ 0 ], 0, dml::tshapePropertyName ));
      dip::Tensor tensor( tshape, tsize[ 0 ], tsize[ 1 ] );

      if( nrhs == 2 ) {

         // Get indices
         dip::UnsignedArray indices = dml::GetUnsignedArray( prhs[ 1 ] );
         DIP_START_STACK_TRACE
            dip::uint out = tensor.Index( indices );
            plhs[ 0 ] = dml::GetArray( out );
         DIP_END_STACK_TRACE

      } else {

         // Get the look up table
         std::vector< dip::sint > lut = tensor.LookUpTable();
         plhs[ 0 ] = mxCreateDoubleMatrix( 1, lut.size(), mxREAL );
         auto data = mxGetPr( plhs[ 0 ] );
         for( dip::uint ii = 0; ii < lut.size(); ++ii ) {
            data[ ii ] = lut[ ii ];
         }

      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
