/*
 * DIPlib 3.0
 * This file contains definitions for the monogenic signal and related functionality
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
#include "diplib/analysis.h"
#include "diplib/linear.h"
#include "diplib/transform.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/mapping.h"

namespace dip {

void MonogenicSignal(
      Image const& c_in,
      Image& out,
      FloatArray const& wavelengths,
      dfloat bandwidth,
      String const& inRepresentation,
      String const& outRepresentation
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( c_in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nFrequencyScales = wavelengths.size();
   DIP_THROW_IF( nFrequencyScales < 1, E::ARRAY_PARAMETER_EMPTY );
   DIP_THROW_IF( bandwidth <= 0, E::INVALID_PARAMETER );
   dip::uint nDims = c_in.Dimensionality();
   bool spatialDomainInput;
   DIP_STACK_TRACE_THIS( spatialDomainInput = BooleanFromString( inRepresentation, S::SPATIAL, S::FREQUENCY ));
   bool spatialDomainOutput;
   DIP_STACK_TRACE_THIS( spatialDomainOutput = BooleanFromString( outRepresentation, S::SPATIAL, S::FREQUENCY ));
   bool inputIsReal = spatialDomainInput && !c_in.DataType().IsComplex();
   bool ouputIsReal = inputIsReal && spatialDomainOutput;
   DataType dt = ouputIsReal ? DT_SFLOAT : DT_SCOMPLEX;
   Image in = c_in;
   if( out.Aliases( in )) {
      out.Strip(); // we can't work in-place
   }
   DIP_STACK_TRACE_THIS( out.ReForge( in.Sizes(), ( nDims + 1 ) * nFrequencyScales, dt ));
   out.ReshapeTensor( nDims + 1, nFrequencyScales );
   Image ftIn;
   if( spatialDomainInput ) {
      ftIn = FourierTransform( in );
   } else {
      ftIn = in.QuickCopy();
   }
   UnsignedArray center = in.Sizes();
   center /= 2;
   // Compute Riesz transform
   Image mono;
   DIP_STACK_TRACE_THIS( RieszTransform( ftIn, mono, S::FREQUENCY, S::FREQUENCY ));
   // Compute scale selection filters
   Image radialFilter;
   radialFilter.SetSizes( in.Sizes() ); // We'll use this non-forged image as input to get a filter bank not applied to an image
   DIP_STACK_TRACE_THIS( LogGaborFilterBank( radialFilter, radialFilter, wavelengths, bandwidth, 1, S::FREQUENCY, S::FREQUENCY ));
   // Options for inverse transform
   StringSet options = { S::INVERSE };
   if( spatialDomainOutput && ouputIsReal ) {
      options.insert( S::REAL );
   }
   // Get all combinations by multiplication
   for( dip::uint scale = 0; scale < nFrequencyScales; ++scale ) {
      {
         Image destination = out[ UnsignedArray{ 0, scale } ];
         destination.Protect();          // ensure it will not be reforged
         Image tempStorage;
         Image& ftDestination = ( spatialDomainOutput && ouputIsReal ) ? tempStorage : destination;
         DIP_STACK_TRACE_THIS( Multiply( radialFilter[ scale ], ftIn, ftDestination ));
         ftDestination.At( center ) = 0;
         if( spatialDomainOutput ) {
            DIP_STACK_TRACE_THIS( FourierTransform( ftDestination, destination, options ));
         }
      }
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         Image destination = out[ UnsignedArray{ ii + 1, scale } ];
         destination.Protect();          // ensure it will not be reforged
         Image tempStorage;
         Image& ftDestination = ( spatialDomainOutput && ouputIsReal ) ? tempStorage : destination;
         DIP_STACK_TRACE_THIS( Multiply( radialFilter[ scale ], mono[ ii ], ftDestination ));
         if( spatialDomainOutput ) {
            DIP_STACK_TRACE_THIS( FourierTransform( ftDestination, destination, options ));
         }
      }
   }
}

void MonogenicSignalAnalysis(
      Image const& in,
      ImageRefArray& out,
      StringArray const& outputs,
      dfloat noiseThreshold,
      dfloat frequencySpreadThreshold,
      dfloat sigmoidParameter,
      dfloat deviationGain,
      String const& polarity
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( in.DataType() != DT_SFLOAT, E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( in.TensorShape() != Tensor::Shape::COL_MAJOR_MATRIX || ( in.TensorRows() != nDims + 1 ), "Input must be a tensor image as produced by dip::MonogenicSignal" );
   dip::uint nScales = in.TensorColumns();
   dip::uint nOut = out.size();
   DIP_THROW_IF( outputs.size() != nOut, E::ARRAY_SIZES_DONT_MATCH );
   Image* congruency = nullptr;
   Image* orientation = nullptr;
   Image* phase = nullptr;
   Image* energy = nullptr;
   Image* symmetry = nullptr;
   Image* symenergy = nullptr;
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      if( outputs[ ii ] == "congruency" ) {
         congruency = &out[ ii ].get();
      } else if( outputs[ ii ] == "orientation" ) {
         DIP_THROW_IF( nDims != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         orientation = &out[ ii ].get();
      } else if( outputs[ ii ] == "phase" ) {
         phase = &out[ ii ].get();
      } else if( outputs[ ii ] == "energy" ) {
         energy = &out[ ii ].get();
      } else if( outputs[ ii ] == "symmetry" ) {
         symmetry = &out[ ii ].get();
      } else if( outputs[ ii ] == "symenergy" ) {
         symenergy = &out[ ii ].get();
      } else {
         DIP_THROW_INVALID_FLAG( outputs[ ii ] );
      }
   }
   if( congruency || symmetry ) {
      DIP_THROW_IF( noiseThreshold <= 0, E::INVALID_PARAMETER );
   }
   bool kovesi = false;
   if( congruency ) {
      if(( nDims == 2 ) && ( nScales > 2 )) {
         DIP_THROW_IF( frequencySpreadThreshold <= 0, E::INVALID_PARAMETER );
         DIP_THROW_IF( sigmoidParameter <= 0, E::INVALID_PARAMETER );
         DIP_THROW_IF( deviationGain <= 0, E::INVALID_PARAMETER );
         kovesi = true;
      } else {
         DIP_THROW_IF( nScales != 2, "Phase congruency for dimensionalities other than 2 can only be computed when given two scales." );
      }
   }
   dip::sint polarityValue = 0;
   if( symmetry || symenergy ) {
      if( polarity == S::BLACK ) {
         polarityValue = -1;
      } else if( polarity == S::WHITE ) {
         polarityValue = 1;
      } else if( polarity != S::BOTH ) {
         DIP_THROW_INVALID_FLAG( polarity );
      }
   }

   Image temp1;
   if( congruency && !energy ) {
      energy = &temp1;
   }
   Image temp2;
   if( symmetry && !symenergy ) {
      symenergy = &temp2;
   }

   // Accumulate data across scales
   Image thisScale = in.TensorColumn( 0 );
   Image sum;
   if( orientation || phase || energy ) {
      sum = Image( in.TensorColumn( 0 )).Copy();   // sum of individual components across scales
   }
   Image oddAmplitude = SquareNorm( thisScale[ Range( 1, -1 ) ] );
   Image sumAmplitude = thisScale[ 0 ] * thisScale[ 0 ];
   sumAmplitude += oddAmplitude;
   Sqrt( sumAmplitude, sumAmplitude );    // sum of amplitudes across scales
   Image maxAmplitude;
   if( congruency ) {
      maxAmplitude = sumAmplitude.Copy();          // maximum amplitude across scales
   }
   if( symenergy ) {
      if( polarityValue > 0 ) {              // S::WHITE
         ( *symenergy ).Copy( thisScale[ 0 ] );
      } else if( polarityValue < 0 ) {       // S::BLACK
         Invert( thisScale[ 0 ], *symenergy );
      } else {                               // S::BOTH
         Abs( thisScale[ 0 ], *symenergy );
      }
      Sqrt( oddAmplitude, oddAmplitude );
      Subtract( *symenergy, oddAmplitude, *symenergy );
   }
   for( dip::uint scale = 1; scale < nScales; ++scale ) {
      thisScale = in.TensorColumn( scale );
      oddAmplitude = SquareNorm( thisScale[ Range( 1, -1 ) ] );
      Image amp = thisScale[ 0 ] * thisScale[ 0 ];
      amp += oddAmplitude;
      Sqrt( amp, amp );
      sumAmplitude += amp;
      if( orientation || phase || energy ) {
         sum += in.TensorColumn( scale );
      }
      if( congruency ) {
         Supremum( maxAmplitude, amp, maxAmplitude );
      }
      if( symenergy ) {
         if( polarityValue == 1 ) {             // S::WHITE
            Add( *symenergy, thisScale[ 0 ], *symenergy );
         } else if( polarityValue < 0 ) {       // S::BLACK
            Subtract( *symenergy, thisScale[ 0 ], *symenergy );
         } else {                               // S::BOTH
            Add( *symenergy, Abs( thisScale[ 0 ] ), *symenergy );
         }
         Sqrt( oddAmplitude, oddAmplitude );
         Subtract( *symenergy, oddAmplitude, *symenergy );
      }
   }

   if( orientation ) {
      Orientation( sum[ Range( 1, 2 ) ], *orientation ); // specifically 2D.
   }
   if( phase ) {
      Atan2( sum[ 0 ], Norm( sum[ Range( 1, -1 ) ] ), *phase );
   }
   if( energy ) {
      Norm( sum, *energy );
   }

   if( congruency ) {

      if( kovesi ) {
         // Kovesi's method, 2D only

         // Compute the sigmoidal weighting
         Image width = maxAmplitude; // re-use the memory allocated for maxAmplitude, which we won't need any more.
         SafeDivide( sumAmplitude, maxAmplitude, width );
         width -= 1;
         width /= nScales - 1; // value related to the width of the frequency distribution, between 0 and 1.
         Image& weight = width; // again re-use memory
         weight = 1.0 / ( 1.0 + Exp(( frequencySpreadThreshold - width ) * sigmoidParameter )); // TODO: make this part better

         // Phase congruency
         SafeDivide( *energy, sumAmplitude, *congruency );
         Acos( *congruency, *congruency );
         *congruency *= deviationGain;
         Subtract( 1, *congruency, *congruency );
         Clip( *congruency, *congruency, 0, 0, S::LOW );
         MultiplySampleWise( weight, *congruency, *congruency );

         Image excessEnergy = *energy - noiseThreshold;
         Clip( excessEnergy, excessEnergy, 0, 0, S::LOW );
         SafeDivide( excessEnergy, *energy, excessEnergy );

         MultiplySampleWise( *congruency, excessEnergy, *congruency );

      } else {
         // Felsberg's method
         // TODO: repeating some computations here, refine code above to prevent this!

         Image dot = DotProduct( in.TensorColumn( 0 ), in.TensorColumn( 1 ));
         Image prodAmplitude = Norm( in.TensorColumn( 0 ));
         prodAmplitude *= Norm( in.TensorColumn( 1 ));

         Image sinphi = dot / prodAmplitude;
         Acos( sinphi, sinphi );
         Sin( sinphi, sinphi );
         sinphi += 1;
         sinphi *= prodAmplitude;

         Subtract( dot, noiseThreshold, *congruency );
         Clip( *congruency, *congruency, 0, 0, S::LOW );
         *congruency /= sinphi;

      }
   }

   if( symmetry ) {
      // Phase symmetry
      Subtract( *symenergy, noiseThreshold, *symmetry, DT_SFLOAT );
      Clip( *symmetry, *symmetry, 0, 0, S::LOW );
      SafeDivide( *symmetry, sumAmplitude, *symmetry );
   }
}

} // namespace dip
