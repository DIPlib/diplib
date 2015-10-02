/*
 * DIPlib 3.0
 * This file contains functionality for the MATLAB interface.
 *
 * (c)2015, Cris Luengo.
 * Based on original DIPlib/MATLAB interface code: (c)1999-2014, Delft University of Technology.
 */

#ifndef DIP_MATLAB_H
#define DIP_MATLAB_H

#include "mex.h"
#include "diplib.h"

#include <map>

namespace dml {

// These are the names of the fields of the dip_image structure in MATLAB:
static const char* dip_DataFieldName = "data";
static const char* dip_TypeFieldName = "dip_type";
static const char* dip_DimsFieldName = "dims";

constexpr dip::uint DML_FEATURE_NAME_LENGTH = 50;

// An error message
static const char* InputImageError = "MATLAB image data of unsupported type.";

// To create an output image:
//    dml::MATLAB_Interface mi;
//    dip::Image img_out0( &mi );
//    dip::Image img_out1( &mi );
// To return that image back to MATLAB:
//    plhs[0] = mi.GetArray( img_out0.GetData() );
//    plhs[1] = mi.GetArray( img_out1.GetData() );
// If we don't GetArray(), the mxArray will be destroyed when img_out0 goes out of scope.
// This interface handler doesn't own any data.
class MATLAB_Interface : public dip::ExternalInterface {
   private:
      std::map<void*,mxArray*> mla;          // This map holds mxArray pointers, we can
                                             // find the right mxArray if we have the data
                                             // pointer.
      // This is the deleter functor we'll associate to the shared_ptr.
      class StripHandler {
         private:
            MATLAB_Interface& interface;
         public:
            StripHandler( MATLAB_Interface& mi ) : interface{mi} {};
            void operator()( void* p ) {
               if( interface.mla.count( p )==0 ) {
                  mexPrintf( "   Not destroying mxArray!\n" );
               } else {
                  mxDestroyArray( interface.mla[p] );
                  interface.mla.erase( p );
                  mexPrintf( "   Destroyed mxArray!\n" );
               }
            };
      };

   public:
      // This is the required AllocateData function. It allocates a MATLAB mxArray
      // and returns a shared_ptr to the mxArray data pointer, with our custom
      // deleter functor.
      virtual std::shared_ptr<void> AllocateData(
         const dip::UnsignedArray& dims,
         dip::IntegerArray&        strides,
         dip::Tensor&              tensor,
         dip::sint                 tstride,
         dip::DataType             datatype
      ) override {
         dip::ThrowIf( !tensor.IsScalar(), "Tensor images not yet supported" );
         // Copy size array
         dip::UnsignedArray mldims = dims;
         // Create stride array
         dip::uint s = 1;
         dip::uint n = dims.size();
         strides.resize( n );
         for( dip::uint ii=0; ii<n; ii++ ) {
            strides[ii] = s;
            s *= dims[ii];
         }
         // Find the right data type
         mxClassID type = mxUINT8_CLASS;
         switch (datatype) {
            case dip::DT_BIN :
            case dip::DT_UINT8 :
               type = mxUINT8_CLASS;
               break;
            case dip::DT_SINT8 :
               type = mxINT8_CLASS;
               break;
            case dip::DT_UINT16 :
               type = mxUINT16_CLASS;
               break;
            case dip::DT_SINT16 :
               type = mxINT16_CLASS;
               break;
            case dip::DT_UINT32 :
               type = mxUINT32_CLASS;
               break;
            case dip::DT_SINT32 :
               type = mxINT32_CLASS;
               break;
            case dip::DT_SFLOAT :
               type = mxSINGLE_CLASS;
               break;
            case dip::DT_DFLOAT :
               type = mxDOUBLE_CLASS;
               break;
            case dip::DT_SCOMPLEX :
               //type = mxSINGLE_CLASS;
               //break;
            case dip::DT_DCOMPLEX :
               //type = mxDOUBLE_CLASS;
               dip::Throw( "Complex images not yet supported" );
               // We should support these by allocating an mxArray with an extra first dimension of 2.
               // The function that links stuff back to MATLAB should take care of splitting the image
               // into a real and imaginary parts, and putting those into an mxArray as the two components.
               // That whole process might be accomplished with a single callback to MATLAB.
            default:
               dip::Throw( "This is not possible!!!" ); // Should not be possible
         }
         // Allocate MATLAB matrix
         if (n >= 2) {
            std::swap( mldims[0], mldims[1] );
            std::swap( strides[0], strides[1] );
         } else {
            mldims.resize(2,1);  // add singleton dimensions
         }
         mxArray* m = mxCreateNumericArray( n, mldims.data(), type, mxREAL );
         mexPrintf( "   Created mxArray as dip::Image data block\n" );
         void* p = mxGetData( m );
         mla[p] = m;
         return std::shared_ptr<void>( p, StripHandler( *this ) );
      };

