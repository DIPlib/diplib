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
/// be set. When the `dip::ImageDisplay::Output` method is called, a 2D, UINT8 image is prepared for display. A
/// const reference to this image is returned. The image is updated every time the `%Output` method is called,
/// not when display options are set. The display options are designed to be settable by a user using the image
/// display window.
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

      /// \brief The constructor takes an image with at least 2 dimensions. If it is a non-color tensor image,
      /// no more than three tensor elements are allowed, which are interpreted as RGB values.
      ImageDisplay( Image const& image, ExternalInterface* externalInterface = nullptr ) :
            image_( image.QuickCopy() ), colorspace_( image.ColorSpace() ) {
         DIP_THROW_IF( !image_.IsForged(), E::IMAGE_NOT_FORGED );
         dip::uint nDims = image_.Dimensionality();
         DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         if( colorspace_.empty() ) {
            DIP_THROW_IF( image_.TensorElements() > 3, "ImageDisplay only works for tensor images with up to three components." );
            if( !image_.IsScalar() ) {
               colorspace_ = "RGB";
            }
         }
         coordinates_.resize( image_.Dimensionality(), 0 );
         if( externalInterface ) {
            output_.SetExternalInterface( externalInterface );
         }
      }

      /// \brief Retrives a reference to the output image. This function also causes an update of the output if
      /// any of the modes changed.
      DIP_EXPORT Image const& Output();

      /// \brief Gets input image intensities at a given 2D point (automatically finds corresponding nD location).
      /// Because the pixel can be of different types (integer, float, complex) and can have up to three samples,
      /// a string is returned with appropriately formatted values.
      DIP_EXPORT String Pixel( dip::uint x, dip::uint y );

      /// \brief Sets the projection/slicing direction, as the two image dimensions to show along the x and y axis
      /// of the display.
      void SetDirection( dip::uint dim1, dip::uint dim2 ) {
         dip::uint nDim = image_.Dimensionality();
         DIP_THROW_IF(( dim1 >= nDim ) || ( dim2 >= nDim ), E::ILLEGAL_DIMENSION );
         DIP_THROW_IF( dim1 == dim2, "Two different dimensions are needed for display" );
         if(( dim1_ != dim1 ) || ( dim2_ != dim2 )) {
            dim1_ = dim1;
            dim2_ = dim2;
            sliceIsDirty_ = true;
         }
      }

      /// \brief Sets the current coordinates. This affects the slice displayed.
      void SetCoordinates( UnsignedArray coordinates ) {
         DIP_THROW_IF( coordinates.size() != coordinates_.size(), E::ARRAY_ILLEGAL_SIZE );
         for( dip::uint ii = 0; ii < coordinates_.size(); ++ii ) {
            if( coordinates[ ii ] >= image_.Size( ii ) ) {
               coordinates[ ii ] = image_.Size( ii ) - 1;
            }
            if( coordinates_[ ii ] != coordinates[ ii ] ) {
               coordinates_[ ii ] = coordinates[ ii ];
               if(( ii != dim1_ ) && ( ii != dim2_ )) {
                  sliceIsDirty_ = true;
               }
            }
         }
      }

      /// \brief Sets the projection mode. Has no effect on 2D images.
      void SetProjectionMode( ProjectionMode projectionMode ) {
         if(( image_.Dimensionality() > 2 ) && ( projectionMode_ != projectionMode )) {
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

      /// \brief Sets the projection mode. Has no effect on 2D images.
      ///
      /// Valid projection modes are:
      /// - "slice": the 2D image shown is a slice through the nD image.
      /// - "max": the 2D image shown is the max projection of the nD image.
      /// - "mean": the 2D image shown is the mean projection of the nD image.
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

      /// \brief Sets the complex to real mapping mode. Has no effect when projection mode is set to "max".
      void SetComplexMode( ComplexMode complexMode ) {
         if(( projectionMode_ != ProjectionMode::MAX ) && ( complexMode_ != complexMode )) {
            complexMode_ = complexMode;
            outputIsDirty_ = true;
         }
      }

      /// \brief Sets the complex to real mapping mode. Has no effect when projection mode is set to "max".
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

      /// \brief Sets the intensity mapping mode.
      void SetMappingMode( MappingMode mappingMode ) {
         if( mappingMode_ != mappingMode ) {
            mappingMode_ = mappingMode;
            outputIsDirty_ = true;
         }
      }

      /// \brief Sets the range of intensities to be mapped to the output range. Forces intensity mapping mode to linear.
      void SetRange( Limits range ) {
         DIP_THROW_IF( range.lower >= range.upper, E::PARAMETER_OUT_OF_RANGE );
         mappingMode_ = MappingMode::MANUAL;
         if(( range_.lower != range.lower ) ||
            ( range_.upper != range.upper )) {
            range_ = range;
            outputIsDirty_ = true;
         }
      }

      /// \brief Sets the range of intensities to be mapped to the output range. Forces intensity mapping mode to linear.
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

      /// \brief Sets the global stretch mode. Has no effect on 2D images.
      ///
      /// Valid global stretch modes are:
      /// - "yes": intensity stretching is computed using all values in the image.
      /// - "no": intensity stretching is computed using only values visible in the current slice.
      void SetGlobalStretch( String const& globalStretch ) {
         if( globalStretch == "yes" ) {
            SetGlobalStretch( true );
         } else if( globalStretch == "no" ) {
            SetGlobalStretch( false );
         } else {
            DIP_THROW( E::INVALID_FLAG );
         }
      }

      /// \brief Get the projection/slicing direction.
      std::pair< dip::uint, dip::uint > GetDirection() const { return { dim1_, dim2_ }; };
      /// \brief Get the current coordinates.
      UnsignedArray GetCoordinates() const { return coordinates_; }
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
      /// \brief Get the current global stretch mode.
      bool GetGlobalStretch() const { return globalStretch_; }

   private:

      // A copy of the original image, so we're not dependent on the original image still existing. This is where data
      // is fetched when slice mode, direction or location is changed.
      Image image_;
      // The 2D slice to be displayed (could be either shared data with the image, or in case of a projection,
      // owning its own data). This is where intensity lookup is performed.
      Image slice_;
      // Another 2D slice, either identical to the previous, or converted to RGB if it was a color image.
      // This prevents repeated (possibly expensive) conversion to RGB when changing mapping modes.
      Image rgbSlice_;
      // The output image: 2D UINT8.
      // The external interface controls how the color channel dimension is configured.
      Image output_;

      // Changing display flags causes one or more "dirty" flags to be set. This indicates that the corresponding
      // image needs to be recomputed.
      bool sliceIsDirty_ = true;  // corresponds to slice_ and rgbSlice_
      bool outputIsDirty_ = true; // corresponds to output_

      // The color space of the input image
      String colorspace_;

      // Display flags
      dip::uint dim1_ = 0;        // slicing direction
      dip::uint dim2_ = 1;        // slicing direction
      UnsignedArray coordinates_; // coordinates through which the slice is taken
      ProjectionMode projectionMode_ = ProjectionMode::SLICE;
      ComplexMode complexMode_ = ComplexMode::MAGNITUDE;
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

      DIP_NO_EXPORT void ComputeLimits(); // computes limits for current mode, if they hadn't been computed yet
      DIP_NO_EXPORT void InvalidateSliceLimits();

      DIP_NO_EXPORT void UpdateSlice();
      DIP_NO_EXPORT void UpdateOutput();
};


// \}

} // namespace dip

#endif // DIP_DISPLAY_H
