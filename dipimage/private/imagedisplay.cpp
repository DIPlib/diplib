/*
 * DIPimage 3.0
 * This MEX-file implements the `imagedisplay` function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Interface:
 *
 * Create an object of type dip_imagedisplay. IMAGE must be of type dip_image.
 *    handle = imagedisplay(image)
 *
 * Destroy an object. HANDLE must have been returned by the first syntax.
 *    imagedisplay(handle,'clear')
 *
 * Set display modes.
 *    imagedisplay(handle,'coordinates',[x,y,z,...])
 *    imagedisplay(handle,'slicing',[a,b])          % slice along dimensions a-1 and b-1
 *    imagedisplay(handle,'slicemode','slice')      % or: 'max', 'mean' for projections
 *    imagedisplay(handle,'globalstretch','yes')    % or: 'no'
 *    imagedisplay(handle,'globalstretch',true)     % or: false, idem to above
 *    imagedisplay(handle,'complexmapping','abs')   % or: 'magnitude', 'phase', 'real', 'imag'
 *    imagedisplay(handle,'mappingmode','lin')      % or: 'log', 'based', '8bit', etc. etc. etc.
 *    imagedisplay(handle,'mappingmode',[a,b])      % map the given range
 *
 * Get display modes.
 *    mode = imagedisplay(handle,'coordinates')
 *    mode = imagedisplay(handle,'slicing')
 *    mode = imagedisplay(handle,'slicemode')
 *    mode = imagedisplay(handle,'globalstretch')
 *    mode = imagedisplay(handle,'complexmapping')
 *    mode = imagedisplay(handle,'mappingmode')
 *    mode = imagedisplay(handle,'range')
 *
 * Get an image for display. OUT can be directly passed to an Image handle graphics object.
 *    out = imagedisplay(handle)
 *
 * Get the pixel value at the given 2D coordinates in the displayed image. VALUE is a string.
 *    value = imagedisplay(handle,coords)
 *
 * Unlock the MEX-file so it can be deleted:
 *    imagedisplay('unlock')
 *    clear imagedisplay
 *
 * For testing: list all handles stored in the MEX-file:
 *    imagedisplay('debug')
 *
 * Internals:
 *
 * This function is based on a MATLAB handle class, which stores an integer handle to a dip::ImageDisplay
 * object, stored in this MEX-file. The MEX-file is locked in memory.
 *
 * The integer handle is mapped to an object through a std::map.
 *
 */

#undef DIP__ENABLE_DOCTEST

#include <include/diplib/library/error.h>
#include "dip_matlab_interface.h"
#include "diplib/display.h"

// This is a simplified version of dml::MatlabInterface that creates 2D UINT8 images only.
//  - The tensor dimension is always at the end.
//  - The mxArray is made persistent, and always destroyed when the dip::Image is destroyed.
//  - The GetArray method does not convert to dip_image, but it returns a copy of the mxArray.
class MatlabInterfaceUInt8 : public dip::ExternalInterface {
   private:
      static void StripHandler( void* p ) {
         mxDestroyArray( static_cast< mxArray* >( p ));
      }

   public:

      virtual dip::DataSegment AllocateData(
            void*& origin,
            dip::DataType datatype,
            dip::UnsignedArray const& sizes,
            dip::IntegerArray& strides,
            dip::Tensor const& tensor,
            dip::sint& tstride
      ) override {
         DIP_THROW_IF( datatype != dip::DT_UINT8, dip::E::DATA_TYPE_NOT_SUPPORTED );
         DIP_THROW_IF( sizes.size() != 2, dip::E::DIMENSIONALITY_NOT_SUPPORTED );
         dip::UnsignedArray mlsizes( 3 );
         mlsizes[ 0 ] = sizes[ 1 ];
         mlsizes[ 1 ] = sizes[ 0 ];
         mlsizes[ 2 ] = tensor.Elements();
         strides.resize( 2 );
         strides[ 0 ] = static_cast< dip::sint >( sizes[ 1 ] );
         strides[ 1 ] = 1;
         tstride = static_cast< dip::sint >( sizes[ 0 ] * sizes[ 1 ] );
         mxArray* array = mxCreateNumericArray( mlsizes.size(), mlsizes.data(), mxUINT8_CLASS, mxREAL );
         mexMakeArrayPersistent( array );
         origin = mxGetData( array );
         return dip::DataSegment{ array, &MatlabInterfaceUInt8::StripHandler };
      }

