/*
 * DIPlib 3.0
 * This file contains the definition for the various projection functions.
 *
 * (c)2017-2019, Cris Luengo, Erik Schuitema.
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
#include "diplib/statistics.h"
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"
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
      Image& out,
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
         mask.ExpandSingletonTensor( input.TensorElements() ); // We've checked that it has a single tensor element
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
      //std::cout << "Projection framework: nothing to do!\n";
      out = c_in; // This ignores the mask image
      return;
   }

   // Adjust output if necessary (and possible)
   DIP_START_STACK_TRACE
      if( out.IsOverlappingView( input ) || out.IsOverlappingView( mask )) {
         out.Strip();
      }
      out.ReForge( outSizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      // NOTE: Don't use c_in any more from here on. It has possibly been reforged!
      out.ReshapeTensor( outTensor );
   DIP_END_STACK_TRACE
   out.SetPixelSize( pixelSize );
   out.SetColorSpace( colorSpace );
   Image output = out.QuickCopy();

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
      //std::cout << "Projection framework: no need to loop!\n";
      function.SetNumberOfThreads( 1 );
      if( output.DataType() != outImageType ) {
         Image outBuffer( {}, 1, outImageType ); // a single sample
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
   tempIn.SetOriginUnsafe( input.Origin());
   tempIn.Squeeze(); // we want to make sure that function.Project() won't be looping over singleton dimensions
   // TODO: instead of Squeeze, do a FlattenAsMuchAsPossible. But Mask must be flattened in the same way.
   // Create view over mask image, identically to input
   Image tempMask;
   if( hasMask ) {
      tempMask.CopyProperties( mask );
      tempMask.SetSizes( procSizes );
      tempMask.SetOriginUnsafe( mask.Origin());
      tempMask.Squeeze(); // keep in sync with tempIn.
   }
   // Create view over output image that doesn't contain the processing dimensions or other singleton dimensions
   Image tempOut;
   tempOut.CopyProperties( output );
   // Squeeze tempOut, but keep inStride, maskStride, outStride and outSizes in sync
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
   tempOut.SetOriginUnsafe( output.Origin());
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
         tempIn.ShiftOriginUnsafe( inStride[ dd ] );
         if( hasMask ) {
            tempMask.ShiftOriginUnsafe( maskStride[ dd ] );
         }
         tempOut.ShiftOriginUnsafe( outStride[ dd ] );
         // Check whether we reached the last pixel of the line
         if( position[ dd ] != outSizes[ dd ] ) {
            break;
         }
         // Rewind along this dimension
         tempIn.ShiftOriginUnsafe( -inStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
         if( hasMask ) {
            tempMask.ShiftOriginUnsafe( -maskStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
         }
         tempOut.ShiftOriginUnsafe( -outStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
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

template< typename TPI, bool ComputeMean_ >
class ProjectionSumMean : public ProjectionScanFunction {
      using TPO = FlexType< TPI >;
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         dip::uint n = 0;
         TPO sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  sum += static_cast< TPO >( it.template Sample< 0 >() );
                  if( ComputeMean_ ) {
                     ++n;
                  }
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               sum += static_cast< TPO >( *it );
            } while( ++it );
            if( ComputeMean_ ) {
               n = in.NumberOfPixels();
            }
         }
         if( ComputeMean_ ) {
            *static_cast< TPO* >( out ) = ( n > 0 ) ? ( sum / static_cast< FloatType< TPI >>( n ))
                                                    : ( sum );
         } else {
            *static_cast< TPO* >( out ) = sum;
         }
      }
};

template< typename TPI >
using ProjectionSum = ProjectionSumMean< TPI, false >;

template< typename TPI >
using ProjectionMean = ProjectionSumMean< TPI, true >;

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
   } else if( mode.empty() ) {
      DIP_OVL_NEW_ALL( lineFilter, ProjectionMean, (), in.DataType() );
   } else {
      DIP_THROW_INVALID_FLAG( mode );
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
   DIP_OVL_NEW_ALL( lineFilter, ProjectionSum, (), in.DataType() );
   ProjectionScan( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *lineFilter );
}

namespace {

template< typename TPI, bool ComputeMean_  >
class ProjectionProductGeomMean : public ProjectionScanFunction {
      using TPO = FlexType< TPI >;
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         dip::uint n = 0;
         TPO product = 1.0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  product *= static_cast< TPO >( it.template Sample< 0 >() );
                  if( ComputeMean_ ) {
                     ++n;
                  }
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               product *= static_cast< TPO >( *it );
            } while( ++it );
            if( ComputeMean_ ) {
               n = in.NumberOfPixels();
            }
         }
         if( ComputeMean_ ) {
            *static_cast< TPO* >( out ) = ( n > 0 ) ? ( std::pow( product, 1 / static_cast< FloatType< TPO >>( n )))
                                                    : ( product );
         } else {
            *static_cast< TPO* >( out ) = product;
         }
      }
};

template< typename TPI >
using ProjectionProduct = ProjectionProductGeomMean< TPI, false >;

template< typename TPI >
using ProjectionGeometricMean = ProjectionProductGeomMean< TPI, true >;

} // // namespace

void GeometricMean( Image const& in, Image const& mask, Image& out, BooleanArray const& process ) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, ProjectionGeometricMean, (), in.DataType() );
   ProjectionScan( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *lineFilter );
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

template< typename TPI, bool ComputeMean_ >
class ProjectionSumMeanAbs : public ProjectionScanFunction {
      using TPO = FlexType< TPI >;
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         dip::uint n = 0;
         FloatType< TPI > sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  sum += std::abs( static_cast< TPO >( it.template Sample< 0 >() ));
                  if ( ComputeMean_ ) {
                     ++n;
                  }
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               sum += std::abs( static_cast< TPO >( *it ));
            } while( ++it );
            if( ComputeMean_ ) {
               n = in.NumberOfPixels();
            }
         }
         if( ComputeMean_ ) {
            *static_cast< TPO* >( out ) = ( n > 0 ) ? ( sum / static_cast< FloatType< TPI >>( n ))
                                                    : ( sum );
         } else {
            *static_cast< TPO* >( out ) = sum;
         }
      }
};

template< typename TPI >
using ProjectionSumAbs = ProjectionSumMeanAbs< TPI, false >;

template< typename TPI >
using ProjectionMeanAbs = ProjectionSumMeanAbs< TPI, true >;

} // namespace

void MeanAbs(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if( in.DataType().IsUnsigned() ) {
      DIP_OVL_NEW_UNSIGNED( lineFilter, ProjectionMean, (), in.DataType() );
   } else {
      DIP_OVL_NEW_SIGNED( lineFilter, ProjectionMeanAbs, (), in.DataType() );
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
      DIP_OVL_NEW_UNSIGNED( lineFilter, ProjectionSum, (), in.DataType() );
   } else {
      DIP_OVL_NEW_SIGNED( lineFilter, ProjectionSumAbs, (), in.DataType() );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *lineFilter );
}

namespace {

template< typename TPI, bool ComputeMean_ >
class ProjectionSumMeanSquare : public ProjectionScanFunction {
      using TPO = FlexType< TPI >;
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         dip::uint n = 0;
         TPO sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  TPO v = static_cast< TPO >( it.template Sample< 0 >() );
                  sum += v * v;
                  if( ComputeMean_ ) {
                     ++n;
                  }
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               TPO v = static_cast< TPO >( *it );
               sum += v * v;
            } while( ++it );
            if( ComputeMean_ ) {
               n = in.NumberOfPixels();
            }
         }
         if( ComputeMean_ ) {
            *static_cast< TPO* >( out ) = ( n > 0 ) ? ( sum / static_cast< FloatType< TPI >>( n ))
                                                    : ( sum );
         } else {
            *static_cast< TPO* >( out ) = sum;
         }
      }
};

template< typename TPI >
using ProjectionSumSquare = ProjectionSumMeanSquare< TPI, false >;

template< typename TPI >
using ProjectionMeanSquare = ProjectionSumMeanSquare< TPI, true >;

} // namespace

void MeanSquare(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   if( in.DataType().IsBinary() ) {
      DIP_OVL_NEW_BINARY( lineFilter, ProjectionMean, (), DT_BIN );
   } else {
      DIP_OVL_NEW_NONBINARY( lineFilter, ProjectionMeanSquare, (), in.DataType() );
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
      DIP_OVL_NEW_BINARY( lineFilter, ProjectionSum, (), DT_BIN );
   } else {
      DIP_OVL_NEW_NONBINARY( lineFilter, ProjectionSumSquare, (), in.DataType() );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *lineFilter );
}

namespace {

template< typename TPI, bool ComputeMean_ >
class ProjectionSumMeanSquareModulus : public ProjectionScanFunction {
      // TPI is a complex type.
      using TPO = FloatType< TPI >;
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         dip::uint n = 0;
         TPO sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  TPI v = it.template Sample< 0 >();
                  //sum += ( v * std::conj( v )).real();
                  sum += v.real() * v.real() + v.imag() * v.imag();
                  if( ComputeMean_ ) {
                     ++n;
                  }
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               TPI v = *it;
               //sum += ( v * std::conj( v )).real();
               sum += v.real() * v.real() + v.imag() * v.imag();
            } while( ++it );
            if( ComputeMean_ ) {
               n = in.NumberOfPixels();
            }
         }
         if( ComputeMean_ ) {
            *static_cast< TPO* >( out ) = ( n > 0 ) ? ( sum / static_cast< TPO >( n ))
                                                    : ( sum );
         } else {
            *static_cast< TPO* >( out ) = sum;
         }
      }
};

template< typename TPI >
using ProjectionSumSquareModulus = ProjectionSumMeanSquareModulus< TPI, false >;

template< typename TPI >
using ProjectionMeanSquareModulus = ProjectionSumMeanSquareModulus< TPI, true >;

} // namespace

void MeanSquareModulus(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   if( in.DataType().IsComplex() ) {
      std::unique_ptr< ProjectionScanFunction > lineFilter;
      DIP_OVL_NEW_COMPLEX( lineFilter, ProjectionMeanSquareModulus, (), in.DataType() );
      ProjectionScan( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *lineFilter );
      return;
   }
   MeanSquare( in, mask, out, process );
}

void SumSquareModulus(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   if( in.DataType().IsComplex() ) {
      std::unique_ptr< ProjectionScanFunction > lineFilter;
      DIP_OVL_NEW_COMPLEX( lineFilter, ProjectionSumSquareModulus, (), in.DataType() );
      ProjectionScan( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *lineFilter );
      return;
   }
   SumSquare( in, mask, out, process );
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
using ProjectionVarianceStable = ProjectionVariance< TPI, VarianceAccumulator >;

template< typename TPI >
using ProjectionVarianceFast = ProjectionVariance< TPI, FastVarianceAccumulator >;

template< typename TPI >
using ProjectionVarianceDirectional = ProjectionVariance< TPI, DirectionalStatisticsAccumulator >;

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
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionVarianceStable, ( false ), in.DataType() );
   } else if( mode == S::FAST ) {
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionVarianceFast, ( false ), in.DataType() );
   } else if( mode == S::DIRECTIONAL ) {
      DIP_OVL_NEW_FLOAT( lineFilter, ProjectionVarianceDirectional, ( false ), in.DataType() );
   } else {
      DIP_THROW_INVALID_FLAG( mode );
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
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionVarianceStable, ( true ), in.DataType() );
   } else if( mode == S::FAST ) {
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionVarianceFast, ( true ), in.DataType() );
   } else if( mode == S::DIRECTIONAL ) {
      DIP_OVL_NEW_FLOAT( lineFilter, ProjectionVarianceDirectional, ( true ), in.DataType() );
   } else {
      DIP_THROW_INVALID_FLAG( mode );
   }
   ProjectionScan( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *lineFilter );
}

namespace {

// Some functor object to compute maximum and minimum
template< typename TPI >
struct MaxComputer {
   static TPI compare( TPI a, TPI b ) { return std::max( a, b ); }
   static constexpr TPI init_value = std::numeric_limits< TPI >::lowest();
};

template< typename TPI >
struct MinComputer {
   static TPI compare( TPI a, TPI b ) { return std::min( a, b ); }
   static constexpr TPI init_value = std::numeric_limits< TPI >::max();
};

template< typename TPI, typename Computer >
class ProjectionMaxMin : public ProjectionScanFunction {
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         TPI res = Computer::init_value;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  res = Computer::compare( res, it.template Sample< 0 >() );
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               res = Computer::compare( res, *it );
            } while( ++it );
         }
         *static_cast< TPI* >( out ) = res;
      }
};

template< typename TPI >
using ProjectionMaximum = ProjectionMaxMin< TPI, MaxComputer< TPI >>;

template< typename TPI >
using ProjectionMinimum = ProjectionMaxMin< TPI, MinComputer< TPI >>;

} // namespace

void Maximum(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   if( in.DataType().IsBinary() ) {
      Any( in, mask, out, process );
      return;
   }
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_REAL( lineFilter, ProjectionMaximum, (), in.DataType() );
   ProjectionScan( in, mask, out, in.DataType(), process, *lineFilter );
}

void Minimum(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   if( in.DataType().IsBinary() ) {
      All( in, mask, out, process );
      return;
   }
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_REAL( lineFilter, ProjectionMinimum, (), in.DataType() );
   ProjectionScan( in, mask, out, in.DataType(), process, *lineFilter );
}

namespace {

template< typename TPI, typename Computer >
class ProjectionMaxMinAbs : public ProjectionScanFunction {
      using TPO = AbsType< TPI >;
   public:
      virtual void Project( Image const& in, Image const& mask, void* out, dip::uint ) override {
         TPO res = Computer::init_value;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            it.OptimizeAndFlatten();
            do {
               if( it.template Sample< 1 >() ) {
                  res = Computer::compare( res, static_cast< TPO >( std::abs( it.template Sample< 0 >() )));
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            it.OptimizeAndFlatten();
            do {
               res = Computer::compare( res, static_cast< TPO >( std::abs( *it )));
            } while( ++it );
         }
         *static_cast< TPO* >( out ) = res;
      }
};

template< typename TPI >
using ProjectionMaximumAbs = ProjectionMaxMinAbs< TPI, MaxComputer< AbsType< TPI >>>;

template< typename TPI >
using ProjectionMinimumAbs = ProjectionMaxMinAbs< TPI, MinComputer< AbsType< TPI >>>;

} // namespace

void MaximumAbs(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   DataType dt = in.DataType();
   if( dt.IsUnsigned() ) {
      Maximum( in, mask, out, process );
      return;
   }
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_SIGNED( lineFilter, ProjectionMaximumAbs, (), dt );
   dt = DataType::SuggestAbs( dt );
   ProjectionScan( in, mask, out, dt, process, *lineFilter );
}

void MinimumAbs(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   DataType dt = in.DataType();
   if( dt.IsUnsigned() ) {
      Minimum( in, mask, out, process );
      return;
   }
   std::unique_ptr< ProjectionScanFunction > lineFilter;
   DIP_OVL_NEW_SIGNED( lineFilter, ProjectionMinimumAbs, (), dt );
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

void MedianAbsoluteDeviation(
      Image const& c_in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   Image in = c_in;
   Median( in, mask, out, process );
   Image tmp = Subtract( in, out, DataType::SuggestSigned( out.DataType() ));
   Abs( tmp, tmp );
   Median( tmp, mask, out, process ); // Might need to reallocate `out` again, as `tmp` has a different data type than `out`.
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
         *static_cast< dip::uint32* >( out ) = clamp_cast< dip::uint32 >( limitCoords.front() );
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
         *static_cast< dip::uint32* >( out ) = clamp_cast< dip::uint32 >( percentileCoords.front() );
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


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

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
   out = dip::Maximum( img, {}, { false, true, true } );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 3 );
   DOCTEST_CHECK( out.Size( 0 ) == 3 );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 2, 3, 4 } ));
   DOCTEST_CHECK( out.At( 1, 0, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 2, 0, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));

   // Project over another two dimensions
   out = dip::Maximum( img, {}, { true, false, true } );
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

   // Using a mask
   img = dip::Image{ dip::UnsignedArray{ 3, 4, 2 }, 3, dip::DT_UINT8 };
   img = { 1, 1, 1 };
   img.At( 0, 0, 0 ) = { 2, 3, 4 };
   img.At( 0, 1, 0 ) = { 3, 2, 2 };
   img.At( 0, 0, 1 ) = { 4, 2, 3 };
   img.At( 1, 0, 0 ) = { 4, 2, 1 };
   dip::Image mask{ img.Sizes(), 1, dip::DT_BIN };
   mask = 1;
   img.At( 0, 0, 0 ) = 0;
   out = dip::Maximum( img, mask, { true, true, false } );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 4, 2, 2 } )); // not {4,3,4}
   DOCTEST_CHECK( out.At( 0, 0, 1 ) == dip::Image::Pixel( { 4, 2, 3 } ));

   // Using a view
   out = dip::Maximum( img.At( mask ));
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 4, 2, 3 } )); // not {4,3,4}
}

#endif // DIP__ENABLE_DOCTEST
