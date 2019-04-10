/*
 * DIPlib 3.0
 * This file contains the definition of grey-value mapping functions
 *
 * (c)2017, 2019, Cris Luengo.
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
#include "diplib/mapping.h"
#include "diplib/statistics.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

struct ClipOptions {
   bool clipLow = true;
   bool clipHigh = true;
   bool range = false;
};

ClipOptions ParseClipOptions( String const& mode ) {
   ClipOptions options;
   if( mode == S::BOTH ) {
      // nothing to do
   } else if( mode == S::LOW ) {
      options.clipHigh = false;
   } else if( mode == S::HIGH ) {
      options.clipLow = false;
   } else if( mode == S::RANGE ) {
      options.range = true;
   } else {
      DIP_THROW_INVALID_FLAG( mode );
   }
   return options;
}

} // namespace

void Clip(
      Image const& in,
      Image& out,
      dfloat low,
      dfloat high,
      String const& mode
) {
   DataType dtype = in.DataType();
   DIP_THROW_IF( !dtype.IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   ClipOptions options;
   DIP_STACK_TRACE_THIS( options = ParseClipOptions( mode ));
   if( options.range ) {
      dfloat tmp = low - high / 2.0;
      high = low + high / 2.0;
      low = tmp;
   }
   if( options.clipLow && options.clipHigh ) {
      if( low > high ) {
         std::swap( low, high );
      }
   }
   if( !options.clipLow ) {
      low = -infinity;
   }
   if( !options.clipHigh ) {
      high = infinity;
   }
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter, Framework::NewMonadicScanLineFilter, (
         [ = ]( auto its ) { return clamp( *its[ 0 ], static_cast< decltype( *its[ 0 ] ) >( low ), decltype( *its[ 0 ] )( high )); }, 2
   ), dtype );
   DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, dtype, dtype, in.TensorElements(), *scanLineFilter, Framework::ScanOption::TensorAsSpatialDim ));
}

namespace {

class ErfClipLineFilter: public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return ( options_.clipLow || options_.clipHigh ) ? 22 : 1;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
            dfloat value = *in - threshold_;
            if(( options_.clipLow && options_.clipHigh ) ||
               ( !options_.clipLow && ( value > 0.0 )) ||
               ( !options_.clipHigh && ( value < 0.0 ))) {
               *out = threshold_ + ( scale2_  * std::erf( value * scale1_ ));
            } else {
               *out = *in;
            }
            in += inStride;
            out += outStride;
         }
      }
      ErfClipLineFilter( dfloat threshold, dfloat range, ClipOptions const& options ):
            threshold_( threshold ), scale1_( std::sqrt( pi ) / range ), scale2_( range / 2.0 ), options_( options ) {}
   private:
      dfloat threshold_;
      dfloat scale1_;
      dfloat scale2_;
      ClipOptions const& options_;
};

} // namespace

void ErfClip(
      Image const& in,
      Image& out,
      dfloat low,
      dfloat high,
      String const& mode
) {
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   ClipOptions options;
   DIP_STACK_TRACE_THIS( options = ParseClipOptions( mode ));
   dfloat threshold, range;
   if( options.range ) {
      threshold = low;
      range = high;
   } else {
      if( low > high ) {
         std::swap( low, high );
      }
      threshold = ( low + high ) / 2.0;
      range = high - low;
   }
   ErfClipLineFilter scanLineFilter( threshold, range, options );
   DataType outType = DataType::SuggestFloat( in.DataType() );
   Framework::ScanMonadic( in, out, DT_DFLOAT, outType, in.TensorElements(), scanLineFilter, Framework::ScanOption::TensorAsSpatialDim );
}

void Zero(
      Image const& in,
      Image& out,
      dfloat threshold
) {
   DataType dtype = in.DataType();
   DIP_THROW_IF( !dtype.IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter, Framework::NewMonadicScanLineFilter, (
         [ = ]( auto its ) { return static_cast< dfloat >( *its[ 0 ] ) < threshold ? static_cast< decltype( *its[ 0 ] ) >( 0 ) : *its[ 0 ]; }, 2
   ), dtype );
   DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, dtype, dtype, in.TensorElements(), *scanLineFilter, Framework::ScanOption::TensorAsSpatialDim ));
}

namespace {

class ContrastStretchLineFilter_Linear : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 4; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
            dfloat value = dip::clamp( *in, inMin_, inMax_ );
            *out = offset_ + scale_ * value;
            in += inStride;
            out += outStride;
         }
      }
      ContrastStretchLineFilter_Linear( dfloat inMin, dfloat inMax, dfloat outMin, dfloat outMax ):
            inMin_( inMin ), inMax_( inMax ), scale_(( outMax - outMin ) / ( inMax - inMin )),
            offset_( outMin - scale_ * inMin ) {}
   private:
      dfloat inMin_;
      dfloat inMax_;
      dfloat scale_;
      dfloat offset_;
};

class ContrastStretchLineFilter_Logarithmic : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 27; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
            dfloat value = dip::clamp( *in, inMin_, inMax_ );
            *out = offset_ + scale_ * std::log( value - inMin_ + 1.0 );
            in += inStride;
            out += outStride;
         }
      }
      ContrastStretchLineFilter_Logarithmic( dfloat inMin, dfloat inMax, dfloat outMin, dfloat outMax ):
            inMin_( inMin ), inMax_( inMax ), offset_( outMin ),
            scale_(( outMax - outMin ) / std::log( inMax - inMin + 1.0 )) {}
   private:
      dfloat inMin_;
      dfloat inMax_;
      dfloat offset_;
      dfloat scale_;
};

class ContrastStretchLineFilter_SignedLogarithmic : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 27; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
            dfloat value = dip::clamp( *in, inMin_, inMax_ );
            if( value > 0.0 ) {
               *out = offset_ + scale_ * std::log( value + 1.0 );
            } else {
               *out = offset_ - scale_ * std::log( inMax_ + value + 1.0 );
            }
            in += inStride;
            out += outStride;
         }
      }
      ContrastStretchLineFilter_SignedLogarithmic( dfloat inMin, dfloat inMax, dfloat outMin, dfloat outMax ) {
         inMax_ = std::max( std::abs( inMin ), std::abs( inMax ));
         inMin_ = -inMax_;
         scale_ = ( outMax - outMin ) / ( 2.0 * log( inMax_ + 1.0 ));
         offset_ = ( outMax + outMin ) / 2.0;
      }
   private:
      dfloat inMin_;
      dfloat inMax_;
      dfloat offset_;
      dfloat scale_;
};

class ContrastStretchLineFilter_Erf : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 30; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
            dfloat value = *in - threshold_;
            *out = offset_ + ( outScale_  * std::erf( value * inScale_ )); // Note that erf() replaces clamp().
            in += inStride;
            out += outStride;
         }
      }
      ContrastStretchLineFilter_Erf( dfloat inMin, dfloat inMax, dfloat outMin, dfloat outMax ):
            outScale_(( outMax - outMin ) / 2.0 ), offset_( outScale_ + outMin ),
            inScale_( std::sqrt( pi ) / ( inMax - inMin )), threshold_(( inMax + inMin ) / 2.0 ) {}

   private:
      dfloat outScale_;
      dfloat offset_;
      dfloat inScale_;
      dfloat threshold_;
};

class ContrastStretchLineFilter_Decade : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 30; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
            dfloat value = dip::clamp( *in, inMin_, inMax_ );
            dfloat decade = std::log10( inScale_ / ( value - inMin_ + 10.0 * std::numeric_limits< dfloat >::min() ));
            if( decade < maxDecade_ ) {
               decade -= std::floor( decade );
               *out = offset_ + outScale_ * ( 1.0 - decade );
            } else {
               *out = 0.0;
            }
            in += inStride;
            out += outStride;
         }
      }
      ContrastStretchLineFilter_Decade( dfloat inMin, dfloat inMax, dfloat outMin, dfloat outMax, dfloat parameter1 ):
            inMin_( inMin ), inMax_( inMax ), offset_( outMin ),
            inScale_( inMax - inMin ), outScale_( outMax - outMin ), maxDecade_( parameter1 ) {}
   private:
      dfloat inMin_;
      dfloat inMax_;
      dfloat offset_;
      dfloat inScale_;
      dfloat outScale_;
      dfloat maxDecade_;
};

inline dfloat sigmoid( dfloat x ) { return x / ( 1.0 + std::abs( x )); }

class ContrastStretchLineFilter_Sigmoid : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 10; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         dfloat* out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
            dfloat value = dip::clamp( *in, inMin_, inMax_ );
            value = slope_ * value + point_;
            *out = offset_ + scale_ * sigmoid( value );
            in += inStride;
            out += outStride;
         }
      }
      ContrastStretchLineFilter_Sigmoid( dfloat inMin, dfloat inMax, dfloat outMin, dfloat outMax,
                                         dfloat parameter1, dfloat parameter2 ):
            inMin_( inMin ), inMax_( inMax ), slope_( parameter1 ), point_( parameter2 ) {
         dfloat min = sigmoid( slope_ * inMin_ + point_ );
         dfloat max = sigmoid( slope_ * inMax_ + point_ );
         scale_ = ( outMax - outMin ) / ( max - min );
         offset_ = outMin - scale_ * min;
      }
   private:
      dfloat inMin_;
      dfloat inMax_;
      dfloat slope_;
      dfloat point_;
      dfloat offset_;
      dfloat scale_;
};

} // namespace

void ContrastStretch(
      Image const& in,
      Image& out,
      dfloat lowerBound,
      dfloat upperBound,
      dfloat outMin,
      dfloat outMax,
      String const& method,
      dfloat parameter1,
      dfloat parameter2
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   Image tmp = in.QuickCopy();
   tmp.TensorToSpatial();
   dfloat inMin = dip::Percentile( tmp, {}, lowerBound ).As< dfloat >();
   dfloat inMax = dip::Percentile( tmp, {}, upperBound ).As< dfloat >();
   if( inMax < inMin ) {
      std::swap( inMax, inMin );
   }
   DataType outType = DataType::SuggestFloat( in.DataType() );
   if(( inMax == inMin ) || ( outMax == outMin )) {
      PixelSize pixelSize = in.PixelSize();
      String colorSpace = in.ColorSpace();
      out.ReForge( in.Sizes(), in.TensorElements(), outType, Option::AcceptDataTypeChange::DO_ALLOW );
      out.Fill( outMin );
      out.SetPixelSize( pixelSize );
      out.SetColorSpace( colorSpace );
      return;
   }
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   if( method == "linear" ) {
      lineFilter = std::make_unique< ContrastStretchLineFilter_Linear >( inMin, inMax, outMin, outMax );
   } else if( method == "signed linear" ) {
      inMax = std::max( std::abs( inMin ), std::abs( inMax ));
      inMin = -inMax;
      lineFilter = std::make_unique< ContrastStretchLineFilter_Linear >( inMin, inMax, outMin, outMax );
   } else if( method == "logarithmic" ) {
      lineFilter = std::make_unique< ContrastStretchLineFilter_Logarithmic >( inMin, inMax, outMin, outMax );
   } else if( method == "signed logarithmic" ) {
      lineFilter = std::make_unique< ContrastStretchLineFilter_SignedLogarithmic >( inMin, inMax, outMin, outMax );
   } else if( method == "erf" ) {
      lineFilter = std::make_unique< ContrastStretchLineFilter_Erf >( inMin, inMax, outMin, outMax );
   } else if( method == "decade" ) {
      lineFilter = std::make_unique< ContrastStretchLineFilter_Decade >( inMin, inMax, outMin, outMax, parameter1 );
   } else if( method == "sigmoid" ) {
      lineFilter = std::make_unique< ContrastStretchLineFilter_Sigmoid >( inMin, inMax, outMin, outMax, parameter1, parameter2 );
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
   Framework::ScanMonadic( in, out, DT_DFLOAT, outType, in.TensorElements(), *lineFilter, Framework::ScanOption::TensorAsSpatialDim );
}

} // namespace dip
