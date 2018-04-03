/*
 * DIPlib 3.0
 * This file contains definitions of functions for adaptive Gaussian filtering.
 *
 * (c)2018, Erik Schuitema.
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

#include "diplib/nonlinear.h"
#include "diplib/framework.h"
#include "diplib/generation.h"
#include "diplib/overload.h"
#include "diplib/pixel_table.h"
#include "diplib/private/constfor.h"

#if defined(__GNUG__) || defined(__clang__)
// For this file, turn off -Wsign-conversion, Eigen is really bad at this!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#if __GNUC__ >= 7
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#endif
#endif

#include <Eigen/Geometry>

namespace dip {

namespace {

// KernelTransform: transforms kernel pixel coordinates according to one or more parameter images
// The base class performs no specific transformation,
// other than simply adding kernel coordinates to the current image coordinates
class KernelTransform
{
public:
   // Virtual destructor
   virtual ~KernelTransform() {}

   // Clone the kernel transform. This is done when creating a copy for each thread
   // to avoid unwanted sharing between threads of the members altered inside SetImageCoords().
   virtual KernelTransform* Clone() const { return new KernelTransform( *this ); }

   virtual void SetImageCoords( UnsignedArray const& imgCoords ) {
      // Cast-copy
      imgCoords_.resize( imgCoords.size() );
      for( UnsignedArray::size_type ii = 0; ii < imgCoords.size(); ++ii ) {
         imgCoords_[ ii ] = static_cast< dfloat >( imgCoords[ ii ] );
      }
      // Note for writing derived classes: perform any parameter computation or other preparation here,
      // so it is done only once per input pixel.
   }

   // Transforms kernel coordinates to input image coordinates
   virtual void Transform( IntegerArray const& kernelCoords, dip::uint /*tensorIndex*/, FloatArray& transformedCoords ) const {
      transformedCoords[ 0 ] = imgCoords_[ 0 ] + static_cast< dfloat >( kernelCoords[ 0 ] );
      transformedCoords[ 1 ] = imgCoords_[ 1 ] + static_cast< dfloat >( kernelCoords[ 1 ] );
   }

protected:
   FloatArray imgCoords_;
};

// Scaling helper class
template< dip::uint nDims>
class KernelTransformScale
{
public:
   KernelTransformScale( Image const& kernelScale, dip::uint inputTensorElements )
         : scaleAtImgCoords_( inputTensorElements ), inputTensorElements_( inputTensorElements ) {
      // The kernel scale image must be a COL_MAJOR_MATRIX tensor image with the following tensor size:
      // Rows == input tensor size (the input image has a column vector tensor)
      // Cols == input dimensionality == kernelScaleDimensionality
      kernelScale_ = kernelScale.QuickCopy();
      dip::uint tensorRows = inputTensorElements;
      dip::uint tensorCols = kernelScale.Dimensionality();
      // See if we need to reshape the tensor
      if( kernelScale_.TensorElements() == 1 ) {
         // Singleton expansion to COL_MAJOR_MATRIX tensor
         kernelScale_.ExpandSingletonTensor( tensorRows * tensorCols );
         kernelScale_.ReshapeTensor( tensorRows, tensorCols );
         scaleTensorLUT_ = kernelScale_.Tensor().LookUpTable();
      } else if( inputTensorElements == 1 && kernelScale_.TensorShape() == Tensor::Shape::COL_VECTOR ) {
         // The scale tensor is a col vector but must be a row vector
         kernelScale_.ReshapeTensor( tensorRows, tensorCols );
         scaleTensorLUT_ = kernelScale_.Tensor().LookUpTable();
      } else if( kernelScale_.TensorColumns() != tensorCols  && kernelScale_.TensorRows() == tensorRows ) {
         // Only the row size matches: tensor contains a scalar per input tensor element -> create special LUT but leave the tensor itself unchanged
         // LUT is indexed as: [col * NUM_ROWS + row]
         scaleTensorLUT_ = kernelScale_.Tensor().LookUpTable(); // LUT for column vector
         //kernelScale_.ExpandSingletonTensor(tensorRows * tensorCols);
         //kernelScale_.ReshapeTensor( tensorRows, tensorCols );
         // Complete LUT by adding the same indices for each column
         dip::sint LUTColSize = static_cast< dip::sint >( scaleTensorLUT_.size() );
         for( dip::uint iDim = 1; iDim < nDims; ++iDim ) {
            scaleTensorLUT_.insert( scaleTensorLUT_.end(), scaleTensorLUT_.begin(), scaleTensorLUT_.begin() + LUTColSize );
         }
      }
      else {
         // Dimensions are ok, only create LUT
         scaleTensorLUT_ = kernelScale_.Tensor().LookUpTable();
      }

      // Multiply the LUT elements with the tensor stride so that we don't have to do that later
      for( dip::uint iLUT = 0; iLUT < scaleTensorLUT_.size(); ++iLUT ) {
         scaleTensorLUT_[ iLUT ] *= kernelScale_.TensorStride();
      }
   }

