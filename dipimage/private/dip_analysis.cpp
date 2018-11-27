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

#include "diplib/analysis.h"
#include "diplib/distance.h"

namespace {

void chordlength( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const mask = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image{};
   dip::uint probes = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 100000;
   dip::uint length = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 100;
   dip::String estimator = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::S::RANDOM;
   dip::Distribution out = dip::ChordLength( in, mask, probes, length, estimator );
   plhs[ 0 ] = dml::GetArray( out );
}

void distancedistribution( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 5 );
   dip::Image const object = dml::GetImage( prhs[ 0 ] );
   dip::Image const region = dml::GetImage( prhs[ 1 ] );
   dip::uint length = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 100;
   dip::Distribution out = dip::DistanceDistribution( object, region, length );
   plhs[ 0 ] = dml::GetArray( out );
}

dip::String GetPolarity( const mxArray* mx ) {
   dip::String polarity = dml::GetString( mx );
   if( polarity == "dark" ) {
      polarity = dip::S::CLOSING;
   } else if( polarity == "light" ) {
      polarity = dip::S::OPENING;
   }
   return polarity;
}

void granulometry( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image mask;
   std::vector< dip::dfloat > scales;
   dip::String type = "isotropic";
   dip::String polarity = dip::S::OPENING;
   dip::StringSet options;
   if(( nrhs > 1) && !mxIsEmpty( prhs[ 1 ] ) && mxIsDouble( prhs[ 1 ] ) && dml::IsVector( prhs[ 1 ] )) {
      // Old-style params: in,scales,minimumFilterSize,maximumFilterSize,minimumZoom,maximumZoom,options,polarity
      DML_MAX_ARGS( 8 );
      scales = dml::GetStdVectorOfFloats( prhs[ 1 ] );
      if( nrhs > 6 ) {
         options = dml::GetStringSet( prhs[ 6 ] ); // this one first, so we can add to it later.
      }
      // Ignore parameters 2 and 3.
      if( nrhs > 4 ) {
         dip::dfloat minimumZoom = dml::GetFloat( prhs[ 4 ] );
         if( minimumZoom != 1 ) {
            options.insert( "subsample" );
         }
      }
      if( nrhs > 5 ) {
         dip::dfloat maximumZoom = dml::GetFloat( prhs[ 5 ] );
         if( maximumZoom != 1 ) {
            options.insert( "interpolate" );
         }
      }
      polarity = nrhs > 7 ? GetPolarity( prhs[ 7 ] ) : dip::S::CLOSING;
   } else {
      // New-style params: in,mask,scales,type,polarity,options
      DML_MAX_ARGS( 6 );
      if( nrhs > 1 ) { mask = dml::GetImage( prhs[ 1 ] ); }
      if( nrhs > 2 ) { scales = dml::GetStdVectorOfFloats( prhs[ 2 ] ); }
      if( nrhs > 3 ) { type = dml::GetString( prhs[ 3 ] ); }
      if( nrhs > 4 ) { polarity = GetPolarity( prhs[ 4 ] ); }
      if( nrhs > 5 ) { options = dml::GetStringSet( prhs[ 5 ] ); }
   }
   dip::Distribution out = dip::Granulometry( in, mask, scales, type, polarity, options );
   plhs[ 0 ] = dml::GetArray( out );
}

void paircorrelation( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 7 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const mask = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image{};
   dip::uint probes = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 1000000;
   dip::uint length = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 100;
   dip::String estimator = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::S::RANDOM;
   dip::StringSet options;
   if( nrhs > 5 ) {
      if( mxIsCell( prhs[ 5 ] )) {
         options = dml::GetStringSet( prhs[ 5 ] );
         DML_MAX_ARGS( 6 );
      } else {
         if( dml::GetBoolean( prhs[ 5 ] )) {
            options.emplace( "covariance" );
         }
         if( nrhs > 6 ) {
            dip::String normalisation = dml::GetString( prhs[ 6 ] );
            if( normalisation != "none" ) {
               options.insert( normalisation );
            }
         }
      }
   }
   dip::Distribution out = in.DataType().IsFloat()
                           ? dip::ProbabilisticPairCorrelation( in, mask, probes, length, estimator, options )
                           : dip::PairCorrelation( in, mask, probes, length, estimator, options );
   plhs[ 0 ] = dml::GetArray( out );
}

