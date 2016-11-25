/*
 * Testing assorted DIPlib functionality
 *
 */

#include <iostream>
#include "dip_matlab_interface.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   if( nrhs != 2 ) {
      mexErrMsgTxt( "Two input images expected" );
   }

   try {

      dml::MatlabInterface mi;

      //mexPrintf( "About to get input images:\n" );
      dip::Image const in1 = dml::GetImage( prhs[ 0 ] );

      dip::Image const in2 = dml::GetImage( prhs[ 1 ] );
      //std::cout << in2 << std::endl;

      //mexPrintf( "About to create output images:\n" );
      dip::Image out = mi.NewImage();
      //out.CopyProperties( in1 );
      out.Copy( in1 ); // so we can modify the image

      //mexPrintf( "About to call Forge() on output image:\n" );
      //out.Forge();
      //out.Copy( in1.At( 0 ) );
      //out.Copy( in1 );
      //out = out.At( 0 );

      mexPrintf( "About to call the DIPlib function:\n" );
      out.Fill(56.0e12);
      dip::Add( in1, in2, out, dip::DataType::SuggestArithmetic( in1.DataType(), in2.DataType() ) );

      std::cout << "\nAt():\n";
      out = out.At( dip::Range(0,-1,2), dip::Range(0,4,3) );

      //out.CopyProperties( tmp );
      //out.SetDataType( dip::DT_SINT16 );
      //out.Forge();
      //std::cout << "\nCopy():\n";
      //out.Copy( tmp );
      std::cout << out << double(out) << std::endl;

      std::cout << "\nConvert():\n";
      out.Convert( dip::DT_SINT16 );
      std::cout << out << double(out) << std::endl;

      mexPrintf( "About to extract mxArray from output image:\n" );
      plhs[ 0 ] = mi.GetArray( out );

      mexPrintf( "End of scope for interface object\n" );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