   // Computes scaleAtImgCoords_.
   // Prerequisite: SetImageCoords() must have been called to populate the image coords
   void SetScaleAtImgCoords( UnsignedArray const& imgCoords ) {
      /// Given a tensor with `M` rows and `N` columns, tensor element `(m,n)` can
      /// be found by adding `Tensor::LookUpTable()[n*M+m] * tstride` to the pixel's pointer.
      for( dip::uint iTE = 0; iTE < inputTensorElements_; ++iTE ) {
         dip::uint scaleTensorRowIndex = iTE; // The scale tensor has a row for each input tensor element.
         dip::sint scaleOffset = kernelScale_.Offset( imgCoords );
         for( dip::uint iDim = 0; iDim < nDims; ++iDim ) {
            dip::uint scaleTensorColIndex = iDim; // The scale tensor has a column for each kernel/input dimension ( 0 for X, 1 for Y ).
            dip::sint offset = scaleOffset + scaleTensorLUT_[ scaleTensorColIndex  * inputTensorElements_ + scaleTensorRowIndex ];
            void* scalePtr = static_cast<uint8*>(kernelScale_.Origin()) + offset * static_cast<dip::sint>(kernelScale_.DataType().SizeOf());
            scaleAtImgCoords_[ iTE ][ iDim ] = *static_cast<dfloat*>(scalePtr);   // TODO: either assert that kernelScale_ is of type dfloat or support multiple types
         }
      }
   }

protected:
   std::vector< std::array< dfloat, nDims > > scaleAtImgCoords_; // The vector is over the input tensor elements; the array is over the kernel dimensions
   Image kernelScale_;
   dip::uint inputTensorElements_;
   std::vector< dip::sint > scaleTensorLUT_;
};

// Kernel transformation: 2D rotation
class KernelTransform2DRotation : public KernelTransform
{
public:
   KernelTransform2DRotation( Image const& orientation ) : orientation_( orientation ) {
      csn_.resize( orientation.TensorElements() ), sn_.resize( orientation.TensorElements() );
   }

   virtual KernelTransform* Clone() const override { return new KernelTransform2DRotation( *this ); }

   virtual void SetImageCoords( UnsignedArray const& imgCoords ) {
      KernelTransform::SetImageCoords( imgCoords );
      Image::Pixel dirPixel = orientation_.At( imgCoords );
      // Iterate over tensor elements
      for( dip::uint iTE = 0; iTE < orientation_.TensorElements(); ++iTE ) {
         csn_[ iTE ] = std::cos( dip::pi * 0.5 - dirPixel[ iTE ].As< dfloat >() );
         sn_[ iTE ] = std::sin( dip::pi * 0.5 - dirPixel[ iTE ].As< dfloat >() );
      }
   }
   virtual void Transform( IntegerArray const& kernelCoords, dip::uint tensorIndex, FloatArray& transformedCoords ) const override {
      transformedCoords[ 0 ] = imgCoords_[ 0 ] + static_cast< dfloat >( kernelCoords[ 0 ] ) * csn_[ tensorIndex ]
                                               + static_cast< dfloat >( kernelCoords[ 1 ] ) * sn_[ tensorIndex ];
      transformedCoords[ 1 ] = imgCoords_[ 1 ] - static_cast< dfloat >( kernelCoords[ 0 ] ) * sn_[ tensorIndex ]
                                               + static_cast< dfloat >( kernelCoords[ 1 ] ) * csn_[ tensorIndex ];
   }
protected:
   std::vector<dfloat> csn_, sn_;   // Length is equal to number of input tensor elements
   Image const& orientation_;
};

class KernelTransform2DScaledRotation : public KernelTransform2DRotation, public KernelTransformScale<2>
{
public:
   KernelTransform2DScaledRotation( Image const& orientation, Image const& kernelScale )
         : KernelTransform2DRotation( orientation ), KernelTransformScale<2>( kernelScale, orientation.TensorElements() ) {}
   virtual KernelTransform* Clone() const override { return new KernelTransform2DScaledRotation( *this ); }

   virtual void SetImageCoords( UnsignedArray const& imgCoords ) {
      KernelTransform2DRotation::SetImageCoords( imgCoords );
      SetScaleAtImgCoords( imgCoords );
   }

   virtual void Transform( IntegerArray const& kernelCoords, dip::uint tensorIndex, FloatArray& transformedCoords ) const override {
      // First scale, then rotate
      dfloat scaledKernelCoords[ 2 ] = { scaleAtImgCoords_[ tensorIndex ][ 0 ] * static_cast< dfloat >( kernelCoords[ 0 ] ),
                                         scaleAtImgCoords_[ tensorIndex ][ 1 ] * static_cast< dfloat >( kernelCoords[ 1 ] ) };
      transformedCoords[ 0 ] = imgCoords_[ 0 ] + scaledKernelCoords[ 0 ] * csn_[ tensorIndex ]
                                               + scaledKernelCoords[ 1 ] * sn_[ tensorIndex ];
      transformedCoords[ 1 ] = imgCoords_[ 1 ] - scaledKernelCoords[ 0 ] * sn_[ tensorIndex ]
                                               + scaledKernelCoords[ 1 ] * csn_[ tensorIndex ];
   }
};

// Kernel transformation: 3D rotation using phi3 and theta3
class KernelTransform3DRotationZ : public KernelTransform
{
public:
   KernelTransform3DRotationZ( Image const& phi3, Image const& theta3 )
         : phi3_( phi3 ), theta3_( theta3 ), R_( phi3.TensorElements() ) {}

   virtual KernelTransform* Clone() const override { return new KernelTransform3DRotationZ( *this ); }

