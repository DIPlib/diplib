/*
 * DIPlib 3.0
 * This file contains functionality for the MATLAB interface.
 *
 * (c)2015-2017, Cris Luengo.
 * Based on original DIPlib/MATLAB interface code: (c)1999-2014, Delft University of Technology.
 */

#ifndef DIP_MATLAB_H
#define DIP_MATLAB_H

#include <map>
#include <utility>
#include <streambuf>
#include <iostream>

#include "mex.h"
#include "diplib.h"


/// \file
/// \brief This file should be included in each MEX file. It defines the
/// \ref dml namespace. Since it defines functions that are not `inline`,
/// you will not be able to include this header into more than one source
/// file that will be linked together.


/// \brief The dml namespace contains the interface between *MATLAB* and *DIPlib*. It defines
/// the functions needed to convert between `mxArray` objects and dip::Image objects.
// TODO: add more documentation here on how to use the dml interface.
namespace dml {

// These are the names of the fields of the dip_image structure in MATLAB:
static constexpr char const* dataFieldName = "data";
static constexpr char const* typeFieldName = "dip_type";
static constexpr char const* dimsFieldName = "dims";
static constexpr char const* tensorFieldName = "tensor";

constexpr dip::uint maxNameLength = 50;

// An error message
static constexpr char const* InputImageError = "MATLAB image data of unsupported type.";

//
// Private funtions
//

static bool IsMatlabStrides(
      dip::UnsignedArray const& sizes,
      dip::uint telem,
      dip::IntegerArray const& strides,
      dip::sint tstride
) {
   if( sizes.size() != strides.size() ) {
      return false;
   }
   if( sizes.size() < 2 ) {
      return true;
   }
   if( strides[ 1 ] != 1 ) {
      return false;
   }
   dip::sint total = ( dip::sint )sizes[ 1 ];
   if( strides[ 0 ] != total ) {
      return false;
   }
   total *= sizes[ 0 ];
   for( dip::uint ii = 2; ii < sizes.size(); ++ii ) {
      if( strides[ ii ] != total ) {
         return false;
      }
      total *= sizes[ ii ];
   }
   if( ( telem > 1 ) && ( tstride != total ) ) {
      return false;
   }
   return true;
}

static bool MatchDimensions(
      dip::UnsignedArray const& sizes,
      dip::uint telem,
      mwSize const* psizes,
      mwSize ndims
) {
   dip::uint n = sizes.size() + ( telem > 1 ? 1 : 0 );
   if( n == 0 ) {
      return !( ( ndims != 2 ) || ( psizes[ 0 ] != 1 ) || ( psizes[ 1 ] != 1 ) );
   } else if( n == 1 ) {
      dip::uint m = sizes[ 0 ] * telem;
      return !( ( ndims != 2 ) || ( psizes[ 0 ] != m ) || ( psizes[ 1 ] != 1 ) );
   } else {
      if( ( ndims != n ) || ( psizes[ 0 ] != sizes[ 1 ] ) || ( psizes[ 1 ] != sizes[ 0 ] ) ) {
         return false;
      }
      for( dip::uint ii = 3; ii < n; ++ii ) {
         if( sizes[ ii ] != psizes[ ii ] ) {
            return false;
         }
      }
      return true;
   }
}

static mxClassID GetMatlabClassID(
      dip::DataType dt
) {
   mxClassID type = mxUINT8_CLASS;
   switch( dt ) {
      case dip::DT_BIN:
         type = mxLOGICAL_CLASS;
         break;
      case dip::DT_UINT8:
         type = mxUINT8_CLASS;
         break;
      case dip::DT_SINT8:
         type = mxINT8_CLASS;
         break;
      case dip::DT_UINT16:
         type = mxUINT16_CLASS;
         break;
      case dip::DT_SINT16:
         type = mxINT16_CLASS;
         break;
      case dip::DT_UINT32:
         type = mxUINT32_CLASS;
         break;
      case dip::DT_SINT32:
         type = mxINT32_CLASS;
         break;
      case dip::DT_SFLOAT:
      case dip::DT_SCOMPLEX:
         type = mxSINGLE_CLASS;
         break;
      case dip::DT_DFLOAT:
      case dip::DT_DCOMPLEX:
         type = mxDOUBLE_CLASS;
         break;
      default: DIP_THROW_ASSERTION( "Unhandled DataType" ); // Should not be possible
   }
   return type;
}

/// \brief This class is the dip::ExternalInterface for the *MATLAB* interface.
///
/// In a MEX-file, use the following code when declaring images to be
/// used as the output to a function:
///
///     dml::MatlabInterface mi;
///     dip::Image img_out0 = mi.NewImage();
///     dip::Image img_out1 = mi.NewImage();
///
/// To return those images back to *MATLAB*, use the GetArray() method:
///
///     plhs[0] = mi.GetArray( img_out0 );
///     plhs[1] = mi.GetArray( img_out1 );
///
/// If you don't use the GetArray() method, the `mxArray` that contains
/// the pixel data will be destroyed when the dip::Image object goes out
/// of scope.
///
/// Remember to not assing a result into the images created with NewImage,
/// as they will be overwritten and no longer contain data allocated by
/// *MATLAB*. Instead, use the *DIPlib* functions that take output images as
/// function arguments:
///
///     img_out0 = in1 + in2; // WRONG! img_out0 will not contain data allocated by *MATLAB*
///     dip::Add( in1, in2, out, DataType::SuggestArithmetic( in1.DataType(), in1.DataType() ) ); // Correct
///
/// This interface handler doesn't own any image data.
class MatlabInterface : public dip::ExternalInterface {
   private:
      std::map< void const*, mxArray* > mla;          // This map holds mxArray pointers, we can
      // find the right mxArray if we have the data
      // pointer.
      // This is the deleter functor we'll associate to the shared_ptr.
      class StripHandler {
         private:
            MatlabInterface& interface;
         public:
            StripHandler( MatlabInterface& mi ) : interface{ mi } {};
            void operator()( void const* p ) {
               if( interface.mla.count( p ) == 0 ) {
                  //mexPrintf( "   Not destroying mxArray!\n" );
               } else {
                  mxDestroyArray( interface.mla[ p ] );
                  interface.mla.erase( p );
                  //mexPrintf( "   Destroyed mxArray!\n" );
               }
            };
      };

