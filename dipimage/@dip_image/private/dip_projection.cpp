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
#include "diplib/statistics.h"
#include "diplib/math.h"

namespace {

struct MaskAndProcessArray{
   dip::Image mask;
   dip::BooleanArray process;
   bool hasProcess = false;
};
MaskAndProcessArray GetMaskAndProcessArray( int nrhs, const mxArray* prhs[], dip::uint nDims ) {
   MaskAndProcessArray out;
   out.mask = nrhs > 0 ? dml::GetImage( prhs[ 0 ] ) : dip::Image{};
   if( nrhs > 1 ) {
      out.process = dml::GetProcessArray( prhs[ 1 ], nDims );
      out.hasProcess = true;
   }
   return out;
}

using TensorProjectionFunction = void (*)( dip::Image const&, dip::Image& );
void TensorProjection( TensorProjectionFunction function, dip::Image const& in, mxArray** plhs ) {
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   function( in, out );
   *plhs = dml::GetArray( out );
}

using BasicProjectionFunction = void (*)( dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& );
void BasicProjection(
      BasicProjectionFunction function,
      dip::Image const& in,
      mxArray** plhs, int nrhs, const mxArray* prhs[]
) {
   DML_MAX_ARGS( 2 );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   MaskAndProcessArray mapa = GetMaskAndProcessArray( nrhs, prhs, in.Dimensionality() );
   function( in, mapa.mask, out, mapa.process );
   if( mapa.hasProcess ) {
      *plhs = dml::GetArray( out );
   } else {
      *plhs = dml::GetArray( out.At( 0 ));
   }
}

using ProjectionWithModeFunction = void (*)( dip::Image const&, dip::Image const&, dip::Image&, dip::String const&, dip::BooleanArray const& );
void ProjectionWithMode(
      ProjectionWithModeFunction function,
      dip::Image const& in,
      mxArray** plhs, int nrhs, const mxArray* prhs[],
      dip::String const& defaultMode
) {
   dip::String mode = defaultMode;
   if(( nrhs > 0 ) && mxIsChar( prhs[ nrhs - 1 ] )) {
      mode = dml::GetString( prhs[ nrhs - 1 ] );
      --nrhs;
   }
   DML_MAX_ARGS( 2 );
   MaskAndProcessArray mapa = GetMaskAndProcessArray( nrhs, prhs, in.Dimensionality() );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   function( in, mapa.mask, out, mode, mapa.process );
   if( mapa.hasProcess ) {
      *plhs = dml::GetArray( out );
   } else {
      *plhs = dml::GetArray( out.At( 0 ));
   }
}

using MaxMinProjectionFunction = void (*)( dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& );
using MaxMinPixelFunction = dip::UnsignedArray (*)( dip::Image const&, dip::Image const&, dip::String const& );
using MaxMinPositionProjectionFunction = void (*)( dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::String const& );
using MaxMinFunction = void (*)( dip::Image const&, dip::Image const&, dip::Image& );
void MaxMinProjection(
      MaxMinProjectionFunction projection,
      MaxMinPixelFunction projectionPixel,
      MaxMinPositionProjectionFunction positionProjection,
      MaxMinFunction function,
      dip::Image const& in,
      int nlhs, mxArray* plhs[],
      int nrhs, const mxArray* prhs[]
) {
   DML_MAX_ARGS( 2 );
   MaskAndProcessArray mapa = GetMaskAndProcessArray( nrhs, prhs, in.Dimensionality() );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   if( !mapa.mask.IsForged() || mapa.mask.DataType().IsBinary() ) {
      // Maximum pixel projection
      projection( in, mapa.mask, out, mapa.process );
      if( mapa.hasProcess ) {
         plhs[ 0 ] = dml::GetArray( out );
      } else {
         plhs[ 0 ] = dml::GetArray( out.At( 0 ));
      }
      if( nlhs > 1 ) {
         // Compute position also
         dip::uint k = mapa.process.count();
         if( !mapa.hasProcess || ( k == in.Dimensionality() )) {
            plhs[ 1 ] = dml::GetArray( projectionPixel( in, mapa.mask, dip::S::FIRST ));
         } else if( k == 1 ) {
            dip::Image out2 = mi.NewImage();
            positionProjection( in, mapa.mask, out2, mapa.process.find( true ), dip::S::FIRST );
            plhs[ 1 ] = dml::GetArray( out2 );
         } else {
            DIP_THROW( "Cannot produce position value for more than one dimension" );
         }
      }
   } else {
      // Maximum over two images
      function( in, mapa.mask, out );
      plhs[ 0 ] = dml::GetArray( out );
   }
}

void PercentileProjectionTensor( dip::Image& in, int nlhs, mxArray** plhs, int nrhs, mxArray const** prhs ) {
   DML_MIN_ARGS( 1 );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::dfloat percentile = dml::GetFloat( prhs[ 0 ] );
   dip::uint nDims = in.Dimensionality();
   in.TensorToSpatial( nDims );
   dip::BooleanArray process( nDims + 1, false );
   process[ nDims ] = true;
   dip::Percentile( in, {}, out, percentile, process );
   out.Squeeze( nDims );
   plhs[ 0 ] = dml::GetArray( out );
}

void PercentileProjection( dip::Image& in, int nlhs, mxArray** plhs, int nrhs, mxArray const** prhs ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 3 );
   dip::dfloat percentile = dml::GetFloat( prhs[ 0 ] );
   MaskAndProcessArray mapa = GetMaskAndProcessArray( nrhs - 1, prhs + 1, in.Dimensionality() );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Percentile( in, mapa.mask, out, percentile, mapa.process );
   if( mapa.hasProcess ) {
      plhs[ 0 ] = dml::GetArray( out );
   } else {
      plhs[ 0 ] = dml::GetArray( out.At( 0 ));
   }
   if( nlhs == 2 ) { // Output position as well
      DIP_THROW_IF( mapa.process.count() != 1, "Cannot produce position value for more than one dimension" );
      dip::Image out2 = mi.NewImage();
      dip::PositionPercentile( in, mapa.mask, out2, percentile, mapa.process.find( true ));
      plhs[ 1 ] = dml::GetArray( out2 );
   }
}

} // namespace

