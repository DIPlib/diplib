/*
 * DIPimage 3.0
 * This MEX-file implements the `imagedisplay` function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#define DOCTEST_CONFIG_IMPLEMENT

#include "dip_matlab_interface.h"
#include "diplib/display.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   char const* wrongParamsStruct = "Wrong params struct.";

   try {

      DML_MIN_ARGS( 5 );
      DML_MAX_ARGS( 5 );

      // TODO: output image should be created using a custom version of MatlabInterface that does not handle complex
      // images, and puts the tensor dimension at the end.
      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

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

      dip::ImageDisplay( in, out, coordinates, dim1, dim2, params );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
