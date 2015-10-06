/*
 * DIPlib 3.0
 * This file contains definitions for exceptions and support functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#include "diplib.h"
#endif

#ifndef DIP_ERROR_H
#define DIP_ERROR_H

#include <stdexcept> // std::logic_error and other exception classes

namespace dip {

// All errors thrown in DIPlib are of this class:
typedef std::logic_error Error;

namespace E {
// These are some of the standard what() strings thrown.
// These should all happen in multiple places. You don't need to add
// a string here that is used in only one function or one file.
// This list comes from the original DIPlib. Many of these might not
// be useful any more.
// The strings are declared here and defined in error.cpp
extern const char* IMAGE_IS_LOCKED;
extern const char* IMAGE_NOT_RAW;
extern const char* IMAGE_NOT_FORGED;
extern const char* IMAGE_NOT_VALID;
extern const char* IMAGES_NOT_UNIQUE;
extern const char* IMAGE_LOCK_INVALID_KEY;
extern const char* DATA_TYPE_NOT_SUPPORTED;
extern const char* ILLEGAL_DIMENSIONALITY;
extern const char* DIMENSIONALITY_NOT_SUPPORTED;
extern const char* ILLEGAL_DIMENSION;
extern const char* DIMENSIONALITY_EXCEEDS_LIMIT;
extern const char* NO_NORMAL_STRIDE;
extern const char* IMAGE_MUST_BE_ROI;
extern const char* IMAGE_IS_ROI;
extern const char* INVALID_ROI_MAP;
extern const char* INTERFACE_ERROR;
extern const char* IF_IMAGE_TYPE_NOT_SUPPORTED;
extern const char* IF_DATA_TYPE_NOT_SUPPORTED;
extern const char* NOT_IMPLEMENTED;
extern const char* ARRAY_ILLEGAL_SIZE;
extern const char* ARRAY_SIZES_DONT_MATCH;
extern const char* ARRAY_OVERFLOW;
extern const char* FILTER_SHAPE_NOT_SUPPORTED;
extern const char* BOUNDARY_CONDITION_NOT_SUPPORTED;
extern const char* IMAGES_DONT_MATCH;
extern const char* TYPES_DONT_MATCH;
extern const char* DATA_TYPES_DONT_MATCH;
extern const char* DIMENSIONALITIES_DONT_MATCH;
extern const char* DIMENSIONS_DONT_MATCH;
extern const char* INVALID_PARAMETER;
extern const char* INVALID_FLAG;
extern const char* PARAMETER_OUT_OF_RANGE;
extern const char* ARRAY_PARAMETER_WRONG_LENGTH;
extern const char* NO_MASK;
extern const char* MASK_IS_NOT_BINARY;
extern const char* PIXEL_TABLE_IS_NOT_ALLOCATED;
extern const char* PIXEL_TABLE_NOT_ENOUGH_RUNS;
extern const char* PIXEL_TABLE_RUN_HAS_NO_DATA;
}

//
// Test and throw exception
//

/// Test a condition, throw an Error if the condition is met.
inline void ThrowIf( bool test, const char* str ) {
   if( test )
      throw Error( str );
}
/// Throw an Error.
inline void Throw( const char* str ) {
   throw Error( str );
}

// These are the old DIPlib names, let's not use them any more:
#define DIPASSERT( test , str ) ThrowIf( !(test), str )
#define DIPTS( test , str )     ThrowIf( test, str )  // TEST something and SET error
#define DIPSJ( str )            Throw( str )          // SET error and JUMP

} // namespace dip

#endif // DIP_ERROR_H
