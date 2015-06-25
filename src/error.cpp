#include "diplib.h"

/* memory allocation errors */
const char * dip::DIP_E_NO_MEMORY = 
           "Could not allocate memory";
const char * dip::DIP_E_MEMORY_INIT_FAILED = 
           "Initialisation of memory routines failed";

/* image creation errors */
const char * dip::DIP_E_IMAGE_IS_LOCKED = 
           "Image is locked";
const char * dip::DIP_E_IMAGE_NOT_RAW = 
           "Image is not raw";
const char * dip::DIP_E_IMAGE_NOT_FORGED = 
           "Image is not forged";
const char * dip::DIP_E_IMAGE_NOT_VALID = 
           "Image is not valid";
const char * dip::DIP_E_IMAGES_NOT_UNIQUE = 
           "Images are not unique";
const char * dip::DIP_E_IMAGE_LOCK_INVALID_KEY = 
           "Cannot unlock; invalid key";

/* image type errors */
const char * dip::DIP_E_ILLEGAL_IMAGE_TYPE = 
           "Illegal image type";
const char * dip::DIP_E_IMAGE_TYPE_DOES_NOT_EXIST = 
           "Image type does not exist";
const char * dip::DIP_E_IMAGE_TYPE_ALREADY_EXISTS = 
           "Image type already exists";
const char * dip::DIP_E_IMAGE_TYPE_NOT_SUPPORTED = 
           "Image type not supported";
const char * dip::DIP_E_IMAGE_TYPE_HANDLER_MISSING = 
           "Image type handler missing";

/* image data type errors */
const char * dip::DIP_E_ILLEGAL_DATA_TYPE = 
           "Illegal data type";
const char * dip::DIP_E_DATA_TYPE_NOT_SUPPORTED = 
           "Data type not supported";

/* image dimensionality and dimensions error */
const char * dip::DIP_E_ILLEGAL_DIMENSIONALITY = 
           "Illegal dimensionality";
const char * dip::DIP_E_DIMENSIONALITY_NOT_SUPPORTED = 
           "Dimensionality not supported";
const char * dip::DIP_E_ILLEGAL_DIMENSION = 
           "Illegal dimension";

/* image properties errors */
const char * dip::DIP_E_NO_NORMAL_STRIDE = 
           "Image has a non-normal stride";

/* image roi errors */
const char * dip::DIP_E_IMAGE_MUST_BE_ROI = 
           "Image must be a ROI";
const char * dip::DIP_E_IMAGE_IS_ROI = 
           "Image is a ROI, which it shouldn't be";
const char * dip::DIP_E_INVALID_ROI_MAP = 
           "Invalid ROI map";

/* error produced by the interface to DIPlib */
const char * dip::DIP_E_INTERFACE_ERROR = 
           "The interface with DIPlib produced an error";
const char * dip::DIP_E_IF_IMAGE_TYPE_NOT_SUPPORTED = 
           "The interface does not support the DIPlib image type";
const char * dip::DIP_E_IF_DATA_TYPE_NOT_SUPPORTED = 
           "The interface does not support the DIPlib data type";


/* resource tracking errors */
const char * dip::DIP_E_RESOURCE_TRACKING_REQUIRED = 
           "Resources structure required";
const char * dip::DIP_E_RESOURCE_NOT_FOUND = 
           "Resource not found";

/* miscellaneous errors */
const char * dip::DIP_E_NO_GLOBAL_STRUCTURE = 
           "No global structure";
const char * dip::DIP_E_SWITCH_ERROR = 
           "Internal switch error";
const char * dip::DIP_E_NOT_IMPLEMENTED = 
           "Functionality has not (yet) been implemented";

/* array errors */
const char * dip::DIP_E_ARRAY_ILLEGAL_SIZE = 
           "Array has an illegal size";
const char * dip::DIP_E_ARRAY_SIZES_DONT_MATCH = 
           "Array sizes don't match";
const char * dip::DIP_E_ARRAY_OVERFLOW = 
           "Array overflow";

/* boundary and filter shape errors */
const char * dip::DIP_E_FILTER_SHAPE_NOT_SUPPORTED = 
           "Filter shape is not supported";
const char * dip::DIP_E_BOUNDARY_CONDITION_NOT_SUPPORTED = 
           "Boundary condition is not supported";

/* dip_ImagesCompareTwo errors */
const char * dip::DIP_E_IMAGES_DONT_MATCH = 
           "Images don't match";
const char * dip::DIP_E_TYPES_DONT_MATCH = 
           "Image types don't match";
const char * dip::DIP_E_DATA_TYPES_DONT_MATCH = 
           "Image data types don't match";
const char * dip::DIP_E_DIMENSIONALITIES_DONT_MATCH = 
           "Image dimensionalities don't match";
const char * dip::DIP_E_DIMENSIONS_DONT_MATCH = 
           "Image dimensions don't match";

/* function parameter errors */
const char * dip::DIP_E_INVALID_PARAMETER = 
           "Parameter has invalid value";
const char * dip::DIP_E_INVALID_FLAG = 
           "Invalid flag";
const char * dip::DIP_E_PARAMETER_OUT_OF_RANGE = 
           "Parameter value out of range";

/* mask error codes */
const char * dip::DIP_E_NO_MASK = 
           "No default mask allowed";
const char * dip::DIP_E_MASK_IS_NOT_BINARY = 
           "Mask is not a binary image";

/* pixel table errors */
const char * dip::DIP_E_PIXEL_TABLE_IS_NOT_ALLOCATED = 
           "PixelTable is not allocated";
const char * dip::DIP_E_PIXEL_TABLE_NOT_ENOUGH_RUNS = 
           "PixelTable does not have enough runs";
const char * dip::DIP_E_PIXEL_TABLE_RUN_HAS_NO_DATA = 
           "PixelTable run has no data";

