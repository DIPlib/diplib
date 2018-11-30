/*
 * DIPimage 3.0
 *
 * (c)2017-2018, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

#include "dip_matlab_interface.h"

#include "diplib/morphology.h"
#include "diplib/binary.h"

namespace {

using BasicFilterFunction = void ( * )( dip::Image const&, dip::Image&, dip::StructuringElement const&, dip::StringArray const& );
void BasicFilter( BasicFilterFunction function, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   int index = 1;
   auto se = dml::GetKernel< dip::StructuringElement >( nrhs, prhs, index, in.Dimensionality() );
   dip::StringArray bc = nrhs > index ? dml::GetStringArray( prhs[ index ] ) : dip::StringArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   function( in, out, se, bc );
   plhs[ 0 ] = dml::GetArray( out );
}

void areaopening( mxArray* plhs[], int nrhs, const mxArray* prhs[], char const* polarity ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint filterSize = nrhs > 1 ? dml::GetUnsigned( prhs[ 1 ] ) : 50;
   dip::uint connectivity = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 1;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::AreaOpening( in, {}, out, filterSize, connectivity, polarity );
   plhs[ 0 ] = dml::GetArray( out );
}

void asf( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 6 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Range sizes = nrhs > 1 ? dml::GetRange( prhs[ 1 ] ) : dip::Range{ 3, 7, 2 };
   dip::String shape = nrhs > 2 ? dml::GetString( prhs[ 2 ] ) : dip::S::ELLIPTIC;
   dip::String mode = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::S::STRUCTURAL;
   dip::String polarity = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::S::OPENCLOSE;
   dip::StringArray bc = nrhs > 5 ? dml::GetStringArray( prhs[ 5 ] ) : dip::StringArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::AlternatingSequentialFilter( in, out, sizes, shape, mode, polarity, bc );
   plhs[ 0 ] = dml::GetArray( out );
}

void hitmiss( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const se0 = dml::GetImage( prhs[ 1 ] );
   dip::Image se1, se2;
   int index = 2;
   if(( nrhs > index ) && !mxIsChar( prhs[ index ] )) {
      se1 = se0;
      se2 = dml::GetImage( prhs[ index ] );
      ++index;
   } else {
      se1 = se0 == 1;
      se2 = se0 == 0;
   }
   DML_MAX_ARGS( index + 2 );
   dip::String mode = nrhs > index ? dml::GetString( prhs[ index ] ) : dip::S::UNCONSTRAINED;
   dip::StringArray bc = nrhs > index + 1 ? dml::GetStringArray( prhs[ index + 1 ] ) : dip::StringArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::HitAndMiss( in, out, se1, se2, mode, bc );
   plhs[ 0 ] = dml::GetArray( out );
}

using ScalarParamFilterFunction = void ( * )( dip::Image const&, dip::Image&, dip:: dfloat, dip::uint );
void ScalarParamFilter( ScalarParamFilterFunction function, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::dfloat h = dml::GetFloat( prhs[ 1 ] );
   dip::uint connectivity = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 1;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   function( in, out, h, connectivity );
   plhs[ 0 ] = dml::GetArray( out );
}

void lee( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 6 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   int index = 1;
   auto se = dml::GetKernel< dip::StructuringElement >( nrhs, prhs, index, in.Dimensionality());
   dip::String edgeType = nrhs > index ? dml::GetString( prhs[ index ] ) : dip::S::TEXTURE;
   dip::String sign = nrhs > index + 1 ? dml::GetString( prhs[ index + 1 ] ) : dip::S::UNSIGNED;
   dip::StringArray bc = nrhs > index + 2 ? dml::GetStringArray( prhs[ index + 2 ] ) : dip::StringArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Lee( in, out, se, edgeType, sign, bc );
   plhs[ 0 ] = dml::GetArray( out );
}

using FlagParamFilterFunction = void ( * )( dip::Image const&, dip::Image&, dip::uint, dip::String const& );
void FlagParamFilter( FlagParamFilterFunction function, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint connectivity = nrhs > 1 ? dml::GetUnsigned( prhs[ 1 ] ) : 1;
   dip::String flag = nrhs > 2 ? dml::GetString( prhs[ 2 ] ) : dip::S::BINARY;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   function( in, out, connectivity, flag );
   plhs[ 0 ] = dml::GetArray( out );
}

void pathopening( mxArray* plhs[], int nrhs, const mxArray* prhs[], char const* polarity ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::IntegerArray params = nrhs > 1 ? dml::GetIntegerArray( prhs[ 1 ] ) : dip::IntegerArray{ 7 };
   dip::StringSet mode = nrhs > 2 ? dml::GetStringSet( prhs[ 2 ] ) : dip::StringSet{};
   mode.erase( "normal" ); // Allow this flag, for backwards compatibility.
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   if( params.size() < 2 ) {
      dip::uint length = 7;
      if( !params.empty()) {
         length = dip::clamp_cast< dip::uint >( params[ 0 ] );
      }
      dip::PathOpening( in, {}, out, length, polarity, mode );
   } else {
      dip::DirectedPathOpening( in, {}, out, params, polarity, mode );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

using RankFilterFunction = void ( * )( dip::Image const&, dip::Image&, dip::StructuringElement, dip::uint, dip::StringArray const& );
void RankFilter( RankFilterFunction function, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint rank = nrhs > 1 ? dml::GetUnsigned( prhs[ 1 ] ) : 2;
   int index = 2;
   auto se = dml::GetKernel< dip::StructuringElement >( nrhs, prhs, index, in.Dimensionality());
   dip::StringArray bc = nrhs > index ? dml::GetStringArray( prhs[ index ] ) : dip::StringArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   function( in, out, se, rank, bc );
   plhs[ 0 ] = dml::GetArray( out );
}

void reconstruction( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   dip::Image const marker = dml::GetImage( prhs[ 0 ] );
   dip::Image const in = dml::GetImage( prhs[ 1 ] );
   dip::dfloat maxDistance = 0;
   int index = 2;
   if( nrhs > 3 && dml::IsScalar( prhs[ 3 ] ) && mxIsDouble( prhs[ 3 ] )) {
      // maxDistance is given
      maxDistance = dml::GetFloat( prhs[ index ] );
      ++index;
   }
   DML_MAX_ARGS( index + 2 );
   dip::uint connectivity = nrhs > index ? dml::GetUnsigned( prhs[ index ] ) : 1;
   dip::String flag = nrhs > index + 1 ? dml::GetString( prhs[ index + 1 ] ) : dip::S::DILATION;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   if( maxDistance >= 1 ) {
      dip::LimitedMorphologicalReconstruction( marker, in, out, maxDistance, connectivity, flag );
   } else {
      dip::MorphologicalReconstruction( marker, in, out, connectivity, flag );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void tophat( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 6 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   int index = 1;
   auto se = dml::GetKernel< dip::StructuringElement >( nrhs, prhs, index, in.Dimensionality());
   dip::String edgeType = nrhs > index ? dml::GetString( prhs[ index ] ) : dip::S::TEXTURE;
   dip::String polarity = nrhs > index + 1 ? dml::GetString( prhs[ index + 1 ] ) : dip::S::WHITE;
   dip::StringArray bc = nrhs > index + 2 ? dml::GetStringArray( prhs[ index + 2 ] ) : dip::StringArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Tophat( in, out, se, edgeType, polarity, bc );
   plhs[ 0 ] = dml::GetArray( out );
}

void waterseed( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 6 );
   dip::Image const seeds = dml::GetImage( prhs[ 0 ] );
   dip::Image const in = dml::GetImage( prhs[ 1 ] );
   dip::uint connectivity = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 1;
   dip::dfloat maxDepth = nrhs > 3 ? dml::GetFloat( prhs[ 3 ] ) : 0.0;
   dip::uint maxSize = nrhs > 4 ? dml::GetUnsigned( prhs[ 4 ] ) : 0;
   dip::StringSet flags = nrhs > 5 ? dml::GetStringSet( prhs[ 5 ] ) : dip::StringSet{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::SeededWatershed( in, seeds, {}, out, connectivity, maxDepth, maxSize, flags );
   plhs[ 0 ] = dml::GetArray( out );
}

void watershed( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint connectivity = nrhs > 1 ? dml::GetUnsigned( prhs[ 1 ] ) : 1;
   dip::dfloat maxDepth = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 0.0;
   dip::uint maxSize = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 0;
   dip::StringSet flags = nrhs > 4 ? dml::GetStringSet( prhs[ 4 ] ) : dip::StringSet{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Watershed( in, {}, out, connectivity, maxDepth, maxSize, flags );
   plhs[ 0 ] = dml::GetArray( out );
}

dip::String GetEdgeCondition( int index, int nrhs, const mxArray* prhs[], char const* defaultValue ) {
   dip::String edgeCondition = defaultValue;
   if( nrhs > index ) {
      if( mxIsChar( prhs[ index ] )) {
         edgeCondition = dml::GetString( prhs[ index ] );
      } else {
         if( dml::GetBoolean( prhs[ index ] )) {
            edgeCondition = dip::S::OBJECT;
         } else {
            edgeCondition = dip::S::BACKGROUND;
         }
      }
   }
   return edgeCondition;
}

using BinaryBasicFilterFunction = void ( * )( dip::Image const&, dip::Image&, dip::sint, dip::uint, dip::String const& );
void BinaryBasicFilter( BinaryBasicFilterFunction function, mxArray* plhs[], int nrhs, const mxArray* prhs[], char const* defaultValue ) {
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint iterations = nrhs > 1 ? dml::GetUnsigned( prhs[ 1 ] ) : 1;
   dip::sint connectivity = nrhs > 2 ? dml::GetInteger( prhs[ 2 ] ) : -1;
   dip::String edgeCondition = GetEdgeCondition( 3, nrhs, prhs, defaultValue );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   function( in, out, connectivity, iterations, edgeCondition );
   plhs[ 0 ] = dml::GetArray( out );

}

void bpropagation( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 5 );
   dml::streambuf b;
   dip::Image const inSeed = dml::GetImage( prhs[ 0 ] );
   dip::Image const inMask = dml::GetImage( prhs[ 1 ] );
   dip::uint iterations = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 0;
   dip::sint connectivity = nrhs > 3 ? dml::GetInteger( prhs[ 3 ] ) : -1;
   dip::String edgeCondition = GetEdgeCondition( 4, nrhs, prhs, dip::S::OBJECT );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::BinaryPropagation( inSeed, inMask, out, connectivity, iterations, edgeCondition );
   plhs[ 0 ] = dml::GetArray( out );
}

void bskeleton( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::String edgeCondition = GetEdgeCondition( 1, nrhs, prhs, dip::S::BACKGROUND );
   dip::String endPixelCondition = dip::S::NATURAL;
   if( nrhs > 2 ) {
      endPixelCondition = dml::GetString( prhs[ 2 ] );
      if( endPixelCondition == "looseendsaway" ) {
         endPixelCondition = dip::S::LOOSE_ENDS_AWAY;
      } else if( endPixelCondition == "1neighbor" ) {
         endPixelCondition = dip::S::ONE_NEIGHBOR;
      } else if( endPixelCondition == "2neighbors" ) {
         endPixelCondition = dip::S::TWO_NEIGHBORS;
      } else if( endPixelCondition == "3neighbors" ) {
         endPixelCondition = dip::S::THREE_NEIGHBORS;
      }
   }
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::EuclideanSkeleton( in, out, endPixelCondition, edgeCondition );
   plhs[ 0 ] = dml::GetArray( out );
}

void countneighbors( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::String mode = dip::S::FOREGROUND;
   if( nrhs > 1 ) { // Note order difference between MATLAB and C++
      if( mxIsChar( prhs[ 1 ] )) {
         mode = dml::GetString( prhs[ 1 ] );
      } else {
         if( !dml::GetBoolean( prhs[ 1 ] )) {
            mode = dip::S::ALL;
         }
      }
   }
   dip::uint connectivity = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 0;
   dip::String edgeCondition = GetEdgeCondition( 3, nrhs, prhs, dip::S::BACKGROUND );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::CountNeighbors( in, out, connectivity, mode, edgeCondition );
   plhs[ 0 ] = dml::GetArray( out );
}

} // namespace

// Gateway function

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 2 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      prhs += 1;
      nrhs -= 1;

      if( function == "areaopening" ) {
         areaopening( plhs, nrhs, prhs, dip::S::OPENING );
      } else if( function == "areaclosing" ) {
         areaopening( plhs, nrhs, prhs, dip::S::CLOSING );
      } else if( function == "asf" ) {
         asf( plhs, nrhs, prhs );
      } else if( function == "closing" ) {
         BasicFilter( dip::Closing, plhs, nrhs, prhs );
      } else if( function == "dilation" ) {
         BasicFilter( dip::Dilation, plhs, nrhs, prhs );
      } else if( function == "erosion" ) {
         BasicFilter( dip::Erosion, plhs, nrhs, prhs );
      } else if( function == "hitmiss" ) {
         hitmiss( plhs, nrhs, prhs );
      } else if( function == "hmaxima" ) {
         ScalarParamFilter( dip::HMaxima, plhs, nrhs, prhs );
      } else if( function == "hminima" ) {
         ScalarParamFilter( dip::HMinima, plhs, nrhs, prhs );
      } else if( function == "lee" ) {
         lee( plhs, nrhs, prhs );
      } else if( function == "maxima" ) {
         FlagParamFilter( dip::Maxima, plhs, nrhs, prhs );
      } else if( function == "minima" ) {
         FlagParamFilter( dip::Minima, plhs, nrhs, prhs );
      } else if( function == "opening" ) {
         BasicFilter( dip::Opening, plhs, nrhs, prhs );
      } else if( function == "pathopening" ) {
         pathopening( plhs, nrhs, prhs, dip::S::OPENING );
      } else if( function == "pathclosing" ) {
         pathopening( plhs, nrhs, prhs, dip::S::CLOSING );
      } else if( function == "rankmax_opening" ) {
         RankFilter( dip::RankMaxOpening, plhs, nrhs, prhs );
      } else if( function == "rankmin_closing" ) {
         RankFilter( dip::RankMinClosing, plhs, nrhs, prhs );
      } else if( function == "reconstruction" ) {
         reconstruction( plhs, nrhs, prhs );
      } else if( function == "tophat" ) {
         tophat( plhs, nrhs, prhs );
      } else if( function == "waterseed" ) {
         waterseed( plhs, nrhs, prhs );
      } else if( function == "watershed" ) {
         watershed( plhs, nrhs, prhs );

      } else if( function == "bclosing" ) {
         BinaryBasicFilter( dip::BinaryClosing, plhs, nrhs, prhs, dip::S::SPECIAL );
      } else if( function == "bdilation" ) {
         BinaryBasicFilter( dip::BinaryDilation, plhs, nrhs, prhs, dip::S::BACKGROUND );
      } else if( function == "berosion" ) {
         BinaryBasicFilter( dip::BinaryErosion, plhs, nrhs, prhs, dip::S::OBJECT );
      } else if( function == "bopening" ) {
         BinaryBasicFilter( dip::BinaryOpening, plhs, nrhs, prhs, dip::S::SPECIAL );
      } else if( function == "bpropagation" ) {
         bpropagation( plhs, nrhs, prhs );
      } else if( function == "bskeleton" ) {
         bskeleton( plhs, nrhs, prhs );
      } else if( function == "countneighbors" ) {
         countneighbors( plhs, nrhs, prhs );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

   } DML_CATCH
}
