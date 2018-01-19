/*
 * DIPlib 3.0
 * This file contains the definition for the various projection functions.
 *
 * (c)2017-2018, Cris Luengo, Erik Schuitema.
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

#include <cmath>

#include "diplib.h"
#include "diplib/statistics.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"
#include "diplib/accumulators.h"
#include "diplib/library/copy_buffer.h"
#include "diplib/geometry.h"

namespace dip {


namespace {

class ProjectionScanFunction {
   public:
      // The filter to be applied to each sub-image, which fills out a single sample in `out`. The `out` pointer
      // must be cast to the requested `outImageType` in the call to `ProjectionScan`.
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint thread ) = 0;
      // The derived class can define this function if it needs this information ahead of time.
      virtual void SetNumberOfThreads( dip::uint /*threads*/ ) {}
      // A virtual destructor guarantees that we can destroy a derived class by a pointer to base
      virtual ~ProjectionScanFunction() {}
};

void ProjectionScan(
      Image const& c_in,
      Image const& c_mask,
      Image& c_out,
      DataType outImageType,
      BooleanArray process,   // taken by copy so we can modify
      ProjectionScanFunction& function
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();

   // Check inputs
   if( process.empty() ) {
      // An empty process array means all dimensions are to be processed
      process.resize( nDims, true );
   } else {
      DIP_THROW_IF( process.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   }

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image input = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();
   String colorSpace = c_in.ColorSpace();
   Tensor outTensor = c_in.Tensor();

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   bool hasMask = false;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( inSizes, Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( inSizes );
      DIP_END_STACK_TRACE
      hasMask = true;
   }

   // Determine output sizes
   UnsignedArray outSizes = inSizes;
   UnsignedArray procSizes = inSizes;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( inSizes[ ii ] == 1 ) {
         process[ ii ] = false;
      }
      if( process[ ii ] ) {
         outSizes[ ii ] = 1;
      } else {
         procSizes[ ii ] = 1;
      }
   }

   // Is there anything to do?
   if( !process.any() ) {
      //std::cout << "Projection framework: nothing to do!" << std::endl;
      c_out = c_in; // This ignores the mask image
      return;
   }

   // Adjust output if necessary (and possible)
   DIP_START_STACK_TRACE
      if( c_out.IsOverlappingView( input ) || c_out.IsOverlappingView( mask )) {
         c_out.Strip();
      }
      c_out.ReForge( outSizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      // NOTE: Don't use c_in any more from here on. It has possibly been reforged!
      c_out.ReshapeTensor( outTensor );
   DIP_END_STACK_TRACE
   c_out.SetPixelSize( pixelSize );
   c_out.SetColorSpace( colorSpace );
   Image output = c_out.QuickCopy();

   // Do tensor to spatial dimension if necessary
   if( outTensor.Elements() > 1 ) {
      input.TensorToSpatial( 0 );
      if( hasMask ) {
         mask.TensorToSpatial( 0 );
      }
      output.TensorToSpatial( 0 );
      process.insert( 0, false );
      outSizes = output.Sizes(); // == outSizes.insert( 0, outTensor.Elements() );
      procSizes.insert( 0, 1 );
      nDims = outSizes.size();
   }

   // Do we need to loop at all?
   if( process.all() ) {
      //std::cout << "Projection framework: no need to loop!" << std::endl;
      function.SetNumberOfThreads( 1 );
      if( output.DataType() != outImageType ) {
         Image outBuffer( {}, 1, outImageType );
         function.Project( input, mask, outBuffer.Origin(), 0 );
         detail::CopyBuffer( outBuffer.Origin(), outBuffer.DataType(), 1, 1,
                             output.Origin(), output.DataType(), 1, 1, 1, 1 );
      } else {
         function.Project( input, mask, output.Origin(), 0 );
      }
      return;
   }

   // Can we treat the images as if they were 1D?
   // TODO: This is an opportunity for improving performance if the non-processing dimensions in in, mask and out have the same layout and simple stride

   // TODO: Determine the number of threads we'll be using. The size of the data has an influence. As is the number of sub-images that we can generate
   function.SetNumberOfThreads( 1 );

   // TODO: Start threads, each thread makes its own temp image.
   dip::uint thread = 0;

   // Create view over input image, that spans the processing dimensions
   Image tempIn;
   tempIn.CopyProperties( input );
   tempIn.SetSizes( procSizes );
   tempIn.dip__SetOrigin( input.Origin() );
   tempIn.Squeeze(); // we want to make sure that function.Project() won't be looping over singleton dimensions
   // TODO: instead of Squeeze, do a FlattenAsMuchAsPossible. But Mask must be flattened in the same way.
   // Create view over mask image, identically to input
   Image tempMask;
   if( hasMask ) {
      tempMask.CopyProperties( mask );
      tempMask.SetSizes( procSizes );
      tempMask.dip__SetOrigin( mask.Origin() );
      tempMask.Squeeze(); // keep in sync with tempIn.
   }
   // Create view over output image that doesn't contain the processing dimensions or other singleton dimensions
   Image tempOut;
   tempOut.CopyProperties( output );
   // Squeeze tempOut, but keep inStride, maskStride, outStride and outSizes in synch
   IntegerArray inStride = input.Strides();
   IntegerArray maskStride( nDims );
   if( hasMask ) {
      maskStride = mask.Strides();
   }
   IntegerArray outStride = output.Strides();
   dip::uint jj = 0;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( outSizes[ ii ] > 1 ) {
         inStride[ jj ] = inStride[ ii ];
         maskStride[ jj ] = maskStride[ ii ];
         outStride[ jj ] = outStride[ ii ];
         outSizes[ jj ] = outSizes[ ii ];
         ++jj;
      }
   }
   inStride.resize( jj );
   maskStride.resize( jj );
   outStride.resize( jj );
   outSizes.resize( jj );
   nDims = jj;
   tempOut.SetSizes( outSizes );
   tempOut.dip__SetOrigin( output.Origin() );
   // Create a temporary output buffer, to collect a single sample in the data type requested by the calling function
   bool useOutputBuffer = false;
   Image outBuffer;
   if( output.DataType() != outImageType ) {
      // We need a temporary space for the output sample also, because `function.Project` expects `outImageType`.
      outBuffer.SetDataType( outImageType );
      outBuffer.Forge(); // By default it's a single sample.
      useOutputBuffer = true;
   }

   // Iterate over the pixels in the output image. For each, we create a view in the input image.
   UnsignedArray position( nDims, 0 );
   for( ;; ) {

      // Do the thing
      if( useOutputBuffer ) {
         function.Project( tempIn, tempMask, outBuffer.Origin(), thread );
         // Copy data from output buffer to output image
         detail::CopyBuffer( outBuffer.Origin(), outBuffer.DataType(), 1, 1,
                             tempOut.Origin(), tempOut.DataType(), 1, 1, 1, 1 );
      } else {
         function.Project( tempIn, tempMask, tempOut.Origin(), thread );
      }

      // Next output pixel
      dip::uint dd;
      for( dd = 0; dd < nDims; dd++ ) {
         ++position[ dd ];
         tempIn.dip__ShiftOrigin( inStride[ dd ]);
         if( hasMask ) {
            tempMask.dip__ShiftOrigin( maskStride[ dd ]);
         }
         tempOut.dip__ShiftOrigin( outStride[ dd ]);
         // Check whether we reached the last pixel of the line
         if( position[ dd ] != outSizes[ dd ] ) {
            break;
         }
         // Rewind along this dimension
         tempIn.dip__ShiftOrigin( -inStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
         if( hasMask ) {
            tempMask.dip__ShiftOrigin( -maskStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
         }
         tempOut.dip__ShiftOrigin( -outStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
         position[ dd ] = 0;
         // Continue loop to increment along next dimension
      }
      if( dd == nDims ) {
         break;            // We're done!
      }
   }

   // TODO: End threads.
}

} // namespace


namespace {

template< typename TPI >
class ProjectionMean : public ProjectionScanFunction {
   public:
      ProjectionMean( bool computeMean ) : computeMean_( computeMean ) {}
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         dip::uint n = 0;
         FlexType< TPI > sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  sum += static_cast< FlexType< TPI >>( it.template Sample< 0 >() );
                  ++n;
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               sum += static_cast< FlexType< TPI >>( *it );
            } while( ++it );
            n = in.NumberOfPixels();
         }
         *static_cast< FlexType< TPI >* >( out ) = ( computeMean_ && ( n > 0 ))
                                                   ? ( sum / static_cast< FloatType< TPI >>( n ))
                                                   : ( sum );
      }
   private:
      bool computeMean_ = true;
};

