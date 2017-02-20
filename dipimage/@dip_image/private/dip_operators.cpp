/*
 * DIPimage 3.0
 * This MEX-file implements all monadic and diadic operators
 *
 * Input arguments:
 *  prhs[0] -> operator (a character)
 *  prhs[1] -> lhs operand
 *  prhs[2] -> rhs operand (or none if monadic operator)
 * Output arguments:
 *  plhs[0] -> result (duh!)
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#define DOCTEST_CONFIG_IMPLEMENT
#include "dip_matlab_interface.h"


void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   dml::streambuf streambuf;
   try {

      dml::MatlabInterface mi;

      dip::Image lhs;
      dip::Image rhs;
      dip::Image out = mi.NewImage();

      // Get operator
      DML_MIN_ARGS( 2 );
      DIP_THROW_IF( !mxIsChar( prhs[ 0 ] ), "First argument must be a string.");
      mxChar* ch = mxGetChars( prhs[ 0 ] );

      // Get images
      lhs = dml::GetImage( prhs[ 1 ] );
      if(( *ch == '~' ) || ( *ch == 'u' ) || ( *ch == '\'' )) {
         DIP_THROW_IF( nrhs != 2, "Wrong number of input arguments." );
      } else {
         DIP_THROW_IF( nrhs != 3, "Wrong number of input arguments." );
         rhs = dml::GetImage( prhs[ 2 ] );
      }

      // Apply operator
      switch( *ch ) {
      // Arithmetic operators
         case '+': // +=
            dip::Add( lhs, rhs, out, dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ));
            break;
         case '-': // -
            dip::Subtract( lhs, rhs, out, dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ));
            break;
         case '*': // *
            dip::Multiply( lhs, rhs, out, dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ));
            break;
         case '.': // .*
            dip::MultiplySampleWise( lhs, rhs, out, dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ));
            break;
         case '/': // /
            dip::Divide( lhs, rhs, out, dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ));
            break;
      // Comparison operators
         case '=': // ==
            dip::Equal( lhs, rhs, out );
            break;
         case '>': // >
            dip::Greater( lhs, rhs, out );
            break;
         case '<': // <
            dip::Lesser( lhs, rhs, out );
            break;
         case 'g': // >=
            dip::NotLesser( lhs, rhs, out );
            break;
         case 'l': // <=
            dip::NotGreater( lhs, rhs, out );
            break;
         case 'n': // ~=
            dip::NotEqual( lhs, rhs, out );
            break;
      // Boolean (bit-wise) operators
         case '&': // and : for binary images only (but does bit-wise for integers too)
            dip::And( lhs, rhs, out );
            break;
         case '|': // or : for binary images only (but does bit-wise for integers too)
            dip::Or( lhs, rhs, out );
            break;
         case '^': // xor : for binary images only (but does bit-wise for integers too)
            dip::Xor( lhs, rhs, out );
            break;
      // Unary operators
         case '~': // unary not : negate
            dip::Not( lhs, out );
            break;
         case 'u': // unary - : invert
            dip::Invert( lhs, out );
            break;
         case '\'': // unary (post) ' : transpose
            out.Copy( lhs );
            out.Transpose();
            break;
      // That's it!
         default:
            DIP_THROW( "Unknown operator." );
            break;
      }

      // Done
      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
