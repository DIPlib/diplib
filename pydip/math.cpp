/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0
 *
 * (c)2017, Flagship Biosciences, Inc., written by Cris Luengo.
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

#include "pydip.h"
#include "diplib/display.h"
#include "diplib/math.h"

void init_math( py::module& m ) {
   m.def( "GetMaximumAndMinimum", []( dip::Image const& in, dip::Image const& mask ) {
                dip::MinMaxAccumulator acc = dip::GetMaximumAndMinimum( in, mask );
                return py::make_tuple( acc.Minimum(), acc.Maximum() );
          },
          "in"_a,
          "mask"_a = dip::Image{} );
   m.def( "GetSampleStatistics", []( dip::Image const& in, dip::Image const& mask ) {
                dip::StatisticsAccumulator acc = dip::GetSampleStatistics( in, mask );
                return py::make_tuple( acc.Mean(), acc.Variance(), acc.Skewness(), acc.ExcessKurtosis() );
          },
          "in"_a,
          "mask"_a = dip::Image{} );
   m.def( "Percentile", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::BooleanArray >( &dip::Percentile ),
          "in"_a,
          "mask"_a = dip::Image{},
          "percentile"_a = 50.0,
          "process"_a = dip::BooleanArray{} );
}