   public:
      /// This function overrides dip::ExternalInterface::AllocateData().
      /// It is called when an image with this `ExternalInterface` is forged.
      /// It allocates a *MATLAB* `mxArray` and returns a `std::shared_ptr` to the
      /// `mxArray` data pointer, with a custom deleter functor. It also
      /// adjusts strides to match the `mxArray` storage.
      ///
      /// A user will never call this function.
      virtual std::shared_ptr< void > AllocateData(
            dip::UnsignedArray const& sizes,
            dip::IntegerArray& strides,
            dip::Tensor const& tensor,
            dip::sint& tstride,
            dip::DataType datatype
      ) override {
         // Complex arrays are stored differently in MATLAB than in DIPlib.
         // We simply allocate an array using std::malloc(), then copy the data
         // over to a MATLAB array when pushing the image back to MATLAB.
         if( datatype.IsComplex() ) {
            //mexPrintf( "   Complex image is not put into an mxArray.\n" );
            return std::shared_ptr< void >();
         } else {
            // Find the right MATLAB class
            mxClassID type = GetMatlabClassID( datatype );
            // Copy size array
            dip::UnsignedArray mlsizes = sizes;
            dip::uint n = sizes.size();
            // MATLAB arrays switch y and x axes
            if( n >= 2 ) {
               using std::swap;
               swap( mlsizes[ 0 ], mlsizes[ 1 ] );
            }
            // Create stride array
            dip::uint s = 1;
            strides.resize( n );
            for( dip::uint ii = 0; ii < n; ii++ ) {
               strides[ ii ] = s;
               s *= mlsizes[ ii ];
            }
            // Append tensor dimension as the last dimension of the mxArray
            if( tensor.Elements() > 1 ) {
               mlsizes.push_back( tensor.Elements() );
            }
            tstride = s;
            // MATLAB arrays switch y and x axes
            if( n >= 2 ) {
               using std::swap;
               swap( strides[ 0 ], strides[ 1 ] );
            }
            // MATLAB arrays have at least 2 dimensions.
            if( mlsizes.size() < 2 ) {
               mlsizes.resize( 2, 1 );  // add singleton dimensions at end
            }
            // Allocate MATLAB matrix
            void* p;
            if( type == mxLOGICAL_CLASS ) {
               mxArray* m = mxCreateLogicalArray( mlsizes.size(), mlsizes.data() );
               p = mxGetLogicals( m );
               mla[ p ] = m;
            } else {
               mxArray* m = mxCreateNumericArray( mlsizes.size(), mlsizes.data(), type, mxREAL );
               p = mxGetData( m );
               mla[ p ] = m;
            }
            //mexPrintf( "   Created mxArray as dip::Image data block. Data pointer = %p.\n", p );
            return std::shared_ptr< void >( p, StripHandler( *this ) );
         }
      }

