/*
 * DIPlib 3.0
 * This file defines the "RGB" and "nlRGB" color spaces
 *
 * (c)2017, Cris Luengo.
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

#include "diplib.h"
#include "diplib/measurement.h"

namespace dip {

namespace {

class rgb2grey : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const { return "RGB"; }
      virtual String OutputColorSpace() const { return "grey"; }
      virtual dip::uint Cost() const { return 100; }
      virtual void Convert( dfloat const* input, dfloat* output ) {
         // TODO: configure white point
         output[ 0 ] = input[ 0 ] * 0.357580 +
                       input[ 0 ] * 0.715160 +
                       input[ 0 ] * 0.119193;
      }
};

class grey2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const { return "grey"; }
      virtual String OutputColorSpace() const { return "RGB"; }
      virtual dip::uint Cost() const { return 100; }
      virtual void Convert( dfloat const* input, dfloat* output ) {
         // TODO: configure white point
         output[ 0 ] = input[ 0 ];
         output[ 1 ] = input[ 0 ];
         output[ 2 ] = input[ 0 ];
      }
};

class rgb2nlrgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const { return "RGB"; }
      virtual String OutputColorSpace() const { return "nlRGB"; }
      virtual dip::uint Cost() const { return 100; }
      virtual void Convert( dfloat const* input, dfloat* output ) {
         output[ 0 ] = std::pow( input[ 0 ] * ( 1.0 / 255.0 ), 2.5 ) * 255.0;
         output[ 1 ] = std::pow( input[ 1 ] * ( 1.0 / 255.0 ), 2.5 ) * 255.0;
         output[ 2 ] = std::pow( input[ 2 ] * ( 1.0 / 255.0 ), 2.5 ) * 255.0;
      }
};

class nlrgb2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const { return "nlRGB"; }
      virtual String OutputColorSpace() const { return "RGB"; }
      virtual dip::uint Cost() const { return 100; }
      virtual void Convert( dfloat const* input, dfloat* output ) {
         output[ 0 ] = std::pow( input[ 0 ] * ( 1.0 / 255.0 ), 0.4 ) * 255.0;
         output[ 1 ] = std::pow( input[ 1 ] * ( 1.0 / 255.0 ), 0.4 ) * 255.0;
         output[ 2 ] = std::pow( input[ 2 ] * ( 1.0 / 255.0 ), 0.4 ) * 255.0;
      }
};

} // namespace

} // namespace dip
