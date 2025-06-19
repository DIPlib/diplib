/*
 * (c)2016-2025, Cris Luengo.
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

#include "diplib/color.h"

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <queue>
#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"

namespace dip {
namespace {
// XYZ matrix for conversion between RGB and XYZ.
using XYZMatrix = std::array< dfloat, 9 >;
}
}

// Note order of inclusion must not change, some include files depend on stuff previously declared in others.
#include "rgb.h"
#include "cmyk.h"
#include "hsi.h"
#include "ish.h"
#include "hcv.h"
#include "ycbcr.h"
#include "xyz.h"
#include "lab.h"
#include "oklab.h"
#include "wavelength.h"

namespace dip {

constexpr XYZ ColorSpaceManager::IlluminantA;
constexpr XYZ ColorSpaceManager::IlluminantD50;
constexpr XYZ ColorSpaceManager::IlluminantD55;
constexpr XYZ ColorSpaceManager::IlluminantD65;
constexpr XYZ ColorSpaceManager::IlluminantE;

ColorSpaceManager::ColorSpaceManager() {
   // grey (or gray)
   Define( dip::S::GREY, 1 );
   DefineAlias( "gray", dip::S::GREY );
   // RGB
   Define( RGB_name, 3 );
   DefineAlias( "rgb", RGB_name );
   Register( std::make_shared< grey2rgb >() );
   Register( std::make_shared< rgb2grey >() );
   // sRGB
   Define( sRGB_name, 3 );
   DefineAlias( "srgb", sRGB_name );
   DefineAlias( "R'G'B'", sRGB_name );
   DefineAlias( "r'g'b'", sRGB_name );
   Register( std::make_shared< rgb2srgb >() );
   Register( std::make_shared< srgb2rgb >() );
   // sRGBA
   Define( sRGBA_name, 4 );
   DefineAlias( "srgba", sRGBA_name );
   Register( std::make_shared< srgba2srgb >() );
   Register( std::make_shared< srgb2srgba >() );
   // CMY
   Define( CMY_name, 3 );
   DefineAlias( "cmy", CMY_name );
   Register( std::make_shared< rgb2cmy >() );
   Register( std::make_shared< cmy2rgb >() );
   // CMYK
   Define( CMYK_name, 4 );
   DefineAlias( "cmyk", CMYK_name );
   Register( std::make_shared< cmy2cmyk >() );
   Register( std::make_shared< cmyk2cmy >() );
   // HSI
   Define( HSI_name, 3 );
   DefineAlias( "hsi", HSI_name );
   Register( std::make_shared< grey2hsi >() );
   Register( std::make_shared< hsi2grey >() );
   Register( std::make_shared< rgb2hsi >() );
   Register( std::make_shared< hsi2rgb >() );
   // ICH
   Define( ICH_name, 3 );
   DefineAlias( "ich", HSI_name );
   Register( std::make_shared< grey2ich >() );
   Register( std::make_shared< ich2grey >() );
   Register( std::make_shared< rgb2ich >() );
   Register( std::make_shared< ich2rgb >() );
   // ISH
   Define( ISH_name, 3 );
   DefineAlias( "ish", HSI_name );
   Register( std::make_shared< grey2ish >() );
   Register( std::make_shared< ish2grey >() );
   Register( std::make_shared< ich2ish >() );
   Register( std::make_shared< ish2ich >() );
   // HCV
   Define( HCV_name, 3 );
   DefineAlias( "hcv", HCV_name );
   Register( std::make_shared< rgb2hcv >() );
   Register( std::make_shared< hcv2rgb >() );
   // HSV
   Define( HSV_name, 3 );
   DefineAlias( "hsv", HSV_name );
   Register( std::make_shared< hcv2hsv >() );
   Register( std::make_shared< hsv2hcv >() );
   // YPbPr
   Define( YPbPr_name, 3 );
   DefineAlias( "y'pbpr", YPbPr_name );
   DefineAlias( "YPbPr", YPbPr_name );
   DefineAlias( "ypbpr", YPbPr_name );
   DefineAlias( "YPP", YPbPr_name );
   DefineAlias( "ypp", YPbPr_name );
   Register( std::make_shared< srgb2ypbpr >() );
   Register( std::make_shared< ypbpr2srgb >() );
   // YCbCr
   Define( YCbCr_name, 3 );
   DefineAlias( "y'cbcr", YCbCr_name );
   DefineAlias( "YCbCr", YCbCr_name );
   DefineAlias( "ycbcr", YCbCr_name );
   DefineAlias( "YCC", YCbCr_name );
   DefineAlias( "ycc", YCbCr_name );
   Register( std::make_shared< ypbpr2ycbcr >() );
   Register( std::make_shared< ycbcr2ypbpr >() );
   // XYZ
   Define( XYZ_name, 3 );
   DefineAlias( "xyz", XYZ_name );
   Register( std::make_shared< grey2xyz >() );
   Register( std::make_shared< rgb2xyz >() );
   Register( std::make_shared< xyz2grey >() );
   Register( std::make_shared< xyz2rgb >() );
   // Yxy
   Define( Yxy_name, 3 );
   DefineAlias( "yxy", Yxy_name );
   Register( std::make_shared< xyz2yxy >() );
   Register( std::make_shared< yxy2grey >() );
   Register( std::make_shared< yxy2xyz >() );
   // Lab (or L*a*b*, CIELAB)
   Define( Lab_name, 3 );
   DefineAlias( "lab", Lab_name );
   DefineAlias( "L*a*b*", Lab_name );
   DefineAlias( "l*a*b*", Lab_name );
   DefineAlias( "CIELAB", Lab_name );
   DefineAlias( "cielab", Lab_name );
   Register( std::make_shared< grey2lab >() );
   Register( std::make_shared< xyz2lab >() );
   Register( std::make_shared< lab2grey >() );
   Register( std::make_shared< lab2xyz >() );
   // Luv (or L*u*v*, CIELUV)
   Define( Luv_name, 3 );
   DefineAlias( "luv", Luv_name );
   DefineAlias( "L*u*v*", Luv_name );
   DefineAlias( "l*u*v*", Luv_name );
   DefineAlias( "CIELUV", Luv_name );
   DefineAlias( "cieluv", Luv_name );
   Register( std::make_shared< grey2luv >() );
   Register( std::make_shared< xyz2luv >() );
   Register( std::make_shared< luv2xyz >() );
   Register( std::make_shared< luv2grey >() );
   // LCH
   Define( LCH_name, 3 );
   DefineAlias( "lch", LCH_name );
   DefineAlias( "L*C*H*", LCH_name );
   DefineAlias( "l*c*h*", LCH_name );
   Register( std::make_shared< grey2lch >() );
   Register( std::make_shared< lab2lch >() );
   Register( std::make_shared< lch2lab >() );
   Register( std::make_shared< lch2grey >() );
   // Oklab
   Define( Oklab_name, 3 );
   DefineAlias( "oklab", Oklab_name );
   Register( std::make_shared< grey2oklab >() );
   Register( std::make_shared< xyz2oklab >() );
   Register( std::make_shared< oklab2grey >() );
   Register( std::make_shared< oklab2xyz >() );
   // Oklch
   Define( Oklch_name, 3 );
   DefineAlias( "oklch", Oklch_name );
   Register( std::make_shared< grey2oklch >() );
   Register( std::make_shared< oklab2oklch >() );
   Register( std::make_shared< oklch2grey >() );
   Register( std::make_shared< oklch2oklab >() );
   // wavelength
   Define( wavelength_name, 1 );
   Register( std::make_shared< wavelength2xyz >() );
   Register( std::make_shared< wavelength2rgb >() );
}


namespace {

struct ConversionStep {
   ColorSpaceConverter const* converterFunction = nullptr;
   dip::uint nOutputChannels = 0;
   bool last = false;
};

using ConversionStepArray = std::vector< ConversionStep >;

class ConverterLineFilter : public Framework::ScanLineFilter {
   public:
      explicit ConverterLineFilter( ConversionStepArray const& steps ) :
            steps_( steps ),
            maxIntermediateChannels_(steps[ 0 ].nOutputChannels) {
         for( dip::uint ii = 1; ii < steps.size() - 1; ++ii ) {
            maxIntermediateChannels_ = std::max( maxIntermediateChannels_, steps[ 1 ].nOutputChannels );
         }
         nBuffers_ = std::min< dip::uint >( 2, steps.size() - 1 );
      }
      void SetNumberOfThreads( dip::uint threads ) override {
         buffer1_.resize( threads );
         buffer2_.resize( threads );
      }
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override {
         dip::uint cost = 0;
         for( auto const& step : steps_ ) {
            dip::uint c = step.converterFunction->Cost();
            if( c >= 100 ) {
               c -= 99; // This is usually the case for conversion to grey, to indicate data loss
            }
            cost += 50 * c; // This is very rough, most methods indicate a cost of 1 through 3, which we map here to 50-150.
         }
         return cost;
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
   dip::uint endIndex = Index( endColorSpace.empty() ? dip::S::GREY : endColorSpace );
   if( startColorSpace.empty() && ( colorSpaces_[ endIndex ].nChannels == in.TensorElements() )) {
      out = in;
   } else {
      dip::uint startIndex = Index( startColorSpace.empty() ? dip::S::GREY : startColorSpace );
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
                                  ( startColorSpace.empty() ? dip::S::GREY : startColorSpace ) + " and " +
                                  ( endColorSpace.empty() ? dip::S::GREY : endColorSpace ) );
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
         Framework::ScanMonadic( in, out, DT_DFLOAT, DataType::SuggestFloat( in.DataType() ), steps.back().nOutputChannels, lineFilter );
      DIP_END_STACK_TRACE
      out.ReshapeTensorAsVector();
   }
   String const& newColorSpace = colorSpaces_[ endIndex ].name;
   if( newColorSpace == dip::S::GREY ) {
      out.ResetColorSpace();
   } else {
      out.SetColorSpace( newColorSpace );
   }
}

namespace {
struct QueueElement {
   dip::uint cost;
   dip::uint index;
   bool operator >( QueueElement const& other ) const { return cost > other.cost; }
};
using PriorityQueueLowFirst = std::priority_queue< QueueElement, std::vector< QueueElement >, std::greater<>>;
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
      for( auto const& edge : colorSpaces_[ k ].edges ) {
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
XYZMatrix ComputeXYZMatrix( XYZ const& whitePoint ) {
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

void ColorSpaceManager::SetWhitePoint( dip::XYZ whitePoint ) {
   whitePoint[ 0 ] /= whitePoint[ 1 ]; // Xn
   whitePoint[ 2 ] /= whitePoint[ 1 ]; // Zn
   whitePoint[ 1 ] = 1.0;              // Yn
   XYZMatrix matrix = ComputeXYZMatrix( whitePoint );
   XYZMatrix inverseMatrix;
   Inverse( 3, matrix.data(), inverseMatrix.data() );
   for( auto& cs : colorSpaces_ ) {
      for ( auto& conv : cs.edges ) {
         conv.second->SetWhitePoint( whitePoint, matrix, inverseMatrix );
      }
   }
}

} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include <cmath>

DOCTEST_TEST_CASE( "[DIPlib] testing the ColorSpaceManager class" ) {
   dip::ColorSpaceManager csm;
   DOCTEST_CHECK( csm.NumberOfChannels( "rgb" ) == 3 );
   DOCTEST_CHECK( csm.NumberOfChannels( "CMYK" ) == 4 );
   DOCTEST_CHECK( csm.NumberOfChannels( "grey" ) == 1 );
   DOCTEST_CHECK( csm.CanonicalName( "CIELUV" ) == "Luv" );
   DOCTEST_CHECK( csm.GetColorSpaceConverter( "rgb", "cmy" )->Cost() == 1 );
   DOCTEST_CHECK( csm.GetColorSpaceConverter( "rgb", "grey" )->Cost() == 100 );

   // Test no conversion
   dip::Image img( {}, 1 );
   img.Fill( 100 );
   dip::Image out = csm.Convert( img, "wavelength" );
   DOCTEST_CHECK( out.ColorSpace() == "wavelength" );
   DOCTEST_CHECK( out.TensorElements() == 1 );
   DOCTEST_CHECK( out.At( 0 ) == 100 );

   // Test grey -> RGB conversion
   out = csm.Convert( img, "RGB" );
   DOCTEST_CHECK( out.ColorSpace() == "RGB" );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0 ) == dip::Image::Pixel( { 100, 100, 100 } ));

   // CMYK should have 4 tensor elements, not 3!
   img.SetColorSpace( "CMYK" );
   DOCTEST_CHECK_THROWS( csm.Convert( img, img, "RGB" ));

   // Run through a long conversion chain, probably just for Valgrind.
   img = dip::Image( {}, 4 );
   img.Fill( 0 );
   img.SetColorSpace( "CMYK" );
   csm.Convert( img, out, "LCH" ); // This is the longest path we have so far.
   DOCTEST_CHECK( out.ColorSpace() == "LCH" );
   DOCTEST_CHECK( out.TensorElements() == 3 );

   // Check that converting RGB -> XYZ using default settings, and then XYZ -> RGB using D65, yields the input image again.
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

   // Check that RGB -> XYZ yields something different when using a different white point.
   csm.SetWhitePoint( dip::ColorSpaceManager::IlluminantD50 );
   csm.Convert( img, out, "XYZ" );
   DOCTEST_CHECK_FALSE( xyz.At( 0 ) == out.At( 0 ));

   // Check the XYX <-> Oklab pairs shown by Ottosson
   auto round = []( double x ) { return std::round( x * 1000 ) / 1000; };
   img = { 0.950,	1.000,	1.089 };
   img.SetColorSpace( "XYZ" );
   dip::Image oklab = csm.Convert( img, "Oklab" );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 0 ].As< dip::dfloat >() ) == 1.000 );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 1 ].As< dip::dfloat >() ) == 0.000 );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 2 ].As< dip::dfloat >() ) == 0.000 );
   out = csm.Convert( img, "XYZ" );
   DOCTEST_CHECK( img.At( 0 )[ 0 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 0 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 1 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 1 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 2 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 2 ].As< dip::dfloat >() ));
   img = { 1.000, 0.000, 0.000 };
   img.SetColorSpace( "XYZ" );
   oklab = csm.Convert( img, "Oklab" );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 0 ].As< dip::dfloat >() ) ==  0.450 );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 1 ].As< dip::dfloat >() ) ==  1.236 );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 2 ].As< dip::dfloat >() ) == -0.019 );
   out = csm.Convert( img, "XYZ" );
   DOCTEST_CHECK( img.At( 0 )[ 0 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 0 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 1 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 1 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 2 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 2 ].As< dip::dfloat >() ));
   img = { 0.000, 1.000, 0.000 };
   img.SetColorSpace( "XYZ" );
   oklab = csm.Convert( img, "Oklab" );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 0 ].As< dip::dfloat >() ) ==  0.922 );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 1 ].As< dip::dfloat >() ) == -0.671 );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 2 ].As< dip::dfloat >() ) ==  0.263 );
   out = csm.Convert( img, "XYZ" );
   DOCTEST_CHECK( img.At( 0 )[ 0 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 0 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 1 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 1 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 2 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 2 ].As< dip::dfloat >() ));
   img = { 0.000, 0.000, 1.000 };
   img.SetColorSpace( "XYZ" );
   oklab = csm.Convert( img, "Oklab" );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 0 ].As< dip::dfloat >() ) ==  0.153 );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 1 ].As< dip::dfloat >() ) == -1.415 );
   DOCTEST_CHECK( round( oklab.At( 0 )[ 2 ].As< dip::dfloat >() ) == -0.449 );
   out = csm.Convert( img, "XYZ" );
   DOCTEST_CHECK( img.At( 0 )[ 0 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 0 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 1 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 1 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 2 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 2 ].As< dip::dfloat >() ));

   // Check sRGB <-> Y'CbCr roundtrip
   img = { 200, 150, 100 };
   img.SetColorSpace( "sRGB" );
   dip::Image ycbcr = csm.Convert( img, "Y'CbCr" );
   out = csm.Convert( ycbcr, "sRGB" );
   DOCTEST_CHECK( img.At( 0 )[ 0 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 0 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 1 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 1 ].As< dip::dfloat >() ));
   DOCTEST_CHECK( img.At( 0 )[ 2 ].As< dip::dfloat >() == doctest::Approx( out.At( 0 )[ 2 ].As< dip::dfloat >() ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