   virtual void SetImageCoords( UnsignedArray const& imgCoords ) {
      KernelTransform::SetImageCoords( imgCoords );
      Image::Pixel phiPixel = phi3_.At( imgCoords );
      Image::Pixel thetaPixel = theta3_.At( imgCoords );
      for( dip::uint iTE = 0; iTE < phi3_.TensorElements(); ++iTE ) {
         dfloat phi = phiPixel[ iTE ].As<dfloat>();  // TODO: handle tensor elements
         dfloat theta = thetaPixel[ iTE ].As<dfloat>();  // TODO: handle tensor elements
         dfloat cs_p = std::cos( phi );
         dfloat sn_p = std::sin( phi );
         dfloat cs_t = std::cos( theta );
         dfloat sn_t = std::sin( theta );

         RotMatrix& R = R_[ iTE ];

         R[ 0 ] = cs_p * cs_t;  R[ 1 ] = -sn_p;  R[ 2 ] = cs_p * sn_t;
         R[ 3 ] = sn_p * cs_t;  R[ 4 ] = cs_p;   R[ 5 ] = sn_p * sn_t;
         R[ 6 ] = -sn_t;        R[ 7 ] = 0;      R[ 8 ] = cs_t;
      }
   }

   virtual void Transform( IntegerArray const& kernelCoords, dip::uint tensorIndex, FloatArray& transformedCoords ) const override {
      RotMatrix const& R = R_[ tensorIndex ];
      transformedCoords[ 0 ] = imgCoords_[ 0 ] + static_cast< dfloat >( kernelCoords[ 0 ] ) * R[ 0 ]
                                               + static_cast< dfloat >( kernelCoords[ 1 ] ) * R[ 1 ]
                                               + static_cast< dfloat >( kernelCoords[ 2 ] ) * R[ 2 ];
      transformedCoords[ 1 ] = imgCoords_[ 1 ] + static_cast< dfloat >( kernelCoords[ 0 ] ) * R[ 3 ]
                                               + static_cast< dfloat >( kernelCoords[ 1 ] ) * R[ 4 ]
                                               + static_cast< dfloat >( kernelCoords[ 2 ] ) * R[ 5 ];
      transformedCoords[ 2 ] = imgCoords_[ 2 ] + static_cast< dfloat >( kernelCoords[ 0 ] ) * R[ 6 ]
                                               + static_cast< dfloat >( kernelCoords[ 2 ] ) * R[ 8 ];
   }

protected:
   Image const& phi3_;
   Image const& theta3_;
   using RotMatrix = std::array<dfloat, 9>;
   std::vector< RotMatrix > R_;  // Rotation matrix. The vector is over the input tensor elements. // TODO: use Matrix class?
};

// Kernel transformation: 3D rotation using phi2, theta2, phi3 and theta3
class KernelTransform3DRotationXY : public KernelTransform
{
public:
   KernelTransform3DRotationXY( Image const& phi2, Image const& theta2, Image const& phi3, Image const& theta3 ) : phi2_( phi2 ), theta2_( theta2 ), phi3_( phi3 ), theta3_( theta3 ), T_( phi2.TensorElements() ) {}

   virtual KernelTransform* Clone() const override { return new KernelTransform3DRotationXY( *this ); }

   virtual void SetImageCoords( UnsignedArray const& imgCoords ) {
      Image::Pixel phi2Pixel = phi2_.At( imgCoords );
      Image::Pixel theta2Pixel = theta2_.At( imgCoords );
      Image::Pixel phi3Pixel = phi3_.At( imgCoords );
      Image::Pixel theta3Pixel = theta3_.At( imgCoords );
      for( dip::uint iTE = 0; iTE < phi2_.TensorElements(); ++iTE ) {
         dfloat phi2 = phi2Pixel[ iTE ].As<dfloat>();  // TODO: handle tensor elements
         dfloat theta2 = theta2Pixel[ iTE ].As<dfloat>();  // TODO: handle tensor elements
         dfloat phi3 = phi3Pixel[ iTE ].As<dfloat>();  // TODO: handle tensor elements
         dfloat theta3 = theta3Pixel[ iTE ].As<dfloat>();  // TODO: handle tensor elements

         // Create transformation matrix from basis vectors
         Eigen::Vector3d xAxis( GetAxis( phi2, theta2 ) );
         Eigen::Vector3d yAxis( GetAxis( phi3, theta3 ) );
         Eigen::Vector3d zAxis( xAxis.cross( yAxis ).normalized() ); // TODO: do we need to normalize? Only needed if xAxis and zAxis are not perpendicular..
         // Set rotation part
         T_[ iTE ].linear() << xAxis, yAxis, zAxis;
         // Set translation part
         T_[ iTE ].translation() << Eigen::Vector3d( imgCoords_[ 0 ], imgCoords_[ 1 ], imgCoords_[ 2 ] );
      }
   }

   virtual void Transform( IntegerArray const& /*kernelCoords*/, dip::uint tensorIndex, FloatArray& transformedCoords ) const override {
      Eigen::Map<Eigen::Vector3d> output( transformedCoords.data() );   // Create an Eigen map that directly accesses transformedCoords
      output = T_[ tensorIndex ] * output;
   }

protected:
   Eigen::Vector3d GetAxis( dfloat phi, dfloat theta ) {
      dfloat sin_phi = std::sin( phi );
      dfloat cos_phi = std::cos( phi );
      dfloat sin_theta = std::sin( theta );
      dfloat cos_theta = std::cos( theta );
      return Eigen::Vector3d( sin_phi*cos_theta, sin_phi*sin_theta, cos_phi );
   }

