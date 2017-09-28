/*
 * DIPlib 3.0
 * This file contains declarations for look-up tables and related functionality
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

#ifndef DIP_LOOKUP_TABLE_H
#define DIP_LOOKUP_TABLE_H

#include "diplib.h"


/// \file
/// \brief Lookup tables and related functionality.
/// \see mapping


namespace dip {


/// \addtogroup mapping
/// \{


/// \brief Encapsulates the concept of the look-up table (LUT).
///
/// The `Apply` method takes a real, scalar input image and applies the LUT to it, yielding an output that
/// depends on the characteristics of the LUT, as described below.
///
/// If `HasIndex` is true, the value of each input pixel is looked up in the index, using interpolation,
/// yielding a location in the LUT. Again using interpolation, the corresponding LUT values are written
/// to the output image.
///
/// If `HasIndex` is false, the value of each input pixel is directly interpreted as a location in the LUT.
/// For a floating-point input image, interpolation is used to find the corresponding LUT value. For an
/// integer-valued input image, the pixel value is directly considered the index into the LUT (as interpolation
/// makes no sense here). Note that the first LUT value is at index 0.
///
/// The LUT can contain tensor values, yielding a tensor output image. This is useful to produce e.g. an
/// RGB image from an index representation, as used in GIF files and some TIFF files. It is also useful,
/// for example, to create a color representation from a labeled image.
///
/// The output image will have the data type of the LUT.
///
/// **Out-of-bounds handling:**
/// When an input value is outside the bounds provided by the index, it is clamped to the index range.
/// This behavior can be modified using the `SetOutOfBoundsValue` method (uses the given value or values for all
/// pixels that are out of bounds), or  the `KeepInputValueOnOutOfBounds` method (which sets the output
/// value to the original input value). The `ClampOutOfBoundsValues` method returns behavior to the
/// default.
///
/// Note that for binary images, the function `dip::Select(dip::Image,dip::Image,dip::Image,dip::Image)`
/// is available for a similar result.
class DIP_NO_EXPORT LookupTable{
   public:

      enum class OutOfBoundsMode {
            USE_OUT_OF_BOUNDS_VALUE,
            KEEP_INPUT_VALUE,
            CLAMP_TO_RANGE
      };

      enum class InterpolationMode {
            LINEAR,
            NEAREST_NEIGHBOR,
            ZERO_ORDER_HOLD
      };

      /// \brief The look-up table values are provided through an image. Optionally, provide the index.
      ///
      /// `values` must be 1D, but otherwise can be of any data type and have any number of tensor elements.
      /// The result of applying the LUT will be an image with the same data type and number of tensor
      /// elements as `values`.
      ///
      /// If `index` is given, it must have the same number of elements as pixels are in `values`, and it
      /// must be sorted small to large. No check is done on the sort order of `index`. If `index` is given,
      /// `HasIndex` will be true.
      LookupTable( Image const& values, FloatArray const& index = {} ) : values_( values ), index_( index ) {
         DIP_THROW_IF( !values_.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( values_.Dimensionality() != 1, "The look-up table must be one-dimensional" );
         if( !index_.empty() ) {
            DIP_THROW_IF( index_.size() != values_.Size( 0 ), E::SIZES_DONT_MATCH );
            // TODO: check that `index_` is sorted?
         }
      }

      template< typename InputIterator >
      LookupTable( InputIterator begin, InputIterator end, FloatArray const& index = {} ) : index_( index ) {
         using TPI = typename InputIterator::value_type;
         dip::sint n = std::distance( begin, end );
         DIP_THROW_IF( n <= 0, "The iterator range is empty" );
         if( !index_.empty() ) {
            DIP_THROW_IF( index_.size() != static_cast< dip::uint >( n ), E::SIZES_DONT_MATCH );
            // TODO: check that `index_` is sorted?
         }
         values_.ReForge( { static_cast< dip::uint >( n ) }, 1, dip::DataType( TPI() ));
         TPI* dest = static_cast< TPI* >( values_.Origin() );
         std::copy( begin, end, dest );
      }

      /// \brief True if the LUT has an index.
      bool HasIndex() const { return !index_.empty(); }

      /// \brief Returns the data type of the LUT, which will also be the data type of the result of applying the LUT.
      dip::DataType DataType() const { return values_.DataType(); }

      /// \brief Sets out-of-bounds behavior to using `value`.
      void SetOutOfBoundsValue( dfloat value ) {
         outOfBoundsLowerValue_ = value;
         outOfBoundsUpperValue_ = value;
         outOfBoundsMode_ = OutOfBoundsMode::USE_OUT_OF_BOUNDS_VALUE;
      }

      /// \brief Sets out-of-bounds behavior to using `lowerValue` and `upperValue`.
      void SetOutOfBoundsValue( dfloat lowerValue, dfloat upperValue ) {
         outOfBoundsLowerValue_ = lowerValue;
         outOfBoundsUpperValue_ = upperValue;
         outOfBoundsMode_ = OutOfBoundsMode::USE_OUT_OF_BOUNDS_VALUE;
      }

      /// \brief Sets out-of-bounds behavior to using the input value.
      void KeepInputValueOnOutOfBounds() {
         outOfBoundsMode_ = OutOfBoundsMode::KEEP_INPUT_VALUE;
      }

      /// \brief Returns out-of-bounds behavior to the default.
      void ClampOutOfBoundsValues() {
         outOfBoundsMode_ = OutOfBoundsMode::CLAMP_TO_RANGE;
      }

      DIP_EXPORT void Apply( Image const& in, Image& out, InterpolationMode interpolation = InterpolationMode::LINEAR ) const;

      /// \brief Apply the LUT to a scalar, real-valued image.
      ///
      /// See the description for `dip::LookupTable` for how this function works. `interpolation` can be one of:
      ///  - `"linear"`: the default, uses linear interpolation.
      ///  - `"nearest"`: uses nearest neighbor interpolation (i.e. rounds the input value to the nearest index).
      ///  - `"zero order"`: uses zero order hold interpolation (i.e. uses the `floor` of the input value).
      void Apply( Image const& in, Image& out, String const& interpolation ) const {
         InterpolationMode mode;
         DIP_START_STACK_TRACE
            mode = DecodeInterpolationMode( interpolation );
         DIP_END_STACK_TRACE
         Apply( in, out, mode );
      }
      Image Apply( Image const& in, String const& interpolation = "linear"  ) const {
         Image out;
         Apply( in, out, interpolation );
         return out;
      }

      DIP_EXPORT Image::Pixel Apply( dfloat value, InterpolationMode interpolation = InterpolationMode::LINEAR ) const;

      /// \brief Apply the LUT to a scalar value.
      Image::Pixel Apply( dfloat value, String const& interpolation ) const {
         InterpolationMode mode;
         DIP_START_STACK_TRACE
            mode = DecodeInterpolationMode( interpolation );
         DIP_END_STACK_TRACE
         return Apply( value, mode );
      }

   private:
      Image values_;       // The table containing the output values. 1D image, any type, possibly tensor-valued.
      FloatArray index_;   // This is where the input value is looked up, has same number of elements as `values_`, strictly monotonic.
      // If `index_` is empty, the input value is directly used as an index into `values_`.
      // Otherwise, the input value is looked up in `index_`, using interpolation, and the corresponding value in
      // `values_` is found, again using interpolation. Interpolation is always linear.

      OutOfBoundsMode outOfBoundsMode_ = OutOfBoundsMode::CLAMP_TO_RANGE;
      dfloat outOfBoundsLowerValue_;  // Used when outOfBoundsMode_==OutOfBoundsMode::USE_OUT_OF_BOUNDS_VALUE
      dfloat outOfBoundsUpperValue_;  // LowerValue is for below the lower bound, UpperValue for above the upper bound

      static InterpolationMode DecodeInterpolationMode( String const& interpolation ) {
         if( interpolation == "linear" ) {
            return InterpolationMode::LINEAR;
         } else if( interpolation == "nearest" ) {
            return InterpolationMode::NEAREST_NEIGHBOR;
         } else if( interpolation == "zero order" ) {
            return InterpolationMode::ZERO_ORDER_HOLD;
         } else {
            DIP_THROW_INVALID_FLAG( interpolation );
         }
      }
};


/// \}

} // namespace dip

#endif // DIP_LOOKUP_TABLE_H