template< typename TPI >
class ProjectionMeanDirectional : public ProjectionScanFunction {
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         DirectionalStatisticsAccumulator acc;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  acc.Push( static_cast< dfloat >( it.template Sample< 0 >() ));
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               acc.Push( static_cast< dfloat >( *it ));
            } while( ++it );
         }
         *static_cast< FloatType< TPI >* >( out ) = static_cast< FloatType< TPI >>( acc.Mean() ); // Is the same as FlexType< TPI > because TPI is not complex here.
      }
};

} // namespace

void Mean(
      Image const& in,
      Image const& mask,
      Image& out,
      String const& mode,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if( mode == S::DIRECTIONAL ) {
      DIP_OVL_NEW_FLOAT( lineFilter, ProjectionMeanDirectional, (), in.DataType() );
   } else {
      DIP_OVL_NEW_ALL( lineFilter, ProjectionMean, ( true ), in.DataType() );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *lineFilter );
}

void Sum(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, ProjectionMean, ( false ), in.DataType() );
   ProjectionScan( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *lineFilter );
}

namespace {

template< typename TPI >
class ProjectionProduct : public ProjectionScanFunction {
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         FlexType< TPI > product = 1.0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  product *= static_cast< FlexType< TPI >>( it.template Sample< 0 >() );
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               product *= static_cast< FlexType< TPI >>( *it );
            } while( ++it );
         }
         *static_cast< FlexType< TPI >* >( out ) = product;
      }
};

} // // namespace

void Product(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, ProjectionProduct, (), in.DataType() );
   ProjectionScan( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *lineFilter );
}

namespace {

template< typename TPI >
class ProjectionMeanAbs : public ProjectionScanFunction {
   public:
      ProjectionMeanAbs( bool computeMean ) : computeMean_( computeMean ) {}
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         dip::uint n = 0;
         FloatType< TPI > sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  sum += std::abs( static_cast< FlexType< TPI >>( it.template Sample< 0 >() ));
                  ++n;
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               sum += std::abs( static_cast< FlexType< TPI >>( *it ));
            } while( ++it );
            n = in.NumberOfPixels();
         }
         *static_cast< FloatType< TPI >* >( out ) = ( computeMean_ && ( n > 0 ))
                                                    ? ( sum / static_cast< FloatType< TPI >>( n ))
                                                    : ( sum );
      }
   private:
      bool computeMean_ = true;
};

} // namespace

void MeanAbs(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if( in.DataType().IsUnsigned() ) {
      DIP_OVL_NEW_UNSIGNED( lineFilter, ProjectionMean, ( true ), in.DataType() );
   } else {
      DIP_OVL_NEW_SIGNED( lineFilter, ProjectionMeanAbs, ( true ), in.DataType() );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *lineFilter );
}

void SumAbs(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if( in.DataType().IsUnsigned() ) {
      DIP_OVL_NEW_UNSIGNED( lineFilter, ProjectionMean, ( false ), in.DataType() );
   } else {
      DIP_OVL_NEW_SIGNED( lineFilter, ProjectionMeanAbs, ( false ), in.DataType() );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *lineFilter );
}

namespace {

template< typename TPI >
class ProjectionMeanSquare : public ProjectionScanFunction {
   public:
      ProjectionMeanSquare( bool computeMean ) : computeMean_( computeMean ) {}
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         dip::uint n = 0;
         FlexType< TPI > sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  FlexType< TPI > v = static_cast< FlexType< TPI >>( it.template Sample< 0 >() );
                  sum += v * v;
                  ++n;
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               FlexType< TPI > v = static_cast< FlexType< TPI >>( *it );
               sum += v * v;
            } while( ++it );
            n = in.NumberOfPixels();
         }
         *static_cast< FlexType< TPI >* >( out ) = ( computeMean_ && ( n > 0 ))
                                                   ? ( sum / static_cast< FloatType< TPI >>( n ))
                                                   : ( sum );
      }
   private:
      bool computeMean_ = true;
};

} // namespace

void MeanSquare(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if( in.DataType().IsBinary() ) {
      DIP_OVL_NEW_BINARY( lineFilter, ProjectionMean, ( true ), DT_BIN );
   } else {
      DIP_OVL_NEW_NONBINARY( lineFilter, ProjectionMeanSquare, ( true ), in.DataType() );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *lineFilter );
}

void SumSquare(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if( in.DataType().IsBinary() ) {
      DIP_OVL_NEW_BINARY( lineFilter, ProjectionMean, ( false ), DT_BIN );
   } else {
      DIP_OVL_NEW_NONBINARY( lineFilter, ProjectionMeanSquare, ( false ), in.DataType() );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *lineFilter );
}