      mxArray* GetArray( void* p ) {
         mxArray* m = mla[p];
         mla.erase( p );
         return m;
         // We should add code here to turn the array into a dip_image object.
         // We should also test for strides being "normal MATLAB strides", and
         // copy the image over if this is not the case.
      }
};

// A deleter that doesn't delete.
void VoidStripHandler( void* p ) {
   mexPrintf( "   Input mxArray not being destroyed\n" );
};

// Passing an mxArray to DIPlib, keeping ownership of data.
dip::Image GetImage( const mxArray* mx ) {

   // TODO: test for an empty array as input. How do we handle those? throw()? non-forged image?
   // TODO: handle complex images and tensor images.

   // Find image properties
   bool complex = false, binary = false;
   mwSize ndims;
   mxClassID type;
   const mxArray* mxdata;
   if (mxIsClass (mx, "dip_image")) {
      mxdata = mxGetField (mx, 0, dip_DataFieldName);
      mxArray* mxtype = mxGetField (mx, 0, dip_TypeFieldName);
      char buf[DML_FEATURE_NAME_LENGTH];
      mxGetString (mxtype, buf, DML_FEATURE_NAME_LENGTH);
      if (!strncmp (buf, "bin", 3)) {
         binary = true;
      }
      if (!strncmp (buf+1, "complex", 7)) {
         complex = true;
      }
      type = mxGetClassID (mxdata);
      ndims = mxGetScalar (mxGetField (mx, 0, dip_DimsFieldName));
   }
   else {
      mxdata = mx;
      ndims = mxGetNumberOfDimensions (mxdata);
      if (ndims <= 2) {
         const mwSize* pdims = mxGetDimensions (mxdata);
         if (pdims[0]==1 && pdims[1]==1)
            ndims = 0;
         else if (pdims[0]>1 && pdims[1]>1)
            ndims = 2;
         else
            ndims = 1;
      }
      binary = mxIsLogical (mxdata);
      if (binary) {
         type = mxUINT8_CLASS;
         complex = false;
      }
      else {
         type = mxGetClassID (mxdata);
         complex = mxIsComplex (mxdata);
      }
   }
   if (complex)
      dip::Throw( "Complex images not yet supported" );
   dip::DataType datatype;
   switch (type) {
      case mxDOUBLE_CLASS:    // dfloat
         if (complex) datatype = dip::DT_DCOMPLEX;
         else datatype = dip::DT_DFLOAT;
         break;
      case mxSINGLE_CLASS:    // sfloat
         if (complex) datatype = dip::DT_SCOMPLEX;
         else datatype = dip::DT_SFLOAT;
         break;
      case mxINT8_CLASS:      // sint8
         dip::ThrowIf (complex, InputImageError);
         datatype = dip::DT_SINT8;
         break;
      case mxUINT8_CLASS:     // uint8 , bin
         dip::ThrowIf (complex, InputImageError);
         if (binary)
            datatype = dip::DT_BIN;
         else
            datatype = dip::DT_UINT8;
         break;
      case mxINT16_CLASS:     // sint16
         dip::ThrowIf (complex, InputImageError);
         datatype = dip::DT_SINT16;
         break;
      case mxUINT16_CLASS:    // uint16
         dip::ThrowIf (complex, InputImageError);
         datatype = dip::DT_UINT16;
      case mxINT32_CLASS:     // sint32
         dip::ThrowIf (complex, InputImageError);
         datatype = dip::DT_SINT32;
         break;
      case mxUINT32_CLASS:    // uint32
         dip::ThrowIf (complex, InputImageError);
         datatype = dip::DT_UINT32;
         break;
      default:
         dip::Throw( "Image data is not numeric." );
   }

   // Create the size and stride arrays
   dip::UnsignedArray dims(ndims);
   const mwSize* pdims = mxGetDimensions (mxdata);
   if (ndims==1) {
      dims[0] = pdims[0]*pdims[1];  // for a 1D image, we expect one of the two dimensions to be 1. This also handles the case that one of them is 0.
   } else if (ndims>1) {
      for (dip::uint ii=0; ii<ndims; ii++) {
         dims[ii] = pdims[ii];
      }
   }
   dip::uint s = 1;
   dip::IntegerArray strides(ndims);
   for (dip::uint ii=0; ii<ndims; ii++) {
      strides[ii] = s;
      s *= dims[ii];
   }
   if (ndims >= 2) {
      std::swap( dims[0], dims[1] );
      std::swap( strides[0], strides[1] );
   }

   // Create Image object
   // If (complex), do something different here: make two images for mxGetData() and mxGetImagData(),
   // call a DIPlib function to merge them as a single complex image (will need to copy data).
   std::shared_ptr<void> p( mxGetData( mxdata ), VoidStripHandler );
   return dip::Image( p, datatype, dims, strides, {}, {}, nullptr );
}

} // namespace dml

#endif