      /// \brief Find the `mxArray` that holds the data for the dip::Image `img`.
      mxArray* GetArray( dip::Image const& img ) {
         DIP_THROW_IF( !img.IsForged(), dip::E::IMAGE_NOT_FORGED );
         mxArray* m;
         if( img.DataType().IsComplex() ) {
            //mexPrintf( "   Copying complex data from dip::Image to mxArray.\n" );
            // Complex images must be split and copied over to new mxArrays.
            // Copy the two planes into two mxArrays
            dip::Image real = NewImage();
            real.Copy( img.Real() );
            dip::Image imag = NewImage();
            imag.Copy( img.Imaginary() );
            // Retrieve the two mxArrays
            mxArray* c[2];
            c[ 0 ] = mla[ real.Data() ];
            c[ 1 ] = mla[ imag.Data() ];
            // Call MATLAB's "complex" (*shouldn't* copy data...)
            mexCallMATLAB( 1, &m, 2, c, "complex" );
         } else {
            void const* p = img.Data();
            m = mla[ p ];
            if( !m ) { mexPrintf( "   ...that was a nullptr mxArray\n" ); } // TODO: temporary warning, to be removed.
            // Does the image point to a modified view of the mxArray? Or to a non-MATLAB array?
            if( !m || ( p != img.Origin() ) ||
                !IsMatlabStrides(
                      img.Sizes(), img.TensorElements(),
                      img.Strides(), img.TensorStride() ) ||
                !MatchDimensions(
                      img.Sizes(), img.TensorElements(),
                      mxGetDimensions( m ), mxGetNumberOfDimensions( m ) ) ||
                ( mxGetClassID( m ) != GetMatlabClassID( img.DataType() ) )
                  ) {
               // Yes, it does. We need to make a copy of the image into a new MATLAB array.
               mexPrintf( "   Copying data from dip::Image to mxArray\n" ); // TODO: temporary warning, to be removed.
               dip::Image tmp = NewImage();
               tmp.Copy( img );
               std::cout << tmp << std::endl;
               p = tmp.Data();
               m = mla[ p ];
               mla.erase( p );
            } else {
               // No, it doesn't. Directly return the mxArray.
               mla.erase( p );
               //mexPrintf( "   Retrieving mxArray out of output dip::Image object\n" );
            }
         }
         // TODO: We need to add code here to turn the array into a dip_image object.
         return m;
      }

