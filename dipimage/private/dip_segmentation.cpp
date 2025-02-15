/*
 * (c)2017-2024, Cris Luengo.
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

#include <limits>

#include "dip_matlab_interface.h"
#include "diplib/analysis.h"
#include "diplib/chain_code.h"
#include "diplib/detection.h"
#include "diplib/polygon.h"
#include "diplib/regions.h"
#include "diplib/segmentation.h"
#include "diplib/statistics.h"

namespace {

void label( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint connectivity = nrhs > 1 ? dml::GetUnsigned( prhs[ 1 ] ) : in.Dimensionality();
   dip::uint minSize = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 0;
   dip::uint maxSize = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 0;
   dip::StringArray boundary = nrhs > 4 ? dml::GetStringArray( prhs[ 4 ] ) : dip::StringArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Label( in, out, connectivity, minSize, maxSize, boundary );
   plhs[ 0 ] = dml::GetArray( out );
}

void growregions( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 4 );
   dip::Image const label = dml::GetImage( prhs[ 0 ] );
   dip::Image const mask = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image{};
   dip::sint connectivity = nrhs > 2 ? dml::GetInteger( prhs[ 2 ] ) : -1;
   dip::uint iterations = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 0;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::GrowRegions( label, mask, out, connectivity, iterations );
   plhs[ 0 ] = dml::GetArray( out );
}

void growregionsweighted( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 5 );
   dip::Image const label = dml::GetImage( prhs[ 0 ] );
   dip::Image const grey = dml::GetImage( prhs[ 1 ] );
   dip::Image const mask = nrhs > 2 ? dml::GetImage( prhs[ 2 ] ) : dip::Image{};
   // dip::uint chamfer = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 5; // chamfer parameter is ignored
   dip::dfloat distance = nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : dip::infinity;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::GrowRegionsWeighted( label, grey, mask, out, distance );
   plhs[ 0 ] = dml::GetArray( out );
}

void smallobjectsremove( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint threshold = nrhs > 1 ? dml::GetUnsigned( prhs[ 1 ] ) : 10;
   dip::uint connectivity = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 1;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::SmallObjectsRemove( in, out, threshold, connectivity );
   plhs[ 0 ] = dml::GetArray( out );
}

inline mxArray* GetArray( dip::Polygon const& in ) {
   dip::uint n = in.vertices.size();
   if( n == 0 ) {
      return mxCreateDoubleMatrix( 0, 0, mxREAL );
   }
   dip::uint ndims = 2;
   mxArray* mx = mxCreateDoubleMatrix( n, ndims, mxREAL );
   double* data = mxGetPr( mx );
   for( auto& v : in.vertices ) {
      data[ 0 ] = v.x;
      data[ n ] = v.y;
      ++data;
   }
   return mx;
}

inline mxArray* GetArray( dip::ChainCode const& in ) {
   dip::uint n = in.codes.size();
   if( n == 0 ) {
      return mxCreateNumericMatrix( 0, 0, mxUINT8_CLASS, mxREAL );
   }
   mxArray* mx = mxCreateNumericMatrix( n, 1, mxUINT8_CLASS, mxREAL );
   dip::uint8* data = static_cast< dip::uint8* >( mxGetData( mx ));
   for( auto v : in.codes ) {
      *data = static_cast< dip::uint8 >( v );
      ++data;
   }
   return mx;
}

void traceobjects( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   std::vector< dip::LabelType > objectIDs = nrhs > 1 ? dml::GetStdVector< dip::LabelType >( prhs[ 1 ] ) : std::vector< dip::LabelType >{};
   dip::uint connectivity = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 2;
   dip::String output = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : "polygon";
   bool computePolygon = true;
   bool computeConvexHull = false;
   bool smoothPolygon = false;
   bool simplifyPolygon = false;
   bool needsParam = false;
   if (output == "chain code") {
      computePolygon = false;
   } else if (output == "polygon") {
      // (the default)
   } else if (output == "convex hull") {
      computeConvexHull = true;
   } else if (output == "smoothed polygon") {
      smoothPolygon = true;
      needsParam = true;
   } else if (output == "simplified polygon") {
      simplifyPolygon = true;
      needsParam = true;
   } else {
      DIP_THROW_INVALID_FLAG( output );
   }
   dip::dfloat param = 1.0;
   if( needsParam ) {
      param = nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : param;
   } else {
      DML_MAX_ARGS( 4 );
   }
   dip::Image const& labels = in.DataType().IsBinary() ? dip::Label( in, connectivity ) : in;
   dip::ChainCodeArray ccs = GetImageChainCodes( labels, objectIDs, connectivity );
   dip::uint n = ccs.size();
   plhs[ 0 ] = mxCreateCellMatrix( n, 1 );
   for( dip::uint ii = 0; ii < n; ++ii ) {
      mxArray* v = nullptr;
      if( computePolygon ) {
         dip::Polygon p = ccs[ ii ].Polygon();
         if( computeConvexHull ) {
            p = p.ConvexHull().Polygon();
         } else if( smoothPolygon ) {
            p.Smooth( param );
         } else if( simplifyPolygon ) {
            p.Simplify( param );
         }
         v = GetArray( p );
      } else {
         v = GetArray( ccs[ ii ] );
      }
      mxSetCell( plhs[ 0 ], ii, v );
   }
}

void cluster( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint nClusters = nrhs > 1 ? dml::GetUnsigned( prhs[ 1 ] ) : 2;
   dip::String method = nrhs > 2 ? dml::GetString( prhs[ 2 ] ) : "minvariance";
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::CoordinateArray coords;
   if( method == "kmeans" ) {
      coords = dip::KMeansClustering( in, out, nClusters );
   } else if( method == "minvariance" ) {
      coords = dip::MinimumVariancePartitioning( in, out, nClusters );
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
   plhs[ 0 ] = dml::GetArray( out );
   if( nlhs > 1 ) {
      plhs[ 1 ] = dml::GetArray( coords );
   }
}

void superpixels( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::dfloat density = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 0.005;
   dip::dfloat compactness = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 1.0;
   dip::String method = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::S::CW;
   dip::StringSet flags = nrhs > 4 ? dml::GetStringSet( prhs[ 4 ] ) : dip::StringSet{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Superpixels( in, out, density, compactness, method, flags );
   plhs[ 0 ] = dml::GetArray( out );
}

void threshold( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image mask;
   int index = 1;
   if(( nrhs > index ) && !dml::IsString( prhs[ index ] )) {
      mask = dml::GetImage( prhs[ index ] );
      ++index;
   }
   dip::String method = nrhs > index ? dml::GetString( prhs[ index ] ) : "isodata";
   ++index;
   DML_MAX_ARGS( index + 1 );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   if(( method == "double" ) || ( method == dip::S::HYSTERESIS )) {
      dip::dfloat param1{};
      dip::dfloat param2{};
      if( nrhs > index ) {
         dip::FloatArray parameter = dml::GetFloatArray( prhs[ index ] );
         DIP_THROW_IF( parameter.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
         param1 = parameter[ 0 ];
         param2 = parameter[ 1 ];
      } else {
         auto lims = dip::MaximumAndMinimum( in, mask );
         dip::dfloat range = lims.Maximum() - lims.Minimum();
         param1 = lims.Minimum() + range / 3.0;
         param2 = lims.Minimum() + 2.0 * range / 3.0;
      }
      if( method == "double" ) {
         dip::RangeThreshold( in, out, param1, param2 );
      } else {
         dip::HysteresisThreshold( in, out, param1, param2 );
      }
      if( nlhs > 1 ) {
         plhs[ 1 ] = dml::CreateDouble2Vector( param1, param2 );
      }
   } else if(( method == dip::S::ISODATA ) || ( method == dip::S::KMEANS ) || ( method == dip::S::GMM )) {
      dip::uint nThresholds = 1;
      if( nrhs > index ) {
         dip::dfloat parameter = dml::GetFloat( prhs[ index ] );
         if(( parameter > 1.0 ) && ( parameter <= std::numeric_limits< dip::uint16 >::max() )) {
            nThresholds = static_cast< dip::uint >( parameter );
         }
      }
      dip::FloatArray thresholds = ( method == dip::S::GMM ) ? GaussianMixtureModelThreshold( in, mask, out, nThresholds )
                                                             : IsodataThreshold( in, mask, out, nThresholds );
      if( nlhs > 1 ) {
         plhs[ 1 ] = dml::GetArray( thresholds );
      }
   } else {
      dip::dfloat parameter = std::numeric_limits< dip::dfloat >::infinity();
      if( nrhs > index ) {
         parameter = dml::GetFloat( prhs[ index ] );
      }
      dip::dfloat threshold = dip::Threshold( in, mask, out, method, parameter );
      if( nlhs > 1 ) {
         plhs[ 1 ] = dml::GetArray( threshold );
      }
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void canny( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::FloatArray sigmas = nrhs > 1 ? dml::GetFloatArray( prhs[ 1 ] ) : dip::FloatArray{ 1 };
   dip::dfloat lower = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 0.5;
   dip::dfloat upper = nrhs > 3 ? dml::GetFloat( prhs[ 3 ] ) : 0.9;
   dip::String selection = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : "all";
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Canny( in, out, sigmas, lower, upper, selection );
   plhs[ 0 ] = dml::GetArray( out );
}

void cornerdetector( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::String method = ( nrhs > 1 ) ? dml::GetString( prhs[ 1 ] ) : "shitomasi";
   dip::ToLowerCase( method );
   dip::FloatArray sigmas = ( nrhs > 2 ) ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 2.0 };
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   if( method == "harris" ) {
      dip::dfloat kappa = ( nrhs > 3 ) ? dml::GetFloat( prhs[ 3 ] ) : 0.04;
      dip::HarrisCornerDetector( in, out, kappa, sigmas );
   } else if( method == "shitomasi" ) {
      dip::ShiTomasiCornerDetector( in, out, sigmas );
   } else if( method == "noble" ) {
      dip::NobleCornerDetector( in, out, sigmas );
   } else if( method == "wangbrady" ) {
      dip::dfloat threshold = ( nrhs > 3 ) ? dml::GetFloat( prhs[ 3 ] ) : 0.1;
      dip::WangBradyCornerDetector( in, out, threshold, sigmas );
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void linedetector( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::String method = ( nrhs > 1 ) ? dml::GetString( prhs[ 1 ] ) : "frangi";
   dip::ToLowerCase( method );
   dip::String polarity = ( nrhs > 4 ) ? dml::GetString( prhs[ 4 ] ) : dip::S::WHITE;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   if( method == "frangi" ) {
      dip::FloatArray sigmas = ( nrhs > 2 ) ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 2.0 };
      dip::FloatArray parameters = ( nrhs > 3 ) ? dml::GetFloatArray( prhs[ 3 ] ) : dip::FloatArray{};
      dip::FrangiVesselness( in, out, sigmas, parameters, polarity );
   } else if( method == "danielsson" ) {
      dip::FloatArray sigmas = ( nrhs > 2 ) ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 2.0 };
      dip::DanielssonLineDetector( in, out, sigmas, polarity );
   } else if( method == "matched" ) {
      dip::dfloat sigma = ( nrhs > 2 ) ? dml::GetFloat( prhs[ 2 ] ) : 2.0;
      dip::dfloat length = ( nrhs > 3 ) ? dml::GetFloat( prhs[ 3 ] ) : 10.0;
      dip::MatchedFiltersLineDetector2D( in, out, sigma, length, polarity );
   } else if( method == "rorpo" ) {
      dip::uint length = ( nrhs > 2 ) ? dml::GetUnsigned( prhs[ 2 ] ) : 15;
      dip::RORPOLineDetector( in, out, length, polarity );
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

mxArray* GetArray( dip::RadonCircleParametersArray const& params ) {
   dip::uint n = params.size();
   if( n == 0 ) {
      return mxCreateDoubleMatrix( 0, 0, mxREAL );
   }
   dip::uint nDims = params[ 0 ].origin.size();
   mxArray* mx = mxCreateDoubleMatrix( n, nDims + 1, mxREAL );
   double* data = mxGetPr( mx );
   for( auto& p : params ) {
      DIP_ASSERT( p.origin.size() == nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         data[ ii * n ] = p.origin[ ii ];
      }
      data[ nDims * n ] = p.radius;
      ++data;
   }
   return mx;
}

void radoncircle( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 6 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Range radii = ( nrhs > 1 ) ? dml::GetRange( prhs[ 1 ] ) : dip::Range{ 10, 30 };
   dip::dfloat sigma = ( nrhs > 2 ) ? dml::GetFloat( prhs[ 2 ] ) : 1.0;
   dip::dfloat threshold = ( nrhs > 3 ) ? dml::GetFloat( prhs[ 3 ] ) : 1.0;
   dip::String mode = ( nrhs > 4 ) ? dml::GetString( prhs[ 4 ] ) : dip::S::FULL;
   dip::StringSet options = ( nrhs > 5 ) ? dml::GetStringSet( prhs[ 5 ] ) : dip::StringSet{  dip::S::NORMALIZE, dip::S::CORRECT };
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::RadonCircleParametersArray params = dip::RadonTransformCircles( in, out, radii, sigma, threshold, mode, options );
   if( nlhs > 1 ) {
      // Return both `out` and `params`
      plhs[ 0 ] = dml::GetArray( out );
      plhs[ 1 ] = GetArray( params );
   } else {
      if( out.IsForged() ) {
         // we're returning `out` only
         plhs[ 0 ] = dml::GetArray( out );
      } else {
         // we're returning `params` only
         plhs[ 0 ] = GetArray( params );
      }
   }
}

using FindExtremaFunction = dip::SubpixelLocationArray ( * )( dip::Image const&, dip::Image const&, dip::String const& );
void FindExtrema( FindExtremaFunction function, int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image mask;
   int index = 1;
   if(( nrhs > index ) && ( !dml::IsString( prhs[ index ] ))) {
      mask = dml::GetImage( prhs[ index ] );
      ++index;
   }
   dip::String method = dip::S::PARABOLIC_SEPARABLE;
   if( nrhs > index ) {
      method = dml::GetString( prhs[ index ] );
      // Method names are different in the DIPimage interface...
      if(( method == "parabolic nonseparable" ) || ( method == "parabolic_nonseparable" )) {
         method = dip::S::PARABOLIC;
      } else if(( method == "gaussian nonseparable" ) || ( method == "gaussian_nonseparable" )) {
         method = dip::S::GAUSSIAN;
      } else if( method == "parabolic" ) {
         method = dip::S::PARABOLIC_SEPARABLE;
      } else if( method == "gaussian" ) {
         method = dip::S::GAUSSIAN_SEPARABLE;
      }
   }
   dip::SubpixelLocationArray out = function( in, mask, method );
   dip::uint N = out.size();
   dip::uint nDims = in.Dimensionality();
   plhs[ 0 ] = mxCreateDoubleMatrix( N, nDims, mxREAL );
   double* data = mxGetPr( plhs[ 0 ] );
   for( auto const& loc : out ) {
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         data[ ii * N ] = loc.coordinates[ ii ];
      }
      ++data;
   }
   if( nlhs > 1 ) {
      plhs[ 1 ] = mxCreateDoubleMatrix( N, 1, mxREAL );
      data = mxGetPr( plhs[ 1 ] );
      for( auto const& loc : out ) {
         *data = loc.value;
         ++data;
      }
   }
}

} // namespace

// Gateway function

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 2 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      prhs += 1;
      nrhs -= 1;

      if( function == "label" ) {
         label( plhs, nrhs, prhs );
      } else if( function == "growregions" ) {
         growregions( plhs, nrhs, prhs );
      } else if( function == "growregionsweighted" ) {
         growregionsweighted( plhs, nrhs, prhs );
      } else if( function == "smallobjectsremove" ) {
         smallobjectsremove( plhs, nrhs, prhs );
      } else if( function == "traceobjects" ) {
         traceobjects( plhs, nrhs, prhs );

      } else if( function == "cluster" ) {
         cluster( nlhs, plhs, nrhs, prhs );
      } else if( function == "superpixels" ) {
         superpixels( plhs, nrhs, prhs );
      } else if( function == "threshold" ) {
         threshold( nlhs, plhs, nrhs, prhs );

      } else if( function == "canny" ) {
         canny( plhs, nrhs, prhs );
      } else if( function == "cornerdetector" ) {
         cornerdetector( plhs, nrhs, prhs );
      } else if( function == "linedetector" ) {
         linedetector( plhs, nrhs, prhs );
      } else if( function == "radoncircle" ) {
         radoncircle( nlhs, plhs, nrhs, prhs );

      } else if( function == "findmaxima" ) {
         FindExtrema( dip::SubpixelMaxima, nlhs, plhs, nrhs, prhs );
      } else if( function == "findminima" ) {
         FindExtrema( dip::SubpixelMinima, nlhs, plhs, nrhs, prhs );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

   } DML_CATCH
}