   Image const& phi2_;
   Image const& theta2_;
   Image const& phi3_;
   Image const& theta3_;
   using CompactTransform = Eigen::Transform<dfloat, 3, Eigen::AffineCompact>;
   std::vector< CompactTransform, Eigen::aligned_allocator< CompactTransform > > T_;  // Transformation matrix. The vector is over the input tensor elements.
};

// Kernel transformation: 2D skew
class KernelTransform2DSkew : public KernelTransform
{
public:
   KernelTransform2DSkew( Image const& skew ) : skew_( skew ), s_( skew.TensorElements() ) {}

   virtual KernelTransform* Clone() const override { return new KernelTransform2DSkew( *this ); }

   virtual void SetImageCoords( UnsignedArray const& imgCoords ) {
      KernelTransform::SetImageCoords( imgCoords );
      Image::Pixel skewPixel = skew_.At( imgCoords );
      for( dip::uint iTE = 0; iTE < skew_.TensorElements(); ++iTE ) {
         s_[ iTE ] = skewPixel[ iTE ].As<dfloat>();
      }
   }
   virtual void Transform( IntegerArray const& kernelCoords, dip::uint tensorIndex, FloatArray& transformedCoords ) const override {
      dfloat kernelCoordX = static_cast<dfloat>(kernelCoords[ 0 ]);
      transformedCoords[ 0 ] = imgCoords_[ 0 ] + kernelCoordX;
      transformedCoords[ 1 ] = imgCoords_[ 1 ] + static_cast< dfloat >( kernelCoords[ 1 ] ) + s_[ tensorIndex ] * kernelCoordX;
   }
protected:
   Image const& skew_;
   std::vector< dfloat > s_;  // The vector is over the input tensor elements
};

// Kernel transformation: 2D banana
class KernelTransform2DBanana : public KernelTransform2DRotation
{
public:
   KernelTransform2DBanana( Image const& orientation, Image const& hcurvature ) : KernelTransform2DRotation( orientation), hcurvature_( hcurvature ), hcurv_( orientation.TensorElements() ) {}

   virtual KernelTransform* Clone() const override { return new KernelTransform2DBanana( *this ); }


   virtual void SetImageCoords( UnsignedArray const& imgCoords ) {
      KernelTransform2DRotation::SetImageCoords( imgCoords );
      Image::Pixel cPixel = hcurvature_.At( imgCoords );
      for( dip::uint iTE = 0; iTE < orientation_.TensorElements(); ++iTE ) {
         hcurv_[ iTE ] = -0.5*cPixel[ iTE ].As<dfloat>();
      }
   }
   virtual void Transform( IntegerArray const& kernelCoords, dip::uint tensorIndex, FloatArray& transformedCoords ) const override {
      dfloat kernelCoordX = static_cast<dfloat>(kernelCoords[ 0 ]);
      dfloat kernelCoordY = static_cast<dfloat>(kernelCoords[ 1 ]) + (hcurv_[ tensorIndex ] * kernelCoordX * kernelCoordX);
      transformedCoords[ 0 ] = imgCoords_[ 0 ] + kernelCoordX * csn_[ tensorIndex ] + kernelCoordY * sn_[ tensorIndex ];
      transformedCoords[ 1 ] = imgCoords_[ 1 ] - kernelCoordX * sn_[ tensorIndex ] + kernelCoordY * csn_[ tensorIndex ];
   }
protected:
   Image const& hcurvature_;
   std::vector< dfloat > hcurv_; // The vector is over the input tensor elements
};

class KernelTransform2DScaledBanana : public KernelTransform2DBanana, public KernelTransformScale<2>
{
public:
   KernelTransform2DScaledBanana( Image const& orientation, Image const& hcurvature, Image const& kernelScale ) : KernelTransform2DBanana( orientation, hcurvature ), KernelTransformScale<2>( kernelScale, orientation.TensorElements() ) {}
   virtual KernelTransform* Clone() const override { return new KernelTransform2DScaledBanana( *this ); }

   virtual void SetImageCoords( UnsignedArray const& imgCoords ) {
      KernelTransform2DBanana::SetImageCoords( imgCoords );
      SetScaleAtImgCoords( imgCoords );
   }

   virtual void Transform( IntegerArray const& kernelCoords, dip::uint tensorIndex, FloatArray& transformedCoords ) const override {
      // First scale, then curve, then rotate
      dfloat kernelCoordX = scaleAtImgCoords_[tensorIndex][ 0 ] * static_cast<dfloat>(kernelCoords[ 0 ]);
      dfloat kernelCoordY = scaleAtImgCoords_[tensorIndex][ 1 ] * static_cast<dfloat>(kernelCoords[ 1 ]) + (hcurv_[ tensorIndex ] * kernelCoordX * kernelCoordX);
      transformedCoords[ 0 ] = imgCoords_[ 0 ] + kernelCoordX * csn_[ tensorIndex ] + kernelCoordY * sn_[ tensorIndex ];
      transformedCoords[ 1 ] = imgCoords_[ 1 ] - kernelCoordX * sn_[ tensorIndex ]  + kernelCoordY * csn_[ tensorIndex ];
   }
};

