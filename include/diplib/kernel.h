/*
 * DIPlib 3.0
 * This file contains the declaration for dip::Kernel.
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

#ifndef DIP_KERNEL_H
#define DIP_KERNEL_H

#include "diplib.h"


/// \file
/// \brief Defines a class that describes a filtering kernel


namespace dip {


// forward declaration, from diplib/pixel_table.h
class DIP_NO_EXPORT PixelTable;


/// \addtogroup infrastructure
/// \{

/// \brief Represents the shape and size of a filtering kernel.
///
/// Some image filters allow the specification of arbitrary kernels: the user can specify the shape
/// name and the size of a pre-defined kernel, or the user can pass an image containing the kernel.
///
/// Objects of type `dip::Image`, `dip::FloatArray` and `dip::String` implicitly convert to
/// a `%dip::Kernel`, so it should be convenient to use these various representations in your
/// code.
///
/// To define a kernel by shape and size, pass a string defining the shape, and a floating-point
/// array with the size along each dimension.
/// These are the valid shape strings:
///
/// - `"elliptic"`: The unit circle in Euclidean (\f$L^2\f$) metric.
///
/// - `"rectangular"`: A box, the unit circle in \f$L^1\f$ metric.
///
/// - `"diamond"`: A box rotated 45 degrees, the unit circle in \f$L^\infty\f$ metric (max-norm).
///
/// In each of these cases, the `size` array indicates the diameter of the circle. The value can
/// be different along each dimension, simply stretching the shape. Note that the sizes are not
/// necessarily odd, and don't even need to be integers. Pixels included in the neighborhood are
/// those covered by the circle, with the origin on a pixel. In the case of the rectangle, however,
/// the box is shifted by half a pixel if `floor(size)` is even. This means that the rectangular
/// kernel is not necessarily symmetric. Set the `size` to odd values to ensure symmetry. Any
/// size that is smaller or equal to 1 causes the kernel to not have an extent in that direction.
///
/// To define a kernel through an image, provide a binary image. The "on" or "true" pixels form
/// the kernel. Note that, for most filters, the image is directly used as neighborhood (i.e. no
/// mirroring is applied). As elsewhere, the origin of the kernel is in the middle of the image,
/// and on the pixel to the right of the center in case of an even-sized image. If the image
/// is a grey-value image, then all pixels with a finite value form the kernel. The kernel then
/// has the given weights associated to each pixel.
///
/// See dip::StructuringElement, dip::NeighborList, dip::PixelTable
class DIP_NO_EXPORT Kernel{
   public:
      enum class ShapeCode {
            RECTANGULAR,
            ELLIPTIC,
            DIAMOND,
            CUSTOM
      };

      /// \brief The default kernel is a disk with a diameter of 7 pixels.
      Kernel() : shape_( ShapeCode::ELLIPTIC ), params_( { 7 } ) {}

      /// \brief A string implicitly converts to a kernel, it is interpreted as a shape.
      Kernel( String const& shape ) : params_( { 7 } ) {
         SetShape( shape );
      }

      /// \brief A `dip::FloatArray` implicitly converts to a kernel, it is interpreted as the
      /// parameter for each dimension. A second argument specifies the shape.
      Kernel( FloatArray const& params, String const& shape = "elliptic" ) : params_( params ) {
         SetShape( shape );
      }

      /// \brief A floating-point value implicitly converts to a kernel, it is interpreted as the
      /// parameter for all dimensions. A second argument specifies the shape.
      Kernel( dfloat param, String const& shape = "elliptic" ) : params_( FloatArray{ param } ) {
         SetShape( shape );
      }

      /// \brief Low-level constructor mostly for internal use.
      Kernel( ShapeCode shape, FloatArray const& params ) : shape_( shape ), params_( params ) {}

      /// \brief An image implicitly converts to a kernel, optionally with weights.
      Kernel( Image const& image ) : shape_( ShapeCode::CUSTOM ), image_( image.QuickCopy() ) {
         DIP_THROW_IF( !image_.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !image_.IsScalar(), E::IMAGE_NOT_SCALAR );
         DIP_THROW_IF( image_.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
      }

      /// \brief Mirrors the kernel. This has no effect on elliptic or diamond kernels, which are always symmetric.
      void Mirror() {
         mirror_ = !mirror_;
      }

      /// \brief Creates a `dip::PixelTable` structure representing the shape of the kernel
      dip::PixelTable PixelTable( UnsignedArray const& imsz, dip::uint procDim ) const;

      /// \brief Retrieves the size of the kernel, adjusted to an image of size `imsz`.
      UnsignedArray Sizes( UnsignedArray const& imsz ) const {
         dip::uint nDim = imsz.size();
         DIP_THROW_IF( nDim < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         UnsignedArray out;
         if( IsCustom() ) {
            DIP_THROW_IF( image_.Dimensionality() > nDim, E::DIMENSIONALITIES_DONT_MATCH );
            out = image_.Sizes();
            out.resize( nDim, 1 ); // expand dimensionality by adding singletons
         } else {
            FloatArray sz = params_;
            DIP_START_STACK_TRACE
               ArrayUseParameter( sz, nDim, 1.0 );
            DIP_END_STACK_TRACE
            out.resize( nDim );
            bool rect = IsRectangular();
            for( dip::uint ii = 0; ii < nDim; ++ii ) {
               out[ ii ] = rect
                           ? static_cast< dip::uint >( sz[ ii ] )
                           : ( static_cast< dip::uint >( sz[ ii ] ) / 2 ) * 2 + 1;
            }
         }
         return out;
      }

      /// \brief Returns the kernel parameters, not adjusted to image dimensionality.
      FloatArray const& Params() const { return params_; }

      /// \brief Returns the kernel shape
      ShapeCode const& Shape() const { return shape_; }

      /// \brief Returns the kernel shape
      String ShapeString() const {
         switch( shape_ ) {
            case ShapeCode::RECTANGULAR:
               return "rectangular";
            case ShapeCode::ELLIPTIC:
               return "elliptic";
            case ShapeCode::DIAMOND:
               return "diamond";
            //case ShapeCode::CUSTOM:
            default:
               return "custom";
         }
      }

      /// \brief Tests to see if the kernel is rectangular
      bool IsRectangular() const { return shape_ == ShapeCode::RECTANGULAR; }

      /// \brief Tests to see if the kernel is a custom shape
      bool IsCustom() const { return shape_ == ShapeCode::CUSTOM; }

      /// \brief Tests to see if the kernel has weights
      bool HasWeights() const {
         return ( shape_ == ShapeCode::CUSTOM ) && !image_.DataType().IsBinary();
      }

   private:
      ShapeCode shape_;
      FloatArray params_;
      Image image_;
      bool mirror_ = false;

      void SetShape( String const& shape ) {
         if( shape == "elliptic" ) {
            shape_ = ShapeCode::ELLIPTIC;
         } else if( shape == "rectangular" ) {
            shape_ = ShapeCode::RECTANGULAR;
         } else if( shape == "diamond" ) {
            shape_ = ShapeCode::DIAMOND;
         } else {
            DIP_THROW_INVALID_FLAG( shape );
         }
      }
};

/// \}

} // namespace dip

#endif // DIP_KERNEL_H