namespace {

template< typename TPI, typename ACC >
class ProjectionVariance : public ProjectionScanFunction {
   public:
      ProjectionVariance( bool computeStD ) : computeStD_( computeStD ) {}
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         ACC acc;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  acc.Push( static_cast< dfloat >( it.template Sample< 0 >() ));
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               acc.Push( static_cast< dfloat >( *it ));
            } while( ++it );
         }
         *static_cast< FloatType< TPI >* >( out ) = clamp_cast< FloatType< TPI >>(
               computeStD_ ? acc.StandardDeviation() : acc.Variance() );
      }
   private:
      bool computeStD_ = true;
};

template< typename TPI >
inline std::unique_ptr< ProjectionScanFunction > NewProjectionVarianceStable( bool computeStD ) {
   return static_cast< std::unique_ptr< ProjectionScanFunction >>( new ProjectionVariance< TPI, VarianceAccumulator >( computeStD ));
}
template< typename TPI >
inline std::unique_ptr< ProjectionScanFunction > NewProjectionVarianceFast( bool computeStD ) {
   return static_cast< std::unique_ptr< ProjectionScanFunction >>( new ProjectionVariance< TPI, FastVarianceAccumulator >( computeStD ));
}
template< typename TPI >
inline std::unique_ptr< ProjectionScanFunction > NewProjectionVarianceDirectional( bool computeStD ) {
   return static_cast< std::unique_ptr< ProjectionScanFunction >>( new ProjectionVariance< TPI, DirectionalStatisticsAccumulator >( computeStD ));
}

} // namespace

void Variance(
      Image const& in,
      Image const& mask,
      Image& out,
      String mode,
      BooleanArray const& process
) {
   // TODO: This exists also for complex numbers, yielding a real value
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if(( in.DataType().SizeOf() <= 2 ) && ( mode == S::STABLE )) {
      mode = S::FAST;
   }
   if( mode == S::STABLE ) {
      DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewProjectionVarianceStable, ( false ), in.DataType() );
   } else if( mode == S::FAST ) {
      DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewProjectionVarianceFast, ( false ), in.DataType() );
   } else if( mode == S::DIRECTIONAL ) {
      DIP_OVL_CALL_ASSIGN_FLOAT( lineFilter, NewProjectionVarianceDirectional, ( false ), in.DataType() );
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *lineFilter );
}

void StandardDeviation(
      Image const& in,
      Image const& mask,
      Image& out,
      String mode,
      BooleanArray const& process
) {
   // TODO: This exists also for complex numbers, yielding a real value
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if(( in.DataType().SizeOf() <= 2 ) && ( mode == S::STABLE )) {
      mode = S::FAST;
   }
   if( mode == S::STABLE ) {
      DIP_OVL_CALL_ASSIGN_FLOAT( lineFilter, NewProjectionVarianceStable, ( true ), in.DataType() );
   } else if( mode == S::FAST ) {
      DIP_OVL_CALL_ASSIGN_FLOAT( lineFilter, NewProjectionVarianceFast, ( true ), in.DataType() );
   } else if( mode == S::DIRECTIONAL ) {
      DIP_OVL_CALL_ASSIGN_FLOAT( lineFilter, NewProjectionVarianceDirectional, ( true ), in.DataType() );
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *lineFilter );
}

namespace {

template< typename TPI >
class ProjectionMaximum : public ProjectionScanFunction {
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         TPI max = std::numeric_limits< TPI >::lowest();
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  max = std::max( max, it.template Sample< 0 >() );
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               max = std::max( max, *it );
            } while( ++it );
         }
         *static_cast< TPI* >( out ) = max;
      }
};

} // namespace

void Maximum(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionMaximum, (), in.DataType() );
   ProjectionScan( in, mask, out, in.DataType(), process, *lineFilter );
}

namespace {

template< typename TPI >
class ProjectionMinimum : public ProjectionScanFunction {
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         TPI min = std::numeric_limits< TPI >::max();
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  min = std::min( min, it.template Sample< 0 >() );
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               min = std::min( min, *it );
            } while( ++it );
         }
         *static_cast< TPI* >( out ) = min;
      }
};

} // namespace

void Minimum(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionMinimum, (), in.DataType() );
   ProjectionScan( in, mask, out, in.DataType(), process, *lineFilter );
}

namespace {

template< typename TPI >
class ProjectionMaximumAbs : public ProjectionScanFunction {
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         AbsType< TPI > max = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  max = std::max( max, static_cast< AbsType< TPI >>( abs( it.template Sample< 0 >() )));
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               max = std::max( max, static_cast< AbsType< TPI >>( abs( *it )));
            } while( ++it );
         }
         *static_cast< AbsType< TPI >* >( out ) = max;
      }
};

} // namespace

void MaximumAbs(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DataType dt = in.DataType();
   if( dt.IsUnsigned() ) {
      DIP_OVL_NEW_UNSIGNED( lineFilter, ProjectionMaximum, (), dt );
   } else {
      DIP_OVL_NEW_SIGNED( lineFilter, ProjectionMaximumAbs, (), dt );
   }
   dt = DataType::SuggestAbs( dt );
   ProjectionScan( in, mask, out, dt, process, *lineFilter );
}

namespace {

template< typename TPI >
class ProjectionMinimumAbs : public ProjectionScanFunction {
      using TPO = AbsType< TPI >;
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         AbsType< TPI > min = std::numeric_limits< AbsType< TPI >>::max();
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  min = std::min( min, static_cast< AbsType< TPI >>( abs( it.template Sample< 0 >() )));
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               min = std::min( min, static_cast< AbsType< TPI >>( abs( *it )));
            } while( ++it );
         }
         *static_cast< AbsType< TPI >* >( out ) = min;
      }
};

} // namespace

void MinimumAbs(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DataType dt = in.DataType();
   if( dt.IsUnsigned() ) {
      DIP_OVL_NEW_UNSIGNED( lineFilter, ProjectionMinimum, (), dt );
   } else {
      DIP_OVL_NEW_SIGNED( lineFilter, ProjectionMinimumAbs, (), dt );
   }
   dt = DataType::SuggestAbs( dt );
   ProjectionScan( in, mask, out, dt, process, *lineFilter );
}

