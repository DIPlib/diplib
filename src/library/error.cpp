#include "diplib.h"

/* image creation errors */
const char* dip::E::IMAGE_NOT_RAW =
           "Image is not raw";
const char* dip::E::IMAGE_NOT_FORGED =
           "Image is not forged";
const char* dip::E::IMAGE_NOT_VALID =
           "Image is not valid";

/* image data type errors */
const char* dip::E::DATA_TYPE_NOT_SUPPORTED =
           "Data type not supported";

/* image dimensionality and dimensions error */
const char* dip::E::DIMENSIONALITY_EXCEEDS_LIMIT =
           "Dimensionality exceeds address limit";
const char* dip::E::ILLEGAL_DIMENSIONALITY =
           "Illegal dimensionality";
const char* dip::E::DIMENSIONALITY_NOT_SUPPORTED =
           "Dimensionality not supported";
const char* dip::E::ILLEGAL_DIMENSION =
           "Illegal dimension";
const char* dip::E::DIMENSIONS_DONT_MATCH =
           "Dimensions don't match";
const char* dip::E::NOT_SCALAR =
           "Image is not scalar";
const char* dip::E::TENSORSIZES_DONT_MATCH =
           "Number of tensor elements doesn't match";

/* image properties errors */
const char* dip::E::NO_NORMAL_STRIDE =
           "Image has a non-normal stride";

/* indexing errors */
const char* dip::E::INDEX_OUT_OF_RANGE =
           "Index out of range";

/* error produced by the interface to DIPlib */
const char* dip::E::INTERFACE_ERROR =
           "The interface with DIPlib produced an error";
const char* dip::E::IF_IMAGE_TYPE_NOT_SUPPORTED =
           "The interface does not support the DIPlib image type";
const char* dip::E::IF_DATA_TYPE_NOT_SUPPORTED =
           "The interface does not support the DIPlib data type";

/* miscellaneous errors */
const char* dip::E::NOT_IMPLEMENTED =
           "Functionality has not (yet) been implemented";

/* array errors */
const char* dip::E::ARRAY_ILLEGAL_SIZE =
           "Array has an illegal size";
const char* dip::E::ARRAY_SIZES_DONT_MATCH =
           "Array sizes don't match";
const char* dip::E::ARRAY_OVERFLOW =
           "Array overflow";

/* boundary and filter shape errors */
const char* dip::E::FILTER_SHAPE_NOT_SUPPORTED =
           "Filter shape is not supported";
const char* dip::E::BOUNDARY_CONDITION_NOT_SUPPORTED =
           "Boundary condition is not supported";

/* function parameter errors */
const char* dip::E::INVALID_PARAMETER =
           "Parameter has invalid value";
const char* dip::E::INVALID_FLAG =
           "Invalid flag";
const char* dip::E::PARAMETER_OUT_OF_RANGE =
           "Parameter value out of range";
const char* dip::E::ARRAY_PARAMETER_WRONG_LENGTH =
           "Array Parameter has the wrong number of elements";

/* pixel table errors */
const char* dip::E::PIXEL_TABLE_IS_NOT_ALLOCATED =
           "PixelTable is not allocated";
const char* dip::E::PIXEL_TABLE_NOT_ENOUGH_RUNS =
           "PixelTable does not have enough runs";
const char* dip::E::PIXEL_TABLE_RUN_HAS_NO_DATA =
           "PixelTable run has no data";
