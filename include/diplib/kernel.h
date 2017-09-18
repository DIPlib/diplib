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
/// - `"line"`: A one-pixel thick straight line.
///
/// In the first three of these cases, the `size` array indicates the diameter of the circle. The value can
/// be different along each dimension, simply stretching the shape. Note that the sizes are not
/// necessarily odd, and don't even need to be integers. Pixels included in the neighborhood are
/// those covered by the circle, with the origin on a pixel. In the case of the rectangle, however,
/// the box is shifted by half a pixel if `floor(size)` is even. This means that the rectangular
/// kernel is not necessarily symmetric. Set the `size` to odd values to ensure symmetry. Any
/// size that is smaller or equal to 1 causes the kernel to not have an extent in that direction.
///
/// For the case of the `"line"` kernel, the `size` array gives the size of the bounding box (rounded to
/// the nearest integer), as well as the direction of the line. A negative value for one dimension means
/// that the line runs from high to low along that dimension. The line will always run from one corner of
/// the bounding box to the opposite corner, and run through the origin.

///
/// To define a kernel through an image, provide a binary image. The "on" or "true" pixels form
/// the kernel. Note that, for most filters, the image is directly used as neighborhood (i.e. no
/// mirroring is applied). As elsewhere, the origin of the kernel is in the middle of the image,
/// and on the pixel to the right of the center in case of an even-sized image. If the image
/// is a grey-value image, then all pixels with a finite value form the kernel. The kernel then
/// has the given weights associated to each pixel.
///
/// See dip::StructuringElement, dip::NeighborList, dip::PixelTable
class DIP_NO_EXPORT Kernel {
   public:
      enum class ShapeCode {
            RECTANGULAR,
            ELLIPTIC,
            DIAMOND,
            LINE,
            CUSTOM
      };

      /// \brief The default kernel is a disk with a diameter of 7 pixels.
      Kernel() : shape_( ShapeCode::ELLIPTIC ), params_( { 7 } ) {}

      /// \brief A string implicitly converts to a kernel, it is interpreted as a shape.
      Kernel( String const& shape ) : params_( { 7 } ) {
         SetShape( shape );
      }
      Kernel( char const* shape ) : params_( { 7 } ) {
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

      /// Shifts the kernel by the given amount along each of the axes.
      ///
      /// Note that the shift is only used when converting the kernel to a pixel table. Some algorithms
      /// will ignore the shift for some kernel shapes.
      ///
      /// The shift if not cumulative, any previous shift is ignored. Any mirroring is applied after the
      /// shift, whether `Mirror` is called before or after calling `%Shift`.
      ///
      /// Big shifts can be very expensive, it is recommended to use this feature only for shifting by one pixel
      /// to adjust the location of even-sized kernels.
      void Shift( IntegerArray const& shift ) {
         shift_ = shift;
      }

      /// Retrieves the amount that the is shifted.
      IntegerArray const& Shift() const { return shift_; }

      /// \brief Mirrors the kernel. This has no effect on elliptic or diamond kernels, which are always symmetric.
      void Mirror() {
         mirror_ = !mirror_;
      }

      /// \brief True if kernel is mirrored
      bool IsMirrored() const { return mirror_; }

      /// \brief Creates a `dip::PixelTable` structure representing the shape of the kernel, given the dimensionality
      /// `nDim`. Pixel table runs will be along dimension `procDim`.
      dip::PixelTable PixelTable( dip::uint nDims, dip::uint procDim ) const;

      /// \brief Retrieves the size of the kernel, adjusted to an image of size `imsz`. When computing required
      /// boundary extension, use `Boundary` instead.
      UnsignedArray Sizes( dip::uint nDims ) const {
         DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         UnsignedArray out;
         if( IsCustom() ) {
            DIP_THROW_IF( image_.Dimensionality() > nDims, E::DIMENSIONALITIES_DONT_MATCH );
            out = image_.Sizes();
            out.resize( nDims, 1 ); // expand dimensionality by adding singletons
         } else {
            FloatArray sz = params_;
            DIP_START_STACK_TRACE
               ArrayUseParameter( sz, nDims, 1.0 );
            DIP_END_STACK_TRACE
            out.resize( nDims );
            if( IsLine() ) {
               for( dip::uint ii = 0; ii < nDims; ++ii ) {
                  out[ ii ] = std::max( static_cast< dip::uint >( std::round( std::abs( sz[ ii ] ))), dip::uint( 1 ));
               }
            } else if( IsRectangular() ) {
               for( dip::uint ii = 0; ii < nDims; ++ii ) {
                  out[ ii ] = sz[ ii ] > 1.0 ? static_cast< dip::uint >( sz[ ii ] ) : 1;
               }
            } else {
               for( dip::uint ii = 0; ii < nDims; ++ii ) {
                  out[ ii ] = sz[ ii ] > 1.0 ? ( static_cast< dip::uint >( sz[ ii ] ) / 2 ) * 2 + 1 : 1;
               }
            }
         }
         return out;
      }

      /// \brief Returns the size of the boundary extension along each dimension that is necessary to accommodate the
      /// kernel on the edge pixels of the image, given an image of size `imsz`.
      UnsignedArray Boundary( dip::uint nDims ) const {
         UnsignedArray boundary = Sizes( nDims );
         for( dip::uint& b : boundary ) {
            b /= 2;
         }
         if( !shift_.empty() ) {
            dip::uint n = std::min( shift_.size(), boundary.size() );
            for( dip::uint ii = 0; ii < n; ++ii ) {
               boundary[ ii ] += static_cast< dip::uint >( std::abs( shift_[ ii ] ));
            }
         }
         return boundary;
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
            case ShapeCode::LINE:
               return "line";
            //case ShapeCode::CUSTOM:
            default:
               return "custom";
         }
      }

      /// \brief Tests to see if the kernel is rectangular
      bool IsRectangular() const { return shape_ == ShapeCode::RECTANGULAR; }

      /// \brief Tests to see if the kernel is a line
      bool IsLine() const { return shape_ == ShapeCode::LINE; }

      /// \brief Tests to see if the kernel is a custom shape
      bool IsCustom() const { return shape_ == ShapeCode::CUSTOM; }

      /// \brief Tests to see if the kernel has weights
      bool HasWeights() const {
         return ( shape_ == ShapeCode::CUSTOM ) && !image_.DataType().IsBinary();
      }

      /// \brief Returns the number of pixels in the kernel, given the image dimensionality `nDims`.
      /// This requires the creation of a `dip::PixelTable` for the kernel, so is not a trivial function.
      DIP_EXPORT dip::uint NumberOfPixels( dip::uint nDims ) const;

   private:
      ShapeCode shape_;
      FloatArray params_;
      IntegerArray shift_;
      Image image_;
      bool mirror_ = false;

      void SetShape( String const& shape ) {
         if( shape == "elliptic" ) {
            shape_ = ShapeCode::ELLIPTIC;
         } else if( shape == "rectangular" ) {
            shape_ = ShapeCode::RECTANGULAR;
         } else if( shape == "diamond" ) {
            shape_ = ShapeCode::DIAMOND;
         } else if( shape == "line" ) {
            shape_ = ShapeCode::LINE;
         } else {
            DIP_THROW_INVALID_FLAG( shape );
         }
      }
};

/// \}

} // namespace dip

#endif // DIP_KERNEL_H
