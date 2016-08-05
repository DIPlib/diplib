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
   mexPrintf( "Image %dD (", img.Dimensionality() );
   dip::UnsignedArray dims = img.Dimensions();
   for( dip::uint ii=0; ii<dims.size(); ++ii) {
      mexPrintf( " %d ", dims[ii] );
   }
   mexPrintf( "), %s, strides: (", img.DataType().Name() );
   dip::IntegerArray stride = img.Strides();
   for( dip::uint ii=0; ii<stride.size(); ++ii) {
      mexPrintf( " %d ", stride[ii] );
   }
   mexPrintf( ")\n" );
   if( img.IsForged() ) {
      mexPrintf( "   origin pointer: %lu", (dip::uint)img.Origin() );
      if( img.HasContiguousData() ) {
         if( img.HasNormalStrides() ) {
            mexPrintf( " (strides are normal)" );
         } else {
            mexPrintf( " (strides are contiguous but not normal)" );
         }
      }
   } else {
      mexPrintf( "   not forged" );
   }
   mexPrintf("\n");
}

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
   try {

      mexPrintf("Creating output image img_out0\n");
      dml::MatlabInterface mi;
      dip::Image img_out0 = mi.NewImage();
      img_out0.SetDimensions( {3,5} );
      print_info( img_out0 );
      img_out0.Forge();
      print_info( img_out0 );

      mexPrintf("Reallocating output image img_out0\n");
      img_out0.Strip();
      img_out0.Forge();

      mexPrintf("Copying output image img_out0 to img_out1\n");
      dip::Image img_out1 = img_out0;
      mexPrintf("Reallocating output image img_out1\n");
      img_out1.Strip();
      img_out1.SetDimensions( {2,3} );
      img_out1.Forge();

      if( nrhs > 0 ) {
         mexPrintf("Obtaining input image img_in0\n");
         dip::Image img_in0 = dml::GetImage( prhs[0] );
         print_info( img_in0 );
         mexPrintf("Exiting scope\n");
      }

      mexPrintf("The two output images:\n");
      print_info( img_out0 );
      print_info( img_out1 );

      mexPrintf("Getting the array for img_out0\n");
      plhs[0] = mi.GetArray( img_out0 );

      mexPrintf("Exiting scope\n");

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
