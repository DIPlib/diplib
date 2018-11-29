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

#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/mapping.h"
#include "diplib/lookup_table.h"
#include "diplib/display.h"
#include "diplib/histogram.h"

#include "diplib/multithreading.h"

namespace {

void integral_image( mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const mask = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image{};
   dip::BooleanArray process = nrhs > 2 ? dml::GetProcessArray( prhs[ 2 ], in.Dimensionality()) : dip::BooleanArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::CumulativeSum( in, mask, out, process );
   plhs[ 0 ] = dml::GetArray( out );
}

void select( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   if( nrhs == 3 ) {
      dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
      dip::Image const in2 = dml::GetImage( prhs[ 1 ] );
      dip::Image const mask = dml::GetImage( prhs[ 2 ] );
      dip::Select( in1, in2, mask, out );
   } else if( nrhs == 5 ) {
      dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
      dip::Image const in2 = dml::GetImage( prhs[ 1 ] );
      dip::Image const in3 = dml::GetImage( prhs[ 2 ] );
      dip::Image const in4 = dml::GetImage( prhs[ 3 ] );
      dip::String selector = dml::GetString( prhs[ 4 ] );
      if( selector == "~=" ) {
         selector = "!=";
      }
      dip::Select( in1, in2, in3, in4, out, selector );
   } else {
      DIP_THROW( "Need either 3 or 5 input arguments." );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void getmaximumandminimum( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::MinMaxAccumulator minmax;
   if( nrhs > 1 ) {
      dip::Image const mask = dml::GetImage( prhs[ 1 ] );
      minmax = dip::MaximumAndMinimum( in, mask );
   } else {
      minmax = dip::MaximumAndMinimum( in );
   }
   plhs[ 0 ] = mxCreateDoubleMatrix( 1, 2, mxREAL );
   auto data = mxGetPr( plhs[ 0 ] );
   data[ 0 ] = minmax.Minimum();
   data[ 1 ] = minmax.Maximum();
}

void getsamplestatistics( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::StatisticsAccumulator stats;
   if( nrhs > 1 ) {
      dip::Image const mask = dml::GetImage( prhs[ 1 ] );
      stats = dip::SampleStatistics( in, mask );
   } else {
      stats = dip::SampleStatistics( in );
   }
   plhs[ 0 ] = mxCreateDoubleMatrix( 1, 4, mxREAL );
   auto data = mxGetPr( plhs[ 0 ] );
   data[ 0 ] = stats.Mean();
   data[ 1 ] = stats.Variance();
   data[ 2 ] = stats.Skewness();
   data[ 3 ] = stats.ExcessKurtosis();
}

void entropy( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint nBins = nrhs > 1 ? dml::GetUnsigned( prhs[ 1 ] ) : 256;
   plhs[ 0 ] = dml::GetArray( dip::Entropy( in, {}, nBins ));
}

void errormeasure( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const reference = dml::GetImage( prhs[ 1 ] );
   dip::Image const mask = ( nrhs > 2 ) ? dml::GetImage( prhs[ 2 ] ) : dip::Image{};
   dip::String method = ( nrhs > 3 ) ? dml::GetString( prhs[ 3 ] ) : "mse";
   dml::ToLower( method );
   dip::dfloat error;
   if( method == "mse" ) {
      error = dip::MeanSquareError( in, reference, mask );
   } else if( method == "rmse" ) {
      error = dip::RootMeanSquareError( in, reference, mask );
   } else if( method == "me" ) {
      error = dip::MeanError( in, reference, mask );
   } else if( method == "mae" ) {
      error = dip::MeanAbsoluteError( in, reference, mask );
   } else if( method == "idivergence" ) {
      error = dip::IDivergence( in, reference, mask );
   } else if( method == "inproduct" ) {
      error = dip::InProduct( in, reference, mask );
   } else if( method == "lnnormerror" ) {
      error = dip::LnNormError( in, reference, mask );
   } else if( method == "psnr" ) {
      error = dip::PSNR( in, reference, mask );
   } else if( method == "ssim" ) {
      error = dip::SSIM( in, reference, mask );
   } else if( method == "mutualinformation" ) {
      dip::uint nThreads = dip::GetNumberOfThreads();
      dip::SetNumberOfThreads( 1 );                            // Make sure we don't use OpenMP.
      error = dip::MutualInformation( in, reference, mask );   // Can crash if there is more than one tensor dimension when using OpenMP.
      dip::SetNumberOfThreads( nThreads );
   } else if( method == "dice" ) {
      error = dip::DiceCoefficient( in, reference );
   } else if( method == "jaccard" ) {
      error = dip::JaccardIndex( in, reference );
   } else if( method == "specificity" ) {
      error = dip::Specificity( in, reference );
   } else if( method == "sensitivity" ) {
      error = dip::Sensitivity( in, reference );
   } else if( method == "accuracy" ) {
      error = dip::Accuracy( in, reference );
   } else if( method == "precision" ) {
      error = dip::Precision( in, reference );
   } else if( method == "hausdorff" ) {
      error = dip::HausdorffDistance( in, reference );
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
   plhs[ 0 ] = dml::GetArray( error );
}

void noisestd( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const mask = nrhs > 1 ?dml::GetImage( prhs[ 1 ] ) : dip::Image{};
   dip::dfloat res = dip::EstimateNoiseVariance( in, mask );
   res = std::sqrt( res );
   plhs[ 0 ] = dml::GetArray( res );
}

using RadialProjectionFunction = void ( * )( dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::String const&, dip::FloatArray const& );
void RadialProjection( RadialProjectionFunction function, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   int arg = 1;
   dip::Image mask;
   if( nrhs > arg ) {
      if( !mxIsNumeric( prhs[ arg ] ) || !dml::IsScalar( prhs[ arg ] )) {
         // It seems we might have a mask image as 2nd input argument
         mask = dml::GetImage( prhs[ arg ] );
         ++arg;
      }
   }
   dip::dfloat binSize = nrhs > arg ? dml::GetFloat( prhs[ arg ] ) : 1;
   dip::String maxRadius = dip::S::OUTERRADIUS;
   if( nrhs > arg + 1 ) {
      if( mxIsNumeric( prhs[ arg + 1 ] ) && dml::IsScalar( prhs[ arg + 1 ] )) {
         maxRadius = dml::GetBoolean( prhs[ arg + 1 ] ) ? dip::S::INNERRADIUS : dip::S::OUTERRADIUS;
      } else {
         maxRadius = dml::GetString( prhs[ arg + 1 ] );
      }
   }
   dip::FloatArray center = {};
   if( nrhs > arg + 2 ) {
      if( mxIsChar( prhs[ arg + 2 ] )) {
         center = in.GetCenter( dml::GetString( prhs[ arg + 2 ] ) );
      } else {
         center = dml::GetFloatArray( prhs[ arg + 2 ] );
      }
   }
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   function( in, mask, out, binSize, maxRadius, center );
   plhs[ 0 ] = dml::GetArray( out );
}

void clip( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::dfloat low = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 0.0;
   dip::dfloat high = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 255.0;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Clip( in, out, low, high, dip::S::BOTH );
   plhs[ 0 ] = dml::GetArray( out );
}

void erfclip( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::dfloat threshold = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 128.0;
   dip::dfloat range = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 64.0;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::ErfClip( in, out, threshold, range, dip::S::RANGE );
   plhs[ 0 ] = dml::GetArray( out );
}

void hist_equalize( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   if(( nrhs > 1 ) && !mxIsEmpty( prhs[ 1 ] ) && !dml::IsScalar( prhs[ 1 ] )) {
      // Get input as float array, and convert to something we can stick in a histogram
      dip::FloatArray data = dml::GetFloatArray( prhs[ 1 ] );
      dip::dfloat minV = std::numeric_limits< double >::max();
      dip::dfloat maxV = 0;
      for( auto v : data ) {
         if( v < 0 ) {
            v = 0;
         } else if( v > 0 ) {
            minV = std::min( minV, v );
            maxV = std::max( maxV, v );
         }
      }
      dip::dfloat scale = std::min( 1.0 / minV, static_cast< dip::dfloat >( std::numeric_limits< dip::Histogram::CountType >::max() ) / maxV );
      for( auto& v : data ) {
         v *= scale;
      }
      // Create a histogram of the right dimensions
      dip::Histogram::Configuration config( 0.0, static_cast< int >( data.size() ), 1.0 );
      dip::Histogram example( config );
      // Fill it with the input
      dip::Image img = example.GetImage().QuickCopy();
      DIP_ASSERT( img.NumberOfPixels() == data.size() );
      dip::ImageIterator< dip::Histogram::CountType > imit( img );
      for( auto dait = data.begin(); dait != data.end(); ++dait, ++imit ) {
         *imit = static_cast< dip::Histogram::CountType >( *dait );
      }
      // Call function
      dip::HistogramMatching( in, out, example );
   } else {
      dip::uint nBins = 256;
      if(( nrhs > 1 ) && !mxIsEmpty( prhs[ 1 ] ) ) {
         nBins = dml::GetUnsigned( prhs[ 1 ] );
      }
      dip::HistogramEqualization( in, out, nBins );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void stretch( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 8 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::dfloat low = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 0.0;
   dip::dfloat high = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 100.0;
   dip::dfloat minimum = nrhs > 3 ? dml::GetFloat( prhs[ 3 ] ) : 0.0;
   dip::dfloat maximum = nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : 255.0;
   dip::String method = nrhs > 5 ? dml::GetString( prhs[ 5 ] ) : dip::S::LINEAR;
   dip::dfloat param1 = nrhs > 6 ? dml::GetFloat( prhs[ 6 ] ) : 1.0;
   dip::dfloat param2 = nrhs > 7 ? dml::GetFloat( prhs[ 7 ] ) : 0.0;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::ContrastStretch( in, out, low, high, minimum, maximum, method, param1, param2 );
   plhs[ 0 ] = dml::GetArray( out );
}

void lut( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image table = dml::GetImage( prhs[ 1 ] );
   if( table.Dimensionality() == 2 ) {
      DIP_THROW_IF( !table.IsScalar(), dip::E::DIMENSIONALITY_NOT_SUPPORTED );
      table.SpatialToTensor( 0 );
   } else if( table.Dimensionality() != 1 ) {
      DIP_THROW( dip::E::DIMENSIONALITY_NOT_SUPPORTED );
   }
   int index = 2;
   dip::FloatArray indices;
   if(( nrhs > index ) && ( mxIsNumeric( prhs[ index ] ))) {
      indices = dml::GetFloatArray( prhs[ index ] );
      ++index;
   }
   dip::LookupTable lut( table, indices );
   dip::String method = nrhs > index ? dml::GetString( prhs[ index ] ) : dip::S::LINEAR;
   if( nrhs > index + 1 ) {
      if( mxIsNumeric( prhs[ index + 1 ] )) {
         dip::FloatArray bounds = dml::GetFloatArray( prhs[ index + 1 ] );
         if( bounds.size() == 1 ) {
            lut.SetOutOfBoundsValue( bounds[ 0 ] );
         } else if( bounds.size() == 2 ) {
            lut.SetOutOfBoundsValue( bounds[ 0 ], bounds[ 1 ] );
         } else {
            DIP_THROW( dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
         }
      } else {
         dip::String bounds = dml::GetString( prhs[ index + 1 ] );
         if( bounds == "clamp" ) {
            lut.ClampOutOfBoundsValues();
         } else if( bounds == dip::S::KEEP ) {
            lut.KeepInputValueOnOutOfBounds();
         } else {
            DIP_THROW_INVALID_FLAG( bounds );
         }
      }
   }
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   lut.Apply( in, out, method );
   if( table.IsColor() ) {
      out.SetColorSpace( table.ColorSpace() );
   } else if( table.TensorElements() == 3 ) {
      out.SetColorSpace( "RGB" );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void overlay( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 3 );
   dip::Image const grey = dml::GetImage( prhs[ 0 ] );
   dip::Image const bin = dml::GetImage( prhs[ 1 ] );
   dip::Image::Pixel color{ 255, 0, 0 };
   if( nrhs > 2 ) {
      color.swap( dml::GetPixel( prhs[ 2 ] )); // we cannot assign to a pixel!
   }
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Overlay( grey, bin, out, color );
   plhs[ 0 ] = dml::GetArray( out );
}

void mdhistogram( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const mask = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image{};
   dip::Histogram::ConfigurationArray conf;
   if( nrhs > 2 ) {
      DIP_THROW_IF( !mxIsCell( prhs[ 2 ] ), "SPECS parameter must be a cell array" );
      if( !mxIsCell( mxGetCell( prhs[ 2 ], 0 ))) {
         conf.resize( 1 );
         conf[ 0 ] = dml::GetHistogramConfiguration( prhs[ 2 ] );
      } else {
         dip::uint N = mxGetNumberOfElements( prhs[ 2 ] );
         conf.resize( N );
         for( dip::uint ii = 0; ii < N; ++ii ) {
            conf[ ii ] = dml::GetHistogramConfiguration( mxGetCell( prhs[ 2 ], ii ));
         }
      }
   } else {
      // Default configuration
      conf.resize( 1 );
      conf[ 0 ] = { 0.0, 100.0, 100 };
      conf[ 0 ].lowerIsPercentile = true;
      conf[ 0 ].upperIsPercentile = true;
   }
   dip::uint nThreads = dip::GetNumberOfThreads();
   dip::SetNumberOfThreads( 1 );          // Make sure we don't use OpenMP.
   dip::Histogram hist( in, mask, conf ); // Can crash if there is more than one tensor dimension when using OpenMP.
   dip::SetNumberOfThreads( nThreads );
   // Copy the histogram bins to an output array.
   dip::Image const& bins = hist.GetImage();
   dip::uint nDims = bins.Dimensionality();
   plhs[ 0 ] = dml::GetArray( bins );
   // Create the optional 2nd output argument bins
   if( nlhs > 1 ) {
      if( nDims == 1 ) {
         plhs[ 1 ] = dml::GetArray( hist.BinCenters( 0 ));
      } else {
         plhs[ 1 ] = mxCreateCellMatrix( nDims, 1 );
         for( dip::uint ii = 0; ii < nDims; ii++ ) {
            mxSetCell( plhs[ 1 ], ii, dml::GetArray( hist.BinCenters( ii )));
         }
      }
   }
}

void GetBinConfig(
      dip::FloatArray const& bins,
      dip::Histogram::Configuration& conf
) {
   DIP_THROW_IF( bins.size() != conf.nBins, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
   if( conf.nBins == 1 ) {
      conf.lowerBound = bins[ 0 ] - 0.5;
      conf.upperBound = bins[ 0 ] + 0.5;
      conf.binSize = 1.0; // There's no way to determine the original bin size here. But I guess it really doesn't matter.
   } else {
      conf.binSize = bins[ 1 ] - bins[ 0 ];
      conf.lowerBound = bins[ 0 ] - conf.binSize / 2.0;
      conf.upperBound = conf.lowerBound + static_cast< dip::dfloat >( conf.nBins ) * conf.binSize;
   }
}

void mdhistogrammap( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 3 );
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint nDims = in.TensorElements();
   dip::Image histImg = dml::GetImage( prhs[ 1 ] );
   DIP_THROW_IF( !histImg.DataType().IsUnsigned(), dip::E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( nDims != histImg.Dimensionality(), dip::E::NTENSORELEM_DONT_MATCH );
   dip::Histogram::ConfigurationArray conf( nDims );
   for( dip::uint ii = 0; ii < nDims; ii++ ) {
      conf[ ii ].nBins = histImg.Size( ii );
   }
   dip::uint N;
   if( mxIsCell( prhs[ 2 ] )) {
      N = mxGetNumberOfElements( prhs[ 2 ] );
      DIP_THROW_IF( N != nDims, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
      for( dip::uint ii = 0; ii < nDims; ii++ ) {
         dip::FloatArray bins = dml::GetFloatArray( mxGetCell( prhs[ 2 ], ii ) );
         DIP_STACK_TRACE_THIS( GetBinConfig( bins, conf[ ii ] ));
      }
   } else {
      DIP_THROW_IF( 1 != nDims, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
      dip::FloatArray bins = dml::GetFloatArray( prhs[ 2 ] );
      DIP_STACK_TRACE_THIS( GetBinConfig( bins, conf[ 0 ] ));
   }
   dip::BooleanArray excludeOutOfBoundValues = nrhs > 3 ? dml::GetBooleanArray( prhs[ 3 ] ) : dip::BooleanArray{ false };
   // Create the histogram object and copy the histogram values into it (there's no other way)
   dip::Histogram hist( conf );
   dip::Image histImgNew = hist.GetImage();
   histImgNew.Protect();
   DIP_STACK_TRACE_THIS( histImgNew.Copy( histImg ));
   // Apply the mapping
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   hist.ReverseLookup( in, out, excludeOutOfBoundValues );
   plhs[ 0 ] = dml::GetArray( out );
}

} // namespace

// Gateway function

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 2 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      prhs += 1;
      nrhs -= 1;

      if( function == "integral_image" ) {
         integral_image( plhs, nrhs, prhs );
      } else if( function == "select" ) {
         select( plhs, nrhs, prhs );

      } else if( function == "getmaximumandminimum" ) {
         getmaximumandminimum( plhs, nrhs, prhs );
      } else if( function == "getsamplestatistics" ) {
         getsamplestatistics( plhs, nrhs, prhs );
      } else if( function == "entropy" ) {
         entropy( plhs, nrhs, prhs );
      } else if( function == "errormeasure" ) {
         errormeasure( plhs, nrhs, prhs );
      } else if( function == "noisestd" ) {
         noisestd( plhs, nrhs, prhs );
      } else if( function == "radialmax" ) {
         RadialProjection( dip::RadialMaximum, plhs, nrhs, prhs );
      } else if( function == "radialmean" ) {
         RadialProjection( dip::RadialMean, plhs, nrhs, prhs );
      } else if( function == "radialmin" ) {
         RadialProjection( dip::RadialMinimum, plhs, nrhs, prhs );
      } else if( function == "radialsum" ) {
         RadialProjection( dip::RadialSum, plhs, nrhs, prhs );

      } else if( function == "clip" ) {
         clip( plhs, nrhs, prhs );
      } else if( function == "erfclip" ) {
         erfclip( plhs, nrhs, prhs );
      } else if( function == "hist_equalize" ) {
         hist_equalize( plhs, nrhs, prhs );
      } else if( function == "stretch" ) {
         stretch( plhs, nrhs, prhs );

      } else if( function == "lut" ) {
         lut( plhs, nrhs, prhs );

      } else if( function == "overlay" ) {
         overlay( plhs, nrhs, prhs );

      } else if( function == "mdhistogram" ) {
         mdhistogram( nlhs, plhs, nrhs, prhs );
      } else if( function == "mdhistogrammap" ) {
         mdhistogrammap( plhs, nrhs, prhs );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

   } DML_CATCH
}