void perobjecthist( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 5 );
   dip::Image const grey = dml::GetImage( prhs[ 0 ] );
   dip::Image const labels = dml::GetImage( prhs[ 1 ] );
   dip::Histogram::Configuration conf;
   if( nrhs > 2 ) {
      conf = dml::GetHistogramConfiguration( prhs[ 2 ] );
   } else {
      // Default configuration
      conf = { 0.0, 100.0, 100 };
      conf.lowerIsPercentile = true;
      conf.upperIsPercentile = true;
   }
   dip::String mode = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::S::FRACTION;
   dip::String background = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::S::EXCLUDE;
   dip::Distribution out = dip::PerObjectHistogram( grey, labels, {}, conf, mode, background );
   plhs[ 0 ] = dml::GetArray( out );
}

void semivariogram( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const mask = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image{};
   dip::uint probes = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 1000000;
   dip::uint length = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 100;
   dip::String estimator = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::S::RANDOM;
   dip::Distribution out = dip::Semivariogram( in, mask, probes, length, estimator );
   plhs[ 0 ] = dml::GetArray( out );
}

void monogenicsignal( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 9 );
   dip::uint nOut = static_cast< dip::uint >( nlhs );
   if( nOut == 0 ) {
      nOut = 1;
   }
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::FloatArray wavelengths = nrhs > 1 ? dml::GetFloatArray( prhs[ 1 ] ) : dip::FloatArray{ 3.0, 24.0 };
   dip::dfloat bandwidth = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 0.41;
   dip::StringArray outputs = nrhs > 3 ? dml::GetStringArray( prhs[ 3 ] ) : dip::StringArray{};
   dip::dfloat noiseThreshold =  nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : 0.2;
   dip::dfloat frequencySpreadThreshold =  nrhs > 5 ? dml::GetFloat( prhs[ 5 ] ) : 0.5;
   dip::dfloat sigmoidParameter =  nrhs > 6 ? dml::GetFloat( prhs[ 6 ] ) : 10.0;
   dip::dfloat deviationGain =  nrhs > 7 ? dml::GetFloat( prhs[ 7 ] ) : 1.5;
   dip::String polarity =  nrhs > 8 ? dml::GetString( prhs[ 8 ] ) : dip::S::BOTH;
   // Check outputs
   if( outputs.empty() ) {
      DIP_THROW_IF( nOut > 1, "Too many output arguments" );
   } else {
      DIP_THROW_IF( nOut != outputs.size(), "Number of selected output images does not match number of output arguments" );
   }
   DIP_THROW_IF( !outputs.empty() && wavelengths.size() < 2, "nFrequencyScales must be at least 2 to compute phase congruency or symmetry" );
   // Compute monogenic signal
   dml::MatlabInterface mi;
   dip::Image ms = mi.NewImage();
   dip::MonogenicSignal( in, ms, wavelengths, bandwidth, dip::S::SPATIAL, dip::S::SPATIAL);
   if( outputs.empty() ) {
      // If no outputs were requested, just return the structure tensor itself
      plhs[ 0 ] = dml::GetArray( ms );
   } else {
      // Otherwise, compute requested outputs
      dip::ImageArray outar( nOut, mi.NewImage() );
      dip::ImageRefArray out = dip::CreateImageRefArray( outar );
      dip::MonogenicSignalAnalysis( ms, out, outputs, noiseThreshold, frequencySpreadThreshold, sigmoidParameter, deviationGain, polarity );
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         if( outar[ ii ].IsForged() ) {
            plhs[ ii ] = dml::GetArray( outar[ ii ] );
         }
      }
   }
}