namespace {

template< typename TPI >
class ProjectionPercentile : public ProjectionScanFunction {
   public:
      ProjectionPercentile( dfloat percentile ) : percentile_( percentile ) {}
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint thread ) override {
         dip::uint N;
         if( mask.IsForged() ) {
            N = Count( mask );
         } else {
            N = in.NumberOfPixels();
         }
         if( N == 0 ) {
            *static_cast< TPI* >( out ) = TPI{};
            return;
         }
         dip::sint rank = round_cast( static_cast< dfloat >(N - 1) * percentile_ / 100.0 );
         buffer_[ thread ].resize( N );
#if 0 // Strategy 1: Copy data to buffer, and partition at the same time. The issue is finding a good pivot
         auto leftIt = buffer_[ thread ].begin();
         auto rightIt = buffer_[ thread ].rbegin();
         TPI pivot{};
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() ) {
                  pivot = it.template Sample< 0 >();
                  ++it;
                  break;
               }
            } while( ++it );
            do {
               if( it.template Sample< 1 >() ) {
                  TPI v = it.template Sample< 0 >();
                  if( v < pivot ) {
                     *( leftIt++ ) = v;
                  } else {
                     *( rightIt++ ) = v;
                  }
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            pivot = *( it++ );
            do {
               TPI v = *it;
               if( v < pivot ) {
                  *( leftIt++ ) = v;
               } else {
                  *( rightIt++ ) = v;
               }
            } while( ++it );
         }
         DIP_ASSERT( &*leftIt == &*rightIt ); // They should both be pointing to the same array element.
         *leftIt = pivot;
         auto ourGuy = buffer_[ thread ].begin() + rank;
         if( ourGuy < leftIt ) {
            // our guy is to the left
            std::nth_element( buffer_[ thread ].begin(), ourGuy, leftIt );
         } else if( ourGuy > leftIt ){
            // our guy is to the right
            std::nth_element( ++leftIt, ourGuy, buffer_[ thread ].end() );
         } // else: ourGuy == leftIt, which is already sorted correctly.
#else // Strategy 1: Copy data to buffer, let std lib do the partitioning using its OK pivot strategy
         auto outIt = buffer_[ thread ].begin();
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  *( outIt++ ) = it.template Sample< 0 >();
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               *( outIt++ ) = *it;
            } while( ++it );
         }
         auto ourGuy = buffer_[ thread ].begin() + rank;
         std::nth_element( buffer_[ thread ].begin(), ourGuy, buffer_[ thread ].end() );
#endif
         *static_cast< TPI* >( out ) = *ourGuy;
      }
      void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }
   private:
      std::vector< std::vector< TPI >> buffer_;
      dfloat percentile_;
};

} // namespace

void Percentile(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat percentile,
      BooleanArray const& process
) {
   DIP_THROW_IF(( percentile < 0.0 ) || ( percentile > 100.0 ), E::PARAMETER_OUT_OF_RANGE );
   if( percentile == 0.0 ) {
      Minimum( in, mask, out, process );
   } else if( percentile == 100.0 ) {
      Maximum( in, mask, out, process );
   } else {
      std::unique_ptr< ProjectionScanFunction > lineFilter;
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionPercentile, ( percentile ), in.DataType() );
      ProjectionScan( in, mask, out, in.DataType(), process, *lineFilter );
   }
}

namespace {

template< typename TPI >
class ProjectionAll : public ProjectionScanFunction {
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         bool all = true;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() && ( it.template Sample< 0 >() == TPI( 0 ))) {
                  all = false;
                  break;
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               if( *it == TPI( 0 )) {
                  all = false;
                  break;
               }
            } while( ++it );
         }
         *static_cast< bin* >( out ) = all;
      }
};

} // namespace

void All(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, ProjectionAll, (), in.DataType() );
   ProjectionScan( in, mask, out, DT_BIN, process, *lineFilter );
}

namespace {

template< typename TPI >
class ProjectionAny : public ProjectionScanFunction {
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         bool any = false;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() && ( it.template Sample< 0 >() != TPI( 0 ))) {
                  any = true;
                  break;
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               if( *it != TPI( 0 )) {
                  any = true;
                  break;
               }
            } while( ++it );
         }
         *static_cast< bin* >( out ) = any;
      }
};

} // namespace

void Any(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, ProjectionAny, (), in.DataType() );
   ProjectionScan( in, mask, out, DT_BIN, process, *lineFilter );
}


namespace {

// `CompareOp` is the compare operation
template< typename TPI, typename CompareOp >
class ProjectionPositionMinMax : public ProjectionScanFunction {
   public:
      // `limitInitVal` is the initialization value of the variable that tracks the limit value
      // For finding a minimum value, initialize with std::numeric_limits< TPI >::max(),
      // for finding a maximum value, initialize with std::numeric_limits< TPI >::lowest().
      ProjectionPositionMinMax( TPI limitInitVal ) : limitInitVal_( limitInitVal ) {}

      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         // Keep track of the limit (min or max) value
         TPI limit = limitInitVal_;
         dip::UnsignedArray limitCoords( in.Dimensionality(), 0 ); // Coordinates of the pixel with min/max value
         if( mask.IsForged() ) {
            // With mask
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() ) {
                  if( compareOp_( it.template Sample< 0 >(), limit )) {
                     limit = it.template Sample< 0 >();
                     limitCoords = it.Coordinates();
                  }
               }
            } while( ++it );
         }
         else {
            // Without mask
            ImageIterator< TPI > it( in );
            do {
               if( compareOp_( *it, limit )) {
                  limit = *it;
                  limitCoords = it.Coordinates();
               }
            } while( ++it );
         }
         // Store coordinate.
         // Currently, only a single processing dim is supported, so only one coordinate is stored.
         *static_cast< dip::uint32* >( out ) = static_cast< dip::uint32 >( limitCoords.front() );
      }

   protected:
      TPI limitInitVal_;
      CompareOp compareOp_;   // Compare functor
};

// First maximum: compare with '>' and init with lowest()
template< typename TPI >
class ProjectionPositionFirstMaximum: public ProjectionPositionMinMax< TPI, std::greater< TPI >> {
   public:
      ProjectionPositionFirstMaximum(): ProjectionPositionMinMax< TPI, std::greater< TPI >>( std::numeric_limits< TPI >::lowest() ) {}
};

// Last maximum: compare with '>=' and init with lowest()
template< typename TPI >
class ProjectionPositionLastMaximum: public ProjectionPositionMinMax< TPI, std::greater_equal< TPI >> {
   public:
      ProjectionPositionLastMaximum(): ProjectionPositionMinMax< TPI, std::greater_equal< TPI >>( std::numeric_limits< TPI >::lowest() ) {}
};

