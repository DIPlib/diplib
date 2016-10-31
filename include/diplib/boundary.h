/*
 * DIPlib 3.0
 * This file contains functionality related to the boundary condition
 *
 * (c)2016, Cris Luengo.
 */

#ifndef DIP_BOUNDARY_H
#define DIP_BOUNDARY_H

#include <limits>

#include "diplib.h"


/// \file
/// Defines dip::BoundaryCondition and related types, and the functions that
/// implement the logic behind the boundary conditions. Boundary conditions
/// control what sample value is read when reading outside of the boundary
/// of the image.


namespace dip {

/// Ennumerates various ways of extending image data beyond its boundary. This ennumerator
/// is used by the framework functions and some internal functions. Externally, the
/// boundary condition is represented by strings.
enum class BoundaryCondition {
      SYMMETRIC_MIRROR,          ///< The data is mirrored, with the value at -1 equal to the value at 0, at -2 equal to at 1, etc.
      ASYMMETRIC_MIRROR,         ///< The data is mirrored and inverted.
      PERIODIC,                  ///< The data is repeated periodically, with the value at -1 equal to the value of the last pixel.
      ASYMMETRIC_PERIODIC,       ///< The data is repeated periodically and inverted.
      ADD_ZEROS,                 ///< The boundary is filled with zeros.
      ADD_MAX_VALUE,             ///< The boundary is filled with the max value for the data type.
      ADD_MIN_VALUE,             ///< The boundary is filled with the min value for the data type.
      ZERO_ORDER_EXTRAPOLATE,    ///< The value at the border is repeated indefinitely.
      FIRST_ORDER_EXTRAPOLATE,   ///< A linear function is defined based on the two values closest to the border (dangerous to use!).
      SECOND_ORDER_EXTRAPOLATE,  ///< A quadratic function is defined based on the three values closest to the border (dangerous to use!).
      DEFAULT = SYMMETRIC_MIRROR ///< The default value, currently equal to SYMMETRIC_MIRROR.
};

using BoundaryConditionArray = DimensionArray< BoundaryCondition >; ///< An array to hold boundary conditions.


/// Convert a string to a boundary condition.
BoundaryCondition StringToBoundaryCondition( String bc ); // TODO

/// Convert an array of strings to an array of boundary conditions.
BoundaryConditionArray StringArrayToBoundaryConditionArray( StringArray bc ); // TODO


/// Copies the sample values at `coords` into the output iterator `it`. The output iterator needs to
/// have `tensorElements` spaces available. Note that a simple pointer can be used here. If `coords`
/// falls outside the image, then the boundary condition `bc` is used to determine what values to write
/// into the output iterator.
///
/// `dip::BoundaryCondition::FIRST_ORDER_EXTRAPOLATE` and `dip::BoundaryCondition::SECOND_ORDER_EXTRAPOLATE`
/// do the same thing as `dip::BoundaryCondition::ZERO_ORDER_EXTRAPOLATE`, because their functionality
/// is impossible to reproduce in this simple function. Use `dip::ExtendImage` to get the functionality
/// of these boundary conditions.
template< typename DataType, typename OutputIterator >
void ReadPixelWithBoundaryCondition(
      Image const& img,
      OutputIterator it,
      IntegerArray coords, // getting a local copy so we can modify it
      BoundaryConditionArray const& bc
) {
   dip_ThrowIf( coords.size() != img.Dimensionality(), E::ARRAY_ILLEGAL_SIZE );
   bool invert = false;
   for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
      dip::sint sz = img.Size( ii );
      if(( coords[ ii ] < 0 ) || ( coords[ ii ] >= sz )) {
         switch( bc[ ii ] ) {
            case BoundaryCondition::ASYMMETRIC_MIRROR:
               invert = true;
               // fall-through on purpose!
            case BoundaryCondition::SYMMETRIC_MIRROR:
               coords[ ii ] = coords[ ii ] % ( sz * 2 );
               if( coords[ ii ] >= sz ) {
                  coords[ ii ] = 2 * sz - coords[ ii ] - 1;
               }
               break;
            case BoundaryCondition::ASYMMETRIC_PERIODIC:
               invert = true;
               // fall-through on purpose!
            case BoundaryCondition::PERIODIC:
               coords[ ii ] = coords[ ii ] % ( sz );
               break;
            case BoundaryCondition::ADD_ZEROS:
               for( dip::uint jj = 0; jj < img.TensorElements(); ++jj, ++it ) {
                  *it = 0;
               }
               return; // We're done!
            case BoundaryCondition::ADD_MAX_VALUE:
               for( dip::uint jj = 0; jj < img.TensorElements(); ++jj, ++it ) {
                  *it = std::numeric_limits< DataType >::max();
               }
               return; // We're done!
            case BoundaryCondition::ADD_MIN_VALUE:
               for( dip::uint jj = 0; jj < img.TensorElements(); ++jj, ++it ) {
                  *it = std::numeric_limits< DataType >::lowest();
               }
               return; // We're done!
            case BoundaryCondition::ZERO_ORDER_EXTRAPOLATE:
            case BoundaryCondition::FIRST_ORDER_EXTRAPOLATE:  // not implemented, difficult to implement in this framework.
            case BoundaryCondition::SECOND_ORDER_EXTRAPOLATE: // not implemented, difficult to implement in this framework.
               coords[ ii ] = clamp( coords[ ii ], dip::sint( 0 ), sz - 1 );
               break;
         }
      }
   }
   DataType* in = static_cast< DataType* >( img.Pointer( coords ));
   for( dip::uint jj = 0; jj < img.TensorElements(); ++jj ) {
      *it = invert ? ( - *in ) : ( *in );
      ++it;
      in += img.TensorStride();
   }
   return; // We're done!
}


/// Extends the image `in` by `boundary` along each dimension. The new regions are filled using
/// the boundary condition `bc`. If `bc` is an empty array, the default boundary condition is
/// used along all dimensions. If `bc` has a single element, it is used for all dimensions.
/// Similarly, if `boundary` has a single element, it is used for all dimensions.
void ExtendImage( Image in, Image out, UnsignedArray boundary, BoundaryConditionArray bc );
// TODO: implement this function!

Image ExtendImage( Image in, UnsignedArray boundary, BoundaryConditionArray bc ) {
   Image out;
   // TODO: ExtendImage( in, out, boundary, bc );
   return out;
}

// TODO: Write a helper function that takes an input parameter array, compares its length against the dimensionality of an image, and extends it if it has a single element.

} // namespace dip

#endif // DIP_BOUNDARY_H
