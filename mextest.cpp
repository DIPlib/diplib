/*
 * Testing MEX-file functionality
 *
 * Compile with:
 *   mex -setup C++
 *   mex -largeArrayDims mextest.cpp -I./include/ ./src/image.cpp ./src/error.cpp ./src/datatypes.cpp CXXFLAGS='$CXXFLAGS -std=c++11'
 */

#include "dip_matlab.h"

void print_info( const dip::Image & img )
{
   mexPrintf( "Image %dD (", img.GetDimensionality() );
   dip::UnsignedArray dims = img.GetDimensions();
   for( dip::uint ii=0; ii<dims.size(); ++ii) {
      mexPrintf( " %d ", dims[ii] );
   }
   mexPrintf( "), strides: (" );
   dip::IntegerArray stride = img.GetStrides();
   for( dip::uint ii=0; ii<stride.size(); ++ii) {
      mexPrintf( " %d ", stride[ii] );
   }
   mexPrintf( ")\n" );
   if( img.IsForged() ) {
      mexPrintf( "   origin pointer: %d", (dip::uint)img.GetData() );
      if( img.HasContiguousData() ) {
         if( img.HasNormalStrides() ) {
            mexPrintf( " (strides are normal)\n" );
         } else {
            mexPrintf( " (strides are contiguous but not normal)\n" );
         }
      }
   } else {
      mexPrintf( "   not forged\n" );
   }
}

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
   try {
      dml::MATLAB_Interface mi0;
      dip::Image img_out0;
      img_out0.SetExternalInterface( &mi0 );
      img_out0.SetDimensions( {3,5} );
      img_out0.Forge();

      if( nrhs > 0 ) {
         dip::Image img_in0 = dml::GetImage( prhs[0] );
         print_info( img_in0 );
      }

      plhs[0] = mi0.GetArray();
   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
