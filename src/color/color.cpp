/*
 * DIPlib 3.0
 * This file contains main functionality for color image support.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPimage code: (c)2014, Cris Luengo;
 *                                  (c)1999-2014, Delft University of Technology.
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

#include <queue>

#include "diplib.h"
#include "diplib/color.h"
#include "diplib/framework.h"

namespace dip {
namespace {
// XYZ matrix for conversion between RGB and XYZ.
using XYZMatrix = std::array< dfloat, 9 >;
}
}

#include "rgb.h"
#include "cmyk.h"
#include "hsi.h"
#include "ish.h"
#include "hcv.h"
#include "xyz.h"
#include "lab.h"

namespace dip {

constexpr ColorSpaceManager::XYZ ColorSpaceManager::IlluminantA;
constexpr ColorSpaceManager::XYZ ColorSpaceManager::IlluminantD50;
constexpr ColorSpaceManager::XYZ ColorSpaceManager::IlluminantD55;
constexpr ColorSpaceManager::XYZ ColorSpaceManager::IlluminantD65;
constexpr ColorSpaceManager::XYZ ColorSpaceManager::IlluminantE;

ColorSpaceManager::ColorSpaceManager() {
   // grey (or gray)
   Define( "grey", 1 );
   DefineAlias( "gray", "grey" );
   // RGB
   Define( "RGB", 3 );
   DefineAlias( "rgb", "RGB" );
   Register( new grey2rgb );
   Register( new rgb2grey );
   // nlRGB (or R'G'B')
   Define( "sRGB", 3 );
   DefineAlias( "srgb", "sRGB" );
   Register( new rgb2srgb );
   Register( new srgb2rgb );
   // CMY
   Define( "CMY", 3 );
   DefineAlias( "cmy", "CMY" );
   Register( new rgb2cmy );
   Register( new cmy2rgb );
   // CMYK
   Define( "CMYK", 4 );
   DefineAlias( "cmyk", "CMYK" );
   Register( new cmy2cmyk );
   Register( new cmyk2cmy );
   // HSI
   Define( "HSI", 3 );
   DefineAlias( "hsi", "HSI" );
   Register( new grey2hsi );
   Register( new hsi2grey );
   Register( new rgb2hsi );
   Register( new hsi2rgb );
   // HCV
   Define( "HCV", 3 );
   DefineAlias( "hcv", "HCV" );
   Register( new rgb2hcv );
   Register( new hcv2rgb );
   // HSV
   Define( "HSV", 3 );
   DefineAlias( "hsv", "HSV" );
   Register( new hcv2hsv );
   Register( new hsv2hcv );
   // XYZ
   Define( "XYZ", 3 );
   DefineAlias( "xyz", "XYZ" );
   Register( new grey2xyz );
   Register( new rgb2xyz );
   Register( new xyz2grey );
   Register( new xyz2rgb );
   // Yxy
   Define( "Yxy", 3 );
   DefineAlias( "yxy", "Yxy" );
   Register( new xyz2yxy );
   Register( new yxy2grey );
   Register( new yxy2xyz );
   // Lab (or L*a*b*, CIELAB)
   Define( "Lab", 3 );
   DefineAlias( "lab", "Lab" );
   DefineAlias( "L*a*b*", "Lab" );
   DefineAlias( "l*a*b*", "Lab" );
   DefineAlias( "CIELAB", "Lab" );
   DefineAlias( "cielab", "Lab" );
   Register( new grey2lab );
   Register( new xyz2lab );
   Register( new lab2grey );
   Register( new lab2xyz );
   // Luv (or L*u*v*, CIELUV)
   Define( "Luv", 3 );
   DefineAlias( "luv", "Luv" );
   DefineAlias( "L*u*v*", "Luv" );
   DefineAlias( "l*u*v*", "Luv" );
   DefineAlias( "CIELUV", "Luv" );
   DefineAlias( "cieluv", "Luv" );
   Register( new grey2luv );
   Register( new xyz2luv );
   Register( new luv2xyz );
   Register( new luv2grey );
   // LCH
   Define( "LCH", 3 );
   DefineAlias( "lch", "LCH" );
   DefineAlias( "L*C*H*", "LCH" );
   DefineAlias( "l*c*h*", "LCH" );
   Register( new grey2lch );
   Register( new lab2lch );
   Register( new lch2lab );
   Register( new lch2grey );
}


namespace {

struct ConversionStep {
   ColorSpaceConverter const* converterFunction;
   dip::uint nOutputChannels;
   bool last = false;
};

using ConversionStepArray = std::vector< ConversionStep >;

class ConverterLineFilter : public Framework::ScanLineFilter {
   public:
      ConverterLineFilter( ConversionStepArray const& steps ) : steps_( steps ) {
         maxIntermediateChannels_ = steps[ 0 ].nOutputChannels;
         for( dip::uint ii = 1; ii < steps.size() - 1; ++ii ) {
            maxIntermediateChannels_ = std::max( maxIntermediateChannels_, steps[ 1 ].nOutputChannels );
         }
         nBuffers_ = std::min< dip::uint >( 2, steps.size() - 1 );
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffer1_.resize( threads );
         buffer2_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         dip::uint cost = 0;
         for( auto const& step : steps_ ) {
            dip::uint c = step.converterFunction->Cost();
            if( c >= 100 ) {
               c -= 99; // This is usually the case for conversion to gray, to indicate data loss
            }
            cost += 50 * c; // This is very rough, most methods indicate a cost of 1 through 3, which we map here to 50-150.
         }
         return cost;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::uint thread = params.thread;
         dip::uint nPixels = params.bufferLength;
         if( nBuffers_ > 0 ) {
            if( buffer1_[ thread ].size() != nPixels * maxIntermediateChannels_ ) {
               buffer1_[ thread ].resize( nPixels * maxIntermediateChannels_ );
               if( nBuffers_ > 1 ) {
                  buffer2_[ thread ].resize( nPixels * maxIntermediateChannels_ );
               }
            }
         }
         dfloat* buffer1 = buffer1_[ thread ].data(); // Might not be used...
         dfloat* buffer2 = buffer2_[ thread ].data(); // Might not be used...
         // We initialize 'out' parameters to the input.
         dfloat* out = static_cast< dfloat* >( params.inBuffer[ 0 ].buffer );
         dip::sint outStride = params.inBuffer[ 0 ].stride;
         dip::sint outTStride = params.inBuffer[ 0 ].tensorStride;
         dip::uint outChans = params.inBuffer[ 0 ].tensorLength;
         for( auto const& step : steps_ ) {
            // At each iteration, we read from the previous `out`
            ConstLineIterator< dfloat > input( out, nPixels, outStride, outChans, outTStride );
            // At each iteration, we write to either the output image, or an intermediate buffer
            outChans = step.nOutputChannels;
            if( step.last ) {
               out = static_cast< dfloat* >( params.outBuffer[ 0 ].buffer );
               outStride = params.outBuffer[ 0 ].stride;
               outTStride = params.outBuffer[ 0 ].tensorStride;
            } else {
               out = out == buffer1 ? buffer2 : buffer1;
               outStride = static_cast< dip::sint >( outChans );
               outTStride = 1;
            }
            LineIterator< dfloat > output( out, nPixels, outStride, outChans, outTStride );
            step.converterFunction->Convert( input, output );
         }
      }
   private:
      ConversionStepArray const& steps_;
      dip::uint maxIntermediateChannels_;
      dip::uint nBuffers_;
      std::vector< std::vector< dfloat >> buffer1_; // one for each thread
      std::vector< std::vector< dfloat >> buffer2_; // one for each thread
      // We use up to 2 buffers. If there's 1 step, we don't need a buffer, we can read from in and write to out.
      // If we have 2 steps, we need one buffer (in->buffer->out). If we have more steps, then we need 2 buffers,
      // which are alternated at each step. The last steps always writes to out, and the first step always reads
      // from in.
      // This means that the conversion functions don't need to worry about input and output being the same buffer.
      // It also means we don't need to worry about how many channels an intermediate representation needs.
};

} // namespace

void ColorSpaceManager::Convert(
      Image const& in,
      Image& out,
      String const& endColorSpace
) const {
   // Make sure the input color space is consistent
   String const& startColorSpace = in.ColorSpace();
   dip::uint endIndex = Index( endColorSpace.empty() ? "grey" : endColorSpace );
   if( startColorSpace.empty() && in.TensorElements() > 1 ) {
      DIP_THROW_IF( colorSpaces_[ endIndex].nChannels != in.TensorElements(), E::INCONSISTENT_COLORSPACE );
      out = in;
   } else {
      dip::uint startIndex = Index( startColorSpace.empty() ? "grey" : startColorSpace );
      DIP_THROW_IF( in.TensorElements() != colorSpaces_[ startIndex ].nChannels, E::INCONSISTENT_COLORSPACE );
      // Get the output color space
      if( startIndex == endIndex ) {
         // Nothing to do
         out = in;
         return;
      }
      // Find a path from start to end
      std::vector< dip::uint > path = FindPath( startIndex, endIndex );
      DIP_THROW_IF( path.empty(), "No conversion possible between color spaces " +
                                  ( startColorSpace.empty() ? "grey" : startColorSpace ) + " and " +
                                  ( endColorSpace.empty() ? "grey" : endColorSpace ) );
      DIP_ASSERT( path.size() > 1 ); // It should have at least start and stop on it!
      // Collect information about the converter functions along the path
      dip::uint nSteps = path.size() - 1;
      ConversionStepArray steps( nSteps );
      //std::cout << "Found path: ";
      for( dip::uint ii = 0; ii < nSteps; ++ii ) {
         //std::cout << colorSpaces_[ path[ ii ] ].name << " -> ";
         steps[ ii ].nOutputChannels = colorSpaces_[ path[ ii + 1 ] ].nChannels;
         auto it = colorSpaces_[ path[ ii ] ].edges.find( path[ ii + 1 ] );
         DIP_ASSERT( it != colorSpaces_[ path[ ii ] ].edges.end() );
         steps[ ii ].converterFunction = it->second.get();
      }
      steps.back().last = true;
      //std::cout << colorSpaces_[ path.back() ].name << std::endl;
      // Call scan framework
      DIP_START_STACK_TRACE
         ConverterLineFilter lineFilter( steps );
         Framework::ScanMonadic(
               in,
               out,
               DT_DFLOAT,
               DataType::SuggestFloat( in.DataType() ),
               steps.back().nOutputChannels,
               lineFilter
         );
      DIP_END_STACK_TRACE
      out.ReshapeTensorAsVector();
   }
   String const& newColorSpace = colorSpaces_[ endIndex ].name;
   if( newColorSpace == "grey" ) {
      out.ResetColorSpace();
   } else {
      out.SetColorSpace( colorSpaces_[ endIndex ].name );
   }
}

namespace {
struct QueueElement {
   dip::uint cost;
   dip::uint index;
   bool operator >( QueueElement const& other ) const { return cost > other.cost; }
};
using PriorityQueueLowFirst = std::priority_queue< QueueElement, std::vector< QueueElement >, std::greater< QueueElement >>;
} // namespace

std::vector< dip::uint > ColorSpaceManager::FindPath( dip::uint start, dip::uint stop ) const {
   constexpr dip::uint NOT_VISITED = std::numeric_limits< dip::uint >::max();
   std::vector< dip::uint > cost( colorSpaces_.size(), NOT_VISITED );
   std::vector< dip::uint > previous( colorSpaces_.size(), 0 );
   PriorityQueueLowFirst queue;
   queue.push( { 0, start } );
   cost[ start ] = 0;
   while( !queue.empty() ) {
      dip::uint k = queue.top().index;
      dip::uint c = queue.top().cost;
      queue.pop();
      if( cost[ k ] < c ) { // it was already processed earlier.
         continue;
      }
      if( k == stop ) {
         // We're done.
         break;
      }
      for( auto& edge : colorSpaces_[ k ].edges ) {
         dip::uint nk = edge.first; // target color space
         dip::uint nc = c + edge.second->Cost();
         if( cost[ nk ] > nc ) {
            cost[ nk ] = nc;
            previous[ nk ] = k;
            queue.push( { nc, nk } );
         }
      }
   }
   std::vector< dip::uint > path;
   if( cost[ stop ] != NOT_VISITED ) {
      dip::uint k = stop;
      while( k != start ) {
         dip::uint n = k;
         k = previous[ n ];
         path.push_back( k );
         // The path contains indices to the color spaces including "start", but not including "stop".
         // The path is reversed, "start" is the last entry pushed onto it.
      }
      // Reverse path so that "start" is the first element.
      std::reverse( path.begin(), path.end() );
      // Add the stop index to the end of the path.
      path.push_back( stop );
   }
   return path;
}

namespace {

// RGB primaries according to ITU-R Recommendation BT.709 (used in HDTV, but valid for computer monitors too).
constexpr std::array< dfloat, 9 > primaries{{ 0.64, 0.33, 0.03,   0.30, 0.60, 0.10,   0.15, 0.06, 0.79 }};

// Computes the RGB/XYZ transformation matrix based on the primaries
XYZMatrix ComputeXYZMatrix( ColorSpaceManager::XYZ const& whitePoint ) {
   XYZMatrix matrix;
   Inverse( 3, primaries.data(), matrix.data() );
   dfloat a1 = matrix[ 0 ] * whitePoint[ 0 ] + matrix[ 3 ] * whitePoint[ 1 ] + matrix[ 6 ] * whitePoint[ 2 ];
   dfloat a2 = matrix[ 1 ] * whitePoint[ 0 ] + matrix[ 4 ] * whitePoint[ 1 ] + matrix[ 7 ] * whitePoint[ 2 ];
   dfloat a3 = matrix[ 2 ] * whitePoint[ 0 ] + matrix[ 5 ] * whitePoint[ 1 ] + matrix[ 8 ] * whitePoint[ 2 ];
   matrix[ 0 ] = primaries[ 0 ] * a1;
   matrix[ 1 ] = primaries[ 1 ] * a1;
   matrix[ 2 ] = primaries[ 2 ] * a1;
   matrix[ 3 ] = primaries[ 3 ] * a2;
   matrix[ 4 ] = primaries[ 4 ] * a2;
   matrix[ 5 ] = primaries[ 5 ] * a2;
   matrix[ 6 ] = primaries[ 6 ] * a3;
   matrix[ 7 ] = primaries[ 7 ] * a3;
   matrix[ 8 ] = primaries[ 8 ] * a3;
   return matrix;
}

} // namespace

void ColorSpaceManager::SetWhitePoint( XYZ whitePoint ) {
   whitePoint[ 0 ] /= whitePoint[ 1 ]; // Xn
   whitePoint[ 2 ] /= whitePoint[ 1 ]; // Zn
   whitePoint[ 1 ] = 1.0;              // Yn
   XYZMatrix matrix = ComputeXYZMatrix( whitePoint );
   static_cast< rgb2grey* >( GetColorSpaceConverter( "RGB",  "grey" ) )->SetWhitePoint( matrix );
   static_cast< grey2xyz* >( GetColorSpaceConverter( "grey", "XYZ"  ) )->SetWhitePoint( whitePoint );
   static_cast< rgb2xyz*  >( GetColorSpaceConverter( "RGB",  "XYZ"  ) )->SetWhitePoint( matrix );
   static_cast< xyz2rgb*  >( GetColorSpaceConverter( "XYZ",  "RGB"  ) )->SetWhitePoint( matrix );
   static_cast< xyz2lab*  >( GetColorSpaceConverter( "XYZ",  "Lab"  ) )->SetWhitePoint( whitePoint );
   static_cast< lab2xyz*  >( GetColorSpaceConverter( "Lab",  "XYZ"  ) )->SetWhitePoint( whitePoint );
   static_cast< xyz2luv*  >( GetColorSpaceConverter( "XYZ",  "Luv"  ) )->SetWhitePoint( whitePoint );
   static_cast< luv2xyz*  >( GetColorSpaceConverter( "Luv",  "XYZ"  ) )->SetWhitePoint( whitePoint );
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/math.h"

DOCTEST_TEST_CASE("[DIPlib] testing the ColorSpaceManager class") {
   dip::ColorSpaceManager csm;
   DOCTEST_CHECK( csm.NumberOfChannels( "rgb" ) == 3 );
   DOCTEST_CHECK( csm.NumberOfChannels( "CMYK" ) == 4 );
   DOCTEST_CHECK( csm.NumberOfChannels( "grey" ) == 1 );
   DOCTEST_CHECK( csm.CanonicalName( "CIELUV" ) == "Luv" );
   DOCTEST_CHECK( csm.GetColorSpaceConverter( "rgb", "cmy" )->Cost() == 1 );
   DOCTEST_CHECK( csm.GetColorSpaceConverter( "rgb", "grey" )->Cost() == 100 );
   // Test grey->RGB conversion
   dip::Image img( {}, 1 );
   img.Fill( 100 );
   dip::Image out = csm.Convert( img, "RGB" );
   DOCTEST_CHECK( out.ColorSpace() == "RGB" );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0 ) == dip::Image::Pixel( { 100, 100, 100 } ));
   // CMYK should have 4 tensor elements, not 3!
   img.SetColorSpace( "CMYK" );
   DOCTEST_CHECK_THROWS( csm.Convert( img, "RGB" ) );
   // Run through a long conversion chain, probably just for Valgrind.
   img = dip::Image( {}, 4 );
   img.Fill( 0 );
   img.SetColorSpace( "CMYK" );
   csm.Convert( img, out, "LCH" ); // This is the longest path we have so far.
   DOCTEST_CHECK( out.ColorSpace() == "LCH" );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   // Check that converting RGB->XYZ using default settings, and then XYZ->RGB using D65, yields the input image again.
   // This tests the white point setting functions do what they should.
   img = dip::Image( {}, 3 );
   img = { 200, 150, 100 };
   img.SetColorSpace( "RGB" );
   dip::Image xyz = csm.Convert( img, "XYZ" );
   csm.SetWhitePoint( dip::ColorSpaceManager::IlluminantD65 ); // same as default values!
   csm.Convert( xyz, out, "RGB" );
   DOCTEST_CHECK( img.At( 0 )[ 0 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 0 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 1 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 1 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 2 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 2 ].As< dip::dfloat >() ));
   // Check that RGB->XYZ yields something different when using a different white point.
   csm.SetWhitePoint( dip::ColorSpaceManager::IlluminantD50 );
   csm.Convert( img, out, "XYZ" );
   DOCTEST_CHECK_FALSE( xyz.At( 0 ) == out.At( 0 ));
}

#endif // DIP__ENABLE_DOCTEST
