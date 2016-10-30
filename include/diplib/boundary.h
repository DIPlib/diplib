/*
 * DIPlib 3.0
 * This file contains functionality related to the boundary condition
 *
 * (c)2016, Cris Luengo.
 */

#ifndef DIP_BOUNDARY_H
#define DIP_BOUNDARY_H

#include <limits>

#include "diplib/types.h"
#include "diplib/numeric.h"


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
/// have `tensorElements` spaces available. Note that a simple pointer can be used here. `sizes`,
/// `strides`, `tensorElements` and `tensorStrides` describe the image object, and `origin` is the
/// image's origin pointer, cast to the proper data type. If `coords` falls outside the image, then
/// the boundary condition `bc` is used to determine what values to write into the output iterator.
///
/// `dip::BoundaryCondition::FIRST_ORDER_EXTRAPOLATE` and `dip::BoundaryCondition::SECOND_ORDER_EXTRAPOLATE`
/// do the same thing as `dip::BoundaryCondition::ZERO_ORDER_EXTRAPOLATE`, because their functionality
/// is impossible to reproduce in this simple function. Use `dip::ExtendImage` to get the functionality
/// of these boundary conditions.
template< typename DataType, typename OutputIterator >
void ReadPixelWithBoundaryCondition(
      IntegerArray coords,
      DataType* origin,
      UnsignedArray sizes,
      IntegerArray strides,
      dip::uint tensorElements,
      dip::sint tensorStride,
      BoundaryConditionArray bc,
      OutputIterator it
) {
   dip_ThrowIf( coords.size() != sizes.size(), E::ARRAY_SIZES_DONT_MATCH );
   bool invert = false;
   for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
      dip::sint index = coords[ ii ];
      if(( index < 0 ) || ( index >= sizes[ ii ] )) {
         switch( bc[ ii ] ) {
            case BoundaryCondition::ASYMMETRIC_MIRROR:
               invert = true;
               // fall-through on purpose!
            case BoundaryCondition::SYMMETRIC_MIRROR:
               index = index % ( static_cast< dip::sint >( sizes[ ii ] ) * 2 );
               if( index >= sizes[ ii ] ) {
                  index = 2 * sizes[ ii ] - index - 1;
               }
               break;
            case BoundaryCondition::ASYMMETRIC_PERIODIC:
               invert = true;
               // fall-through on purpose!
            case BoundaryCondition::PERIODIC:
               index = index % ( static_cast< dip::sint >( sizes[ ii ] ) );
               break;
            case BoundaryCondition::ADD_ZEROS:
               for( dip::uint jj = 0; jj < tensorElements; ++jj, ++it ) {
                  *it = 0;
               }
               return; // We're done!
            case BoundaryCondition::ADD_MAX_VALUE:
               for( dip::uint jj = 0; jj < tensorElements; ++jj, ++it ) {
                  *it = std::numeric_limits< DataType >::max();
               }
               return; // We're done!
            case BoundaryCondition::ADD_MIN_VALUE:
               for( dip::uint jj = 0; jj < tensorElements; ++jj, ++it ) {
                  *it = std::numeric_limits< DataType >::lowest();
               }
               return; // We're done!
            case BoundaryCondition::ZERO_ORDER_EXTRAPOLATE:
            case BoundaryCondition::FIRST_ORDER_EXTRAPOLATE:  // not implemented, difficult to implement in this framework.
            case BoundaryCondition::SECOND_ORDER_EXTRAPOLATE: // not implemented, difficult to implement in this framework.
               index = clamp( index, dip::sint( 0 ), static_cast< dip::sint >( sizes[ ii ] - 1 ));
               break;
         }
      }
   }
   DataType* in = origin;
   for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
      in += coords[ ii ] * strides[ ii ];
   }
   for( dip::uint jj = 0; jj < tensorElements; ++jj ) {
      *it = *in;
      ++it;
      in += tensorStride;
   }
   return; // We're done!

}

class Image; // Forward declarator

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
