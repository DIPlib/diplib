/*
 * New DIPlib include file
 * This file contains definitions for exceptions and support functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h

#ifndef DIP_ERROR_H
#define DIP_ERROR_H

#include <stdexcept> // std::logic_error and other exception classes

namespace dip {

// All errors thrown in DIPlib are of this class:
typedef std::logic_error Error;

namespace E {
// These are some of the standard what() strings thrown.
// Some functions have specific strings, these are strings that happen in multiple places.
extern const char* NO_MEMORY;
extern const char* MEMORY_INIT_FAILED;
extern const char* IMAGE_IS_LOCKED;
extern const char* IMAGE_NOT_RAW;
extern const char* IMAGE_NOT_FORGED;
extern const char* IMAGE_NOT_VALID;
extern const char* IMAGES_NOT_UNIQUE;
extern const char* IMAGE_LOCK_INVALID_KEY;
extern const char* ILLEGAL_IMAGE_TYPE;
extern const char* IMAGE_TYPE_DOES_NOT_EXIST;
extern const char* IMAGE_TYPE_ALREADY_EXISTS;
extern const char* IMAGE_TYPE_NOT_SUPPORTED;
extern const char* IMAGE_TYPE_HANDLER_MISSING;
extern const char* ILLEGAL_DATA_TYPE;
extern const char* DATA_TYPE_NOT_SUPPORTED;
extern const char* ILLEGAL_DIMENSIONALITY;
extern const char* DIMENSIONALITY_NOT_SUPPORTED;
extern const char* ILLEGAL_DIMENSION;
extern const char* NO_NORMAL_STRIDE;
extern const char* IMAGE_MUST_BE_ROI;
extern const char* IMAGE_IS_ROI;
extern const char* INVALID_ROI_MAP;
extern const char* INTERFACE_ERROR;
extern const char* IF_IMAGE_TYPE_NOT_SUPPORTED;
extern const char* IF_DATA_TYPE_NOT_SUPPORTED;
extern const char* RESOURCE_TRACKING_REQUIRED;
extern const char* RESOURCE_NOT_FOUND;
extern const char* NO_GLOBAL_STRUCTURE;
extern const char* SWITCH_ERROR;
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
extern const char* NO_MASK;
extern const char* MASK_IS_NOT_BINARY;
extern const char* PIXEL_TABLE_IS_NOT_ALLOCATED;
extern const char* PIXEL_TABLE_NOT_ENOUGH_RUNS;
extern const char* PIXEL_TABLE_RUN_HAS_NO_DATA;

}

//
// Test macros to throw exceptions
//
#define DIPASSERT( test , str ) { if( !(test) ) throw dip::Error( str ); } // ASSERT something
#define DIPTS( test , str )     { if(   test  ) throw dip::Error( str ); } // TEST something and SET error (historic name, should we change it?)
#define DIPSJ( str )            {               throw dip::Error( str ); } // SET erro and JUMP (historic name, should we change it?)

} // namespace dip

#endif // DIP_ERROR_H
