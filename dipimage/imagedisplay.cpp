/*
 * DIPimage 3.0
 * This MEX-file implements the `imagedisplay` function
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
 * out = imagedisplay(in,coordinates,dim1,dim2,params)
 *
 * params = struct with:
 *    mode = "lin", "log", "based"
 *    complex = "mag"/"abs", "phase", "real", "imag"
 *    projection = "slice", "max", "mean"
 *    lowerBound = 0.0
 *    upperBound = 1.0
 *
 * There's no defaults, everything must be given. `out` is a plain old MATLAB array, uint8.
 * This function is intended for use solely within DIPSHOW.
 */

#undef DIP__ENABLE_DOCTEST
#include "dip_matlab_interface.h"
#include "diplib/display.h"

// This is a very simple external interface: it can be used for only one image, which should be
// forged only once!
class MatlabInterfaceUInt8 : public dip::ExternalInterface {
   public:
      mxArray* array = nullptr;
      virtual std::shared_ptr< void > AllocateData(
            dip::UnsignedArray const& sizes,
            dip::IntegerArray& strides,
            dip::Tensor const& tensor,
            dip::sint& tstride,
            dip::DataType datatype
      ) override {
         DIP_THROW_IF( datatype != dip::DT_UINT8, dip::E::DATA_TYPE_NOT_SUPPORTED );
         DIP_THROW_IF( sizes.size() != 2, dip::E::DIMENSIONALITY_NOT_SUPPORTED );
         dip::UnsignedArray mlsizes( 3 );
         mlsizes[ 0 ] = sizes[ 1 ];
         mlsizes[ 1 ] = sizes[ 0 ];
         mlsizes[ 2 ] = tensor.Elements();
         strides.resize( 2 );
         strides[ 0 ] = sizes[ 1 ];
         strides[ 1 ] = 1;
         tstride = sizes[ 0 ] * sizes[ 1 ];
         array = mxCreateNumericArray( mlsizes.size(), mlsizes.data(), mxUINT8_CLASS, mxREAL );
         void* p = mxGetData( array );
         return std::shared_ptr< void >( p, dml::VoidStripHandler );
      }
};

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   char const* wrongParamsStruct = "Wrong params struct.";

   try {

      DML_MIN_ARGS( 5 );
      DML_MAX_ARGS( 5 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::UnsignedArray coordinates = dml::GetUnsignedArray( prhs[ 1 ] );
      dip::uint dim1 = dml::GetUnsigned( prhs[ 2 ] );
      dip::uint dim2 = dml::GetUnsigned( prhs[ 3 ] );

      dip::ImageDisplayParams params;
      DIP_THROW_IF( !mxIsStruct( prhs[ 4 ] ), wrongParamsStruct );
      mxArray* mode = mxGetField( prhs[ 4 ], 0, "mode" );
      DIP_THROW_IF( !mode, wrongParamsStruct );
      params.mode = dml::GetString( mode );
      mxArray* complex = mxGetField( prhs[ 4 ], 0, "complex" );
      DIP_THROW_IF( !complex, wrongParamsStruct );
      params.complex = dml::GetString( complex );
      mxArray* projection = mxGetField( prhs[ 4 ], 0, "projection" );
      DIP_THROW_IF( !projection, wrongParamsStruct );
      params.projection = dml::GetString( projection );
      mxArray* lowerBound = mxGetField( prhs[ 4 ], 0, "lowerBound" );
      DIP_THROW_IF( !lowerBound, wrongParamsStruct );
      params.lowerBound = dml::GetFloat( lowerBound );
      mxArray* upperBound = mxGetField( prhs[ 4 ], 0, "upperBound" );
      DIP_THROW_IF( !upperBound, wrongParamsStruct );
      params.upperBound = dml::GetFloat( upperBound );

      MatlabInterfaceUInt8 allocator;
      dip::Image out;
      out.SetExternalInterface( &allocator );

      dip::ImageDisplay( in, out, coordinates, dim1, dim2, params );

      plhs[ 0 ] = allocator.array;

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
