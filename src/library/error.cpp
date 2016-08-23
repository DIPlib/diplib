/*
 * DIPlib 3.0
 * This file contains common error messages.
 *
 * (c)2015-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"

namespace dip {
namespace E {

/* image creation errors */
const char* IMAGE_NOT_RAW = "Image is not raw";
const char* IMAGE_NOT_FORGED = "Image is not forged";
const char* IMAGE_NOT_VALID = "Image is not valid";

/* image data type errors */
const char* DATA_TYPE_NOT_SUPPORTED = "Data type not supported";

/* image dimensionality and dimensions error */
const char* DIMENSIONALITY_EXCEEDS_LIMIT = "Dimensionality exceeds address limit";
const char* ILLEGAL_DIMENSIONALITY = "Illegal dimensionality";
const char* DIMENSIONALITY_NOT_SUPPORTED = "Dimensionality not supported";
const char* ILLEGAL_DIMENSION = "Illegal dimension";
const char* DIMENSIONS_DONT_MATCH = "Dimensions don't match";
const char* NOT_SCALAR = "Image is not scalar";
const char* TENSORSIZES_DONT_MATCH = "Number of tensor elements doesn't match";

/* image properties errors */
const char* NO_NORMAL_STRIDE = "Image has a non-normal stride";

/* mask image properties errors */
const char* MASK_NOT_BINARY = "Mask image not binary";
const char* MASK_TOO_MANY_DIMENSIONS = "Mask image has too many dimensions";

/* indexing errors */
const char* INDEX_OUT_OF_RANGE = "Index out of range";

/* error produced by the interface to DIPlib */
const char* INTERFACE_ERROR = "The interface with DIPlib produced an error";
const char* IF_IMAGE_TYPE_NOT_SUPPORTED = "The interface does not support the DIPlib image type";
const char* IF_DATA_TYPE_NOT_SUPPORTED = "The interface does not support the DIPlib data type";

/* miscellaneous errors */
const char* NOT_IMPLEMENTED = "Functionality has not (yet) been implemented";

/* array errors */
const char* ARRAY_ILLEGAL_SIZE = "Array has an illegal size";
const char* ARRAY_SIZES_DONT_MATCH = "Array sizes don't match";
const char* ARRAY_OVERFLOW = "Array overflow";

/* boundary and filter shape errors */
const char* FILTER_SHAPE_NOT_SUPPORTED = "Filter shape is not supported";
const char* BOUNDARY_CONDITION_NOT_SUPPORTED = "Boundary condition is not supported";

/* function parameter errors */
const char* INVALID_PARAMETER = "Parameter has invalid value";
const char* INVALID_FLAG = "Invalid flag";
const char* PARAMETER_OUT_OF_RANGE = "Parameter value out of range";
const char* ARRAY_PARAMETER_WRONG_LENGTH = "Array Parameter has the wrong number of elements";

/* pixel table errors */
const char* PIXEL_TABLE_IS_NOT_ALLOCATED = "PixelTable is not allocated";
const char* PIXEL_TABLE_NOT_ENOUGH_RUNS = "PixelTable does not have enough runs";
const char* PIXEL_TABLE_RUN_HAS_NO_DATA = "PixelTable run has no data";

} // namespace E
} // namespace dip
