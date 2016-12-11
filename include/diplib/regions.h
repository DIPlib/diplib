/*
 * DIPlib 3.0
 * This file contains declarations for functions that work with labelled images.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_REGIONS_H
#define DIP_REGIONS_H

#include "diplib.h"
#include "diplib/boundary.h"


/// \file
/// \brief Declares image processing functions that work with labelled images.


namespace dip {


/// \defgroup regions Labelled image functions
/// \brief Functions that work on labelled images.
/// \{

// TODO: functions to add:
/*
DIP_ERROR dip_Label          ( dip_Image, dip_Image, dip_int, dip_int,
                               dip_int, dip_int, dip_int *, dip_BoundaryArray );
DIP_ERROR dip_LabelSetBorder ( dip_Image, dip_Image );
DIP_ERROR dip_PlaneDoEdge    ( dip_Image, dip_int, dip_int );
DIP_ERROR dip_ImageDoEdge    ( dip_Image, dip_int );
DIP_ERROR dip_RegionConnectivity
                             ( dip_Image, dip_Image,
                               dip_sint32 ***, dip_sint32 ***,
                               dip_sint32 ***, dip_sint32 ***,
                               dip_int, dip_int, dip_int,
                               dip_int *, dip_int *, dip_int *,
                               dip_int *, dip_int *, dip_int *,
                               dip_BoundaryArray, dip_Resources );

DIP_ERROR dip_GrowRegions    ( dip_Image, dip_Image, dip_Image, dip_Image,
                               dip_int, dip_int, dipf_GreyValueSortOrder );
DIP_ERROR dip_GrowRegionsWeighted
                             ( dip_Image, dip_Image, dip_Image, dip_Image,
                               dip_Image, dip_FloatArray, dip_int, dip_Image );
*/

/// \brief Labels the connected components in a binary image
///
/// The output is an unsigned integer image. Each object (respecting the connectivity,
/// see \ref connectivity) in the input image receives a unique number. This number ranges
/// from 1 to the number of objects in the image. The pixels in the output image corresponding
/// to a given object are set to this number (label). The remaining pixels in the output image
/// are set to 0.
///
/// The `minSize` and `maxSize` set limits on the size of the objects: Objects smaller than `minSize`
/// or larger than `maxSize` do not receive a label and the corresponding pixels in the output
/// image are set to zero. Setting either to zero disables the corresponding check. Setting both
/// to zero causes all objects to be labelled, irrespective of size.
///
/// If `mode` is "LabelIsSize", the objects' labels are set to the objects' sizes. Normal operation
/// is obtained with an empty string.
///
/// The boundary conditions are generally ignored (labeling stops at the boundary). The exception
/// is `dip::BoundaryCondition::PERIODIC`, which is the only one that makes sense for this algorithm.
dip::uint Label(
      Image const& binary,
      Image& out,
      dip::uint connectivity,
      String mode,
      dip::uint minSize,
      dip::uint maxSize,
      BoundaryConditionArray bc
);

Image Label(
      Image const& binary,
      dip::uint connectivity,
      String mode,
      dip::uint minSize,
      dip::uint maxSize,
      BoundaryConditionArray bc
) {
   Image out;
   Label( binary, out, connectivity, mode, minSize, maxSize, bc );
   return out;
}

/// \brief Gets a list of object labels in the labelled image. A labelled image must be of an unsigned type.
UnsignedArray GetObjectLabels(
      Image const& label,
      Image const& mask,
      bool nullIsObject
);

/// \}

} // namespace dip

#endif // DIP_REGIONS_H