// Input interpolation class
template< typename TPI, typename TPO >
class InputInterpolator
{
public:
   InputInterpolator( Image const& in ) : in_( in ), inOrigin_( static_cast<TPI*>(in_.Origin()) ), inTensorStride_( in_.TensorStride() ) {}

   virtual TPO GetInputValue( FloatArray& /*coords*/, dip::uint /*tensorIndex*/, bool /*mirrorAtImageBoundaries*/ ) const { return 0; }

protected:
   // Maps coords to a location inside the image using mirroring at the image boundaries.
   // Returns false if coordinates don't fall inside the image even after mirroring.
   template< dip::uint nDims >
   bool MapCoords_Mirror( dfloat* coords ) const {
      // Make sure all coordinates are within the image. Mirror at the boundaries.
      for( dip::uint iDim = 0; iDim < nDims; ++iDim ) {
         const dfloat cMax = static_cast<dfloat>(in_.Size( iDim ) - 1);
         dfloat& c = coords[ iDim ];
         if( c < 0 ) {
            c = -c;
            if( c > cMax )
               return false;  // Mirroring exceeds border
         } else if( c > cMax ) {
            c = cMax - (c - cMax);
            if( c < 0 )
               return false;  // Mirroring exceeds border
         }
      }
      return true;
   }

   Image const& in_; // Input image
   TPI* inOrigin_;   // Input image origin
   dip::sint inTensorStride_;   // Input image tensor stride
};

// Input interpolation class, templated in the number of input dimensions
template< dip::uint nDims, typename TPI, typename TPO >
class InputInterpolatorFixedDims : public InputInterpolator< TPI, TPO >
{
public:
   InputInterpolatorFixedDims( Image const& in ) : InputInterpolator< TPI, TPO >( in ) {
      // Verify input dimensionality
      DIP_THROW_IF( in_.Dimensionality() != nDims, "Interpolation dimensionality incorrect" );

      // Cache strides and sizes
      for( dip::uint iDim = 0; iDim < nDims; ++iDim ) {
         inStrides_[ iDim ] = in_.Stride( iDim );
         inSizes_[ iDim ] = in_.Size( iDim );
      }
   }
protected:
   using InputInterpolator< TPI, TPO >::in_;
   std::array< dip::sint, nDims > inStrides_;
   std::array< dip::uint, nDims > inSizes_;
};


// Zero order hold input interpolator
template< dip::uint nDims, typename TPI, typename TPO >
class InputInterpolatorZOH : public InputInterpolatorFixedDims< nDims, TPI, TPO >
{
public:
   InputInterpolatorZOH( Image const& in ) : InputInterpolatorFixedDims< nDims, TPI, TPO>( in ) {}

   // Get zero-order-hold input value at floating point coordinates, arbitrary input dimensionality
   TPO GetInputValue( FloatArray& coords, dip::uint tensorIndex, bool mirrorAtImageBoundaries ) const override {
      // If not mirroring at the image boundaries, the input value is considered 0 outside the image
      if( !mirrorAtImageBoundaries ) {
         if( !in_.IsInside( coords ) )
            return 0;
      } else {
         // Mirror input coordinates. May fail -> return 0.
         if( !this->template MapCoords_Mirror< nDims >( &coords[ 0 ] ) )
            return 0;
      }

      // Compute pixel offset
      dip::sint pixelOffset = 0;
      for( dip::uint iDim = 0; iDim < nDims; ++iDim ) {
         dip::sint integerCoord = floor_cast( coords[ iDim ] );
         pixelOffset += integerCoord * inStrides_[ iDim ];
      }

      // Add tensor offset
      pixelOffset += static_cast< dip::sint >( tensorIndex ) * inTensorStride_;

      // Output single input value
      TPI inValue = *(inOrigin_ + pixelOffset);
      return static_cast< TPO >(inValue);
   }

   using InputInterpolatorFixedDims< nDims, TPI, TPO >::inStrides_;
   using InputInterpolator< TPI, TPO >::inTensorStride_;
   using InputInterpolator< TPI, TPO >::inOrigin_;
   using InputInterpolator< TPI, TPO >::in_;
};

// First order hold input interpolator
template< dip::uint nDims, typename TPI, typename TPO >
class InputInterpolatorFOH : public InputInterpolatorFixedDims< nDims, TPI, TPO >
{
public:
   InputInterpolatorFOH( Image const& in ) : InputInterpolatorFixedDims< nDims, TPI, TPO>( in ) {}

