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

namespace dip {

namespace {

enum class CoordinateSystem {
      LEFT,
      RIGHT,
      TRUE,
      CORNER,
      FREQUENCY,
      RADIAL_FREQUENCY
};
struct CoordinateMode {
   CoordinateSystem system;
   bool invertedY;
};

CoordinateMode ParseMode( StringSet const& mode ) {
   CoordinateMode coordinateMode{ CoordinateSystem::CORNER, false };
   bool hasRadial = false;
   for( auto& option : mode ) {
      if( option == "left" ) {
         coordinateMode.system = CoordinateSystem::LEFT;
      } else if( option == "right" ) {
         coordinateMode.system = CoordinateSystem::RIGHT;
      } else if( option == "true" ) {
         coordinateMode.system = CoordinateSystem::TRUE;
      } else if( option == "corner" ) {
         coordinateMode.system = CoordinateSystem::CORNER;
      } else if(( option == "frequency" ) || ( option == "freq" )) {
         coordinateMode.system = CoordinateSystem::FREQUENCY;
      } else if( option == "radfreq" ) {
         coordinateMode.system = CoordinateSystem::FREQUENCY;
         hasRadial = true;
      } else if( option == "radial" ) {
         hasRadial = true;
      } else if( option == "math" ) {
         coordinateMode.invertedY = true;
      } else {
         DIP_THROW( E::INVALID_FLAG );
      }
   }
   if( hasRadial && ( coordinateMode.system == CoordinateSystem::FREQUENCY )) {
      coordinateMode.system = CoordinateSystem::RADIAL_FREQUENCY;
   }
   return coordinateMode;
}

struct Transformation {
   dfloat offset; // applied first
   dfloat scale;  // applied after
};

Transformation FindTransformation( dip::uint size, dip::uint dim, CoordinateMode coordinateMode ) {
   Transformation out;
   switch( coordinateMode.system ) {
      case CoordinateSystem::LEFT:
         out.offset = static_cast< dfloat >(( size - 1 ) / 2 );
         out.scale = 1.0;
         break;
      case CoordinateSystem::RIGHT:
         out.offset = static_cast< dfloat >( size / 2 );
         out.scale = 1.0;
         break;
      case CoordinateSystem::TRUE:
         out.offset = static_cast< dfloat >( size - 1 ) / 2.0;
         out.scale = 1.0;
         break;
      case CoordinateSystem::CORNER:
         out.offset = 0.0;
         out.scale = 1.0;
         break;
      case CoordinateSystem::FREQUENCY:
         out.offset = static_cast< dfloat >( size / 2 );
         out.scale = 1.0 / static_cast< dfloat >( size );
         break;
      case CoordinateSystem::RADIAL_FREQUENCY:
         out.offset = static_cast< dfloat >( size / 2 );
         out.scale = 2.0 * pi / static_cast< dfloat >( size );
         break;
   }
   if(( dim == 1 ) && coordinateMode.invertedY ) {
      out.scale = -out.scale;
   }
   return out;
}

class dip__Ramp : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint pos = params.position[ index_ ];
         if( params.dimension == index_ ) {
            for( dip::uint ii = 0; ii < bufferLength; ++ii, ++pos ) {
               *out = ( static_cast< dfloat >( pos ) - offset_ ) * scale_;
               out += stride;
            }
         } else {
            dfloat v = ( static_cast< dfloat >( pos ) - offset_ ) * scale_;
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
   CoordinateMode coordinateMode = ParseMode( mode );
   Transformation transformation = FindTransformation( out.Size( dimension ), dimension, coordinateMode );
   std::cout << "[FillRamp] offset = " << transformation.offset << ", scale = " << transformation.scale << std::endl;
   dip__Ramp scanLineFilter( dimension, transformation );
   Framework::ScanSingleOutput( out, DT_DFLOAT, scanLineFilter, Framework::Scan_NeedCoordinates );
}

void FillRadiusCoordinate( Image& /*out*/, StringSet const& /*mode*/ ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

void FillPhiCoordinate( Image& /*out*/, StringSet const& /*mode*/ ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

void FillThetaCoordinate( Image& /*out*/, StringSet const& /*mode*/ ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

namespace {

/*
class dip__CartesianCoordinates : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         dip::sint stride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;

         UnsignedArray position = params.position;
         dip::uint dim = params.dimension;
         DIP_ASSERT( tensorLength == position.size());
         auto tensorStride = params.outBuffer[ 0 ].tensorStride;
         if( cartesian_ ) {
            // Cartesian coordinates
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               std::copy( position.begin(), position.end(), SampleIterator( out, tensorStride ));
               ++position[ dim ];
               out += stride;
            }
         } else {
            // Polar coordinates
            pout[ 0 ] = Norm( 3, pin );
            pout[ 1 ] = std::atan2( pin[ 1 ], pin[ 0 ] );
            pout[ 2 ] = std::acos( pin[ 2 ] / pout[ 0 ] );
         }
      }
      dip__CartesianCoordinates( dip::uint index, bool cartesian ) : index_( index ), cartesian_( cartesian ) {}
   private:
      dip::uint index_; // the index into the coordinates array to write into the scalar output
      bool cartesian_; // false means polar coordinates
      // For polar coordinates, index_=0 means radius, 1 means phi, 2 means theta (see dip::CartesianToPolar)
      // If the input is a tensor image, all coordinates are written (should match).
};
*/

} // namespace

void FillCoordinates( Image& /*out*/, StringSet const& /*mode*/ ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

} // namespace dip
