/*
 * DIPlib 3.0
 * This file contains the definition for the Count function.
 *
 * (c)2016-2017, Cris Luengo.
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
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
class dip__Count : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::uint& count = counts[ params.thread ];
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 1 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask && ( *in != TPI( 0 ))) {
                  ++count;
               }
               in += inStride;
               mask += maskStride;
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *in != TPI( 0 )) {
                  ++count;
               }
               in += inStride;
            }
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         counts.resize( threads );
      }
      dip__Count( std::vector< dip::uint >& counts ) : counts( counts ) {}
   private:
      std::vector< dip::uint >& counts;
};

} // namespace

dip::uint Count(
      Image const& in,
      Image const& mask
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   std::vector< dip::uint > counts;
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__Count, ( counts ), in.DataType() );
   // Call the framework function
   Framework::ScanSingleInput( in, mask, in.DataType(), *scanLineFilter );
   // Reduce
   dip::uint out = counts[ 0 ];
   for( dip::uint ii = 1; ii < counts.size(); ++ii ) {
      out += counts[ ii ];
   }
   return out;
}

} // namespace dip
