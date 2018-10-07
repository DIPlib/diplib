/*
 * DIPlib 3.0
 * This file contains the definition of corner detectors.
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/detection.h"
#include "diplib/analysis.h"
#include "diplib/linear.h"
#include "diplib/math.h"
#include "diplib/mapping.h"

namespace dip {

void HarrisCornerDetector(
      Image const& in,
      Image& out,
      dfloat kappa,
      FloatArray const& sigmas,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   Image S;
   DIP_STACK_TRACE_THIS( StructureTensor( in, {}, S, { 1.0 }, sigmas, S::BEST, boundaryCondition ));
   DIP_STACK_TRACE_THIS( Determinant( S, out ));
   Trace( S, S );
   Square( S, S );
   S *= kappa;
   out -= S;
   ClipLow( out, out, 0 );
}

void ShiTomasiCornerDetector(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   Image S;
   DIP_STACK_TRACE_THIS( StructureTensor( in, {}, S, { 1.0 }, sigmas, S::BEST, boundaryCondition ));
   DIP_STACK_TRACE_THIS( SmallestEigenvalue( S, out ));
}

void NobleCornerDetector(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   Image S;
   DIP_STACK_TRACE_THIS( StructureTensor( in, {}, S, { 1.0 }, sigmas, S::BEST, boundaryCondition ));
   DIP_STACK_TRACE_THIS( Determinant( S, out ));
   S = Trace( S );
   SafeDivide( out, S, out, out.DataType() );
}

void WangBradyCornerDetector(
      Image const& in,
      Image& out,
      dfloat threshold,
      FloatArray const& sigmas,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   Image grad;
   DIP_STACK_TRACE_THIS( Gradient( in, grad, sigmas, S::BEST, boundaryCondition ));
   SquareNorm( grad, grad ); // Instead of computing gradient magnitude and then squaring it, this avoids a dip::Sqrt call.
   grad *= threshold;
   DIP_STACK_TRACE_THIS( LaplaceMinusDgg( in, out, sigmas, S::BEST, boundaryCondition ));
   Square( out, out );
   out -= grad;
   ClipLow( out, out, 0 );
}

} // namespace dip
