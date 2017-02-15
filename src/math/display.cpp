/*
 * DIPlib 3.0
 * This file contains the definition of the ImageDisplay function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include "diplib.h"
#include "diplib/display.h"
#include "diplib/math.h"


namespace dip {


void ImageDisplay(
      Image in,
      Image out,
      UnsignedArray const& coordinates,
      dip::uint dim1,
      dip::uint dim2,
      ImageDisplayParams const& params
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( in.IsScalar() && !in.IsColor(), E::NOT_SCALAR );

   // Compute projection
   Image slice = in;
   if( nDims > 2 ) {
      DIP_THROW_IF( !coordinates.empty() && coordinates.size() != nDims, E::ARRAY_ILLEGAL_SIZE );
      DIP_THROW_IF(( dim1 >= nDims ) || ( dim2 >= nDims ), E::PARAMETER_OUT_OF_RANGE );
      DIP_THROW_IF( dim1 == dim2, E::INVALID_PARAMETER );
      if( params.projection == "slice" ) {
         RangeArray rangeArray( nDims ); // By default, covers all image pixels
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if(( ii != dim1 ) && ( ii != dim2 )) {
               rangeArray[ ii ] = Range( coordinates[ ii ] );
            }
         }
         slice = slice.At( rangeArray );
      } else if( params.projection == "max" ) {
         BooleanArray process( nDims, true );
         process[ dim1 ] = false;
         process[ dim2 ] = false;
         Maximum( slice, {}, slice, process );
      } else if( params.projection == "mean" ) {
         BooleanArray process( nDims, true );
         process[ dim1 ] = false;
         process[ dim2 ] = false;
         Mean( slice, {}, slice, "", process );
      } else {
         DIP_THROW( E::INVALID_FLAG );
      }
   }

   // Convert color image
   if( slice.IsColor() ) {
      // TODO
   }

   // Convert complex image to float
   if( slice.DataType().IsComplex() ) {
      if( params.complex == "mag" ) {
         slice.Convert( slice.DataType().Real() );
      } else if( params.complex == "phase" ) {
         // TODO
      } else if( params.complex == "real" ) {
         slice = slice.Real();
      } else if( params.complex == "imag" ) {
         slice = slice.Imaginary();
      } else {
         DIP_THROW( E::INVALID_FLAG );
      }
   }

   // Stretch image and convert to uint8 data type
   slice.Convert( DT_UINT8 ); // TODO: this is temporary
   if( params.mode == "lin" ) {
      // TODO
   } else if( params.mode == "based" ) {
      // TODO
   } else if( params.mode == "log" ) {
      // TODO
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
}


} // namespace dip
