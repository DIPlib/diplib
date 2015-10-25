#include "diplib.h"

/* image creation errors */
const char* dip::E::IMAGE_IS_LOCKED =
           "Image is locked";
const char* dip::E::IMAGE_NOT_RAW =
           "Image is not raw";
const char* dip::E::IMAGE_NOT_FORGED =
           "Image is not forged";
const char* dip::E::IMAGE_NOT_VALID =
           "Image is not valid";
const char* dip::E::IMAGES_NOT_UNIQUE =
           "Images are not unique";
const char* dip::E::IMAGE_LOCK_INVALID_KEY =
           "Cannot unlock; invalid key";

/* image data type errors */
const char* dip::E::DATA_TYPE_NOT_SUPPORTED =
           "Data type not supported";

/* image dimensionality and dimensions error */
const char* dip::E::ILLEGAL_DIMENSIONALITY =
           "Illegal dimensionality";
const char* dip::E::DIMENSIONALITY_NOT_SUPPORTED =
           "Dimensionality not supported";
const char* dip::E::ILLEGAL_DIMENSION =
           "Illegal dimension";
const char* dip::E::DIMENSIONALITY_EXCEEDS_LIMIT =
           "Dimensionality exceeds address limit";

/* image properties errors */
const char* dip::E::NO_NORMAL_STRIDE =
           "Image has a non-normal stride";

/* indexing errors */
const char* dip::E::INDEX_OUT_OF_RANGE =
           "Index out of range";

/* image roi errors */
const char* dip::E::IMAGE_MUST_BE_ROI =
           "Image must be a ROI";
const char* dip::E::IMAGE_IS_ROI =
           "Image is a ROI, which it shouldn't be";
const char* dip::E::INVALID_ROI_MAP =
           "Invalid ROI map";

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

/* dip_ImagesCompareTwo errors */
const char* dip::E::IMAGES_DONT_MATCH =
           "Images don't match";
const char* dip::E::TYPES_DONT_MATCH =
           "Image types don't match";
const char* dip::E::DATA_TYPES_DONT_MATCH =
           "Image data types don't match";
const char* dip::E::DIMENSIONALITIES_DONT_MATCH =
           "Image dimensionalities don't match";
const char* dip::E::DIMENSIONS_DONT_MATCH =
           "Image dimensions don't match";

/* function parameter errors */
const char* dip::E::INVALID_PARAMETER =
           "Parameter has invalid value";
const char* dip::E::INVALID_FLAG =
           "Invalid flag";
const char* dip::E::PARAMETER_OUT_OF_RANGE =
           "Parameter value out of range";
const char* dip::E::ARRAY_PARAMETER_WRONG_LENGTH =
           "Array Parameter has the wrong number of elements";

/* mask error codes */
const char* dip::E::NO_MASK =
           "No default mask allowed";
const char* dip::E::MASK_IS_NOT_BINARY =
           "Mask is not a binary image";

/* pixel table errors */
const char* dip::E::PIXEL_TABLE_IS_NOT_ALLOCATED =
           "PixelTable is not allocated";
const char* dip::E::PIXEL_TABLE_NOT_ENOUGH_RUNS =
           "PixelTable does not have enough runs";
const char* dip::E::PIXEL_TABLE_RUN_HAS_NO_DATA =
           "PixelTable run has no data";