// First minimum: compare with '<' and init with max()
template< typename TPI >
class ProjectionPositionFirstMinimum: public ProjectionPositionMinMax< TPI, std::less< TPI >> {
   public:
      ProjectionPositionFirstMinimum(): ProjectionPositionMinMax< TPI, std::less< TPI >>( std::numeric_limits< TPI >::max() ) {}
};

// Last minimum: compare with '<=' and init with max()
template< typename TPI >
class ProjectionPositionLastMinimum: public ProjectionPositionMinMax< TPI, std::less_equal< TPI >> {
   public:
      ProjectionPositionLastMinimum(): ProjectionPositionMinMax< TPI, std::less_equal< TPI >>( std::numeric_limits< TPI >::max() ) {}
};

void PositionMinMax(
      Image const& in,
      Image const& mask,
      Image& out,
      bool maximum,
      dip::uint dim,
      String const& mode
) {
   DIP_THROW_IF( dim >= in.Dimensionality(), E::ILLEGAL_DIMENSION );

   // Create processing boolean array from the single processing dim
   BooleanArray process( in.Dimensionality(), false );
   process[ dim ] = true;

   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if( maximum ) {
      if( mode == S::FIRST ) {
         DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionPositionFirstMaximum, (), in.DataType() );
      } else if( mode == S::LAST ) {
         DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionPositionLastMaximum, (), in.DataType() );
      } else {
         DIP_THROW( "Unsupported mode for PositionMaximum: " + mode );
      }
   } else { // minimum
      if( mode == S::FIRST ) {
         DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionPositionFirstMinimum, (), in.DataType() );
      } else if( mode == S::LAST ) {
         DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionPositionLastMinimum, (), in.DataType() );
      } else {
         DIP_THROW( "Unsupported mode for PositionMinimum: " + mode );
      }
   }

   // Positions in the out image will be of type DT_UINT32
   ProjectionScan( in, mask, out, DT_UINT32, process, *lineFilter );
}

} // namespace

void PositionMaximum(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint dim,
      String const& mode
) {
   PositionMinMax( in, mask, out, true, dim, mode );
}

void PositionMinimum(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint dim,
      String const& mode
) {
   PositionMinMax( in, mask, out, false, dim, mode );
}


namespace {

template< typename TPI >
class ProjectionPositionPercentile : public ProjectionScanFunction {
   public:
      ProjectionPositionPercentile( dfloat percentile, bool findFirst ) : percentile_( percentile ), findFirst_( findFirst ) {}

      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         // Create a copy of the input image line (single dimension) that can be sorted to find the percentile value
         std::vector< TPI > inBuffer;
         dip::UnsignedArray percentileCoords( in.Dimensionality(), 0 ); // Coordinates of the pixel with min/max value
         // With mask..
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() ) {
                  inBuffer.push_back( it.template Sample< 0 >() );
               }
            } while( ++it );

            if( !inBuffer.empty() ) {
               TPI rankedValue = GetRankedValue( inBuffer );

               // Find the position of the ranked element within the masked pixels
               it.Reset();
               do {
                  if( it.template Sample< 1 >() && ( it.template Sample< 0 >() == rankedValue )) {
                     percentileCoords = it.Coordinates();
                     if( findFirst_ ) {
                        break;
                     }
                  }
               } while( ++it );
            } else {
               // No elements in the buffer; store 0
               percentileCoords.fill( 0 );
            }

         } else {
            // Without mask..
            inBuffer.resize( in.NumberOfPixels() );
            TPI* inBufferPtr = inBuffer.data();
            ImageIterator< TPI > it( in );
            do {
               *inBufferPtr = *it;
               ++inBufferPtr;
            } while( ++it );

            TPI rankedValue = GetRankedValue( inBuffer );

            // Find the position of the ranked element
            it.Reset();
            do {
               if( *it == rankedValue ) {
                  percentileCoords = it.Coordinates();
                  if( findFirst_ ) {
                     break;
                  }
               }
            } while( ++it );

         }
         // Store coordinate.
         // Currently, only a single processing dim is supported, so only one coordinate is stored.
         *static_cast< dip::uint32* >( out ) = static_cast< dip::uint32 >( percentileCoords.front() );
      }

   protected:
      dfloat percentile_;
      bool findFirst_;

      // Get the value in the buffer with rank according to percentile_
      // `buffer` must be non-empty
      TPI GetRankedValue( std::vector< TPI >& buffer ) const {
         dip::sint rank = round_cast( static_cast< dfloat >( buffer.size() - 1 ) * percentile_ / 100.0 );
         auto rankedElement = buffer.begin() + rank;
         std::nth_element( buffer.begin(), rankedElement, buffer.end() );
         return *rankedElement;
      }

};

} // namespace

void PositionPercentile(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat percentile,
      dip::uint dim,
      String const& mode
) {
   DIP_THROW_IF(( percentile < 0.0 ) || ( percentile > 100.0 ), E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( dim >= in.Dimensionality(), E::ILLEGAL_DIMENSION );

   // A percentile of 0.0 means minimum, 100.0 means maximum
   if( percentile == 0.0 ) {
      PositionMinimum( in, mask, out, dim, mode );
   } else if( percentile == 100.0 ) {
      PositionMaximum( in, mask, out, dim, mode );
   } else {
      // Create processing boolean array from the single processing dim
      BooleanArray process( in.Dimensionality(), false );
      process[ dim ] = true;

      // Do the actual position-percentile computation
      std::unique_ptr< ProjectionScanFunction > lineFilter;
      if( mode == S::FIRST ) {
         DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionPositionPercentile, ( percentile, true ), in.DataType() );
      } else if( mode == S::LAST ) {
         DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionPositionPercentile, ( percentile, false ), in.DataType() );
      } else {
         DIP_THROW( "Unsupported mode for PositionPercentile: " + mode );
      }

      // Positions in the out image will be of type DT_UINT32
      ProjectionScan( in, mask, out, DT_UINT32, process, *lineFilter );
   }
}


namespace {

class ProjectionRadialBase: public ProjectionScanFunction {
   public:
      // Reduce the outputs of all threads to a single output
      virtual void Reduce() {}
};

// The projection framework accepts a 'process' array, but cannot add the radial output dimension to the output image.
// Therefore, we use the projection framework and store the output image in the filter object.
// 
// We could use the scanlinefilter framework here, but it doesn't support a 'process' array,
// and the output image is of the wrong size.
template< typename TPI, typename TPO >
class ProjectionRadial : public ProjectionRadialBase {
   public:
      ProjectionRadial( Image& out, dfloat binSize, FloatArray center ) : out_( out ), binSize_( binSize ), center_( std::move( center )) {}

