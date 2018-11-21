/*
 * DIPimage 3.0
 * This MEX-file implements all monadic and diadic operators
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
 * out = dip_operators(operator,lhs,rhs)
 *
 * operator = one or two characters (see switch statement below)
 * lhs = first operand
 * rhs = second operand (some operators use only one operand)
 */

#include "dip_matlab_interface.h"
#include "diplib/math.h"

dip::DataType FindDataType( dip::Image const& lhs, dip::Image const& rhs, mxArray const* keepDataType ) {
   if( keepDataType && dml::GetBoolean( keepDataType )) {
      if( rhs.NumberOfPixels() == 1 ) {
         if( rhs.DataType().IsComplex() ) {
            return dip::DataType::SuggestComplex( lhs.DataType() ); // rhs is a single pixel, but complex: use a complex version of lhs's data type
         }
         return lhs.DataType(); // rhs is a single pixel: use lhs's data type
      }
      if( lhs.NumberOfPixels() == 1 ) {
         if( lhs.DataType().IsComplex() ) {
            return dip::DataType::SuggestComplex( rhs.DataType() ); // lhs is a single pixel, but complex: use a complex version of rhs's data type
         }
         return rhs.DataType(); // lhs is a single pixel: use rhs's data type
      }
      return dip::DataType::SuggestDyadicOperation( lhs.DataType(), rhs.DataType() ); // use a data type that can hold the result of the operation
   }
   return dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ); // use a flex data type
}

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      // Get operator
      DML_MIN_ARGS( 2 );
      DIP_THROW_IF( !mxIsChar( prhs[ 0 ] ), "First argument must be a string.");
      mxChar* ch = mxGetChars( prhs[ 0 ] );

      // Get images
      dip::Image lhs = dml::GetImage( prhs[ 1 ], dml::GetImageMode::REFERENCE, dml::ArrayConversionMode::TENSOR_OPERATOR );
      dip::Image rhs;
      if( *ch == 'm' ) {
         DIP_THROW_IF( nrhs != ( ch[ 1 ] == 'p' ? 3 : 2 ), "Wrong number of input arguments." ); // pinv has an optional 2nd input argument
      } else {
         DIP_THROW_IF(( nrhs < 3 ) || ( nrhs > 4 ), "Wrong number of input arguments." );
         rhs = dml::GetImage( prhs[ 2 ], dml::GetImageMode::REFERENCE, dml::ArrayConversionMode::TENSOR_OPERATOR );
      }

      // Get optional 4th argument too
      mxArray const* keepDataType = nrhs > 3 ? prhs[ 3 ] : nullptr;

      // Create output image
      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      // Apply operator
      switch( *ch ) {
      // Arithmetic operators
         case '+': // +
            dip::Add( lhs, rhs, out, FindDataType( lhs, rhs, keepDataType ));
            break;
         case '-': // -
            dip::Subtract( lhs, rhs, out, FindDataType( lhs, rhs, keepDataType ));
            break;
         case '*': // *
            dip::Multiply( lhs, rhs, out, FindDataType( lhs, rhs, keepDataType ));
            break;
         case '.': // .*
            dip::MultiplySampleWise( lhs, rhs, out, FindDataType( lhs, rhs, keepDataType ));
            break;
         case '/': // ./
            dip::Divide( lhs, rhs, out, FindDataType( lhs, rhs, keepDataType ));
            break;
         case '%': // mod
            dip::Modulo( lhs, rhs, out, lhs.DataType() );
            break;
         case '^': // .^
            dip::Power( lhs, rhs, out, FindDataType( lhs, rhs, keepDataType ));
            break;
         case 'A': // atan2
            dip::Atan2( lhs, rhs, out );
            break;
         case 'H': // hypot
            dip::Hypot( lhs, rhs, out );
            break;
         case 'C': // cross
            dip::CrossProduct( lhs, rhs, out );
            break;
         case 'D': // dot
            dip::DotProduct( lhs, rhs, out );
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
         case 'x': // xor : for binary images only (but does bit-wise for integers too)
            dip::Xor( lhs, rhs, out );
            break;
      // Monadic operators
         case 'm': // These are all the monadic operators, defined by the second letter
            switch( ch[ 1 ] ) {
               case '~': // unary not : negate
                  dip::Not( lhs, out );
                  break;
               case '-': // unary - : invert
                  dip::Invert( lhs, out );
                  break;
               case 'a': // abs
                  dip::Abs( lhs, out );
                  break;
               case 'c': // phase -- complex scalar or real vector
                  if( lhs.DataType().IsComplex() ) {
                     dip::Phase( lhs, out );
                  } else {
                     dip::Angle( lhs, out );
                  }
                  break;
               case 'd': // round
                  dip::Round( lhs, out );
                  break;
               case 'e': // ceil
                  dip::Ceil( lhs, out );
                  break;
               case 'f': // floor
                  dip::Floor( lhs, out );
                  break;
               case 'g': // fix
                  dip::Truncate( lhs, out );
                  break;
               case 'h': // sign
                  dip::Sign( lhs, out );
                  break;
               case 'i': // isnan
                  dip::IsNotANumber( lhs, out );
                  break;
               case 'j': // isinf
                  dip::IsInfinite( lhs, out );
                  break;
               case 'k': // isfinite
                  dip::IsFinite( lhs, out );
                  break;
               case 'l': // det
                  dip::Determinant( lhs, out );
                  break;
               case 'm': // inv
                  dip::Inverse( lhs, out );
                  break;
               case 'n': // norm
                  dip::Norm( lhs, out );
                  break;
               case 'o': // trace
                  dip::Trace( lhs, out );
                  break;
               case 'p': { // pinv
                  dip::dfloat tolerance = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 1e-7;
                  dip::PseudoInverse( lhs, out, tolerance );
                  break; }
               case 'A': // cos
                  dip::Cos( lhs, out );
                  break;
               case 'B': // sin
                  dip::Sin( lhs, out );
                  break;
               case 'C': // tan
                  dip::Tan( lhs, out );
                  break;
               case 'D': // acos
                  dip::Acos( lhs, out );
                  break;
               case 'E': // asin
                  dip::Asin( lhs, out );
                  break;
               case 'F': // atan
                  dip::Atan( lhs, out );
                  break;
               case 'G': // cosh
                  dip::Cosh( lhs, out );
                  break;
               case 'H': // sinh
                  dip::Sinh( lhs, out );
                  break;
               case 'I': // tanh
                  dip::Tanh( lhs, out );
                  break;
               case '1': // sqrt
                  dip::Sqrt( lhs, out );
                  break;
               case '2': // exp
                  dip::Exp( lhs, out );
                  break;
               case '3': // pow10
                  dip::Exp10( lhs, out );
                  break;
               case '4': // pow2
                  dip::Exp2( lhs, out );
                  break;
               case '5': // log
                  dip::Ln( lhs, out );
                  break;
               case '6': // log10
                  dip::Log10( lhs, out );
                  break;
               case '7': // log2
                  dip::Log2( lhs, out );
                  break;
               case '!': // erf
                  dip::Erf( lhs, out );
                  break;
               case '@': // erfc
                  dip::Erfc( lhs, out );
                  break;
               case '#': // gammaln
                  dip::LnGamma( lhs, out );
                  break;
               default:
                  DIP_THROW( "Unknown operator." );
            }
            break;
      // That's it!
         default:
            DIP_THROW( "Unknown operator." );
            break;
      }

      // Done
      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
