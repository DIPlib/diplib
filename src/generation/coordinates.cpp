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
#include "diplib/geometry.h"
#include "diplib/math.h"

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

void ParseModeString( String const& option, CoordinateMode& coordinateMode ) {
   if( option == S::RIGHT ) {
      coordinateMode.system = CoordinateSystem::RIGHT;
   } else if( option == S::LEFT ) {
      coordinateMode.system = CoordinateSystem::LEFT;
   } else if( option == S::TRUE ) {
      coordinateMode.system = CoordinateSystem::TRUE;
   } else if( option == S::CORNER ) {
      coordinateMode.system = CoordinateSystem::CORNER;
   } else if(( option == S::FREQUENCY ) || ( option == "freq" )) {
      coordinateMode.system = CoordinateSystem::FREQUENCY;
   } else if( option == S::RADFREQ ) {
      coordinateMode.system = CoordinateSystem::FREQUENCY;
      coordinateMode.radialFrequency = true;
   } else if( option == S::RADIAL ) {
      coordinateMode.radialFrequency = true;
   } else if( option == S::MATH ) {
      coordinateMode.invertedY = true;
   } else if( option == S::PHYSICAL ) {
      coordinateMode.physical = true;
   } else {
      DIP_THROW_INVALID_FLAG( option );
   }
}

CoordinateMode ParseMode( StringSet const& mode ) {
   CoordinateMode coordinateMode;
   for( auto& option : mode ) {
      ParseModeString( option, coordinateMode );
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


FloatArray Image::GetCenter( String const& mode ) const {
   CoordinateMode coordinateMode;
   ParseModeString( mode, coordinateMode );
   FloatArray center( Dimensionality() );
   for( dip::uint iDim = 0; iDim < center.size(); ++iDim ) {
      center[ iDim ] = FindTransformation( Size( iDim ), iDim, coordinateMode, PixelSize( iDim )).offset;
   }
   return center;
}


void FillDelta( Image& out, String const& origin ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_FORGED );
   CoordinateSystem system;
   if( origin.empty() || ( origin == S::RIGHT )) {
      system = CoordinateSystem::RIGHT;
   } else if( origin == S::LEFT ) {
      system = CoordinateSystem::LEFT;
   } else if( origin == S::CORNER ) {
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

class RampLineFilter : public Framework::ScanLineFilter {
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
      RampLineFilter( dip::uint index, Transformation t ) : index_( index ), offset_( t.offset ), scale_( t.scale ) {}
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
   if( dimension >= out.Dimensionality() ) {
      // The ramp dimension is not one of the image dimensions
      out.Fill( 0 );
      return;
   }
   DIP_START_STACK_TRACE
      CoordinateMode coordinateMode = ParseMode( mode );
      Transformation transformation = FindTransformation( out.Size( dimension ), dimension, coordinateMode, out.PixelSize( dimension ));
      RampLineFilter scanLineFilter( dimension, transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::ScanOption::NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class RadiusLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 20; }
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
      RadiusLineFilter( TransformationArray const& transformation ) : transformation_( transformation ) {}
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
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ));
      }
      RadiusLineFilter scanLineFilter( transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::ScanOption::NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class RadiusSquareLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 4; }
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
      RadiusSquareLineFilter( TransformationArray const& transformation ) : transformation_( transformation ) {}
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
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ));
      }
      RadiusSquareLineFilter scanLineFilter( transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::ScanOption::NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class PhiLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 50; } // worst case (dim != 2)
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
      PhiLineFilter( TransformationArray const& transformation ) : transformation_( transformation ) {}
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
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ));
      }
      PhiLineFilter scanLineFilter( transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::ScanOption::NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class ThetaLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 50; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         DIP_ASSERT( transformation_.size() == 3 );
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint dim = params.dimension;
         dfloat d2 = 0;
         dfloat z = 0; // initialize to prevent compiler warning, value not used.
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
      ThetaLineFilter( TransformationArray const& transformation ) : transformation_( transformation ) {}
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
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ));
      }
      ThetaLineFilter scanLineFilter( transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::ScanOption::NeedCoordinates );
   DIP_END_STACK_TRACE
}

namespace {

class CoordinatesLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint tensorLength ) override {
         return spherical_ ? ( tensorLength == 2 ? 50 : 70 ) : ( 2 + tensorLength );
      }
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
      CoordinatesLineFilter( bool spherical, TransformationArray const& transformation ) : transformation_( transformation ), spherical_( spherical ) {}
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
   DIP_STACK_TRACE_THIS( spherical = BooleanFromString( system, S::SPHERICAL, S::CARTESIAN ));
   DIP_THROW_IF( spherical && (( nDims < 2 ) || ( nDims > 3 )), E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      CoordinateMode coordinateMode = ParseMode( mode );
      TransformationArray transformation( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         transformation[ ii ] = FindTransformation( out.Size( ii ), ii, coordinateMode, out.PixelSize( ii ));
      }
      CoordinatesLineFilter scanLineFilter( spherical, transformation );
      Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::ScanOption::NeedCoordinates );
   DIP_END_STACK_TRACE
}


void FillDistanceToPoint(
      Image& out,
      FloatArray const& point,
      String const& distance,
      FloatArray scaling
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   dip::uint nDims = out.Dimensionality();
   Image::Pixel center( DT_SFLOAT, nDims );
   if( point.empty() ) {
      FloatArray pt = out.GetCenter();
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         center[ ii ] = pt[ ii ];
      }
   } else {
      DIP_THROW_IF( point.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         center[ ii ] = point[ ii ];
      }
   }
   DIP_STACK_TRACE_THIS( ArrayUseParameter( scaling, nDims, 1.0 ));
   Image::Pixel scale( DT_SFLOAT, nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      scale[ ii ] = scaling[ ii ];
   }

   bool protect = out.Protect( true );
   PixelSize ps = out.PixelSize();

   Image coords = CreateCoordinates( out.Sizes(), { S::CORNER }, S::CARTESIAN );
   coords -= center;
   MultiplySampleWise( coords, scale, coords, coords.DataType() );
   if(( distance == S::EUCLIDEAN ) || ( distance == "Euclidean" )) { // allow first letter capitalized
      Norm( coords, out );
   } else if( distance == S::SQUARE ) {
      SquareNorm( coords, out );
   } else if( distance == S::CITY ) {
      Abs( coords, coords );
      SumTensorElements( coords, out );
   } else if( distance == S::CHESS ) {
      Abs( coords, coords );
      MaximumTensorElement( coords, out );
   }

   out.Protect( protect );
   out.SetPixelSize( ps );
}

} // namespace dip