      virtual void Project( Image const& in, Image const& mask, void*, dip::uint thread ) override {

         // Obtain local output image
         // The output of thread 0 is stored in out_; the output of the other threads is stored in outPerThread_
         Image& out = thread == 0 ? out_ : outPerThread_[ thread - 1 ];
         // Force output image (once) and initialize
         if( !out.IsForged() ) {
            out.Forge();
            InitializeOutputImage( out, thread );
         }

         dip::uint procDim = Framework::OptimalProcessingDim( in );
         dip::uint inTensorLength = in.TensorElements();

         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > itMaskedLine( { in, mask }, procDim );   // Iterate over image lines
            do {
               // Compute squared distance from pixel to origin in all dimensions except the processing dim
               dfloat sqrDist = GetPartialSquaredDist( itMaskedLine.Coordinates(), procDim );

               ConstLineIterator< TPI > itPixel = itMaskedLine.template GetConstLineIterator< 0 >();
               ConstLineIterator< bin > itMaskPixel = itMaskedLine.template GetConstLineIterator< 1 >();
               do {
                  if( *itMaskPixel ) { // Masked?
                     // Get bin index
                     dip::uint binIndex = GetBinIndex( itPixel.Coordinate(), procDim, sqrDist );
                     // Verify that the bin is within range (for the INNERRADIUS option, not all pixels are processed)
                     if( binIndex < out.Size( 0 ) ) {
                        ProcessPixel( binIndex, itPixel.begin().Pointer(), static_cast< TPO* >( out.At< TPO >( binIndex ).Origin() ), inTensorLength );
                     }
                  }

               } while( ++itPixel, ++itMaskPixel );

            } while( ++itMaskedLine );
         } else {
            ImageIterator< TPI > itLine( in, procDim );   // Iterate over image lines
            do {
               // Compute squared distance from pixel to origin in all dimensions except the processing dim
               dfloat sqrDist = GetPartialSquaredDist( itLine.Coordinates(), procDim );

               // Iterate over the pixels in the image line
               ConstLineIterator< TPI > itPixel = itLine.GetConstLineIterator();
               do {
                  // Get bin index
                  dip::uint binIndex = GetBinIndex( itPixel.Coordinate(), procDim, sqrDist );
                  // Verify that the bin is within range (for the INNERRADIUS option, not all pixels are processed)
                  if( binIndex < out.Size( 0 ) ) {
                     ProcessPixel( binIndex, itPixel.begin().Pointer(), static_cast< TPO* >( out.At< TPO >( binIndex ).Origin() ), inTensorLength );
                  }
               } while( ++itPixel );

            } while( ++itLine );
         }
      }

      // Initialize the output image.
      // This function is called once for each thread's output image.
      virtual void InitializeOutputImage( Image& out, dip::uint thread ) = 0;

      // Process one pixel
      virtual void ProcessPixel( dip::uint binIndex, const TPI* pIn, TPO* pOut, dip::uint inTensorLength ) = 0;

      // Set number of threads. The output per thread is prepared here
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         // Allocate space for output values. Start at 1, because thread 0 is stored in image_.
         for( dip::uint ii = 1; ii < threads; ++ii ) {
            outPerThread_.emplace_back( out_ );       // makes a copy; image_ is not yet forged, so data is not shared.
         }
         // We don't forge the images here, the Filter() function should do that so each thread allocates its own
         // data segment. This ensures there's no false sharing.
      }

   protected:
      Image& out_;   // Output image for thread 0
      ImageArray outPerThread_;  // Only non-empty when num threads > 1
      dfloat binSize_;  // Bin size of the radial statistics output
      FloatArray center_;// Convert to the following to include scaling: TransformationArray transformation_;

      // Compute squared distance from pixel to origin in all dimensions except one: the processing dim
      dfloat GetPartialSquaredDist( UnsignedArray const& lineOriginCoords, dip::uint dimToSkip ) const {
         dfloat sqrDist = 0;
         for( dip::uint ii = 0; ii < center_.size(); ++ii ) {
            if( ii != dimToSkip ) {
               dfloat dist = static_cast< dfloat >(lineOriginCoords[ ii ]) - center_[ ii ];
               sqrDist += dist * dist;
            }
         }
         return sqrDist;
      }

      dip::uint GetBinIndex( dip::uint procDimCoordinate, dip::uint procDim, dfloat partialSqrDist ) const {
         dfloat dist = static_cast< dfloat >( procDimCoordinate ) - center_[ procDim ];
         dfloat radius = std::sqrt( partialSqrDist + dist * dist );
         return static_cast< dip::uint >( std::floor( radius / binSize_ ));
      }

};

// Radial sum filter
template< typename TPI, typename TPO = DoubleType< TPI >>
class ProjectionRadialSum : public ProjectionRadial< TPI, TPO > {
   public:
      ProjectionRadialSum( Image& out, dfloat binSize, FloatArray const& center ) :
            ProjectionRadial< TPI, TPO >( out, binSize, center ) {}

      virtual void InitializeOutputImage( Image& out, dip::uint ) override {
         // Initialize with zeros
         out.Fill( 0 );
      }

      virtual void ProcessPixel( dip::uint, const TPI* pIn, TPO* pOut, dip::uint inTensorLength ) override {
         for( dip::uint iT = 0; iT < inTensorLength; ++iT, ++pIn, ++pOut ) {
            *pOut += *pIn;
         }
      }

      virtual void Reduce() override {
         // Take sum of all images
         for( auto const& out : outPerThread_ ) {
            out_ += out;
         }
      }
   protected:
      using ProjectionRadial< TPI, TPO >::outPerThread_;
      using ProjectionRadial< TPI, TPO >::out_;
};

// Radial mean filter
template< typename TPI, typename TPO = DoubleType< TPI >>
class ProjectionRadialMean : public ProjectionRadialSum< TPI, TPO > {
public:
   ProjectionRadialMean( Image& out, dfloat binSize, FloatArray const& center ) :
         ProjectionRadialSum< TPI, TPO >( out, binSize, center ) {}

   virtual void ProcessPixel( dip::uint, const TPI* pIn, TPO* pOut, dip::uint inTensorLength ) override {
      for( dip::uint iT = 0; iT < inTensorLength; ++iT, ++pIn, ++pOut ) {
         *pOut += *pIn;
      }
      // The output pixel contains an extra tensor element to store the bin count
      *pOut += 1.0;   // If the output sample type is complex, the bin count is in the real part
   }

