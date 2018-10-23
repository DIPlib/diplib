/*
 * DIPlib 3.0
 * This file contains the definition of line detectors.
 *
 * (c)2018, Cris Luengo.
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

#include "diplib.h"
#include "diplib/detection.h"
#include "diplib/analysis.h"
#include "diplib/linear.h"
#include "diplib/morphology.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/mapping.h"
#include "diplib/generation.h"
#include "diplib/geometry.h"

namespace dip {

void FrangiVesselness(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      FloatArray parameters, // for 3D: { 0.5, 0.5, 500.0 }; for 2D: { 0.5, 15.0 }
      String const& polarity, // S::WHITE, // S::BLACK
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = in.Dimensionality();
   switch( nDims ) {
      case 2:
         if( parameters.empty() ) {
            parameters = { 0.5, 15.0 };
         }
         DIP_THROW_IF( parameters.size() != 2, E::ARRAY_PARAMETER_WRONG_LENGTH );
         DIP_THROW_IF(( parameters[ 0 ] < 0 ) || ( parameters[ 1 ] < 0 ), E::INVALID_PARAMETER );
         break;
      case 3:
         if( parameters.empty() ) {
            parameters = { 0.5, 0.5, 500.0 };
         }
         DIP_THROW_IF( parameters.size() != 3, E::ARRAY_PARAMETER_WRONG_LENGTH );
         DIP_THROW_IF(( parameters[ 0 ] < 0 ) || ( parameters[ 1 ] < 0 ) || ( parameters[ 2 ] < 0 ), E::INVALID_PARAMETER );
         break;
      default:
         DIP_THROW( E::DIMENSIONALITY_NOT_SUPPORTED );
   }
   bool white_vessels;
   DIP_STACK_TRACE_THIS( white_vessels = BooleanFromString( polarity, S::WHITE, S::BLACK ));
   // Compute Hessian eigenvalues, they are sorted |lambdas[1]| >= |lambdas[2]| >= |lambdas[3]|
   Image lambdas;
   DIP_STACK_TRACE_THIS( lambdas = Eigenvalues( Hessian( in, sigmas, S::BEST, boundaryCondition )));
   // A mask to indicate which output values are to be zeroed out
   Image mask = white_vessels ? lambdas[ 0 ] > 0 : lambdas[ 0 ] < 0;
   if( nDims > 2 ) {
      mask |= white_vessels ? lambdas[ 1 ] > 0 : lambdas[ 1 ] < 0;
   }
   // Vesselness measure
   Abs( lambdas, lambdas ); // We no longer need the sign
   Image lambdas_0_1 = nDims == 3 ? lambdas[ 0 ] * lambdas[ 1 ] : Image{}; // Compute the product only for 3D images, not needed in 2D
   Square( lambdas, lambdas ); // All other uses of lambda require squaring
   // Part of R_A (3D) or R_B (2D)
   Image tmp;
   SafeDivide( lambdas[ 1 ], lambdas[ 0 ], tmp );
   tmp /= - 2 * parameters[ 0 ] * parameters[ 0 ];
   if( nDims == 3 ) {
      // R_A
      Exp( tmp, tmp );
      Subtract( 1, tmp, out, tmp.DataType() );
      // R_B
      SafeDivide( lambdas[ 2 ], lambdas_0_1, tmp );
      tmp /= - 2 * parameters[ 1 ] * parameters[ 1 ];
      Exp( tmp, tmp );
      out *= tmp;
   } else {
      // R_B
      Exp( tmp, out );
   }
   // S
   tmp = SumTensorElements( lambdas );
   dfloat c = nDims == 3 ? parameters[ 2 ] : parameters[ 1 ];
   tmp /= - 2 * c * c;
   Exp( tmp, tmp );
   Subtract( 1, tmp, tmp, tmp.DataType() );
   out *= tmp;
   // Mask pixels where eigenvalues have wrong sign
   out.At( mask ) = 0;
}

namespace {

Image CreateMatchedFilter( Image const& coords, dfloat phi, dfloat sigma, dfloat length, bool white_vessels ) {
   Image uv = RotationMatrix2D( phi ) * coords;
   Abs( uv, uv );
   Image mask = ( uv[ 0 ] <= 3 * sigma ) | ( uv[ 1 ] <= length / 2 );
   Image out;
   Square( uv[ 0 ], out );
   out /= - 2 * sigma * sigma;
   Exp( out, out );              // out = exp(-u^2/(2*s^2))
   out -= Mean( out, mask );
   Not( mask, mask );
   out.At( mask ) = 0;
   if( !white_vessels ) {
      Invert( out, out );
   }
   return out;
}

} // namespace

void MatchedFiltersLineDetector2D(
      Image const& in,
      Image& out,
      dip::dfloat sigma,
      dip::dfloat length,
      String const& polarity,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( in.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF(( sigma <= 0 ) || ( length <= 0 ), E::INVALID_PARAMETER );
   bool white_vessels;
   DIP_STACK_TRACE_THIS( white_vessels = BooleanFromString( polarity, S::WHITE, S::BLACK ));
   // Preserve image in case &in==&out
   Image c_in = in;
   if( out.Aliases( c_in )) {
      out.Strip(); // Don't overwrite input data
   }
   // Generate coordinate system:
   dip::uint m = 1 + 2 * static_cast< dip::uint >( std::ceil( std::max( 3 * sigma, length / 2 )));
   Image coords = CreateCoordinates( { m, m } );
   Image kernel = CreateMatchedFilter( coords, 0.0, sigma, length, white_vessels );
   DIP_STACK_TRACE_THIS( GeneralConvolution( c_in, kernel, out, boundaryCondition )); // TODO: depending on the size of the filter, we might prefer using ConvolveFT() here.
   for( dip::uint ii = 1; ii < 12; ++ii ) { // Rotating in steps of 15 degrees, we have 12 different orientations.
      dfloat phi = static_cast< dfloat >( ii ) * 15.0 / 180.0 * pi;
      kernel = CreateMatchedFilter( coords, phi, sigma, length, white_vessels );
      Supremum( out, GeneralConvolution( c_in, kernel, boundaryCondition ), out );
   }
}

void DanielssonLineDetector(
      Image const& in,
      Image& out,
      dip::FloatArray const& sigmas,
      String const& polarity,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   bool white_vessels;
   DIP_STACK_TRACE_THIS( white_vessels = BooleanFromString( polarity, S::WHITE, S::BLACK ));
   // Compute Hessian
   Image H;
   DIP_STACK_TRACE_THIS( Hessian( in, H, sigmas, S::BEST, boundaryCondition ));
   // Compute lineness measure
   dip::uint nDims = in.Dimensionality();
   switch( nDims ) {
      case 2: {
         // 2D case
         // Compute f2 (equation 17 in the paper) -- for white vessels we invert the values in the matrix,
         // which is equivalent to inverting the input image
         Image A( {}, 9, DT_SFLOAT );
         A.ReshapeTensor( 3, 3 );
         if( white_vessels ) {
            A.Fill( { -std::sqrt( 1.0 / 3.0 ), -std::sqrt( 1.0 / 3.0 ),  0.0,
                      -std::sqrt( 2.0 / 3.0 ),  std::sqrt( 2.0 / 3.0 ),  0.0,
                       0.0,                     0.0,                    -std::sqrt( 8.0 / 3.0 ) } );
         } else {
            A.Fill( {  std::sqrt( 1.0 / 3.0 ),  std::sqrt( 1.0 / 3.0 ),  0.0,
                       std::sqrt( 2.0 / 3.0 ), -std::sqrt( 2.0 / 3.0 ),  0.0,
                       0.0,                     0.0,                     std::sqrt( 8.0 / 3.0 ) } );
         }
         H.ReshapeTensorAsVector();
         Multiply( A, H, H );       // H is now f2
         // Computing:
         //    2 * f20 * Sqrt( f21 * f21 + f22 * f22 ) / Sqrt( f20 * f20 + f21 * f21 + f22 * f22 )
         // (equation 33 in the paper)
         Multiply( H[ 0 ], 2.0, out, H.DataType() );
         Square( H, H );            // H is now f2^2
         Image tmp1 = Add( H[ 1 ], H[ 2 ] );
         Image tmp2 = Add( tmp1, H[ 0 ] );
         SafeDivide( tmp1, tmp2, tmp1 );
         Sqrt( tmp1, tmp1 );
         MultiplySampleWise( out, tmp1, out );
         // This is not in the paper -- we want inverted lines to not have a strong negative response
         ClipLow( out, out, 0.0 );
         break;
      }
      case 3: {
         // 3D case
         Image p = Eigenvalues( H );
         H.Strip();
         // Compute p2 (equation 54 in the paper)
         Image A( {}, 9, DT_SFLOAT );
         A.ReshapeTensor( 3, 3 );
         A.Transpose(); // we want to fill row-wise, as it makes for more readable code.
         if( white_vessels ) {
            // The paper assigns p_yy the smallest eigenvalue, and p_zz the middle eigenvalue. We simply swap
            // the last two columns of the matrix A that converts [p_xx, p_yy, p_zz] into [p_20, p_21, p_22].
            A.Fill( {  std::sqrt( 1.0 / 6.0 ),  std::sqrt( 1.0 / 6.0 ),   std::sqrt( 1.0 / 6.0 ),
                       std::sqrt( 5.0 / 6.0 ), -std::sqrt( 5.0 / 24.0 ), -std::sqrt( 5.0 / 24.0 ),
                       0.0,                    -std::sqrt( 5.0 / 8.0 ),   std::sqrt( 5.0 / 8.0 )   } );
         } else {
            // For black vessels, we need to invert the input image. This would lead to an inverted Hessian, which
            // would lead to inverted eigenvalues. We invert the eigenvalues by inverting the elements of the matrix A.
            A.Fill( { -std::sqrt( 1.0 / 6.0 ), -std::sqrt( 1.0 / 6.0 ),  -std::sqrt( 1.0 / 6.0 ),
                      -std::sqrt( 5.0 / 6.0 ),  std::sqrt( 5.0 / 24.0 ),  std::sqrt( 5.0 / 24.0 ),
                       0.0,                     std::sqrt( 5.0 / 8.0 ),  -std::sqrt( 5.0 / 8.0 )   } );
         }
         Multiply( A, p, p );
         // Computing:
         //    4*sqrt(3) * p20 * Sqrt( p21 * p21 + p22 * p22 ) / Sqrt( p20 * p20 + p21 * p21 + p22 * p22 ) * p21 * p22 / ( 3 * p21 * p21 + p22 * p22 )
         // (unnumbered equation just after 68 (next page) in the paper, p_{string})
         // This version is more expensive
         /*
         ProductTensorElements( p, out );
         out *= 4.0 * std::sqrt( 3.0 ); // This bit seems rather useless, no?
         Square( p, p ); // p now contains the squared values
         Image tmp = p[ 1 ] + p[ 2 ];
         Sqrt( tmp, tmp );
         out *= tmp;
         Multiply( p[ 1 ], 3, tmp );
         tmp += p[ 2 ];
         SafeDivide( out, tmp, out );
         SumTensorElements( p, tmp );
         Sqrt( tmp, tmp );
         SafeDivide( out, tmp, out );
         */
         // Computing:
         //    8/sqrt(3) * p20 * p21 * p22 / Sqrt( p20 * p20 + p21 * p21 + p22 * p22 ) / Sqrt( p21 * p21 + p22 * p22 )
         //    = 8/sqrt(3) * p20 * p21 * p22 / Sqrt( ( p20 * p20 + p21 * p21 + p22 * p22 ) * ( p21 * p21 + p22 * p22 ) )
         // (unnumbered equation just after 68 (next page) in the paper, p'_{string})
         // This version is cheaper, and just as good
         ProductTensorElements( p, out );
         out *= 8.0 / std::sqrt( 3.0 ); // This bit seems rather useless, no?
         Square( p, p ); // p now contains the squared values
         Image tmp1 = p[ 1 ] + p[ 2 ];
         Image tmp2 = tmp1 + p[ 0 ];
         tmp1 *= tmp2;
         Sqrt( tmp1, tmp1 );
         SafeDivide( out, tmp1, out );
         // This is not in the paper -- we want inverted lines to not have a strong negative response
         ClipLow( out, out, 0.0 );
         break;
      }
      default:
         DIP_THROW( E::DIMENSIONALITY_NOT_SUPPORTED );
   }
}