      /// \brief Constructs a dip::Image object with the external interface set so that,
      /// when forged, a *MATLAB* `mxArray` will be allocated to hold the samples.
      ///
      /// Use dml::MatlabInterface::GetArray to obtain the `mxArray` and assign
      /// it as a `lhs` argument to your MEX-file.
      dip::Image NewImage() {
         dip::Image out;
         out.SetExternalInterface( this );
         return out;
      }
};

// A deleter that doesn't delete.
void VoidStripHandler( void const* p ) {
   //mexPrintf( "   Input mxArray not being destroyed\n" );
};

/// \brief Passing an `mxArray` to *DIPlib*, keeping ownership of the data.
///
/// This function "converts" an `mxArray` with image data to a dip::Image object.
/// The dip::Image object will point to the data in the `mxArray`, unless
/// the array contains complex numbers. Complex data needs to be copied because
/// *MATLAB* represents it internally as two separate data blocks. In that
/// case, the dip::Image object will own its own data block.
///
/// When calling GetImage with a `prhs` argument in `mexFunction()`, use a const
/// modifier for the output argument. This should prevent acidentally modifying
/// an input array, which is supposed to be illegal in `mexFunction()`:
///
///     dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
dip::Image GetImage( mxArray const* mx ) {

   // TODO: test for an empty array as input. How do we handle those? throw()? non-forged image?

   // Find image properties
   bool complex = false, binary = false;
   dip::Tensor tensor; // scalar by default
   dip::uint ndims;
   mxClassID type;
   mxArray const* mxdata;
   if( mxIsClass( mx, "dip_image" ) ) {
      mxdata = mxGetField( mx, 0, dataFieldName );
      mxArray* mxtype = mxGetField( mx, 0, typeFieldName );
      char buf[maxNameLength];
      mxGetString( mxtype, buf, maxNameLength );
      if( !strncmp( buf, "bin", 3 ) ) {
         binary = true;
      }
      if( !strncmp( buf + 1, "complex", 7 ) ) {
         complex = true;
      }
      type = mxGetClassID( mxdata );
      ndims = static_cast< dip::uint >(mxGetScalar( mxGetField( mx, 0, dimsFieldName )));
      // TODO: read `tensorFieldName`, set the `tensor` variable appropriately.
   } else {
      mxdata = mx;
      ndims = mxGetNumberOfDimensions( mxdata );
      if( ndims <= 2 ) {
         mwSize const* psizes = mxGetDimensions( mxdata );
         if( psizes[ 0 ] == 1 && psizes[ 1 ] == 1 ) {
            ndims = 0;
         } else if( psizes[ 0 ] > 1 && psizes[ 1 ] > 1 ) {
            ndims = 2;
         } else {
            ndims = 1;
         }
      }
      binary = mxIsLogical( mxdata );
      if( binary ) {
         type = mxUINT8_CLASS;
         complex = false;
      } else {
         type = mxGetClassID( mxdata );
         complex = mxIsComplex( mxdata );
      }
      // It's never a tensor.
   }
   dip::DataType datatype;
   switch( type ) {
      case mxDOUBLE_CLASS:    // dfloat
         if( complex ) { datatype = dip::DT_DCOMPLEX; }
         else { datatype = dip::DT_DFLOAT; }
         break;
      case mxSINGLE_CLASS:    // sfloat
         if( complex ) { datatype = dip::DT_SCOMPLEX; }
         else { datatype = dip::DT_SFLOAT; }
         break;
      case mxINT8_CLASS:      // sint8
      DIP_THROW_IF( complex, InputImageError );
         datatype = dip::DT_SINT8;
         break;
      case mxUINT8_CLASS:     // uint8 , bin
      DIP_THROW_IF( complex, InputImageError );
         if( binary ) {
            datatype = dip::DT_BIN;
         } else {
            datatype = dip::DT_UINT8;
         }
         break;
      case mxINT16_CLASS:     // sint16
      DIP_THROW_IF( complex, InputImageError );
         datatype = dip::DT_SINT16;
         break;
      case mxUINT16_CLASS:    // uint16
      DIP_THROW_IF( complex, InputImageError );
         datatype = dip::DT_UINT16;
         break;
      case mxINT32_CLASS:     // sint32
      DIP_THROW_IF( complex, InputImageError );
         datatype = dip::DT_SINT32;
         break;
      case mxUINT32_CLASS:    // uint32
      DIP_THROW_IF( complex, InputImageError );
         datatype = dip::DT_UINT32;
         break;
      default: DIP_THROW( "Image data is not numeric." );
   }
   // Create the size and stride arrays
   dip::UnsignedArray sizes( ndims );
   mwSize const* psizes = mxGetDimensions( mxdata );
   if( ndims == 1 ) {
      // for a 1D image, we expect one of the two dimensions to be 1. This also handles the case that one of them is 0.
      sizes[ 0 ] = psizes[ 0 ] * psizes[ 1 ];
   } else if( ndims > 1 ) {
      for( dip::uint ii = 0; ii < ndims; ii++ ) {
         sizes[ ii ] = psizes[ ii ];
      }
   }
   dip::uint s = 1;
   dip::IntegerArray strides( ndims );
   for( dip::uint ii = 0; ii < ndims; ii++ ) {
      strides[ ii ] = s;
      s *= sizes[ ii ];
   }
   dip::sint tstride = s;
   if( s == 0 ) {
      // This means that the input image was empty. For now we'll represent
      // this case as a non-forged image. We'll see if this carries through
      // in the rest of DIPlib, or if we should just throw here right away.
      return dip::Image();
   }
   if( ndims >= 2 ) {
      using std::swap;
      swap( sizes[ 0 ], sizes[ 1 ] );
      swap( strides[ 0 ], strides[ 1 ] );
   }
   if( complex ) {
      //mexPrintf("   Copying complex image.\n");
      // Create 2 temporary Image objects for the real and complex component,
      // then copy them over into a new image.
      dip::Image out( sizes, 1, datatype );
      dip::DataType dt = datatype == dip::DT_DCOMPLEX ? dip::DT_DFLOAT : dip::DT_SFLOAT;
      std::shared_ptr< void > p_real( mxGetData( mxdata ), VoidStripHandler );
      if( p_real ) {
         dip::Image real( p_real, dt, sizes, strides, tensor, tstride, nullptr );
         out.Real().Copy( real );
      } else {
         out.Real().Fill( 0 );
      }
      std::shared_ptr< void > p_imag( mxGetImagData( mxdata ), VoidStripHandler );
      if( p_imag ) {
         dip::Image imag( p_imag, dt, sizes, strides, tensor, tstride, nullptr );
         out.Imaginary().Copy( imag );
      } else {
         out.Imaginary().Fill( 0 );
      }
      return out;
   } else if( binary ) {
      //mexPrintf("   Encapsulating binary image.\n");
      // Create Image object
      static_assert( sizeof( mxLogical ) == sizeof( dip::bin ), "mxLogical is not one byte!" );
      std::shared_ptr< void > p( mxGetLogicals( mxdata ), VoidStripHandler );
      return dip::Image( p, datatype, sizes, strides, tensor, tstride, nullptr );
   } else {
      //mexPrintf("   Encapsulating non-complex and non-binary image.\n");
      // Create Image object
      std::shared_ptr< void > p( mxGetData( mxdata ), VoidStripHandler );
      return dip::Image( p, datatype, sizes, strides, tensor, tstride, nullptr );
   }
}

/// \brief An output stream buffer for MEX-files.
///
/// Creating an object of this class replaces the stream buffer in `std::cout` with the newly
/// created object. This buffer will be used as long as the object exists. When the object
/// is destroyed (which happens automatically when it goes out of scope), the original
/// stream buffer is replaced.
///
/// Create an object of this class at the beginning of any MEX-file that uses `std::cout` to
/// print information to the *MATLAB* terminal. *DIPlib* defines several classes with a stream
/// insertion operator that would be cumbersome to use with a `std::stringstream` and `mexPrintf`.
/// This class simplifies their use.
class streambuf : public std::streambuf {
   public:
      streambuf() {
         stdoutbuf = std::cout.rdbuf( this );
      }
      ~streambuf() {
         std::cout.rdbuf( stdoutbuf );
      }
   protected:
      virtual std::streamsize xsputn( const char* s, std::streamsize n ) override {
         mexPrintf( "%.*s", n, s );
         return n;
      }
      virtual int overflow( int c = EOF ) override {
         if( c != EOF ) {
            mexPrintf( "%.1s", & c );
         }
         return 1;
      }
   private:
      std::streambuf *stdoutbuf;
};

} // namespace dml

#endif