      mxArray* GetArray( dip::Image const& img ) {
         DIP_ASSERT( img.IsForged() );
         mxArray* mat = static_cast< mxArray* >( img.Data() );
         return mxDuplicateArray( mat );
      }
};

constexpr char const* className = "dip_imagedisplay";

mxArray* CreateHandle( dip::uint handle ) {
   mxArray* out;
   mxArray* in[ 2 ];
   in[ 0 ] = mxCreateString( "create" );
   in[ 1 ] = dml::GetArray( handle );
   mexCallMATLAB( 1, &out, 2, in, className );
   return out;
}

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   // This data is kept in memory
   static MatlabInterfaceUInt8 externalInterface;
   using Handle = dip::uint;
   using Object = std::shared_ptr< dip::ImageDisplay >;
   using ObjectMap = std::map< Handle, Object >;
   static ObjectMap objects;
   static Handle newHandle = 0;

   if( !mexIsLocked() ) {
      mexLock();
   }

   try {

      DML_MIN_ARGS( 1 );

      if( mxIsChar( prhs[ 0 ] )) {

         // --- Generic action ---

         DML_MAX_ARGS( 1 );

         dip::String action = dml::GetString( prhs[ 0 ] );
         if( action == "unlock" ) {
            mexUnlock();
         } else if( action == "debug" ) {
            mexPrintf( "IMAGEDISPLAY currently has %llu objects stored:\n", objects.size() );
            for( auto const& object : objects ) {
               mexPrintf( " - handle number %llu\n", object.first );
            }
         } else {
            DIP_THROW( "Illegal input" );
         }

      } else if( mxIsClass( prhs[ 0 ], className )) {

         Handle handle = dml::GetUnsigned( mxGetProperty( prhs[ 0 ], 0, "handle" ));
         auto it = objects.find( handle );
         DIP_THROW_IF( it == objects.end(), "Handle not known" );
         Object& object = it->second;

         if( nrhs == 1 ) {

            // --- Produce image output ---

            dip::Image const& out = object->Output();
            plhs[ 0 ] = externalInterface.GetArray( out );

         } else {
            if( mxIsChar( prhs[ 1 ] )) {

               dip::String key = dml::GetString( prhs[ 1 ] );

               if( nrhs == 2 ) {
                  std::cout << "Getting property " << key << " for handle " << handle << std::endl;
               } else {
                  std::cout << "Setting property " << key << " for handle " << handle << std::endl;
               }

               if( key == "clear" ) {

                  // --- Clear (destroy) object ---

                  DML_MAX_ARGS( 2 );
                  objects.erase( it );

               } else {

                  // --- Get/Set properties ---

                  DML_MAX_ARGS( 3 );
                  if( key == "coordinates" ) {
                     if( nrhs == 2 ) {
                        plhs[ 0 ] = dml::GetArray( object->GetCoordinates() );
                     } else {
                        object->SetCoordinates( dml::GetUnsignedArray( prhs[ 2 ] ));
                     }
                  } else if( key == "slicing" ) {
                     if( nrhs == 2 ) {
                        dip::uint dim1, dim2;
                        std::tie( dim1, dim2 ) = object->GetDirection();
                        plhs[ 0 ] = dml::CreateDouble2Vector( static_cast< dip::dfloat >( dim1 ) + 1,
                                                              static_cast< dip::dfloat >( dim2 ) + 1 );
                     } else {
                        dip::UnsignedArray dims = dml::GetUnsignedArray( prhs[ 2 ] );
                        DIP_THROW_IF( dims.size() != 2, dip::E::ARRAY_ILLEGAL_SIZE );
                        DIP_THROW_IF(( dims[ 0 ] == 0 ) || ( dims[ 1 ] == 0 ), dip::E::PARAMETER_OUT_OF_RANGE );
                        object->SetDirection( dims[ 0 ] - 1, dims[ 1 ] - 1 );
                     }
                  } else if( key == "slicemode" ) {
                     if( nrhs == 2 ) {
                        plhs[ 0 ] = dml::GetArray( object->GetProjectionMode() );
                     } else {
                        object->SetProjectionMode( dml::GetString( prhs[ 2 ] ));
                     }
                  } else if( key == "globalstretch" ) {
                     if( nrhs == 2 ) {
                        if( object->GetGlobalStretch() ) {
                           plhs[ 0 ] = mxCreateString( "yes" );
                        } else {
                           plhs[ 0 ] = mxCreateString( "no" );
                        }
                     } else {
                        object->SetGlobalStretch( dml::GetString( prhs[ 2 ] ));
                     }
                  } else if( key == "complexmapping" ) {
                     if( nrhs == 2 ) {
                        plhs[ 0 ] = dml::GetArray( object->GetComplexMode() );
                     } else {
                        object->SetComplexMode( dml::GetString( prhs[ 2 ] ));
                     }
                  } else if( key == "mappingmode" ) {
                     if( nrhs == 2 ) {
                        plhs[ 0 ] = dml::GetArray( object->GetMappingMode());
                     } else {
                        if( mxIsChar( prhs[ 2 ] )) {
                           object->SetRange( dml::GetString( prhs[ 2 ] ) );
                        } else {
                           dip::FloatArray arr = dml::GetFloatArray( prhs[ 2 ] );
                           DIP_THROW_IF( arr.size() != 2, dip::E::ARRAY_ILLEGAL_SIZE );
                           object->SetRange( dip::ImageDisplay::Limits{ arr[ 0 ], arr[ 1 ] } );
                        }
                     }
                  } else if( key == "range" ) {
                     if( nrhs == 2 ) {
                        auto lims = object->GetRange();
                        plhs[ 0 ] = dml::CreateDouble2Vector( static_cast< dip::dfloat >( lims.lower ),
                                                              static_cast< dip::dfloat >( lims.upper ));
                     } else {
                        dip::FloatArray arr = dml::GetFloatArray( prhs[ 2 ] );
                        DIP_THROW_IF( arr.size() != 2, dip::E::ARRAY_ILLEGAL_SIZE );
                        object->SetRange( dip::ImageDisplay::Limits{ arr[ 0 ], arr[ 1 ] } );
                     }
                  } else {
                     DIP_THROW( "Illegal input" );
                  }

               }

            } else {

               // --- Get pixel values at given coordinates ---

               std::cout << "Getting pixel values at given coordinates" << std::endl;

               DML_MAX_ARGS( 2 );
               dip::UnsignedArray coords = dml::GetUnsignedArray( prhs[ 1 ] );
               DIP_THROW_IF( coords.size() != 2, dip::E::ARRAY_ILLEGAL_SIZE );
               plhs[ 0 ] = dml::GetArray( object->Pixel( coords[ 0 ], coords[ 1 ] ));

            }
         }

      } else if( mxIsClass( prhs[ 0 ], "dip_image" )) {

         // --- Construct a new object ---

         DML_MAX_ARGS( 1 );

         dip::Image const in = dml::GetImage( prhs[ 0 ] );
         Object object = std::make_shared< dip::ImageDisplay >( in, &externalInterface );
         objects.emplace( newHandle, object );
         plhs[ 0 ] = CreateHandle( newHandle );
         ++newHandle;

      } else {
         DIP_THROW( "Illegal input" );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
