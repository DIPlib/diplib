/*
 * New DIPlib include file
 * This file contains definitions for support classes and functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_ERROR_H
#define DIP_ERROR_H

#include <stdexcept> // std::logic_error and other exception classes

namespace dip {

// All errors thrown in DIPlib are of this class:
typedef std::logic_error Error;

// These are some of the standard what() strings thrown.
// Some functions have specific strings, these are strings that happen in multiple places.
extern const char * DIP_E_NO_MEMORY;
extern const char * DIP_E_MEMORY_INIT_FAILED;
extern const char * DIP_E_IMAGE_IS_LOCKED;
extern const char * DIP_E_IMAGE_NOT_RAW;
extern const char * DIP_E_IMAGE_NOT_FORGED;
extern const char * DIP_E_IMAGE_NOT_VALID;
extern const char * DIP_E_IMAGES_NOT_UNIQUE;
extern const char * DIP_E_IMAGE_LOCK_INVALID_KEY;
extern const char * DIP_E_ILLEGAL_IMAGE_TYPE;
extern const char * DIP_E_IMAGE_TYPE_DOES_NOT_EXIST;
extern const char * DIP_E_IMAGE_TYPE_ALREADY_EXISTS;
extern const char * DIP_E_IMAGE_TYPE_NOT_SUPPORTED;
extern const char * DIP_E_IMAGE_TYPE_HANDLER_MISSING;
extern const char * DIP_E_ILLEGAL_DATA_TYPE;
extern const char * DIP_E_DATA_TYPE_NOT_SUPPORTED;
extern const char * DIP_E_ILLEGAL_DIMENSIONALITY;
extern const char * DIP_E_DIMENSIONALITY_NOT_SUPPORTED;
extern const char * DIP_E_ILLEGAL_DIMENSION;
extern const char * DIP_E_NO_NORMAL_STRIDE;
extern const char * DIP_E_IMAGE_MUST_BE_ROI;
extern const char * DIP_E_IMAGE_IS_ROI;
extern const char * DIP_E_INVALID_ROI_MAP;
extern const char * DIP_E_INTERFACE_ERROR;
extern const char * DIP_E_IF_IMAGE_TYPE_NOT_SUPPORTED;
extern const char * DIP_E_IF_DATA_TYPE_NOT_SUPPORTED;
extern const char * DIP_E_RESOURCE_TRACKING_REQUIRED;
extern const char * DIP_E_RESOURCE_NOT_FOUND;
extern const char * DIP_E_NO_GLOBAL_STRUCTURE;
extern const char * DIP_E_SWITCH_ERROR;
extern const char * DIP_E_NOT_IMPLEMENTED;
extern const char * DIP_E_ARRAY_ILLEGAL_SIZE;
extern const char * DIP_E_ARRAY_SIZES_DONT_MATCH;
extern const char * DIP_E_ARRAY_OVERFLOW;
extern const char * DIP_E_FILTER_SHAPE_NOT_SUPPORTED;
extern const char * DIP_E_BOUNDARY_CONDITION_NOT_SUPPORTED;
extern const char * DIP_E_IMAGES_DONT_MATCH;
extern const char * DIP_E_TYPES_DONT_MATCH;
extern const char * DIP_E_DATA_TYPES_DONT_MATCH;
extern const char * DIP_E_DIMENSIONALITIES_DONT_MATCH;
extern const char * DIP_E_DIMENSIONS_DONT_MATCH;
extern const char * DIP_E_INVALID_PARAMETER;
extern const char * DIP_E_INVALID_FLAG;
extern const char * DIP_E_PARAMETER_OUT_OF_RANGE;
extern const char * DIP_E_NO_MASK;
extern const char * DIP_E_MASK_IS_NOT_BINARY;
extern const char * DIP_E_PIXEL_TABLE_IS_NOT_ALLOCATED;
extern const char * DIP_E_PIXEL_TABLE_NOT_ENOUGH_RUNS;
extern const char * DIP_E_PIXEL_TABLE_RUN_HAS_NO_DATA;

/*
 * Test macros to throw exceptions
 */
#define DIPASSERT( test , str ) { if( !(test) ) throw dip::Error( str ); } // ASSERT SOMETHING
#define DIPTS( test , str )     { if(   test  ) throw dip::Error( str ); } // TEST AND SET (set error)

} // namespace dip

#endif // DIP_ERROR_H
