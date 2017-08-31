/*
 * DIPlib 3.0
 * This file contains definitions for coordinate image generation
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

#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"

namespace dip {

namespace {

enum class CoordinateSystem {
      RIGHT,
      LEFT,
      TRUE,
      CORNER,
      FREQUENCY
};
struct CoordinateMode {
   CoordinateSystem system = CoordinateSystem::RIGHT;
   bool invertedY = false;
   bool physical = false;
   bool radialFrequency = false;
};

CoordinateMode ParseMode( StringSet const& mode ) {
   CoordinateMode coordinateMode;
   for( auto& option : mode ) {
      if( option == "right" ) {
         coordinateMode.system = CoordinateSystem::RIGHT;
      } else if( option == "left" ) {
         coordinateMode.system = CoordinateSystem::LEFT;
      } else if( option == "true" ) {
         coordinateMode.system = CoordinateSystem::TRUE;
      } else if( option == "corner" ) {
         coordinateMode.system = CoordinateSystem::CORNER;
      } else if(( option == "frequency" ) || ( option == "freq" )) {
         coordinateMode.system = CoordinateSystem::FREQUENCY;
      } else if( option == "radfreq" ) {
         coordinateMode.system = CoordinateSystem::FREQUENCY;
         coordinateMode.radialFrequency = true;
      } else if( option == "radial" ) {
         coordinateMode.radialFrequency = true;
      } else if( option == "math" ) {
         coordinateMode.invertedY = true;
      } else if( option == "physical" ) {
         coordinateMode.physical = true;
      } else {
         DIP_THROW_INVALID_FLAG( option );
      }
   }
   return coordinateMode;
}

struct Transformation {
   dfloat offset; // applied first
   dfloat scale;  // applied after
};
using TransformationArray = DimensionArray< Transformation >;

Transformation FindTransformation( dip::uint size, dip::uint dim, CoordinateMode coordinateMode, PhysicalQuantity pixelSize ) {
   Transformation out;
   bool invert = ( dim == 1 ) && coordinateMode.invertedY;
   switch( coordinateMode.system ) {
      default:
      //case CoordinateSystem::RIGHT:
      //case CoordinateSystem::FREQUENCY:
         out.offset = static_cast< dfloat >( size / 2 );
         break;
      case CoordinateSystem::LEFT:
         out.offset = static_cast< dfloat >(( size - 1 ) / 2 );
         break;
      case CoordinateSystem::TRUE:
         out.offset = static_cast< dfloat >( size - 1 ) / 2.0;
         break;
      case CoordinateSystem::CORNER:
         out.offset = invert ? static_cast< dfloat >( size - 1 ) : 0.0;
         break;
   }
   if( coordinateMode.physical ) {
      out.scale = pixelSize.magnitude;
   } else if( coordinateMode.system == CoordinateSystem::FREQUENCY ) {
      out.scale = 1.0 / static_cast< dfloat >( size );
      if( coordinateMode.radialFrequency ) {
         out.scale *= 2.0 * pi;
      }
   } else {
      out.scale = 1.0;
   }
   if( invert ) {
      out.scale = -out.scale;
   }
   return out;
}

} // namespace

void FillDelta( Image& out, String const& origin ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_FORGED );
   CoordinateSystem system = CoordinateSystem::RIGHT;
   if( origin.empty() || ( origin == "right" )) {
      system = CoordinateSystem::RIGHT;
   } else if( origin == "left" ) {
      system = CoordinateSystem::LEFT;
   } else if( origin == "corner" ) {
      system = CoordinateSystem::CORNER;
   } else {
      DIP_THROW_INVALID_FLAG( origin );
   }
   DIP_START_STACK_TRACE
      out.Fill( 0 );
      UnsignedArray pos = out.Sizes();
      for( auto& p : pos ) {
         switch( system ) {
            default:
            //case CoordinateSystem::RIGHT:
               p = p / 2;
               break;
            case CoordinateSystem::LEFT:
               p = ( p - 1 ) / 2;
               break;
            case CoordinateSystem::CORNER:
               p = 0;
               break;
         }
      }
      out.At( pos ) = 1;
   DIP_END_STACK_TRACE
}

namespace {

template< typename TPI >
void dip__SetBorder( Image& out, Image::Pixel& c_value, dip::uint size ) {
   dip::uint nDim = out.Dimensionality();
   UnsignedArray const& sizes = out.Sizes();
   dip::uint nTensor = out.TensorElements();
   // Copy c_value into an array with the right number of elements, and of the right data type
   std::vector< TPI > values( nTensor, c_value[ 0 ].As< TPI >() );
   if( !c_value.IsScalar() ) {
      for( dip::uint ii = 1; ii < nTensor; ++ii ) {
         values[ ii ] = c_value[ ii ].As< TPI >();
      }
   }
   // Iterate over all image lines
   dip::uint procDim = Framework::OptimalProcessingDim( out );
   dip::sint stride = out.Stride( procDim );
   dip::sint lastOffset = ( static_cast< dip::sint >( out.Size( procDim )) - 1 ) * stride;
   dip::sint tensorStride = out.TensorStride();
   ImageIterator< TPI > it( out, procDim );
   do {
      // Is this image line along the image border?
      bool all = false;
      UnsignedArray const& coord = it.Coordinates();
      for( dip::uint ii = 0; ii < nDim; ++ii ) {
         if(( ii != procDim ) && (( coord[ ii ] < size ) || ( coord[ ii ] >= sizes[ ii ] - size ))) {
            all = true;
            break;
         }
      }
      if( all ) {
         // Yes, it is: fill all pixels on the line
         LineIterator< TPI > lit = it.GetLineIterator();
         do {
            std::copy( values.begin(), values.end(), lit.begin() );
         } while( ++lit );
      } else {
         // No, it isn't: fill only the first `size` and last `size` pixels
         TPI* firstPtr = it.Pointer();
         TPI* lastPtr = firstPtr + lastOffset;
         for( dip::uint ii = 0; ii < size; ++ii ) {
            std::copy( values.begin(), values.end(), SampleIterator< TPI >( firstPtr, tensorStride ));
            std::copy( values.begin(), values.end(), SampleIterator< TPI >( lastPtr, tensorStride ));
            firstPtr += stride;
            lastPtr -= stride;
         }
      }
   } while( ++it );
}

} // namespace

void SetBorder( Image& out, Image::Pixel value, dip::uint size ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !value.IsScalar() && ( out.IsScalar() || ( out.TensorElements() != value.TensorElements() )), E::NTENSORELEM_DONT_MATCH );
   DIP_OVL_CALL_ALL( dip__SetBorder, ( out, value, size ), out.DataType());
}

namespace {

class dip__Ramp : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint pp = params.position[ index_ ];
         if( params.dimension == index_ ) {
            // Filling along dimension where the coordinate changes at every step
            for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
               *out = ( static_cast< dfloat >( pp ) - offset_ ) * scale_;
               out += stride;
            }
         } else {
            // Filling along a dimension where the coordinate is constant
            dfloat v = ( static_cast< dfloat >( pp ) - offset_ ) * scale_;
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               *out = v;
               out += stride;
            }
         }
      }
      dip__Ramp( dip::uint index, Transformation t ) : index_( index ), offset_( t.offset ), scale_( t.scale ) {}
   private:
      dip::uint index_; // the index into the coordinates array to write into the scalar output
      dfloat offset_;
      dfloat scale_;
};

} // namespace

void FillRamp( Image& out, dip::uint dimension, StringSet const& mode ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !out.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( dimension >= out.Dimensionality(), E::PARAMETER_OUT_OF_RANGE );
   DIP_START_STACK_TRACE
      CoordinateMode coordinateMode = ParseMode( mode );
      Transformation transformation = FindTransformation( out.Size( dimension ), dimension, coordinateMode, out.PixelSize( dimension ) );
      dip__Ramp scanLineFilter( dimension, transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::Scan_NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class dip__Radius : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint dim = params.dimension;
         dfloat d2 = 0;
         for( dip::uint ii = 0; ii < transformation_.size(); ++ii ) {
            if( ii != dim ) {
               dfloat d = ( static_cast< dfloat >( params.position[ ii ] ) - transformation_[ ii ].offset ) * transformation_[ ii ].scale;
               d2 += d * d;
            }
         }
         dip::uint pp = params.position[ dim ];
         for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
            dfloat d = ( static_cast< dfloat >( pp ) - transformation_[ dim ].offset ) * transformation_[ dim ].scale;
            *out = std::sqrt( d2 + d * d );
            out += stride;
         }
      }
      dip__Radius( TransformationArray const& transformation ) : transformation_( transformation ) {}
   private:
      TransformationArray transformation_;
};

} // namespace

void FillRadiusCoordinate( Image& out, StringSet const& mode ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !out.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      CoordinateMode coordinateMode = ParseMode( mode );
      dip::uint nDims = out.Dimensionality();
      TransformationArray transformation( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ) );
      }
      dip__Radius scanLineFilter( transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::Scan_NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class dip__RadiusSquare : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint dim = params.dimension;
         dfloat d2 = 0;
         for( dip::uint ii = 0; ii < transformation_.size(); ++ii ) {
            if( ii != dim ) {
               dfloat d = ( static_cast< dfloat >( params.position[ ii ] ) - transformation_[ ii ].offset ) * transformation_[ ii ].scale;
               d2 += d * d;
            }
         }
         dip::uint pp = params.position[ dim ];
         for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
            dfloat d = ( static_cast< dfloat >( pp ) - transformation_[ dim ].offset ) * transformation_[ dim ].scale;
            *out = d2 + d * d;
            out += stride;
         }
      }
      dip__RadiusSquare( TransformationArray const& transformation ) : transformation_( transformation ) {}
   private:
      TransformationArray transformation_;
};

} // namespace

void FillRadiusSquareCoordinate( Image& out, StringSet const& mode ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !out.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      CoordinateMode coordinateMode = ParseMode( mode );
      dip::uint nDims = out.Dimensionality();
      TransformationArray transformation( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ) );
      }
      dip__RadiusSquare scanLineFilter( transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::Scan_NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class dip__Phi : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint dim = params.dimension;
         if( dim == 2 ) {
            // In case of 3D image, filling along z axis, all values are identical
            dfloat x = ( static_cast< dfloat >( params.position[ 0 ] ) - transformation_[ 0 ].offset ) * transformation_[ 0 ].scale;
            dfloat y = ( static_cast< dfloat >( params.position[ 1 ] ) - transformation_[ 1 ].offset ) * transformation_[ 1 ].scale;
            dfloat phi = std::atan2( y, x );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               *out = phi;
               out += stride;
            }
         } else {
            // Otherwise, either x or y changes at every step
            dfloat pos[ 2 ];
            dip::uint altdim = dim == 0 ? 1 : 0;
            pos[ altdim ] = ( static_cast< dfloat >( params.position[ altdim ] ) - transformation_[ altdim ].offset ) * transformation_[ altdim ].scale;
            dip::uint pp = params.position[ dim ];
            for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
               pos[ dim ] = ( static_cast< dfloat >( pp ) - transformation_[ dim ].offset ) * transformation_[ dim ].scale;
               *out = std::atan2( pos[ 1 ], pos[ 0 ] );
               out += stride;
            }
         }
      }
      dip__Phi( TransformationArray const& transformation ) : transformation_( transformation ) {}
   private:
      TransformationArray transformation_;
};

} // namespace

void FillPhiCoordinate( Image& out, StringSet const& mode ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !out.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF(( nDims < 2 ) || ( nDims > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      CoordinateMode coordinateMode = ParseMode( mode );
      TransformationArray transformation( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ) );
      }
      dip__Phi scanLineFilter( transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::Scan_NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class dip__Theta : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         DIP_ASSERT( transformation_.size() == 3 );
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint dim = params.dimension;
         dfloat d2 = 0;
         dfloat z;
         for( dip::uint ii = 0; ii < 3; ++ii ) {
            if( ii != dim ) {
               z = ( static_cast< dfloat >( params.position[ ii ] ) - transformation_[ ii ].offset ) * transformation_[ ii ].scale;
               d2 += z * z;
            }
         }
         dip::uint pp = params.position[ dim ];
         if( dim == 2 ) {
            // Filling along dimension where the z coordinate changes at every step
            for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
               z = ( static_cast< dfloat >( pp ) - transformation_[ 2 ].offset ) * transformation_[ 2 ].scale;
               dfloat norm = std::sqrt( d2 + z * z );
               *out = norm == 0.0 ? pi / 2.0 : std::acos( z / norm );
               out += stride;
            }
         } else {
            // Filling along dimension where the z coordinate is constant
            // dim!=2, which means that z has been filled with the current coordinate for the z-axis.
            for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
               dfloat x = ( static_cast< dfloat >( pp ) - transformation_[ dim ].offset ) * transformation_[ dim ].scale;
               // We call it x, but it could be y also
               dfloat norm = std::sqrt( d2 + x * x );
               *out = norm == 0.0 ? pi / 2.0 : std::acos( z / norm );
               out += stride;
            }
         }
      }
      dip__Theta( TransformationArray const& transformation ) : transformation_( transformation ) {}
   private:
      TransformationArray transformation_;
};

} // namespace

void FillThetaCoordinate( Image& out, StringSet const& mode ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !out.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( nDims != 3, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      CoordinateMode coordinateMode = ParseMode( mode );
      TransformationArray transformation( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ) );
      }
      dip__Theta scanLineFilter( transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::Scan_NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class dip__Coordinates : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::sint tensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;
         DIP_ASSERT( tensorLength == transformation_.size() );
         dip::uint bufferLength = params.bufferLength;
         dip::uint dim = params.dimension;
         if( spherical_ ) {
            if( tensorLength == 2 ) {
               // Polar coordinates
               dfloat d2 = 0;
               dfloat coord[ 2 ];
               for( dip::uint ii = 0; ii < 2; ++ii ) {
                  if( ii != dim ) {
                     coord[ ii ] = ( static_cast< dfloat >( params.position[ ii ] ) - transformation_[ ii ].offset ) * transformation_[ ii ].scale;
                     d2 += coord[ ii ] * coord[ ii ];
                  }
               }
               dip::uint pp = params.position[ dim ];
               for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
                  coord[ dim ] = ( static_cast< dfloat >( pp ) - transformation_[ dim ].offset ) * transformation_[ dim ].scale;
                  dfloat norm = std::sqrt( d2 + coord[ dim ] * coord[ dim ] );
                  dfloat* it = out;
                  *it = norm;
                  it += tensorStride;
                  *it = std::atan2( coord[ 1 ], coord[ 0 ] );
                  out += stride;
               }
            } else {
               // Spherical coordinates
               dfloat d2 = 0;
               dfloat coord[ 3 ];
               for( dip::uint ii = 0; ii < 3; ++ii ) {
                  if( ii != dim ) {
                     coord[ ii ] = ( static_cast< dfloat >( params.position[ ii ] ) - transformation_[ ii ].offset ) * transformation_[ ii ].scale;
                     d2 += coord[ ii ] * coord[ ii ];
                  }
               }
               dip::uint pp = params.position[ dim ];
               if( dim == 2 ) {
                  // Filling along dimension where the phi is constant
                  dfloat phi = std::atan2( coord[ 1 ], coord[ 0 ] );
                  for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
                     coord[ 2 ] = ( static_cast< dfloat >( pp ) - transformation_[ 2 ].offset ) * transformation_[ 2 ].scale;
                     dfloat norm = std::sqrt( d2 + coord[ 2 ] * coord[ 2 ] );
                     dfloat* it = out;
                     *it = norm;
                     it += tensorStride;
                     *it = phi;
                     it += tensorStride;
                     *it = norm == 0.0 ? pi / 2.0 : std::acos( coord[ 2 ] / norm );
                     out += stride;
                  }
               } else {
                  // Filling along dimension where the z coordinate is constant
                  for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
                     coord[ dim ] = ( static_cast< dfloat >( pp ) - transformation_[ dim ].offset ) * transformation_[ dim ].scale;
                     dfloat norm = std::sqrt( d2 + coord[ dim ] * coord[ dim ] );
                     dfloat* it = out;
                     *it = norm;
                     it += tensorStride;
                     *it = std::atan2( coord[ 1 ], coord[ 0 ] );
                     it += tensorStride;
                     *it = norm == 0.0 ? pi / 2.0 : std::acos( coord[ 2 ] / norm );
                     out += stride;
                  }
               }
            }
         } else {
            // Cartesian coordinates
            FloatArray coord( tensorLength );
            for( dip::uint ii = 0; ii < tensorLength; ++ii ) {
               if( ii != dim ) {
                  coord[ ii ] = ( static_cast< dfloat >( params.position[ ii ] ) - transformation_[ ii ].offset ) * transformation_[ ii ].scale;
               }
            }
            dip::uint pp = params.position[ dim ];
            for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pp ) {
               coord[ dim ] = ( static_cast< dfloat >( pp ) - transformation_[ dim ].offset ) * transformation_[ dim ].scale;
               std::copy( coord.begin(), coord.end(), SampleIterator< dfloat >( out, tensorStride ));
               out += stride;
            }
         }
      }
      dip__Coordinates( bool spherical, TransformationArray const& transformation ) : transformation_( transformation ), spherical_( spherical ) {}
   private:
      TransformationArray transformation_;
      bool spherical_; // true for polar/spherical coordinates, false for cartesian coordinates
};

} // namespace

void FillCoordinates( Image& out, StringSet const& mode, String const& system ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = out.Dimensionality();
   DIP_THROW_IF( out.TensorElements() != nDims, E::NTENSORELEM_DONT_MATCH );
   bool spherical;
   DIP_START_STACK_TRACE
      spherical = BooleanFromString( system, "spherical", "cartesian" );
   DIP_END_STACK_TRACE
   DIP_THROW_IF( spherical && (( nDims < 2 ) || ( nDims > 3 )), E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      CoordinateMode coordinateMode = ParseMode( mode );
      TransformationArray transformation( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ) );
      }
      dip__Coordinates scanLineFilter( spherical, transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::Scan_NeedCoordinates );
   DIP_END_STACK_TRACE
}

} // namespace dip