   // Get linearly interpolated input value at floating point coordinates, arbitrary input dimensionality
   TPO GetInputValue( FloatArray& coords, dip::uint tensorIndex, bool mirrorAtImageBoundaries ) const override {
      using InputAsFloat = FloatType< TPI >;

      // If not mirroring at the image boundaries, the input value is considered 0 outside the image
      if( !mirrorAtImageBoundaries ) {
         if( !in_.IsInside( coords ) )
            return 0;
      } else {
         // Mirror input coordinates. May fail -> return 0.
         if( !this->template MapCoords_Mirror< nDims >( &coords[ 0 ] ) )
            return 0;
      }

      // Start linear interpolation
      // Compute lower bound index and interpolation factor for all coordinates
      //  E.g.: if coords[i] == 1.1, then loBoundIndices[i] == 1.0 and factors[i] == 0.1
      std::array< dip::sint, nDims> loBoundIndices;
      std::array< InputAsFloat, nDims > factors; // Linear interpolation factors
      for( dip::uint iDim = 0; iDim < nDims; ++iDim ) {
         loBoundIndices[ iDim ] = floor_cast( coords[ iDim ] );   // Take lower bound using floor_cast
         if( loBoundIndices[ iDim ] == static_cast< dip::sint >( inSizes_[ iDim ] - 1 )) { // Because we interpolate between loBound and loBound+1, make sure we don't go beyond the image borders
            loBoundIndices[ iDim ]--;
         }
         factors[ iDim ] = static_cast< InputAsFloat >( coords[ iDim ] ) - static_cast< InputAsFloat >( loBoundIndices[ iDim ] );
      }

      // Compute pixel offset for first pixel of the interpolation input window, including tensor offset
      dip::sint interpOriginOffset = 0;
      for( dip::uint iDim = 0; iDim< nDims; ++iDim )
         interpOriginOffset += loBoundIndices[ iDim ] * inStrides_[ iDim ];

      // Compute pixel offsets and composite interpolation factors for a total of 2 ^ nDims pixels
      // We can avoid an ND-loop by exploiting the fact that only 2 coordinates are processed per dimension.
      constexpr dip::uint numInterpPixels = dip::uint( 1 ) << nDims;

      TPO result = 0.0;
      //for( dip::uint iOffset = 0; iOffset < numInterpPixels; ++iOffset ) {
      const_for< numInterpPixels >( [&]( dip::uint iOffset )
      {
         dip::sint pixelOffset = interpOriginOffset;
         InputAsFloat pixelFactor = 1.0;
         //for( dip::uint iDim = 0; iDim < nDims; ++iDim ) {
         const_for< nDims >( [&]( dip::uint iDim )
         {
            // To process all permutations of lowerbound and upperbound (= lowerbound + 1) in each dimension,
            // decide whether to process the upperbound in this dimension by testing the bits in iOffset
            bool useUpperBound = (iOffset & (dip::uint( 1 ) << iDim)) != 0;
            if( useUpperBound ) {
               pixelOffset += inStrides_[ iDim ];
               pixelFactor *= factors[ iDim ];
            } else {
               pixelFactor *= 1 - factors[ iDim ];
            }
         } );
         // Add tensor offset
         pixelOffset += static_cast< dip::sint >( tensorIndex ) * inTensorStride_;
         // Multiply pixel input values with composite interpolation factors
         TPI inValue = *(inOrigin_ + pixelOffset);
         result += static_cast< TPO >(inValue) * pixelFactor;
      } );

      return result;
   }

   /*
   // GetInputValue_LinearInterpolation() 2D reference implementation for speed comparison
   TPO GetInputValue_LinearInterpolation_2D( FloatArray& coords, bool mirrorAtImageBoundaries ) const {
      using InputAsFloat = FloatType< TPI >;
      assert( in_.Dimensionality() == nDims );

      // Cache strides
      std::array< dip::sint, 2 > strides = { in_.Stride( 0 ), in_.Stride( 1 ) };

      // If not mirroring at the image boundaries, the input value is considered 0 outside the image
      if( !mirrorAtImageBoundaries ) {
         if( !in_.IsInside( coords ) )
            return 0;
      } else {
         // Mirror input coordinates. May fail -> return 0.
         if( !this->template MapCoords_Mirror< 2 >( &coords[ 0 ] ) )
            return 0;
      }

      // Start linear interpolation
      // Compute lower bound index and interpolation factor for all coordinates
      //  E.g.: if coords[i] == 1.1, then loBoundIndices[i] == 1.0 and factors[i] == 0.1
      std::array< dip::uint, 2> loBoundIndices;
      for( dip::uint iDim = 0; iDim < 2; ++iDim ) {
         loBoundIndices[ iDim ] = (floor_cast( coords[ iDim ] ));   // Take lower bound using floor_cast
         if( coords[ iDim ] == in_.Size( iDim ) - 1 )  // Because we interpolate between loBound and loBound+1, make sure we don't go beyond the image borders
            loBoundIndices[ iDim ]--;
      }

      // Compute pixel offset of p00
      dip::sint offset = loBoundIndices[ 0 ] * strides[ 0 ] + loBoundIndices[ 1 ] * strides[ 1 ];

      TPI* origin = static_cast<TPI*>(in_.Origin());
      TPO p00 = static_cast<TPO>(*(origin + offset));
      TPO p01 = static_cast<TPO>(*(origin + offset + strides[ 1 ]));
      TPO p10 = static_cast<TPO>(*(origin + offset + strides[ 0 ]));
      TPO p11 = static_cast<TPO>(*(origin + offset + strides[ 0 ] + strides[ 1 ]));
      InputAsFloat xo = coords[ 0 ] - loBoundIndices[ 0 ];
      InputAsFloat yo = coords[ 1 ] - loBoundIndices[ 1 ];
      InputAsFloat xs = 1 - xo;
      InputAsFloat ys = 1 - yo;

      return (TPO)((p00 * xs * ys)
         + (p10 * xo * ys)
         + (p01 * xs * yo)
         + (p11 * xo * yo));
   }
   */