namespace {

ImageConstRefArray CreateImageConstRefArray( Image const& a, Image const& b, Image const& c, Image const& d ) {
   dip::ImageConstRefArray out;
   out.reserve( 4 );
   out.emplace_back( a );
   out.emplace_back( b );
   out.emplace_back( c );
   out.emplace_back( d );
   return out;
}

} // namespace

void RORPOLineDetector(
      Image const& in,
      Image& out,
      dip::uint length,
      String const& polarity
) {
   // NOTE: We're following closely the code by Odyssee Merveille for the 3D case, as the paper
   // is not very clear on how to implement the limit cases.
   // https://github.com/path-openings/RORPO/blob/master/libRORPO/include/RORPO/RORPO.hpp
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DataType dt = in.DataType();
   DIP_THROW_IF( !dt.IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   bool white_vessels;
   DIP_STACK_TRACE_THIS( white_vessels = BooleanFromString( polarity, S::WHITE, S::BLACK ));
   // Find the 4/7 orientations we'll use for the path openings
   dip::uint nDims = in.Dimensionality();
   dip::sint l = static_cast< dip::sint >( length );
   std::vector< IntegerArray > directions;
   switch( nDims ) {
      case 2:
         directions.resize( 4 );
         directions[ 0 ] = { l,  0 };
         directions[ 1 ] = { 0,  l };
         directions[ 2 ] = { l,  l };
         directions[ 3 ] = { l, -l };
         break;
      case 3:
         directions.resize( 7 );
         directions[ 0 ] = {  l,  0,  0 }; // Same order as in Odyssee's code
         directions[ 1 ] = {  0,  l,  0 };
         directions[ 2 ] = {  0,  0,  l };
         directions[ 3 ] = {  l,  l,  l };
         directions[ 4 ] = {  l,  l, -l };
         directions[ 5 ] = { -l,  l,  l };
         directions[ 6 ] = {  l, -l,  l };
         break;
      default:
         DIP_THROW( E::DIMENSIONALITY_NOT_SUPPORTED );
   }
   // Compute the 4/7 path openings
   Image RPO;
   RPO.SetSizes( in.Sizes() );
   RPO.SetNormalStrides(); // Strides normal for a scalar image
   RPO.SetTensorSizes( directions.size() );
   RPO.SetTensorStride( static_cast< dip::sint >( in.NumberOfPixels() )); // The tensor dimension comes at the end
   RPO.SetDataType( dt );
   RPO.Forge();
   String pol = white_vessels ? S::OPENING : S::CLOSING;
   // TODO: make this loop be parallel
   for( dip::uint ii = 0; ii < directions.size(); ++ii ) {
      Image tmp = RPO[ ii ];
      tmp.Protect();
      DirectedPathOpening( in, {}, tmp, directions[ ii ], pol, { S::ROBUST } );
   }
   if( nDims == 2 ) {
      Subtract( MaximumTensorElement( RPO ), MinimumTensorElement( RPO ), out, dt );
   } else { // nDims == 3
      // When looking for black vessels, we invert supremum/infimum.
      using InfimumFunc = void (*)( ImageConstRefArray const&, Image& );
      InfimumFunc InfimumX = white_vessels ? static_cast< InfimumFunc >( Infimum ) : static_cast< InfimumFunc >( Supremum );
      InfimumFunc SupremumX = white_vessels ? static_cast< InfimumFunc >( Supremum ) : static_cast< InfimumFunc >( Infimum );
      // Limit cases 4 orientations
      Image Imin4;
      InfimumX( CreateImageConstRefArray( RPO[ 0 ], RPO[ 1 ], RPO[ 3 ], RPO[ 6 ] ), Imin4 );
      Image tmp;
      InfimumX( CreateImageConstRefArray( RPO[ 0 ], RPO[ 1 ], RPO[ 4 ], RPO[ 5 ] ), tmp );
      SupremumX( { Imin4, tmp }, Imin4 );
      InfimumX( CreateImageConstRefArray( RPO[ 0 ], RPO[ 2 ], RPO[ 4 ], RPO[ 6 ] ), tmp );
      SupremumX( { Imin4, tmp }, Imin4 );
      InfimumX( CreateImageConstRefArray( RPO[ 0 ], RPO[ 2 ], RPO[ 3 ], RPO[ 5 ] ), tmp );
      SupremumX( { Imin4, tmp }, Imin4 );
      InfimumX( CreateImageConstRefArray( RPO[ 1 ], RPO[ 2 ], RPO[ 3 ], RPO[ 4 ] ), tmp );
      SupremumX( { Imin4, tmp }, Imin4 );
      InfimumX( CreateImageConstRefArray( RPO[ 1 ], RPO[ 2 ], RPO[ 5 ], RPO[ 6 ] ), tmp );
      SupremumX( { Imin4, tmp }, Imin4 );
      // Limit case 5 orientations
      Image Imin5;
      InfimumX( CreateImageConstRefArray( RPO[ 3 ], RPO[ 4 ], RPO[ 5 ], RPO[ 6 ] ), Imin5 );
      // Rank the RPOs
      SortTensorElements( RPO ); // Largest to smallest, as in the paper, not Odyssee's code
      if( !white_vessels ) {
         RPO = RPO[ Range( -1, 0 ) ]; // For dark vessels, we need to sort the result the other way for it to be comparable
      }
      // Main result
      if( white_vessels ) {
         Subtract( RPO[ 0 ], RPO[ 3 ], out, dt );
      } else {
         Subtract( RPO[ 3 ], RPO[ 0 ], out, dt );
      }
      // Handle limit cases
      pol = white_vessels ? S::DILATION : S::EROSION;
      // Limit cases 4 orientations
      MorphologicalReconstruction( RPO[ 4 ], RPO[ 3 ], tmp, 2, pol ); // Yes, connectivity = 2 in 3D (18 neighbors)
      InfimumX( { Imin4, tmp }, tmp );
      if( white_vessels ) {
         Subtract( Imin4, tmp, tmp, dt );
      } else {
         Subtract( tmp, Imin4, tmp, dt );
      }
      Supremum( { out, tmp }, out );
      // Limit case 5 orientations
      MorphologicalReconstruction( RPO[ 5 ], RPO[ 3 ], tmp, 2, pol ); // Yes, connectivity = 2 in 3D (18 neighbors)
      InfimumX( { Imin5, tmp }, tmp );
      if( white_vessels ) {
         Subtract( Imin5, tmp, tmp, dt );
      } else {
         Subtract( tmp, Imin5, tmp, dt );
      }
      Supremum( { out, tmp }, out );
   }
}

} // namespace dip
