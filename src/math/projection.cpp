/*
 * DIPlib 3.0
 * This file contains the definition for the various projection functions.
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

#include <cmath>

#include "diplib.h"
#include "diplib/statistics.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"
#include "diplib/accumulators.h"
#include "diplib/library/copy_buffer.h"


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

}

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

}

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

}

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

}

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
      DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewProjectionVarianceStable, ( false ), in.DataType());
   } else if( mode == S::FAST ) {
      DIP_OVL_CALL_ASSIGN_REAL( lineFilter, NewProjectionVarianceFast, ( false ), in.DataType());
   } else if( mode == S::DIRECTIONAL ) {
      DIP_OVL_CALL_ASSIGN_FLOAT( lineFilter, NewProjectionVarianceDirectional, ( false ), in.DataType());
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFloat( in.DataType()), process, *lineFilter );
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
      DIP_OVL_CALL_ASSIGN_FLOAT( lineFilter, NewProjectionVarianceStable, ( true ), in.DataType());
   } else if( mode == S::FAST ) {
      DIP_OVL_CALL_ASSIGN_FLOAT( lineFilter, NewProjectionVarianceFast, ( true ), in.DataType());
   } else if( mode == S::DIRECTIONAL ) {
      DIP_OVL_CALL_ASSIGN_FLOAT( lineFilter, NewProjectionVarianceDirectional, ( true ), in.DataType());
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

}

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

}

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

}

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

}

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
         dip::sint rank = floor_cast( static_cast< dfloat >( N ) * percentile_ / 100.0 ); // rank < N, because percentile_ < 100
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

}

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

}

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

}

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
