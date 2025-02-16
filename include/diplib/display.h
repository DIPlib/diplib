/*
 * (c)2017-2025, Cris Luengo.
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

#ifndef DIP_DISPLAY_H
#define DIP_DISPLAY_H

#include <array>
#include <utility>

#include "diplib.h"
#include "diplib/color.h"


/// \file
/// \brief Tools to prepare images for display.
/// See \ref display.


namespace dip {


/// \group display Display
/// \brief Image display
/// \addtogroup

/// \brief Encapsulates state of an image in a display window, and provides the functionality for converting the
/// image to a form suitable for display.
///
/// An object is created for a particular image; the image cannot be replaced. Different display options can then
/// be set. When the \ref Output method is called, a 1D or 2D, grey-value or RGB, UINT8 image is
/// prepared for display. A const reference to this image is returned. The image is updated every time the
/// `Output` method is called, not when display options are set. The display options are designed to be
/// settable by a user using the image display window.
///
/// For a scalar input image, the output is always scalar (grey-value). For a color image, if it can be converted
/// to sRGB, and RGB output image is produced. For other tensor images, an RGB image is also produced, the user
/// can select which tensor element is shown in each of the three color channels. Note that for color images,
/// the non-linear sRGB color space is used for display, linear RGB images are gamma-corrected in this way to
/// improve display.
///
/// See the `dipimage/private/imagedisplay.cpp` file implementing the MATLAB interface to this class, and the
/// `dipimage/dipshow.m` function, for an example of how this can be used.
class DIP_NO_EXPORT ImageDisplay {
   public:

      /// Enumerator for the projection mode
      enum class ProjectionMode : uint8 {
         SLICE,      ///< A slice is prepared for display
         MAX,        ///< The max projection is prepared for display
         MEAN        ///< The mean projection is prepared for display
      };
      /// Enumerator for the complex mapping mode
      enum class ComplexMode : uint8 {
         MAGNITUDE,  ///< The magnitude is prepared for display
         PHASE,      ///< The complex phase is prepared for display
         REAL,       ///< The real component is prepared for display
         IMAG        ///< The imaginary component is prepared for display
      };
      /// Enumerator for the intensity mapping mode
      enum class MappingMode : uint8 {
         MANUAL,     ///< \ref Limits are used as-is
         MAXMIN,     ///< The max and min values are taken as the display limits
         PERCENTILE, ///< The 5% and 95% values are taken as the display limits
         BASED,      ///< 0 should remain at the middle of the output range
         LOGARITHMIC,///< A logarithmic mapping is applied
         MODULO      ///< The integer input values are mapped modulo the output range
      };
      /// Intensity mapping limits
      struct Limits {
         dfloat lower; ///< This value is mapped to 0.
         dfloat upper; ///< This value is mapped to 255.
      };

      // No default constructor, no copy construction or assignment. We do allow move!
      // The reason we disallow copy constructors and assignment is that these shouldn't be necessary.
      ImageDisplay() = delete;
      ImageDisplay( ImageDisplay const& ) = delete;
      ImageDisplay( ImageDisplay&& ) noexcept = default;
      ImageDisplay& operator=( ImageDisplay const& ) = delete;
      ImageDisplay& operator=( ImageDisplay&& ) noexcept = default;
      ~ImageDisplay() = default;

      /// \brief The constructor takes an image with at least 1 dimension.
      ///
      /// If `colorSpaceManager` is not `nullptr`, it points to the color space manager object to be
      /// used to convert the color image `image` to sRGB. If `image` is not color, or already is sRGB, the color
      /// space manager is not used. If no color space manager is given, `image` will be shown as is,
      /// no color space conversion is applied.
      ///
      /// If `externalInterface` is not `nullptr`, then it is used to allocate the data segment for the
      /// output image.
      ///
      /// Both `colorSpaceManager` and `externalInterface`, if given, must exist for as long as the
      /// `ImageDisplay` object exists.
      explicit ImageDisplay( Image const& image, ColorSpaceManager* colorSpaceManager = nullptr, ExternalInterface* externalInterface = nullptr ) :
            image_( image ), colorspace_( image.ColorSpace() ), colorSpaceManager_( colorSpaceManager ) {
         DIP_THROW_IF( !image_.IsForged(), E::IMAGE_NOT_FORGED );
         // Dimensionality
         dip::uint nDims = image_.Dimensionality();
         DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         if( nDims == 1 ) {
            twoDimOut_ = false;
            dim2_ = dim1_;
         } else if( nDims > 2 ) {
            FillOrthogonal();
         }
         // Tensor dimension
         if( !colorspace_.empty() ) {
            if( !colorSpaceManager_ ||
                !colorSpaceManager_->IsDefined( colorspace_ ) ||
                colorSpaceManager_->NumberOfChannels( colorspace_ ) != image_.TensorElements() ) {
               // We won't be able to convert this image to sRGB, let's treat it as a tensor image.
               colorspace_.clear();
            }
         }
         if( colorspace_.empty() ) {
            colorSpaceManager_ = nullptr;
            if( !image_.IsScalar() ) {
               // tensor image
               green_ = 1;
               if( image_.TensorElements() > 2 ) {
                  blue_ = 2;
               }
            } // else grey-value image
         } else {
            // color image, shown as RGB
            green_ = 1;
            blue_ = 2;
         }
         // Data type
         if( IsBinary() ) {
            range_ = { 0.0, 1.0 }; // Different default for binary images
         } else if( IsComplex() ) {
            complexMode_ = ComplexMode::MAGNITUDE; // Different default for complex images
         }
         coordinates_.resize( image_.Dimensionality(), 0 );
         if( externalInterface ) {
            output_.SetExternalInterface( externalInterface );
         }
      }

      /// \brief Retrieves a reference to the input image.
      Image const& Input() const {
         return image_;
      }

      /// \brief Retrieves a reference to the raw slice image.
      ///
      /// This function also causes an update of the slice if the projection changed. The raw slice image contains
      /// the input data for the what is shown in \ref Output.
      Image const& Slice() {
         UpdateSlice();
         return slice_;
      }

      /// \brief Retrieves a reference to the output image.
      ///
      /// This function also causes an update of the output if any of the modes changed.
      ///
      /// The output image data segment will be allocated using the external interface provided to the
      /// `ImageDisplay` constructor.
      Image const& Output() {
         UpdateOutput();
         return output_;
      }

      /// \brief Puts a single pixel through the same mapping the image will go through to become \ref Output.
      DIP_EXPORT Image::Pixel MapSinglePixel( Image::Pixel const& input );

      /// \brief Returns true if the next call to \ref Output will yield a different result from the previous one.
      /// That is, the display needs to be redrawn.
      bool OutIsDirty() const { return outputIsDirty_ || rgbSliceIsDirty_ || sliceIsDirty_; }

      /// \brief Returns true if the next call to \ref Output will yield a different slice
      bool SliceIsDirty() const { return sliceIsDirty_; }

      /// \brief Returns true if the next call to \ref Output will yield an output of a different size. That is,
      /// the slicing direction has changed, and this yields a change in sizes.
      bool SizeIsDirty() const { return sizeIsDirty_; }

      /// \brief Gets input image intensities at a given 2D point (automatically finds corresponding nD location).
      /// In case of a 1D \ref Output, `y` is ignored.
      Image::Pixel Pixel( dip::uint x, dip::uint y = 0 ) {
         UpdateSlice();
         if( x >= slice_.Size( 0 )) {
            x = slice_.Size( 0 ) - 1;
         }
         if( slice_.Dimensionality() == 1 ) {
            // 1D slice
            return slice_.At( x );
         } else {
            // 2D slice
            if( y >= slice_.Size( 1 )) {
               y = slice_.Size( 1 ) - 1;
            }
            return slice_.At( x, y );
         }
      }

      /// \brief Sets the projection/slicing direction, as the two image dimensions to show along the x and y axis
      /// of the 2D display. If `dim1==dim2`, a 1D output is produced.
      void SetDirection( dip::uint dim1, dip::uint dim2 ) {
         dip::uint nDim = image_.Dimensionality();
         DIP_THROW_IF(( dim1 >= nDim ) || ( dim2 >= nDim ), E::ILLEGAL_DIMENSION );
         if(( dim1_ != dim1 ) || ( dim2_ != dim2 )) {
            twoDimOut_ = dim1 != dim2;
            // Will the output sizes change?
            if(( dim1_ != dim2_ ) ^ twoDimOut_ ) {
               // changing from 1D to 2D out, or reverse
               sizeIsDirty_ = true;
            } else {
               if(( image_.Size( dim1_ ) != image_.Size( dim1 )) || ( image_.Size( dim2_ ) != image_.Size( dim2 ))) {
                  sizeIsDirty_ = true;
               }
            }
            // Update dimensions
            dim1_ = dim1;
            dim2_ = dim2;
            sliceIsDirty_ = true;
            // Make sure projection mode is always `"slice"` if `ndims(img)==ndims(out)`
            if( twoDimOut_ && nDim == 2 ) {
               projectionMode_ = ProjectionMode::SLICE;
            }
            FillOrthogonal();
         }
      }

      /// \brief Sets the current coordinates. This affects the slice displayed.
      void SetCoordinates( UnsignedArray coordinates ) {
         DIP_THROW_IF( coordinates.size() != coordinates_.size(), E::ARRAY_PARAMETER_WRONG_LENGTH );
         for( dip::uint ii = 0; ii < coordinates_.size(); ++ii ) {
            if( coordinates[ ii ] >= image_.Size( ii )) {
               coordinates[ ii ] = image_.Size( ii ) - 1;
            }
            if( coordinates_[ ii ] != coordinates[ ii ] ) {
               coordinates_[ ii ] = coordinates[ ii ];
               if(( projectionMode_ == ProjectionMode::SLICE ) && ( ii != dim1_ ) && ( ii != dim2_ )) {
                  sliceIsDirty_ = true;
               }
            }
         }
      }

      /// \brief Sets the tensor element to be shown in each of the three output channels.
      ///
      /// This function only has an effect for tensor images without a color space.
      void SetTensorElements( dip::sint red, dip::sint green = -1, dip::sint blue = -1 ) {
         dip::sint N = static_cast< dip::sint >( image_.TensorElements() );
         if(( N > 1 ) && colorspace_.empty() ) {
            red_ = red < N ? red : -1;
            green_ = green < N ? green : -1;
            blue_ = blue < N ? blue : -1;
            rgbSliceIsDirty_ = true;
         }
      }

      /// \brief Sets the projection mode. Has no effect if image dimensionality is equal to projection dimensionality.
      void SetProjectionMode( ProjectionMode projectionMode ) {
         if(( image_.Dimensionality() > ( twoDimOut_ ? 2u : 1u )) && ( projectionMode_ != projectionMode )) {
            projectionMode_ = projectionMode;
            sliceIsDirty_ = true;
            if( projectionMode_ != ProjectionMode::SLICE ) {
               globalStretch_ = false;
            }
            if( projectionMode_ == ProjectionMode::MAX ) {
               complexMode_ = ComplexMode::MAGNITUDE;
            }
         }
      }

      /// \brief Sets the projection mode. Has no effect if image dimensionality is equal to projection dimensionality.
      ///
      /// | `projectionMode` value   | Meaning |
      /// | ------------------------ | ------- |
      /// | `"slice"`                | the 1D/2D image shown is a slice through the nD image |
      /// | `"max"`                  | the 1D/2D image shown is the max projection of the nD image |
      /// | `"mean"`                 | the 1D/2D image shown is the mean projection of the nD image |
      ///
      /// For an image with complex samples, setting the projection mode to `"max"`
      /// forces the complex to real mapping mode to `"magnitude"`.
      ///
      /// For projection modes other than `"slice"`, turns off global stretch mode.
      void SetProjectionMode( String const& projectionMode ) {
         if( projectionMode == "slice" ) {
            SetProjectionMode( ProjectionMode::SLICE );
         } else if( projectionMode == "max" ) {
            SetProjectionMode( ProjectionMode::MAX );
         } else if( projectionMode == "mean" ) {
            SetProjectionMode( ProjectionMode::MEAN );
         } else {
            DIP_THROW_INVALID_FLAG( projectionMode );
         }
      }

      /// \brief Sets the complex to real mapping mode. Has no effect when projection mode is set to `"max"`, or for
      /// non-complex images.
      void SetComplexMode( ComplexMode complexMode ) {
         if( IsComplex() && ( projectionMode_ != ProjectionMode::MAX ) && ( complexMode_ != complexMode )) {
            complexMode_ = complexMode;
            outputIsDirty_ = true;
         }
      }

      /// \brief Sets the complex to real mapping mode. Has no effect when projection mode is set to `"max"`, or for
      /// non-complex images.
      ///
      /// | `complexMode` value   | Meaning |
      /// | --------------------- | ------- |
      /// | `"magnitude"`         | the intensity displayed is the magnitude of the complex values |
      /// | `"abs"`               | synonym for `"magnitude"` |
      /// | `"phase"`             | the intensity displayed is the phase of the complex values |
      /// | `"real"`              | the intensity displayed is the real component of the complex values |
      /// | `"imag"`              | the intensity displayed is the imaginary component of the complex values |
      void SetComplexMode( String const& complexMode )  {
         if(( complexMode == "abs" ) || ( complexMode == "magnitude" )) {
            SetComplexMode( ComplexMode::MAGNITUDE );
         } else if( complexMode == "phase" ) {
            SetComplexMode( ComplexMode::PHASE );
         } else if( complexMode == "real" ) {
            SetComplexMode( ComplexMode::REAL );
         } else if( complexMode == "imag" ) {
            SetComplexMode( ComplexMode::IMAG );
         } else {
            DIP_THROW_INVALID_FLAG( complexMode );
         }
      }

      /// \brief Sets the intensity mapping mode. Has no effect for binary images.
      void SetMappingMode( MappingMode mappingMode ) {
         if( !IsBinary() && ( mappingMode_ != mappingMode )) {
            mappingMode_ = mappingMode;
            outputIsDirty_ = true;
            if( mappingMode_ == MappingMode::MODULO ) {
               range_ = { 0.0, 255.0 };
            }
         }
      }

      /// \brief Sets the range of intensities to be mapped to the output range. Forces intensity mapping mode to linear.
      /// Has no effect for binary images.
      void SetRange( Limits range ) {
         if( !IsBinary() ) {
            mappingMode_ = MappingMode::MANUAL;
            if(( range_.lower != range.lower ) ||
               ( range_.upper != range.upper )) {
               range_ = range;
               outputIsDirty_ = true;
            }
         }
      }

      /// \brief Sets the mapping mode and the range of intensities to be mapped to the output range.
      /// Has no effect for binary images.
      ///
      /// | `range` value   | Meaning |
      /// | --------------- | ------- |
      /// | `"unit"`        | [0, 1] |
      /// | `"8bit"`        | [0, 255] |
      /// | `"12bit"`       | [0, 4095] |
      /// | `"16bit"`       | [0, 65535] |
      /// | `"s8bit"`       | [-128, 127] |
      /// | `"s12bit"`      | [-2048, 2047] |
      /// | `"s16bit"`      | [-32768, 32767] |
      /// | `"angle"`       | [-&pi;, &pi;] |
      /// | `"orientation"` | [-&pi;/2, &pi;/2] |
      /// | `"lin"`         | uses the full range of the image (or slice) |
      /// | `"percentile"`  | uses the 5% to 95% range of the image |
      /// | `"base"`        | uses the full range of the image (or slice), but keeps 0 at the middle of the output range |
      /// | `"log"`         | the logarithm of the intensities are mapped to the full output range |
      /// | `"modulo"`      | the integer input values are mapped modulo the output range |
      /// | Additionally, the following aliases are defined: ||
      /// | `"normal"`      | same as `"8bit"` |
      /// | `"linear"`      | same as `"lin"` |
      /// | `"all"`         | same as `"lin"` |
      /// | `"based"`       | same as `"base"` |
      /// | `"labels"`      | same as `"modulo"` |
      void SetRange( String const& range ) {
         if( range == "unit" ) {
            SetRange( Limits{ 0.0, 1.0 } );
         } else if(( range == "normal" ) || ( range == "8bit" )) {
            SetRange( Limits{ 0.0, 255.0 } );
         } else if( range == "12bit" ) {
            SetRange( Limits{ 0.0, 4095.0 } );
         } else if( range == "16bit" ) {
            SetRange( Limits{ 0.0, 65535.0 } );
         } else if( range == "s8bit" ) {
            SetRange( Limits{ -128.0, 127.0 } );
         } else if( range == "s12bit" ) {
            SetRange( Limits{ -2048.0, 2047.0 } );
         } else if( range == "s16bit" ) {
            SetRange( Limits{ -32768.0, 32767.0 } );
         } else if( range == "angle" ) {
            SetRange( Limits{ -pi, pi } );
         } else if( range == "orientation" ) {
            SetRange( Limits{ -pi / 2.0, pi / 2.0 } );
         } else if(( range == "lin" ) || ( range == "linear" ) || ( range == "all" )) {
            SetMappingMode( MappingMode::MAXMIN );
         } else if( range == "percentile" ) {
            SetMappingMode( MappingMode::PERCENTILE );
         } else if(( range == "base" ) || ( range == "based" )) {
            SetMappingMode( MappingMode::BASED );
         } else if( range == "log" ) {
            SetMappingMode( MappingMode::LOGARITHMIC );
         } else if(( range == "modulo" ) || ( range == "labels" )) {
            SetMappingMode( MappingMode::MODULO );
         } else {
            DIP_THROW_INVALID_FLAG( range );
         }
      }

      /// \brief Sets the global stretch mode. Has no effect on 2D images or when the projection mode is not `"slice"`
      void SetGlobalStretch( bool globalStretch ) {
         if(( projectionMode_ == ProjectionMode::SLICE ) &&
            ( image_.Dimensionality() > 2 ) &&
            ( globalStretch_ != globalStretch )) {
            globalStretch_ = globalStretch;
            outputIsDirty_ = true;
         }
      };

      /// \brief Sets the global stretch mode. Has no effect on 2D images or when the projection mode is not `"slice"`
      ///
      /// | `globalStretch` value | Meaning |
      /// | --------------------- | ------- |
      /// | `"yes"` or `"on"` | intensity stretching is computed using all values in the image |
      /// | `"no"` or `"off"` | intensity stretching is computed using only values visible in the current slice |
      void SetGlobalStretch( String const& globalStretch ) {
         if(( globalStretch == "yes" ) || ( globalStretch == "on" )) {
            SetGlobalStretch( true );
         } else if(( globalStretch == "no" ) || ( globalStretch == "off" )) {
            SetGlobalStretch( false );
         } else {
            DIP_THROW_INVALID_FLAG( globalStretch );
         }
      }

      /// \brief Get the projection/slicing direction. The two values returned are identical when output is 1D.
      std::pair< dip::uint, dip::uint > GetDirection() const { return { dim1_, dim2_ }; };

      /// \brief Returns the array of dimensions orthogonal to those returned by \ref GetDirection. These are the
      /// dimensions not displayed.
      UnsignedArray const& GetOrthogonal() const { return orthogonal_; }

      /// \brief Get the current coordinates.
      UnsignedArray const& GetCoordinates() const { return coordinates_; }

      /// \brief Get the image sizes.
      UnsignedArray const& GetSizes() const { return image_.Sizes(); }

      /// \brief Get the image dimensionality.
      dip::uint Dimensionality() const { return image_.Dimensionality(); }

      /// \brief Get the tensor element to be shown in the red channel.
      dip::sint GetRedTensorElement() const { return red_; }
      /// \brief Get the tensor element to be shown in the green channel.
      dip::sint GetGreenTensorElement() const { return green_; }
      /// \brief Get the tensor element to be shown in the blue channel.
      dip::sint GetBlueTensorElement() const { return blue_; }

      /// \brief Get the current projection mode.
      String GetProjectionMode() const {
         switch( projectionMode_ ) {
            //case ProjectionMode::SLICE:
            default:                    return "slice";
            case ProjectionMode::MAX:   return "max";
            case ProjectionMode::MEAN:  return "mean";
         }
      }

      /// \brief Get the current complex to real mapping mode.
      String GetComplexMode() const {
         switch( complexMode_ ) {
            //case ComplexMode::MAGNITUDE:
            default:                   return "magnitude";
            case ComplexMode::PHASE:   return "phase";
            case ComplexMode::REAL:    return "real";
            case ComplexMode::IMAG:    return "imag";
         }
      }

      /// \brief Get the current intensity mapping mode.
      String GetMappingMode() const {
         switch( mappingMode_ ) {
            //case MappingMode::MANUAL:
            default:                         return "manual";
            case MappingMode::MAXMIN :       return "lin";
            case MappingMode::PERCENTILE :   return "percentile";
            case MappingMode::BASED :        return "based";
            case MappingMode::LOGARITHMIC:   return "log";
            case MappingMode::MODULO:        return "modulo";
         }
      }

      /// \brief Get the current intensity range.
      Limits GetRange() const { return range_; }

      /// \brief Gets the image intensity range (that selected with `"lin"`) for the current slicing and complex
      /// mapping modes. If `compute` is true, it computes them if they're not yet computed.
      DIP_EXPORT Limits GetLimits( bool compute );

      /// \brief Get the current global stretch mode.
      bool GetGlobalStretch() const { return globalStretch_; }

   private:

      // A copy of the original image, so we're not dependent on the original image still existing. This is where
      // data are fetched when slice mode, direction or location is changed.
      Image image_;
      // The 1D/2D slice to be displayed (could be either shared data with the image, or in case of a projection,
      // owning its own data). This is where intensity lookup is performed. Contains all the same tensor elements
      // as `image_`
      Image slice_;
      // Another 1D/2D slice, either identical to slice_, or converted to RGB if slice_ is in a different color
      // space, or with selected tensor elements for display (has 1 or 3 tensor elements). This is where output_
      // is computed from when e.g. the mapping mode changes.
      Image rgbSlice_;
      // The output image: 1D/2D UINT8, 1 or 3 tensor elements.
      // The external interface controls allocation of the data segment for this image.
      Image output_;

      // Changing display flags causes one or more "dirty" flags to be set. This indicates that the corresponding
      // image needs to be recomputed. If one flag is set, the ones below it are also (implicitly) set.
      bool sizeIsDirty_ = true;     // true if a new slice direction was chosen -- output sizes will change
      bool sliceIsDirty_ = true;    // corresponds to slice_
      bool rgbSliceIsDirty_ = true; // corresponds to rgbSlice_
      bool outputIsDirty_ = true;   // corresponds to output_

      // The color space of the input image
      String colorspace_;
      ColorSpaceManager* colorSpaceManager_;

      // Display flags
      dip::uint dim1_ = 0;        // slicing direction x
      dip::uint dim2_ = 1;        // slicing direction y (keep identical to dim1_ if `!twoDimOut_`)
      UnsignedArray orthogonal_;  // dimensions orthogonal to dim1_ and dim2_.
      bool twoDimOut_ = true;     // slice is 1D or 2D?
      dip::sint red_ = 0;         // tensor element to display in the red channel (or in the grey-value image)
      dip::sint green_ = -1;      // tensor element to display in the green channel
      dip::sint blue_ = -1;       // tensor element to display in the blue channel
      UnsignedArray coordinates_; // coordinates through which the slice is taken
      ProjectionMode projectionMode_ = ProjectionMode::SLICE;
      ComplexMode complexMode_ = ComplexMode::REAL;
      MappingMode mappingMode_ = MappingMode::MANUAL;
      Limits range_ = { 0.0, 255.0 };
      bool globalStretch_ = false;

      // Information about the image:
      //    sliceLimits_[ (int)REAL ].maxMin -> max and min values to use when in REAL complex mapping mode.
      // When limits are NaN, it means they haven't been computed yet.
      struct LimitsLists {
         Limits maxMin = { nan, nan };
         Limits percentile = { nan, nan };
      };
      std::array< LimitsLists, 4 > sliceLimits_;  // Limits to use when !globalStretch_
      std::array< LimitsLists, 4 > globalLimits_; // Limits to use when globalStretch_

      bool IsComplex() { return image_.DataType().IsComplex(); }
      bool IsBinary() { return image_.DataType().IsBinary(); }
      bool IsInteger() { return image_.DataType().IsInteger(); }

      // Computes limits for current mode, if they hadn't been computed yet.
      // If `set`, sets the range_ value to the limits for the current mode.
      DIP_NO_EXPORT void ComputeLimits( bool set = true );

      DIP_NO_EXPORT void InvalidateSliceLimits();

      DIP_EXPORT void UpdateSlice();
      DIP_NO_EXPORT void UpdateRgbSlice();
      DIP_EXPORT void UpdateOutput();

      void FillOrthogonal() {
         dip::uint nDims = image_.Dimensionality();
         orthogonal_.resize( nDims - ( twoDimOut_ ? 2 : 1 ));
         dip::uint jj = 0;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if(( ii != dim1_ ) && ( ii != dim2_ )) {
               orthogonal_[ jj++ ] = ii;
            }
         }
      }
};

class DIP_NO_EXPORT LookupTable;  // forward declaration

/// \brief Creates a \ref LookupTable with a pre-computed sRGB color map.
///
/// All color maps have 256 values, and should be applied to images normalized to the range [0, 255].
/// You will need to include \ref "diplib/lookup_table.h" where the output class is defined.
///
/// `colorMap` can currently be one of the following color maps:
///
/// - `"grey"`: Each grey level maps to an RGB value that represents the same grey level.
/// - `"saturation"`: Each grey level maps to an RGB value that represents the same grey level, except pixels
///   with a value 0 and 255, which are colored blue and red respectively. This can be used to show which
///   pixels were likely saturated during acquisition.
/// - `"linear"`: A blue-magenta-yellow highly saturated, perceptually linear color map.
/// - `"diverging"`: A blue-grey-yellow diverging, perceptually linear color map, where the middle value maps
///   to a neutral grey-value, high values map to increasingly bright yellows, and low values map to increasingly
///   bright blues. This is meant to be used in combination with the `"base"` range mode of \ref dip::ImageDisplay.
/// - `"cyclic"`: A magenta-yellow-green-blue cyclic, perceptually linear color map, which allows four orientations
///   or angles to be visualised. Use in combination with the `"angle"` or `"orientation"` range mode of
///   \ref dip::ImageDisplay.
/// - `"label"`: For labeled images, each grey value gets a color that can easily be distinguished from
///   that of nearby grey values. 16 different colors are used. The 0 grey value is considered background
///   and colored black. Use with the `"modulo"` range mode of \ref dip::ImageDisplay.
///
/// The `"linear"`, `"diverging"` and `"cyclic"` are by [Peter Kovesi](https://colorcet.com).
///
/// !!! literature
///     - Peter Kovesi, "Good Colour Maps: How to Design Them", [arXiv:1509.03700](https://arxiv.org/abs/1509.03700) [cs.GR], 2015.
DIP_EXPORT LookupTable ColorMapLut(
      String const& colorMap = "grey"
);

/// \brief Applies a color map to an image prepared for display using \ref dip::ImageDisplay.
///
/// `in` is a scalar, 8-bit unsigned image. `out` will be an image of the same size and type
/// but with three tensor components, and in the "sRGB" color space.
///
/// See \ref ColorMapLut for possible values for `colorMap`.
DIP_EXPORT void ApplyColorMap(
      Image const& in,
      Image& out,
      String const& colorMap = "grey"
);
DIP_NODISCARD inline Image ApplyColorMap(
      Image const& in,
      String const& colorMap
) {
   Image out;
   ApplyColorMap( in, out, colorMap );
   return out;
}

/// \brief Adds a colored overlay to the image `in`, yielding an RGB image.
///
/// `in` must be either scalar (grey-value image) or RGB. `overlay` can be binary or integer.
///
/// In the case of a binary overlay image, the pixels selected by it will be assigned the value `color`,
/// which defaults to red. If `color` is a scalar value, it will be interpreted as an intensity value, producing
/// a grey overlay. In this latter case, if `in` was a scalar image, then the output will be scalar as well.
///
/// In the case of an integer overlay image, \ref dip::ApplyColorMap with the `"label"` option will be used to
/// create a label image overlay. `color` will be ignored.
DIP_EXPORT void Overlay(
      Image const& in,
      Image const& overlay,
      Image& out,
      Image::Pixel const& color = { 255, 0, 0 }
);
DIP_NODISCARD inline Image Overlay(
      Image const& in,
      Image const& overlay,
      Image::Pixel const& color = { 255, 0, 0 }
) {
   Image out;
   Overlay( in, overlay, out, color );
   return out;
}

/// \brief Upscales a labeled image and outlines each region with the background label.
///
/// `out` will be a labeled image like `labels`, but `factor` times as large along each dimension.
/// In the upscaled image, the pixels that form the border of each region are set to 0, the background label.
DIP_EXPORT void MarkLabelEdges(
      Image const& labels,
      Image& out,
      dip::uint factor = 2
);
DIP_NODISCARD inline Image MarkLabelEdges(
      Image const& labels,
      dip::uint factor = 2
) {
   Image out;
   MarkLabelEdges( labels, out, factor );
   return out;
}

/// \endgroup

} // namespace dip

#endif // DIP_DISPLAY_H