   virtual void Reduce() override {
      // Take sum of all images
      for( auto const& out : outPerThread_ ) {
         out_ += out;
      }
      // The last sample in each pixel contains the bin count.
      // Divide the other tensor elements by the last one to obtain the mean.
      ImageIterator< TPO > itOut( out_ );
      do {
         TPO count = *(--itOut.cend() );
         for( auto itSample = itOut.begin(); itSample != --itOut.end(); ++itSample ) {
            if( count != 0.0 ) {
               // Store the mean
               *itSample /= count;
            }
            else {
               // The bin is empty: store zero
               *itSample = 0.0;
            }
         }
      } while( ++itOut );
   }
protected:
   using ProjectionRadialSum< TPI, TPO >::outPerThread_;
   using ProjectionRadialSum< TPI, TPO >::out_;
};

// Radial min/max filter template
// `CompareOp` is the compare operation: std::less or std::greater
template< typename TPI, typename CompareOp >
class ProjectionRadialMinMax : public ProjectionRadial< TPI, TPI > {
   public:
      // `limitInitVal` is the initialization value of the variable that tracks the limit value
      // For finding a minimum value, initialize with std::numeric_limits< TPI >::max(),
      // for finding a maximum value, initialize with std::numeric_limits< TPI >::lowest().
      ProjectionRadialMinMax( Image& out, dfloat binSize, FloatArray const& center, TPI limitInitVal ) :
            ProjectionRadial< TPI, TPI >( out, binSize, center ), limitInitVal_( limitInitVal ) {}

      virtual void InitializeOutputImage( Image& out, dip::uint ) override {
         // Initialize with limitInitVal_
         out.Fill( limitInitVal_ );
      }

      virtual void ProcessPixel( dip::uint, const TPI* pIn, TPI* pOut, dip::uint tensorLength ) override {
         for( dip::uint iT = 0; iT < tensorLength; ++iT, ++pIn, ++pOut ) {
            if ( compareOp_(*pIn, *pOut) )
               *pOut = *pIn;
         }
      }

      virtual void Reduce() override {
         // Take limit of all images
         //TODO: does it help to use Supremum() and Infimum() here?
         for( dip::uint iOut = 0; iOut < outPerThread_.size(); ++iOut ) {
            JointImageIterator< TPI, TPI > itOut( { out_, outPerThread_[ iOut ] } );
            do {
               if( compareOp_( itOut.template Sample< 1 >(), itOut.template Sample< 0 >() ) ) {
                  itOut.template Sample< 0 >() = itOut.template Sample< 1 >();
               }
            } while( ++itOut );
         }
      }


   protected:
      TPI limitInitVal_;
      CompareOp compareOp_;   // Compare functor
      using ProjectionRadial< TPI, TPI >::outPerThread_;
      using ProjectionRadial< TPI, TPI >::out_;
};

// Radial min filter
template< typename TPI >
class ProjectionRadialMin: public ProjectionRadialMinMax< TPI, std::less< TPI >> {
   public:
      ProjectionRadialMin( Image& out, dfloat binSize, FloatArray const& center ):
            ProjectionRadialMinMax< TPI, std::less< TPI >>( out, binSize, center, std::numeric_limits< TPI >::max() ) {}
};

// Radial max filter
template< typename TPI >
class ProjectionRadialMax : public ProjectionRadialMinMax< TPI, std::greater< TPI >> {
   public:
      ProjectionRadialMax( Image& out, dfloat binSize, FloatArray const& center ) :
            ProjectionRadialMinMax< TPI, std::greater< TPI >>( out, binSize, center, std::numeric_limits< TPI >::lowest() ) {}
};

enum class RadialProjectionType { sum, mean, min, max, };

// RadialSum and RadialMean can be done for DIP__OVL__ALL. Output type: either DFLOAT or DCOMPLEX
// RadialMin and RadialMax can be done for DIP__OVL__NONCOMPLEX. Output type: same as input type