   using InputInterpolatorFixedDims< nDims, TPI, TPO >::inSizes_;
   using InputInterpolatorFixedDims< nDims, TPI, TPO >::inStrides_;
   using InputInterpolator< TPI, TPO >::inTensorStride_;
   using InputInterpolator< TPI, TPO >::inOrigin_;
   using InputInterpolator< TPI, TPO >::in_;
};

// The adaptive window convolution filter for adaptive gauss and its variants
template< typename TPI, typename TPO = FlexType< TPI > >
class AdaptiveWindowConvolutionLineFilter : public Framework::FullLineFilter
{
public:
   AdaptiveWindowConvolutionLineFilter( Image const& in, Kernel const& kernel, ImageArray const& params, String const& interpolation, BoundaryCondition bc, String const& transform )
         : in_( in ), kernel_( kernel ) {
      // Determine kernel transformation
      if( in_.Dimensionality() == 2 ) {
         // === 2D ===
         // Construct input interpolator
         ConstructInputInterpolator< 2 >( in, interpolation );

         // Construct kernel transformation
         ConstructKernelTransform2D( transform, params );

      } else if( in_.Dimensionality() == 3 ) {
         // === 3D ===
         // Construct input interpolator
         ConstructInputInterpolator< 3 >( in, interpolation );

         // Determine kernel transformation
         ConstructKernelTransform3D( transform, params );
      }
      else {
         DIP_THROW( "No transform \"" + transform + "\" known for input dimensionality " + std::to_string( in_.Dimensionality()));
      }

      // Store boundary condition. We only support mirroring or zeros for now.
      DIP_THROW_IF( bc != BoundaryCondition::SYMMETRIC_MIRROR  && bc != BoundaryCondition::ADD_ZEROS, "Unsupported boundary condition" );
      mirrorAtInputBoundaries_ = (bc == BoundaryCondition::SYMMETRIC_MIRROR);
   }

   virtual void SetNumberOfThreads( dip::uint numThreads, PixelTableOffsets const& pixelTable ) override {
      offsets_ = pixelTable.Offsets();
      kernelTransforms_.resize( numThreads - 1 );  // kernelTransform_ is used for thread 0
   }

   virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
      TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
      dip::sint inStride = params.inBuffer.stride;
      TPO* out = static_cast< TPO* >( params.outBuffer.buffer );
      dip::sint outStride = params.outBuffer.stride;
      dip::sint outTensorStride = params.outBuffer.tensorStride;
      dip::uint length = params.bufferLength;
      PixelTableOffsets const& pixelTableOffsets = params.pixelTable;
      std::vector< dfloat > const& weights = pixelTableOffsets.Weights();
      UnsignedArray inCoords( params.position );
      PixelTable pixelTable = kernel_.PixelTable( in_.Dimensionality(), params.dimension );  // Todo: move to constructor
      FloatArray transformedKernelCoords( in_.Dimensionality() );

      // Obtain kernel transform for this thread
      std::unique_ptr< KernelTransform >& kernelTransform = params.thread == 0 ? kernelTransform_ : kernelTransforms_[ params.thread - 1 ];
      // Clone kernelTransform_ for this thread if not done already.
      // Needed because the KernelTransform computes and stores values per input pixel and uses those repeatedly for each kernel element.
      if( !kernelTransform ) {
         kernelTransform.reset( kernelTransform_->Clone() );
      }

      for( dip::uint ii = 0; ii < length; ++ii ) {
         for( dip::uint iTE = 0; iTE < in_.TensorElements(); ++iTE) {
            *( out + static_cast< dip::sint >( iTE ) * outTensorStride ) = 0;
         }
         std::vector< dfloat >::const_iterator itWeight = weights.begin();
         // Prepare kernel transform for current input coordinates
         kernelTransform->SetImageCoords( inCoords );
         // Apply kernel
         for( PixelTable::iterator itPT = pixelTable.begin(); itPT != pixelTable.end(); ++itPT ) {
            for( dip::uint iTE = 0; iTE < in_.TensorElements(); ++iTE ) {
               // Apply kernel transformation
               kernelTransform->Transform( *itPT, iTE, transformedKernelCoords );
               // Obtain input value at the transformed kernel coords
               *( out + static_cast< dip::sint >( iTE ) * outTensorStride ) +=
                     inputInterpolator_->GetInputValue( transformedKernelCoords, iTE, mirrorAtInputBoundaries_ ) * static_cast< FloatType< TPO >>( *itWeight );
            }
            ++itWeight;
         }
         // Prepare next buffer element
         inCoords[ params.dimension ]++;
         in += inStride;
         out += outStride;
      }
   }