void structuretensor( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 7 );
   dip::uint nOut = static_cast< dip::uint >( nlhs );
   if( nOut == 0 ) {
      nOut = 1;
   }
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::FloatArray gradientSigmas = nrhs > 1 ? dml::GetFloatArray( prhs[ 1 ] ) : dip::FloatArray{ 1.0 };
   dip::FloatArray tensorSigmas = nrhs > 2 ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 5.0 };
   dip::StringArray outputs = nrhs > 3 ? dml::GetStringArray( prhs[ 3 ] ) : dip::StringArray{};
   dip::String method = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::S::BEST;
   dip::StringArray bc = nrhs > 5 ? dml::GetStringArray( prhs[ 5 ] ) : dip::StringArray{};
   dip::dfloat truncation = nrhs > 6 ? dml::GetFloat( prhs[ 6 ] ) : 3.0;
   // Check outputs
   if( outputs.empty() ) {
      DIP_THROW_IF( nOut > 1, "Too many output arguments" );
   } else {
      DIP_THROW_IF( nOut != outputs.size(), "Number of selected output images does not match number of output arguments" );
   }
   // Compute structure tensor
   dml::MatlabInterface mi;
   dip::Image st = mi.NewImage();
   dip::StructureTensor( in, {}, st, gradientSigmas, tensorSigmas, method, bc, truncation );
   if( outputs.empty() ) {
      // If no outputs were requested, just return the structure tensor itself
      plhs[ 0 ] = dml::GetArray( st );
   } else {
      // Otherwise, compute requested outputs
      dip::ImageArray outar( nOut, mi.NewImage() );
      dip::ImageRefArray out = dip::CreateImageRefArray( outar );
      dip::StructureTensorAnalysis( st, out, outputs );
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         plhs[ ii ] = dml::GetArray( out[ ii ] );
      }
   }
}

using EDTFunction = void ( * )( dip::Image const&, dip::Image&, dip::String const&, dip::String const& );
void EDT( EDTFunction function, mxArray* plhs[], int nrhs, const mxArray* prhs[], char const* defaultMethod ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::String edgeCondition = dip::S::OBJECT;
   if( nrhs > 1 ) {
      if( mxIsChar( prhs[ 1 ] )) {
         edgeCondition = dml::GetString( prhs[ 1 ] );
      } else {
         if( !dml::GetBoolean( prhs[ 1 ] )) {
            edgeCondition = dip::S::BACKGROUND;
         }
      }
   }
   dip::String method = defaultMethod;
   if( nrhs > 2 ) {
      method = dml::GetString( prhs[ 2 ] );
      if( method == "bruteforce" ) {
         method = dip::S::BRUTE_FORCE;
      }
   }
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   function( in, out, edgeCondition, method );
   plhs[ 0 ] = dml::GetArray( out );
}

void gdt( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const grey = dml::GetImage( prhs[ 1 ] );
   dip::Metric metric( dip::S::CHAMFER, 1 );
   if( nrhs > 2 ) {
      dip::uint chamfer = dml::GetUnsigned( prhs[ 2 ] );
      switch( chamfer ) {
         case 1:
            metric = dip::Metric( dip::S::CONNECTED, 1 );
            break;
         case 3:
            break;
         case 5:
            metric = dip::Metric( dip::S::CHAMFER, 2 );
            break;
         default:
            DIP_THROW( dip::E::INVALID_PARAMETER );
      }
   }
   dip::String outputMode = nlhs > 1 ? dip::S::BOTH : dip::S::GDT;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::GreyWeightedDistanceTransform( grey, in, {}, out, metric, outputMode );
   if( nlhs > 1 ) {
      plhs[ 0 ] = dml::GetArray( out[ 0 ] );
      plhs[ 1 ] = dml::GetArray( out[ 1 ] );
   } else {
      plhs[ 0 ] = dml::GetArray( out );
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

      if( function == "chordlength" ) {
         chordlength( plhs, nrhs, prhs );
      } else if( function == "distancedistribution" ) {
         distancedistribution( plhs, nrhs, prhs );
      } else if( function == "granulometry" ) {
         granulometry( plhs, nrhs, prhs );
      } else if( function == "paircorrelation" ) {
         paircorrelation( plhs, nrhs, prhs );
      } else if( function == "perobjecthist" ) {
         perobjecthist( plhs, nrhs, prhs );
      } else if( function == "semivariogram" ) {
         semivariogram( plhs, nrhs, prhs );

      } else if( function == "monogenicsignal" ) {
         monogenicsignal( nlhs, plhs, nrhs, prhs );
      } else if( function == "structuretensor" ) {
         structuretensor( nlhs, plhs, nrhs, prhs );

      } else if( function == "dt" ) {
         EDT( dip::EuclideanDistanceTransform, plhs, nrhs, prhs, dip::S::SEPARABLE );
      } else if( function == "gdt" ) {
         gdt( nlhs, plhs, nrhs, prhs );
      } else if( function == "vdt" ) {
         EDT( dip::VectorDistanceTransform, plhs, nrhs, prhs, dip::S::FAST );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

   } DML_CATCH
}