void RadialProjection(
   RadialProjectionType type,
   Image const& in,
   Image const& c_mask,
   Image& out,
   dfloat binSize,
   String const& maxRadius,
   FloatArray center    // taken by copy so we can modify
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );

   // TODO: handle 'process' array parameter
   // Process all dimensions until this is passed as parameter and handled properly.
   BooleanArray process( in.Dimensionality(), true );
   DIP_THROW_IF( process.count() <= 1, "Radial projection is not meaningful in less than 2 dimensions" );
   DIP_THROW_IF( binSize <= 0, "Bin size must be larger than 0" );

   // Prepare input references and input buffer types
   ImageConstRefArray inRefs;
   inRefs.push_back( in );
   DataTypeArray inBufTypes{ in.DataType() };
   Image mask;
   if( c_mask.IsForged() ) {
      // If we have a mask, add it to the input array after possible singleton expansion
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( in.Sizes() );
      DIP_END_STACK_TRACE
      inRefs.push_back( mask );
      inBufTypes.push_back( mask.DataType() );
   }

   // Prepare center
   if( center.empty() ) {
      // Use default center
      center = GetCenter( in, { S::FREQUENCY } );
   } else {
      // Verify center dimensionality
      DIP_THROW_IF( center.size() != in.Dimensionality(), "Center has wrong dimensionality" );
      // Verify that the center is inside the image
      DIP_THROW_IF( !in.IsInside( center ), "Center is outside image" );
   }

   // TODO: Create support for using physical pixel sizes to compute the radius. Allows integrating over ellipses.
   //       Probably involves replacing ProjectionRadial::center_ by a TransformationArray member.

   // Determine radius
   dfloat radius;
   if( maxRadius == S::INNERRADIUS ) {
      radius = std::numeric_limits< dfloat >::max();
      // Find minimum size of dims to be processed
      // TODO: handle 'process' array
      for( dip::uint iDim = 0; iDim < in.Dimensionality(); ++iDim ) {
         // Since the filter center might not be in the image's center,
         // check both [0, center] and [center, size-1]
         // TODO: DIPlib v2 seems to return a larger number (+1)
         // TODO: when using physical dimensions, replace the -1 by the proper pixel size in that dimension
         radius = std::min( radius, center[ iDim ] );
         radius = std::min( radius, static_cast< dfloat >( in.Size( iDim ) - 1 ) - center[ iDim ] );
      }
      DIP_THROW_IF( radius < 0.0, "Radial filter: center is outside the image" );
   }
   else if ( maxRadius == S::OUTERRADIUS ) {
      // Find the maximum diagonal
      radius = 0.0;
      for( dip::uint iDim = 0; iDim < in.Dimensionality(); ++iDim ) {
         dfloat dimMax = std::max( center[ iDim ], static_cast< dfloat >( in.Size( iDim ) - 1 ) - center[ iDim ] );
         radius += dimMax * dimMax;
      }
      radius = std::sqrt( radius );
   } else {
      DIP_THROW( "Invalid maxRadius mode" );
   }

   dip::uint numBins = static_cast< dip::uint >(radius / binSize) + 1;

   DIP_START_STACK_TRACE
      std::unique_ptr< ProjectionRadialBase > lineFilter;
      // Create projection scan function object, depending on the radial filter type
      if( type == RadialProjectionType::sum ) {
         DIP_OVL_NEW_ALL( lineFilter, ProjectionRadialSum, ( out, binSize, center ), in.DataType() );
      } else if( type == RadialProjectionType::mean ) {
         DIP_OVL_NEW_ALL( lineFilter, ProjectionRadialMean, ( out, binSize, center ), in.DataType() );
      } else if( type == RadialProjectionType::min ) {
         DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionRadialMin, ( out, binSize, center ), in.DataType() );
      } else if( type == RadialProjectionType::max ) {
         DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionRadialMax, ( out, binSize, center ), in.DataType() );
      } else {
         DIP_THROW( "RadialProjection(): unknown projection type" );
      }

      // Create output image (not forged)
      // For all types other than radialSum and radialMean: output type == input type
      // TODO: This removes any external interface set by the caller!!! Use reforge here!!!
      out = Image();
      out.SetSizes( { numBins } );
      out.SetTensorSizes( in.TensorElements() );
      out.SetDataType( in.DataType() );
      if( type == RadialProjectionType::sum ) {
         out.SetDataType( DataType::SuggestDouble( in.DataType() ));
      } else if( type == RadialProjectionType::mean ) {
         out.SetDataType( DataType::SuggestDouble( in.DataType() ));
         // Allocate an extra tensor element to store the bin count
         if( in.DataType().IsComplex() ) {
            out.SetTensorSizes( in.TensorElements() + 1 );
         } else {
            out.SetTensorSizes( in.TensorElements() + 1 );
         }
      }

      // Output 'out' is initialized by the projection function

      Image frameworkOut_unused;
      // TODO: prevent tensor-to-spatial expansion in ProjectionScan(), because it would alter the radius computation
      ProjectionScan( in, mask, frameworkOut_unused, DT_BIN, process, *lineFilter );
      // Call Reduce() to merge the per-thread results
      lineFilter->Reduce();

      if( type == RadialProjectionType::mean ) {
         // `out` was created with a column vector tensor (default shape). Strip the last tensor element.
         Image::View tensorCorrectedOut = out[ Range( 0, static_cast< dip::sint >( in.TensorElements() ) - 1 ) ];
         out = tensorCorrectedOut;
      }

      // After processing, reshape the output tensor to the input tensor shape
      out.ReshapeTensor( in.Tensor() );

      //dip::uint nDims = in.Dimensionality();
      //std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      //DIP_OVL_NEW_ALL( scanLineFilter, RadialFilter, (out, binSize, center), in.DataType() );
      //ImageRefArray outRefs( { out } );
      //Framework::Scan( inRefs, outRefs, inBufTypes, { out.DataType() }, { out.DataType() }, { out.TensorElements() }, *scanLineFilter, Framework::Scan_NeedCoordinates );
   DIP_END_STACK_TRACE
}

} // namespace

void RadialSum(
      Image const& in,
      Image const& c_mask,
      Image& out,
      dfloat binSize,
      String const& maxRadius,
      FloatArray const& center
) {
   RadialProjection( RadialProjectionType::sum, in, c_mask, out, binSize, maxRadius, center );
}

void RadialMean(
      Image const& in,
      Image const& c_mask,
      Image& out,
      dfloat binSize,
      String const& maxRadius,
      FloatArray const& center
) {
   RadialProjection( RadialProjectionType::mean, in, c_mask, out, binSize, maxRadius, center );
}

void RadialMinimum(
      Image const& in,
      Image const& c_mask,
      Image& out,
      dfloat binSize,
      String const& maxRadius,
      FloatArray const& center
) {
   RadialProjection( RadialProjectionType::min, in, c_mask, out, binSize, maxRadius, center );
}

void RadialMaximum(
      Image const& in,
      Image const& c_mask,
      Image& out,
      dfloat binSize,
      String const& maxRadius,
      FloatArray const& center
) {
   RadialProjection( RadialProjectionType::max, in, c_mask, out, binSize, maxRadius, center );
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the projection functions") {
   // We mostly test that the ProjectionScan framework works appropriately.
   // Whether the computations are performed correctly is trivial to verify during use.
   dip::Image img{ dip::UnsignedArray{ 3, 4, 2 }, 3, dip::DT_UINT8 };
   img = { 1, 1, 1 };
   img.At( 0, 0, 0 ) = { 2, 3, 4 };

   // Project over all dimensions except the tensor dimension
   dip::Image out = dip::Maximum( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_UINT8 );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 1 );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 2, 3, 4 } ));

   // Project over two dimensions
   dip::BooleanArray ps( 3, true );
   ps[ 0 ] = false;
   out = dip::Maximum( img, {}, ps );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 3 );
   DOCTEST_CHECK( out.Size( 0 ) == 3 );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 2, 3, 4 } ));
   DOCTEST_CHECK( out.At( 1, 0, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 2, 0, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));

   // Project over another two dimensions
   ps[ 0 ] = true;
   ps[ 1 ] = false;
   out = dip::Maximum( img, {}, ps );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 4 );
   DOCTEST_CHECK( out.Size( 1 ) == 4 );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 2, 3, 4 } ));
   DOCTEST_CHECK( out.At( 0, 1, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 0, 2, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 0, 3, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));

   // No looping at all, we project over all dimensions and have no tensor dimension
   img = dip::Image{ dip::UnsignedArray{ 3, 4, 2 }, 1, dip::DT_SFLOAT };
   img = 0;
   img.At( 0, 0, 0 ) = 1;
   out = dip::Mean( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 1 );
   DOCTEST_CHECK( out.TensorElements() == 1 );
   DOCTEST_CHECK( out.As< dip::dfloat >() == doctest::Approx( 1.0 / ( 3.0 * 4.0 * 2.0 )));
   out = dip::Mean( img, {}, "directional" );
   DOCTEST_CHECK( out.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 1 );
   DOCTEST_CHECK( out.TensorElements() == 1 );
   DOCTEST_CHECK( out.As< dip::dfloat >() == doctest::Approx(
         std::atan2( std::sin( 1 ), std::cos( 1 ) + ( 3 * 4 * 2 - 1 ))));
}

#endif // DIP__ENABLE_DOCTEST
