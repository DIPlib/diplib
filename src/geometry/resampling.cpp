/*
 * DIPlib 3.0
 * This file contains definitions for coordinate image generation
 *
 * (c)2017, Cris Luengo.
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
#include "diplib/geometry.h"
//#include "diplib/framework.h"
//#include "diplib/overload.h"
//#include "diplib/iterators.h"

#include "interpolation.h"

namespace dip {

void Resampling(
      Image const& in,
      Image& out
) {
   ConstSampleIterator< sfloat > sfloatIn{ static_cast< sfloat* >( in.Origin()), in.Stride( 0 ) };
   SampleIterator< sfloat > sfloatOut{ static_cast< sfloat* >( out.Origin()), out.Stride( 0 ) };

   ConstSampleIterator< dfloat > dfloatIn{ static_cast< dfloat* >( in.Origin()), in.Stride( 0 ) };
   SampleIterator< dfloat > dfloatOut{ static_cast< dfloat* >( out.Origin()), out.Stride( 0 ) };

   ConstSampleIterator< scomplex > scomplexIn{ static_cast< scomplex* >( in.Origin()), in.Stride( 0 ) };
   SampleIterator< scomplex > scomplexOut{ static_cast< scomplex* >( out.Origin()), out.Stride( 0 ) };

   ConstSampleIterator< dcomplex > dcomplexIn{ static_cast< dcomplex* >( in.Origin()), in.Stride( 0 ) };
   SampleIterator< dcomplex > dcomplexOut{ static_cast< dcomplex* >( out.Origin()), out.Stride( 0 ) };

   dip::interpolation::BSpline< sfloat >( sfloatIn, sfloatOut, 100, 2.3, 1.2, nullptr, nullptr );
   dip::interpolation::BSpline< dfloat >( dfloatIn, dfloatOut, 100, 2.3, 1.2, nullptr, nullptr );
   dip::interpolation::BSpline< scomplex >( scomplexIn, scomplexOut, 100, 2.3, 1.2, nullptr, nullptr );
   dip::interpolation::BSpline< dcomplex >( dcomplexIn, dcomplexOut, 100, 2.3, 1.2, nullptr, nullptr );
   dip::interpolation::FourthOrderCubicSpline< sfloat >( sfloatIn, sfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::FourthOrderCubicSpline< dfloat >( dfloatIn, dfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::FourthOrderCubicSpline< scomplex >( scomplexIn, scomplexOut, 100, 2.3, 1.2 );
   dip::interpolation::FourthOrderCubicSpline< dcomplex >( dcomplexIn, dcomplexOut, 100, 2.3, 1.2 );
   dip::interpolation::ThirdOrderCubicSpline< sfloat >( sfloatIn, sfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::ThirdOrderCubicSpline< dfloat >( dfloatIn, dfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::ThirdOrderCubicSpline< scomplex >( scomplexIn, scomplexOut, 100, 2.3, 1.2 );
   dip::interpolation::ThirdOrderCubicSpline< dcomplex >( dcomplexIn, dcomplexOut, 100, 2.3, 1.2 );
   dip::interpolation::Linear< sfloat >( sfloatIn, sfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::Linear< dfloat >( dfloatIn, dfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::Linear< scomplex >( scomplexIn, scomplexOut, 100, 2.3, 1.2 );
   dip::interpolation::Linear< dcomplex >( dcomplexIn, dcomplexOut, 100, 2.3, 1.2 );
   dip::interpolation::NearestNeighbor< sfloat >( sfloatIn, sfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::NearestNeighbor< dfloat >( dfloatIn, dfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::NearestNeighbor< scomplex >( scomplexIn, scomplexOut, 100, 2.3, 1.2 );
   dip::interpolation::NearestNeighbor< dcomplex >( dcomplexIn, dcomplexOut, 100, 2.3, 1.2 );
   dip::interpolation::Lanczos< sfloat, 2 >( sfloatIn, sfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::Lanczos< dfloat, 2 >( dfloatIn, dfloatOut, 100, 2.3, 1.2 );
   dip::interpolation::Lanczos< scomplex, 5 >( scomplexIn, scomplexOut, 100, 2.3, 1.2 );
   dip::interpolation::Lanczos< dcomplex, 5 >( dcomplexIn, dcomplexOut, 100, 2.3, 1.2 );
}

} // namespace dip
