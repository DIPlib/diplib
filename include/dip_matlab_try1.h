/*
 * New DIPlib include file
 * This file contains functionality for the MATLAB interface.
 *
 * (c)2015, Cris Luengo.
 * Based on original DIPlib/MATLAB interface code: (c)1999-2014, Delft University of Technology.
 */

#ifndef DIP_MATLAB_H
#define DIP_MATLAB_H

#include "mex.h"
#include "diplib.h"

namespace dml {

// These are the names of the fields of the dip_image structure in MATLAB:
static const char* dip_DataFieldName = "data";
static const char* dip_TypeFieldName = "dip_type";
static const char* dip_DimsFieldName = "dims";

constexpr dip::uint DML_FEATURE_NAME_LENGTH = 50;

// An error message
static const char* InputImageError = "MATLAB image data of unsupported type.";

// To create an output image:
//    dml::MATLAB_Interface mi0;
//    dip::Image img_out0;
//    img_out0.SetExternalInterface( mi0 );
// Make sure that mi0 exists for as long as img_out0 exists!
// To return that image back to MATLAB:
//    plhs[0] = mi0.GetArray();
// If we don't GetArray(), the mxArray will be destroyed when img_out0 goes out of scope.
// This interface handler doesn't own any data.
class MATLAB_Interface : public dip::ExternalInterface {
   private:
      mxArray* mla = nullptr;
      bool donotfree = false;
      // This is the deleter functor we'll associate to the shared_ptr.
      class FreeHandler {
         private:
            MATLAB_Interface& interface;
         public:
            FreeHandler( MATLAB_Interface& mi ) : interface{mi} {};
            void operator()( void* p ) {
               if( interface.donotfree ) {
                  mexPrintf( "   Not destroying mxArray!\n" );
               } else {
                  mxDestroyArray( interface.mla );
                  interface.mla = nullptr;
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
         const dip::UnsignedArray& tensor_dims,
         dip::IntegerArray&        tensor_strides,
         dip::DataType             datatype
      ) override {
         DIPTS( mla, "External Interface object used by more than one function!" );
         DIPTS( tensor_dims.size() != 0, "Tensor images not yet supported" );
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
         mxClassID type;
         switch (datatype) {
            case dip::DataType::BIN :
            case dip::DataType::UINT8 :
               type = mxUINT8_CLASS;
               break;
            case dip::DataType::SINT8 :
               type = mxINT8_CLASS;
               break;
            case dip::DataType::UINT16 :
               type = mxUINT16_CLASS;
               break;
            case dip::DataType::SINT16 :
               type = mxINT16_CLASS;
               break;
            case dip::DataType::UINT32 :
               type = mxUINT32_CLASS;
               break;
            case dip::DataType::SINT32 :
               type = mxINT32_CLASS;
               break;
            case dip::DataType::SFLOAT :
               type = mxSINGLE_CLASS;
               break;
            case dip::DataType::DFLOAT :
               type = mxDOUBLE_CLASS;
               break;
            case dip::DataType::SCOMPLEX :
               //type = mxSINGLE_CLASS;
               //break;
            case dip::DataType::DCOMPLEX :
               //type = mxDOUBLE_CLASS;
               throw dip::Error( "Complex images not yet supported" );
               // We should support these by allocating an mxArray with an extra first dimension of 2.
               // The function that links stuff back to MATLAB should take care of splitting the image
               // into a real and imaginary parts, and putting those into an mxArray as the two components.
               // That whole process might be accomplished with a single callback to MATLAB.
               break;
            default:
               throw dip::Error( dip::E::ILLEGAL_IMAGE_TYPE ); // Should not be possible
         }
         // Allocate MATLAB matrix
         if (n >= 2) {
            std::swap( mldims[0], mldims[1] );
            std::swap( strides[0], strides[1] );
         } else {
            mldims.resize(2,1);  // add singleton dimensions
         }
         // Alternative is to mxMalloc() data, and pass mxFree() as deleter.
         //    A matrix of the right size and type will have to be created and this
         //    pointer moved into it.
         mla = mxCreateNumericArray( n, mldims.data(), type, mxREAL );
         return std::shared_ptr<void>( mxGetData( mla ), FreeHandler( *this ) );
      };

      mxArray* GetArray() {
         donotfree = true;
         return mla;
         // We should add code here to turn the array into a dip_image object.
      }
};

// A deleter that doesn't delete.
void VoidFreeHandler( void* p ) {};

// Passing an mxArray to DIPlib, keeping ownership of data.
dip::Image GetImage( const mxArray* mx ) {

   // TODO: test for an empty array as input. How do we handle those? throw()?
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
      throw dip::Error( "Complex images not yet supported" );
   dip::DataType datatype;
   switch (type) {
      case mxDOUBLE_CLASS:    /* dfloat */
         if (complex) datatype = dip::DataType::DCOMPLEX;
         else datatype = dip::DataType::DFLOAT;
         break;
      case mxSINGLE_CLASS:    /* sfloat */
         if (complex) datatype = dip::DataType::SCOMPLEX;
         else datatype = dip::DataType::SFLOAT;
         break;
      case mxINT8_CLASS:      /* sint8 */
         DIPTS (complex, InputImageError);
         datatype = dip::DataType::SINT8;
         break;
      case mxUINT8_CLASS:     /* uint8 , bin8 */
         DIPTS (complex, InputImageError);
         if (binary)
            datatype = dip::DataType::BIN;
         else
            datatype = dip::DataType::UINT8;
         break;
      case mxINT16_CLASS:     /* sint16 */
         DIPTS (complex, InputImageError);
         datatype = dip::DataType::SINT16;
         break;
      case mxUINT16_CLASS:    /* uint16, bin16 */
         DIPTS (complex, InputImageError);
         datatype = dip::DataType::UINT16;
      case mxINT32_CLASS:     /* sint32 */
         DIPTS (complex, InputImageError);
         datatype = dip::DataType::SINT32;
         break;
      case mxUINT32_CLASS:    /* uint32, bin32 */
         DIPTS (complex, InputImageError);
         datatype = dip::DataType::UINT32;
      default:
         throw dip::Error( "Image data is not numeric." );
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
   std::shared_ptr<void> p( mxGetData( mxdata ), VoidFreeHandler );
   return dip::Image( p, datatype, dims, strides, {}, {}, nullptr );
}

} // namespace dml

#endif
