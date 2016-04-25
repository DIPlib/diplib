/*
 * DIPlib 3.0
 * This file contains definitions for exceptions and support functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#error "Please don't include this file directly, include diplib.h instead."
#endif

#ifndef DIP_ERROR_H
#define DIP_ERROR_H

#include <stdexcept> // std::logic_error and other exception classes
#include <string>
#include <cassert>

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
   extern const char* IMAGE_NOT_RAW;
   extern const char* IMAGE_NOT_FORGED;
   extern const char* IMAGE_NOT_VALID;

   extern const char* DATA_TYPE_NOT_SUPPORTED;

   extern const char* DIMENSIONALITY_EXCEEDS_LIMIT;
   extern const char* ILLEGAL_DIMENSIONALITY;
   extern const char* DIMENSIONALITY_NOT_SUPPORTED;
   extern const char* ILLEGAL_DIMENSION;
   extern const char* DIMENSIONS_DONT_MATCH;
   extern const char* NOT_SCALAR;
   extern const char* TENSORSIZES_DONT_MATCH;

   extern const char* NO_NORMAL_STRIDE;

   extern const char* INDEX_OUT_OF_RANGE;

   extern const char* INTERFACE_ERROR;
   extern const char* IF_IMAGE_TYPE_NOT_SUPPORTED;
   extern const char* IF_DATA_TYPE_NOT_SUPPORTED;

   extern const char* NOT_IMPLEMENTED;

   extern const char* ARRAY_ILLEGAL_SIZE;
   extern const char* ARRAY_SIZES_DONT_MATCH;
   extern const char* ARRAY_OVERFLOW;

   extern const char* FILTER_SHAPE_NOT_SUPPORTED;
   extern const char* BOUNDARY_CONDITION_NOT_SUPPORTED;

   extern const char* INVALID_PARAMETER;
   extern const char* INVALID_FLAG;
   extern const char* PARAMETER_OUT_OF_RANGE;
   extern const char* ARRAY_PARAMETER_WRONG_LENGTH;

   extern const char* PIXEL_TABLE_IS_NOT_ALLOCATED;
   extern const char* PIXEL_TABLE_NOT_ENOUGH_RUNS;
   extern const char* PIXEL_TABLE_RUN_HAS_NO_DATA;
}

//
// Test and throw exception
//

/// Throw an Error.
#define dip_Throw( str ) { throw dip::Error( std::string(str) + std::string(" in function ") + std::string(__func__) ); }

/// Test a condition, throw an Error if the condition is met.
#define dip_ThrowIf( test, str ) { if( test ) dip_Throw( str ) }

// These are the old DIPlib names, let's not use them any more:
#define DIPASSERT( test , str ) dip_ThrowIf( !(test), str )
#define DIPTS( test , str )     dip_ThrowIf( test, str )  // TEST something and SET error
#define DIPSJ( str )            dip_Throw( str )          // SET error and JUMP

} // namespace dip

#endif // DIP_ERROR_H
