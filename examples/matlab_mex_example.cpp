/*
 * This is an example of a MEX-file that links to DIPlib. It's more or less trivial code
 * that quantizes an input image to a given number of levels.
 *
 * Please read the documentation to the functions in the `dml` namespace to learn about
 * conversion of MATLAB arrays to DIPlib data types.
 *
 * Compile under MATLAB with the `dipmex` command:
 *     dipmex matlab_mex_example.cpp
 * then run the MEX-file as you would any other function:
 *     matlab_mex_example(img,16)
 */

#include <dip_matlab_interface.h> // Always include this, it includes diplib.h and mex.h
#include <diplib/mapping.h> // for dip::ContrastStretch
#include <diplib/math.h> // for dip::Floor

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 ); // This ensures there is at least one input argument
      DML_MAX_ARGS( 2 ); // This ensures there are not more than two input arguments

      // The first input argument is an image
      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      // The second input argument is a double parameter
      dip::dfloat param = 16; // default value
      if( nrhs > 1 ) {
         param = dml::GetFloat( prhs[ 1 ] );
         DIP_THROW_IF(( param < 1 || param > 128 ), dip::E::PARAMETER_OUT_OF_RANGE );
      }

      // Create an output image
      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();
      //    Remember to not assign to this image, but pass it as the output:
      //       out = dip::Gauss( in ); // BAD! This will copy the result of the operation into a MATLAB array
      //       dip::Gauss( in, out );  // GOOD! The function will directly write into a MATLAB array

      // Call DIPlib functions
      dip::ContrastStretch( in, out );
      out /= ( 256.0 / param );
      dip::Floor( out, out );
      out *= ( 256.0 / param );

      // Retrieve the MATLAB array for the output image (it's a dip_image object)
      plhs[ 0 ] = dml::GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
