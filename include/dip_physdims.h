/*
 * DIPlib 3.0
 * This file contains definitions for functionality related to physical dimensions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_PHYSDIMS_H
#define DIP_PHYSDIMS_H

#include "dip_error.h"
#include "dip_types.h"


/// \file
/// Defines support for physical dimensions.


namespace dip {

/// Specifies an image's pixel size in physical units.
/// Currently, the units are always meters. In the future, there should
/// be proper unit management here.
class PhysicalDimensions {

   public:

      /// By default, an image has no physical dimensions. The pixel size is given
      /// as "1 pixel".
      PhysicalDimensions() {};

      /// Returns the pixel size in meters for the given dimension.
      double PixelSize( dip::uint d ) const {
         if( magnitude_.size() == 0 ) {
            return 1;                  // TODO: should be 1 pixel.
         } else if( d >= magnitude_.size() ) {
            return magnitude_.back();  // TODO: units = m ?
         } else {
            return magnitude_[d];      // TODO: units = m ?
         }
      }

      /// Sets the pixel size in meters in the given dimension. Note that
      /// any subsequent dimension, if not explicitly set, will have the same
      /// size. Thus, for an isotropic pixel, only the first dimension needs to
      /// be set.
      void SetPixelSize( double m, dip::uint d = 0 ) {
         if( magnitude_.size() == 0 ) {
            magnitude_.resize( d + 1, 1 );
         } else if( magnitude_.size() <= d ) {
            magnitude_.resize( d + 1, magnitude_.back() );
         }
         magnitude_[d] = m;
      }

      /// Sets the pixel size in meters in the given dimensions.
      void SetPixelSize( const FloatArray& m ) {
         magnitude_ = m;
      }

      /// Clears the pixel sizes, reverting to the default value of 1 pixel.
      void Clear() {
         magnitude_.clear();
      }

      /// Returns the number of dimensions stored.
      dip::uint Dimensions() const {
         return magnitude_.size();
      }

      /// Removes stored dimensions, keeping the first `d` dimensions only.
      void Resize( dip::uint d ) {
         if( d < magnitude_.size() ) {
            magnitude_.resize(d);
         }
      }

      /// Tests the pixel size for isotropy (the pixel has the same size in all dimensions).
      bool IsIsotropic() const {
         for( dip::uint ii = 1; ii < magnitude_.size(); ++ii ) {
            if( magnitude_[ii] != magnitude_[0] ) {
               return false;
            }
         }
         return true;
      }

      /// Converts meters to pixels.
      FloatArray ToPixels( const FloatArray &in ) const {
         FloatArray out( in.size() );
         for( dip::uint ii = 0; ii < in.size(); ++ii ) {
            out[ii] = in[ii] / PixelSize( ii );
         }
         return out;
      }

      /// Converts pixels to meters.
      FloatArray ToPhysical( const FloatArray &in ) const {
         FloatArray out( in.size() );
         for( dip::uint ii = 0; ii < in.size(); ++ii ) {
            out[ii] = in[ii] * PixelSize( ii );
         }
         return out;
      }

   private:
      // The array below stores a series of values. If the image has more dimensions
      // that this array, the last element is presumed repeated across non-defined
      // dimensions. This is useful because many images have isotropic pixels, and
      // therefore need to store only one value.
      // The units are meters. Other units are converted to/from meters.
      FloatArray magnitude_;

      // TODO: This class has become quite simplified, and so we cannot store
      // units that are not physical sizes. A hyperspectral image has one dimension
      // as frequency; a time axis uses seconds; a Fourier transform uses 1/m;
      // none of these can be stored. We need to add functionality for run-time
      // unit management (i.e. an object of this class has units defined at run
      // time, not compile time as in most C++ unit libraries.
      // ( https://github.com/martinmoene/PhysUnits-CT-Cpp11 )
      // ( http://www.boost.org/doc/libs/1_61_0/doc/html/boost_units.html )
      // Compile-time unit management would mean that either all images have
      // the same units (that we already have), or that the dip::Image class
      // is templated according to its units (we don't template it for size,
      // dimensionality, etc. so why for units???).
      // This class then returns values that include units as well as magnitude.
      // Those values we can print and convert as necessary. We also need to be
      // able to multiply and divide those values, yielding values with different
      // units.
};

} // namespace dip

#endif // DIP_PHYSDIMS_H
