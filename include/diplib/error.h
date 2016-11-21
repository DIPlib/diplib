/*
 * DIPlib 3.0
 * This file contains definitions for exceptions and support functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_ERROR_H
#define DIP_ERROR_H

#include <stdexcept> // std::logic_error and other exception classes
#include <string>
#include <cassert>


/// \file
/// \brief Defines error macros and default error strings. This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


/// \brief All errors thrown in DIPlib are of this class, their `what()` string contains
/// a reason for the error and the name of the function that originally threw it.
using Error = std::logic_error;

namespace E {
// These are some of the standard what() strings thrown.
// These should all happen in multiple places. You don't need to add
// a string here that is used in only one function or one file.

// image creation errors
constexpr char const* IMAGE_NOT_RAW = "Image is not raw";
constexpr char const* IMAGE_NOT_FORGED = "Image is not forged";

// image data type errors
constexpr char const* DATA_TYPE_NOT_SUPPORTED = "Data type not supported";
constexpr char const* WRONG_DATA_TYPE = "Data type does not match";

// image dimensionality and dimensions error
constexpr char const* DIMENSIONALITY_EXCEEDS_LIMIT = "Dimensionality exceeds address limit";
constexpr char const* ILLEGAL_DIMENSIONALITY = "Illegal dimensionality";
constexpr char const* DIMENSIONALITY_NOT_SUPPORTED = "Dimensionality not supported";
constexpr char const* DIMENSIONALITIES_DONT_MATCH = "Dimensionalities don't match";
constexpr char const* ILLEGAL_DIMENSION = "Illegal dimension";
constexpr char const* SIZES_DONT_MATCH = "Sizes don't match";
constexpr char const* NOT_SCALAR = "Image is not scalar";
constexpr char const* NTENSORELEM_DONT_MATCH = "Number of tensor elements doesn't match";

// image properties errors
constexpr char const* NO_NORMAL_STRIDE = "Image has a non-normal stride";

// mask image properties errors
constexpr char const* MASK_NOT_BINARY = "Mask image not binary";
constexpr char const* MASK_TOO_MANY_DIMENSIONS = "Mask image has too many dimensions";

// indexing errors
constexpr char const* INDEX_OUT_OF_RANGE = "Index out of range";
constexpr char const* COORDINATES_OUT_OF_RANGE = "Coordinates out of range";
constexpr char const* ITERATOR_NOT_VALID = "Iterator is not valid";

// miscellaneous errors
constexpr char const* NOT_IMPLEMENTED = "Functionality has not (yet) been implemented";

// array errors
constexpr char const* ARRAY_ILLEGAL_SIZE = "Array has an illegal size";
constexpr char const* ARRAY_SIZES_DONT_MATCH = "Array sizes don't match";
constexpr char const* ARRAY_OVERFLOW = "Array overflow";
constexpr char const* INITIALIZERLIST_ILLEGAL_SIZE = "Initializer list has an illegal size";

// function parameter errors
constexpr char const* INVALID_PARAMETER = "Parameter has invalid value";
constexpr char const* INVALID_FLAG = "Invalid flag";
constexpr char const* PARAMETER_OUT_OF_RANGE = "Parameter value out of range";
constexpr char const* ARRAY_PARAMETER_WRONG_LENGTH = "Array parameter has the wrong number of elements";
constexpr char const* FILTER_SHAPE_NOT_SUPPORTED = "Filter shape is not supported";

}

//
// Test and throw exception
//

/// \brief Throw an Error.
#define dip_Throw( str ) { throw dip::Error( std::string(str) + std::string(" (in function ") + std::string(__func__)  + std::string(")") ); }

/// \brief Test a condition, throw an Error if the condition is met.
#define dip_ThrowIf( test, str ) { if( test ) dip_Throw( str ) }

// These are the old DIPlib names, let's not use them any more:
//#define DIPASSERT( test, str ) dip_ThrowIf( !(test), str )
//#define DIPTS( test, str )     dip_ThrowIf( test, str )
//#define DIPSJ( str )           dip_Throw( str )

/// \}

} // namespace dip

#endif // DIP_ERROR_H
