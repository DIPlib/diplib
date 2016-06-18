/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "dip_framework.h"

namespace dip {
namespace Framework {

void Scan(
      const ImageRefArray& c_in,
      ImageRefArray&       out,
      const DataTypeArray& inBuffer,
      const DataTypeArray& outBuffer,
      const DataTypeArray& outImage,
      dip::uint            nTensorElements,
      ScanFilter           lineFilter,
      const void*          functionParameters,
      std::vector<void*>&  functionVariables,
      ScanOptions          opts
) {
   std::size_t nIn = c_in.size();
   std::size_t nOut = out.size();
   if( (nIn == 0) && (nOut == 0) ) return; // Duh!

   // Check array sizes
   dip_ThrowIf( inBuffer.size()  != nIn,  E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( outBuffer.size() != nOut, E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( outImage.size()  != nOut, E::ARRAY_ILLEGAL_SIZE );

   // Make simplified copies of image headers so we can modify them at will
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip an output image without destroying
   // the input pixel data.
   ImageArray in( nIn );
   for( dip::uint ii=0; ii<nIn; ++ii) {
      in[ii] = c_in[ii].get().QuickCopy();
   }

   // Do tensor to spatial dimension if necessary
   bool tensorToSpatial = false;
   if( opts == Scan_TensorAsSpatialDim ) {
      nTensorElements = 1; // Input parameter ignored, output matches singleton-expanded number of tensor elements.
      bool allscalar = true;
      // We either convert all images, or none (so that dimensions still match).
      for( dip::uint ii = 0; ii<nIn; ++ii) {
         if( !in[ii].get().IsScalar() ) {
            allscalar = false;
            break;
         }
      }
      if( !allscalar ) {
         for( dip::uint ii = 0; ii<nIn; ++ii) {
            in[ii].TensorToSpatial( 0 );
         }
         tensorToSpatial = true;
      }
   }

   // Do singleton expansion if necessary
   UnsignedArray imSize;
   if( nIn > 0 ) {
      imSize = SingletonExpandedSize( in );
      for( dip::uint ii = 0; ii<nIn; ++ii) {
         if( in[ii].Dimensions() != imSize ) {
            SingletonExpansion( in[ii], imSize );
         }
      }
   } else {
      // nOut > 0, as was checked way at the top of this function.
      imSize = out[0].get().Dimensions();
   }

   // Adjust output if necessary (and possible)
   int nTensElems = nTensorElements;
   if (tensorToSpatial) {
      nTensElems = 1;
   }
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      Image& tmp = out[ii].get();
      if (tensorToSpatial) {
         tmp.TensorToSpatial( 0 );
      }
      bool good = tmp.CheckProperties( imSize, nTensElems, outImage[ii], Option::ThrowException::doNotThrow );
      if( !good ) {
         tmp.Strip();   // Will throw if image is protected
         tmp.SetDimensions( imSize );
         tmp.SetTensorDimensions( nTensorElements );
         tmp.SetDataType( outImage[ii] );
         tmp.Forge();
      }
   }

   // Plan the processing
   if( opts != Scan_NeedCoordinates ) {

   }


   // Return output image tensors if we made them a spatial dimension
   if (tensorToSpatial) {
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         out[ii].get().SpatialToTensor( 0 );
      }
   }

}

} // namespace Framework
} // namespace dip