// Gateway function

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 2 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      dip::Image in = dml::GetImage( prhs[ 1 ] );
      prhs += 2;
      nrhs -= 2;
      bool doTensor = false;
      if(( nrhs > 0 ) && mxIsChar( prhs[ nrhs - 1 ] )) {
         dip::String flag = dml::GetString( prhs[ 0 ] );
         doTensor = flag == "tensor";
         DIP_THROW_IF( nrhs > ( function == "percentile" ? 2 : 1 ), "Too many input arguments" );
      }
      if( function == "all" ) {
         if( doTensor ) {
            TensorProjection( dip::AllTensorElements, in, plhs );
         } else {
            BasicProjection( dip::All, in, plhs, nrhs, prhs );
         }
      } else if( function == "any" ) {
         if( doTensor ) {
            TensorProjection( dip::AnyTensorElement, in, plhs );
         } else {
            BasicProjection( dip::Any, in, plhs, nrhs, prhs );
         }
      } else if( function == "max" ) {
         if( doTensor ) {
            TensorProjection( dip::MaximumTensorElement, in, plhs );
         } else {
            MaxMinProjection( dip::Maximum, dip::MaximumPixel, dip::PositionMaximum, dip::Supremum, in, nlhs, plhs, nrhs, prhs );
         }
      } else if( function == "mean" ) {
         if( doTensor ) {
            TensorProjection( dip::MeanTensorElement, in, plhs );
         } else {
            ProjectionWithMode( dip::Mean, in, plhs, nrhs, prhs, "" );
         }
      } else if( function == "min" ) {
         if( doTensor ) {
            TensorProjection( dip::MinimumTensorElement, in, plhs );
         } else {
            MaxMinProjection( dip::Minimum, dip::MinimumPixel, dip::PositionMinimum, dip::Infimum, in, nlhs, plhs, nrhs, prhs );
         }
      } else if( function == "percentile" ) {
         if( doTensor ) {
            PercentileProjectionTensor( in, nlhs, plhs, nrhs, prhs );
         } else {
            PercentileProjection( in, nlhs, plhs, nrhs, prhs );
         }
      } else if( function == "prod" ) {
         if( doTensor ) {
            TensorProjection( dip::ProductTensorElements, in, plhs );
         } else {
            BasicProjection( dip::Product, in, plhs, nrhs, prhs );
         }
      } else if( function == "std" ) {
         if( doTensor ) {
            DIP_THROW_INVALID_FLAG( "tensor" );
         }
         ProjectionWithMode(
               []( dip::Image const& in, dip::Image const& mask, dip::Image& out, dip::String const& mode, dip::BooleanArray const& process ) {
                  dip::StandardDeviation( in, mask, out, mode, process );
               }, in, plhs, nrhs, prhs, dip::S::FAST );
      } else if( function == "sum" ) {
         if( doTensor ) {
            TensorProjection( dip::SumTensorElements, in, plhs );
         } else {
            BasicProjection( dip::Sum, in, plhs, nrhs, prhs );
         }
      } else if( function == "var" ) {
         if( doTensor ) {
            DIP_THROW_INVALID_FLAG( "tensor" );
         }
         ProjectionWithMode(
               []( dip::Image const& in, dip::Image const& mask, dip::Image& out, dip::String const& mode, dip::BooleanArray const& process ) {
                  dip::Variance( in, mask, out, mode, process );
               }, in, plhs, nrhs, prhs, dip::S::FAST );
      } else {
         DIP_THROW_INVALID_FLAG( function );
      }
   } DML_CATCH
}