private:
   template< dip::uint nDims >
   void ConstructInputInterpolator( Image const& in, String const& interpolation ) {
      // Determine input interpolator
      if( interpolation == S::ZERO_ORDER ) {
         inputInterpolator_ = std::make_unique< InputInterpolatorZOH< nDims, TPI, TPO >>( in );
      } else if( interpolation == S::LINEAR ) {
         inputInterpolator_ = std::make_unique< InputInterpolatorFOH< nDims, TPI, TPO >>( in );
      } else {
         DIP_THROW( "Unknown interpolation \"" + interpolation + "\"" );
      }
   }

   void ConstructKernelTransform2D( String const& transform, ImageArray const& params ) {
      // Determine kernel transformation
      if( transform == "none" ) {
         kernelTransform_ = std::make_unique< KernelTransform >();
      } else if( transform == "ellipse" ) {
         DIP_ASSERT( params.size() == 1 || params.size() == 2);
         if( params.size() == 1 ) {
            kernelTransform_ = std::make_unique< KernelTransform2DRotation >( params[ 0 ] );
         } else {
            kernelTransform_ = std::make_unique< KernelTransform2DScaledRotation >( params[ 0 ], params[ 1 ] );
         }
      } else if( transform == "banana" ) {
         DIP_ASSERT( params.size() == 2 || params.size() == 3 );
         if( params.size() == 2 ) {
            kernelTransform_ = std::make_unique< KernelTransform2DBanana >( params[ 0 ], params[ 1 ] );
         } else {
            kernelTransform_ = std::make_unique< KernelTransform2DScaledBanana >( params[ 0 ], params[ 1 ], params[ 2 ] );
         }
      } else if( transform == "skew" ) {
         DIP_ASSERT( params.size() == 1 );
         kernelTransform_ = std::make_unique< KernelTransform2DSkew >( params[ 0 ] );
      } else {
         DIP_THROW( "Unknown 2D transform \"" + transform + "\"" );
      }
   }

   void ConstructKernelTransform3D( String const& transform, ImageArray const& params ) {
      if( transform == "none" ) {
         kernelTransform_ = std::make_unique< KernelTransform >();
      } else if( transform == "ellipse" ) {
         if( params.size() == 2 ) {
            kernelTransform_ = std::make_unique< KernelTransform3DRotationZ >( params[ 0 ], params[ 1 ] );
         } else if( params.size() == 4 ) {
            kernelTransform_ = std::make_unique< KernelTransform3DRotationXY >( params[ 0 ], params[ 1 ], params[ 2 ], params[ 3 ] );
         }
      } else {
         DIP_THROW( "Unknown 3D transform \"" + transform + "\"" );
      }
   }

   std::vector< dip::sint > offsets_;  // Pixel table offsets
   Image const& in_; // Input image
   Kernel const& kernel_;  // Kernel
   std::unique_ptr< KernelTransform > kernelTransform_;  // Kernel transform for thread 0
   std::vector< std::unique_ptr< KernelTransform > > kernelTransforms_; // Kernel transforms for remaining threads
   std::unique_ptr< InputInterpolator<TPI, TPO>> inputInterpolator_; // Input interpolator
   bool mirrorAtInputBoundaries_;   // Boundary condition: either mirror or zeros
};

} // namespace


void AdaptiveFilter(
   Image const& in,
   ImageConstRefArray const& params,
   Image& out,
   FloatArray sigmas,
   UnsignedArray const& orders,
   dfloat truncation,
   UnsignedArray const& exponents,
   String const& interpolationMethod,
   String const& boundaryCondition,
   String const& transform
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   // TODO: all param images must be of type DT_DFLOAT?

   // Prepare parameter images: expand singleton dimensions, including the tensor
   ImageArray paramImages( params.size() );
   for( dip::uint iP = 0; iP < params.size(); ++iP ) {
      paramImages[ iP ] = params[ iP ].get().QuickCopy();
      paramImages[ iP ].ExpandSingletonDimensions( in.Sizes() );
      // Make sure the param image tensor has a row for each input image tensor element
      if( paramImages[ iP ].TensorElements() != in.TensorElements() ) {
         // TODO: for the scale image, this is probably wrong.
         paramImages[ iP ].ExpandSingletonTensor( in.TensorElements() );
      }
   }
   DIP_STACK_TRACE_THIS( ArrayUseParameter( sigmas, in.Dimensionality(), 1.0 ));

   DIP_START_STACK_TRACE
      // Create gaussian kernel
      Kernel kernel{ CreateGauss( sigmas, orders, truncation, exponents ) };

      BoundaryCondition bc = StringToBoundaryCondition( boundaryCondition );
      DataType outputType = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_ALL( lineFilter, AdaptiveWindowConvolutionLineFilter, ( in, kernel, paramImages, interpolationMethod, bc, transform ), in.DataType() );
      // We use the full framework to allow multi-threading. Its parameters prevent input or output buffering to minimize overhead. Border expansion is not used either.
      Framework::Full( in, out, in.DataType(), outputType, outputType, in.TensorElements(), { bc }, kernel, *lineFilter, Framework::FullOption::BorderAlreadyExpanded );// for performance comparisons: +Framework::FullOption::NoMultiThreading );

   DIP_END_STACK_TRACE
}

void AdaptiveGauss(
   Image const& in,
   ImageConstRefArray const& params,
   Image& out,
   FloatArray const& sigmas,
   UnsignedArray const& orders,
   dfloat truncation,
   UnsignedArray const& exponents,
   String const& interpolationMethod,
   String const& boundaryCondition
) {
   AdaptiveFilter( in, params, out, sigmas, orders, truncation, exponents, interpolationMethod, boundaryCondition, "ellipse" );
}

void AdaptiveBanana(
   Image const& in,
   ImageConstRefArray const& params,
   Image& out,
   FloatArray const& sigmas,
   UnsignedArray const& orders,
   dfloat truncation,
   UnsignedArray const& exponents,
   String const& interpolationMethod,
   String const& boundaryCondition
) {
   AdaptiveFilter( in, params, out, sigmas, orders, truncation, exponents, interpolationMethod, boundaryCondition, "banana" );
}

} // namespace dip

#if defined(__GNUG__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
