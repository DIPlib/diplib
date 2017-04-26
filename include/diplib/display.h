/*
 * DIPlib 3.0
 * This file contains declarations for the ImageDisplay class
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

#ifndef DIP_DISPLAY_H
#define DIP_DISPLAY_H

#include "diplib.h"


/// \file
/// \brief Declares the `dip::ImageDisplay` class.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


// TODO: need a way to pass a color space manager.


/// \brief Encapsulates state of an image in a display window, and provides the functionality for converting the
/// image to a form suitable for display.
///
/// An object is created for a particular image; the image cannot be replaced. Different display options can then
/// be set. When the `dip::ImageDisplay::Output` method is called, a 1D or 2D, grey-value or RGB, UINT8 image is
/// prepared for display. A const reference to this image is returned. The image is updated every time the
/// `%Output` method is called, not when display options are set. The display options are designed to be
/// settable by a user using the image display window.
///
/// See the `dipimage/imagedisplay.cpp` file implementing the MATLAB interface to this class, and the
/// `dipimage/dipshow.m` function, for an example of how this can be used.
class DIP_NO_EXPORT ImageDisplay{
   public:

      enum class ProjectionMode : unsigned char { SLICE, MAX, MEAN };
      enum class ComplexMode : unsigned char { MAGNITUDE, PHASE, REAL, IMAG };
      enum class MappingMode : unsigned char { MANUAL, MAXMIN, PERCENTILE, BASED, LOGARITHMIC };
      struct Limits {
         dfloat lower;
         dfloat upper;
      };

      // No default constructor, no copy construction or assignment. We do allow move!
      // The reason we disallow copy constructors and assignment is that these shouldn't be necessary.
      ImageDisplay() = delete;
      ImageDisplay( ImageDisplay const& ) = delete;
      ImageDisplay& operator=( const ImageDisplay& ) = delete;

      /// \brief The constructor takes an image with at least 1 dimension.
      ImageDisplay( Image const& image, ExternalInterface* externalInterface = nullptr ) :
            image_( image.QuickCopy() ), colorspace_( image.ColorSpace() ) {
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
         if( image_.IsScalar() ) {
            // grey-value image
            colorspace_.clear();
         } else {
            if(( colorspace_ == "RGB" ) && ( image_.TensorElements() != 3 )) {
               colorspace_.clear();
            }
            if( colorspace_.empty() ) {
               // tensor image
               green_ = 1;
               if( image_.TensorElements() > 2 ) {
                  blue_ = 2;
               }
            } else {
               // color image, shown as RGB
               green_ = 1;
               blue_ = 2;
            }
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

      /// \brief Retrives a reference to the input image.
      Image const& Input() const {
         return image_;
      }

      /// \brief Retrives a reference to the raw slice image. This function also causes an update of the slice
      /// if the projection changed. The raw slice image contains the input data for the what is shown in
      /// `Output`.
      DIP_EXPORT Image const& Slice();

      /// \brief Retrives a reference to the output image. This function also causes an update of the output if
      /// any of the modes changed.
      DIP_EXPORT Image const& Output();

      /// \brief Returns true if the next call to `Output` will yield a different result from the previous one.
      /// That is, the display needs to be redrawn.
      bool OutIsDirty() const { return outputIsDirty_ || rgbSliceIsDirty_ || sliceIsDirty_; }

      /// \brief Returns true if the next call to `Output` will yield a different slice, which means that the
      /// output image could have a different size (and be reallocated).
      bool SliceIsDirty() const { return sliceIsDirty_; }

      /// \brief Gets input image intensities at a given 2D point (automatically finds corresponding nD location).
      /// Because the pixel can be of different types (integer, float, complex) and can have up to three samples,
      /// a string is returned with appropriately formatted values. In case of a 1D `Output`, `y` is ignored.
      DIP_EXPORT String Pixel( dip::uint x, dip::uint y = 0 );

      /// \brief Sets the projection/slicing direction, as the two image dimensions to show along the x and y axis
      /// of the 2D display. If `dim1==dim2`, a 1D output is produced.
      void SetDirection( dip::uint dim1, dip::uint dim2 ) {
         dip::uint nDim = image_.Dimensionality();
         DIP_THROW_IF(( dim1 >= nDim ) || ( dim2 >= nDim ), E::ILLEGAL_DIMENSION );
         bool update = false;
         if( dim1 == dim2 ) {
            if( twoDimOut_ || ( dim1_ != dim1 )) {
               twoDimOut_ = false;
               dim1_ = dim1;
               dim2_ = dim1; // set same as dim1_ so SetCoordinates() is easier.
               update = true;
            }
         } else {
            if( !twoDimOut_ || ( dim1_ != dim1 ) || ( dim2_ != dim2 )) {
               twoDimOut_ = true;
               dim1_ = dim1;
               dim2_ = dim2;
               update = true;
            }
         }
         if( update ) {
            sliceIsDirty_ = true;
            if( twoDimOut_ && nDim == 2 ) { // Make sure projection mode is always "slice" if ndims(img)==ndims(out)
               projectionMode_ = ProjectionMode::SLICE;
            }
            FillOrthogonal();
         }
      }

      /// \brief Sets the current coordinates. This affects the slice displayed.
      void SetCoordinates( UnsignedArray coordinates ) {
         DIP_THROW_IF( coordinates.size() != coordinates_.size(), E::ARRAY_ILLEGAL_SIZE );
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

      /// \brief Sets the projection mode. Has no effect if image dimensionality is equal to projection dimensionality.
      void SetProjectionMode( ProjectionMode projectionMode ) {
         if(( image_.Dimensionality() > ( twoDimOut_ ? 2 : 1 )) && ( projectionMode_ != projectionMode )) {
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
      /// Valid projection modes are:
      /// - "slice": the 1D/2D image shown is a slice through the nD image.
      /// - "max": the 1D/2D image shown is the max projection of the nD image.
      /// - "mean": the 1D/2D image shown is the mean projection of the nD image.
      ///
      /// For an image with complex samples, setting the projection mode to "max"
      /// forces the complex to real mapping mode to "magnitude".
      ///
      /// For projection modes other than "slice", turns off global stretch mode.
      void SetProjectionMode( String const& projectionMode ) {
         if( projectionMode == "slice" ) {
            SetProjectionMode( ProjectionMode::SLICE );
         } else if( projectionMode == "max" ) {
            SetProjectionMode( ProjectionMode::MAX );
         } else if( projectionMode == "mean" ) {
            SetProjectionMode( ProjectionMode::MEAN );
         } else {
            DIP_THROW( E::INVALID_FLAG );
         }
      }

      /// \brief Sets the complex to real mapping mode. Has no effect when projection mode is set to "max", or for
      /// non-complex images.
      void SetComplexMode( ComplexMode complexMode ) {
         if( IsComplex() && ( projectionMode_ != ProjectionMode::MAX ) && ( complexMode_ != complexMode )) {
            complexMode_ = complexMode;
            outputIsDirty_ = true;
         }
      }

      /// \brief Sets the complex to real mapping mode. Has no effect when projection mode is set to "max", or for
      /// non-complex images.
      ///
      /// Valid complex to real mapping modes are:
      /// - "magnitude": the intensity displayed is the magnitude of the complex values
      /// - "abs": synonym for "magnitude".
      /// - "phase": the intensity displayed is the phase of the complex values.
      /// - "real": the intensity displayed is the real component of the complex values.
      /// - "imag": the intensity displayed is the imaginary component of the complex values.
      void SetComplexMode( String const& complexMode )  {
         if( complexMode == "abs" ) {
            SetComplexMode( ComplexMode::MAGNITUDE );
         } else if( complexMode == "magnitude" ) {
            SetComplexMode( ComplexMode::MAGNITUDE );
         } else if( complexMode == "phase" ) {
            SetComplexMode( ComplexMode::PHASE );
         } else if( complexMode == "real" ) {
            SetComplexMode( ComplexMode::REAL );
         } else if( complexMode == "imag" ) {
            SetComplexMode( ComplexMode::IMAG );
         } else {
            DIP_THROW( E::INVALID_FLAG );
         }
      }

      /// \brief Sets the intensity mapping mode. Has no effect for binary images.
      void SetMappingMode( MappingMode mappingMode ) {
         if( !IsBinary() && ( mappingMode_ != mappingMode )) {
            mappingMode_ = mappingMode;
            outputIsDirty_ = true;
         }
      }

      /// \brief Sets the range of intensities to be mapped to the output range. Forces intensity mapping mode to linear.
      /// Has no effect for binary images.
      void SetRange( Limits range ) {
         DIP_THROW_IF( range.lower >= range.upper, E::PARAMETER_OUT_OF_RANGE );
         if( !IsBinary() ) {
            mappingMode_ = MappingMode::MANUAL;
            if(( range_.lower != range.lower ) ||
               ( range_.upper != range.upper )) {
               range_ = range;
               outputIsDirty_ = true;
            }
         }
      }

      /// \brief Sets the range of intensities to be mapped to the output range. Forces intensity mapping mode to linear.
      /// Has no effect for binary images.
      ///
      /// Valid range modes are:
      /// - "unit": [0, 1].
      /// - "normal": same as "8bit".
      /// - "8bit": [0, 255].
      /// - "12bit": [0, 4095].
      /// - "16bit": [0, 65535].
      /// - "s8bit": [-128, 127].
      /// - "s12bit": [-2048, 2047].
      /// - "s16bit": [-32768, 32767].
      /// - "angle": [-pi, pi].
      /// - "orientation": [-pi/2, pi/2].
      /// - "lin": uses the full range of the image (or slice).
      /// - "all": same as "lin".
      /// - "percentile": uses the 5% to 95% range of the image.
      /// - "base": uses the full range of the image (or slice), but keeps 0 at the middle of the output range.
      /// - "based": same as "base".
      /// - "log": the logarithm of the intensities are mapped to the full output range.
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
         } else if(( range == "lin" ) || ( range == "all" )) {
            SetMappingMode( MappingMode::MAXMIN );
         } else if( range == "percentile" ) {
            SetMappingMode( MappingMode::PERCENTILE );
         } else if(( range == "base" ) || ( range == "based" )) {
            SetMappingMode( MappingMode::BASED );
         } else if( range == "log" ) {
            SetMappingMode( MappingMode::LOGARITHMIC );
         } else {
            DIP_THROW( E::INVALID_FLAG );
         }
      }

      /// \brief Sets the global stretch mode. Has no effect on 2D images or when the projection mode is not "slice"
      void SetGlobalStretch( bool globalStretch ) {
         if(( projectionMode_ == ProjectionMode::SLICE ) &&
            ( image_.Dimensionality() > 2 ) &&
            ( globalStretch_ != globalStretch )) {
            globalStretch_ = globalStretch;
            outputIsDirty_ = true;
         }
      };

      /// \brief Sets the global stretch mode. Has no effect on 2D images or when the projection mode is not "slice"
      ///
      /// Valid global stretch modes are:
      /// - "yes"/"on": intensity stretching is computed using all values in the image.
      /// - "no"/"off": intensity stretching is computed using only values visible in the current slice.
      void SetGlobalStretch( String const& globalStretch ) {
         if(( globalStretch == "yes" ) || ( globalStretch == "on" )) {
            SetGlobalStretch( true );
         } else if(( globalStretch == "no" ) || ( globalStretch == "off" )){
            SetGlobalStretch( false );
         } else {
            DIP_THROW( E::INVALID_FLAG );
         }
      }

      /// \brief Get the projection/slicing direction. The two values returned are identical when output is 1D.
      std::pair< dip::uint, dip::uint > GetDirection() const { return { dim1_, dim2_ }; };

      /// \brief Returns the array of dimensions orthogonal to those returned by `GetDirection`. These are the
      /// dimensions not displayed.
      UnsignedArray const& GetOrhthogonal() const { return orthogonal_; }

      /// \brief Get the current coordinates.
      UnsignedArray const& GetCoordinates() const { return coordinates_; }

      /// \brief Get the image sizes.
      UnsignedArray const& GetSizes() const { return image_.Sizes(); }

      /// \brief Get the image dimensionality.
      dip::uint Dimensionality() const { return image_.Dimensionality(); }

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
         }
      }

      /// \brief Get the current intensity range.
      Limits GetRange() const { return range_; }

      /// \brief Gets the image intensity range (that selected with "lin") for the current slicing and complex
      /// mapping modes. If `compute` is true, it computes them if they're not yet computed.
      DIP_EXPORT Limits GetLimits( bool compute );

      /// \brief Get the current global stretch mode.
      bool GetGlobalStretch() const { return globalStretch_; }

   private:

      // A copy of the original image, so we're not dependent on the original image still existing. This is where
      // data is fetched when slice mode, direction or location is changed.
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
      // image needs to be recomputed.
      bool sliceIsDirty_ = true;    // corresponds to slice_
      bool rgbSliceIsDirty_ = true; // corresponds to rgbSlice_
      bool outputIsDirty_ = true;   // corresponds to output_

      // The color space of the input image
      String colorspace_;

      // Display flags
      dip::uint dim1_ = 0;        // slicing direction x
      dip::uint dim2_ = 1;        // slicing direction y (keep identical to dim1_ if `!twoDimOut_`)
      UnsignedArray orthogonal_;  // dimensions orthogonal to dim1_ and dim2_.
      bool twoDimOut_ = true;     // slice is 1D or 2D?
      dip::sint red_ = 0;         // tensor element to display in the red channel (or in the grey-value image)
      dip::sint green_ = -1;      // tensor element to display in the green channel
      dip::sint blue_ = -1;       // tensor element to display in the blue channel
      // TODO: functions to set and get which tensor elements are shown in which channel
      UnsignedArray coordinates_; // coordinates through which the slice is taken
      ProjectionMode projectionMode_ = ProjectionMode::SLICE;
      ComplexMode complexMode_ = ComplexMode::REAL;
      MappingMode mappingMode_ = MappingMode::MANUAL;
      Limits range_ = { 0.0, 255.0 };
      bool globalStretch_ = false;

      // Information about the image:
      //    sliceLimits_[ (int)REAL ].maxMin -> max and min values to use when in REAL complex mapping mode.
      // When limits are NaN, it means they haven't been computed yet.
      constexpr static dfloat NaN = std::numeric_limits< dfloat >::quiet_NaN();
      struct LimitsLists {
         Limits maxMin = { NaN, NaN };
         Limits percentile = { NaN, NaN };
      };
      std::array< LimitsLists, 4 > sliceLimits_;  // Limits to use when !globalStretch_
      std::array< LimitsLists, 4 > globalLimits_; // Limits to use when globalStretch_

      bool IsComplex() { return image_.DataType().IsComplex(); }
      bool IsBinary() { return image_.DataType().IsBinary(); }

      // Computes limits for current mode, if they hadn't been computed yet.
      // If `set`, sets the range_ value to the limits for the current mode.
      DIP_NO_EXPORT void ComputeLimits( bool set = true );

      DIP_NO_EXPORT void InvalidateSliceLimits();

      DIP_NO_EXPORT void UpdateSlice();
      DIP_NO_EXPORT void UpdateRgbSlice();
      DIP_NO_EXPORT void UpdateOutput();

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


// \}

} // namespace dip

#endif // DIP_DISPLAY_H
