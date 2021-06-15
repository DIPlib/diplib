/*
 * DIPlib 3.0
 * This file contains the definition for the various projection functions.
 *
 * (c)2018, Erik Schuitema, Cris Luengo.
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
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"

namespace dip {

namespace {

class RadialProjectionScanFunction {
   public:
      // Initialize the output image.
      virtual void InitializeOutputImage() = 0;
      // Set number of threads. The output per thread is prepared here.
      virtual void SetNumberOfThreads( dip::uint threads ) = 0;
      // The filter to be applied to each sub-image. (currently called only once for the whole image)
      virtual void Project( Image const& in, Image const& mask, dip::uint thread ) = 0;
      // Reduce the outputs of all threads to a single output.
      virtual void Reduce() {}
      // A virtual destructor guarantees that we can destroy a derived class by a pointer to base
      virtual ~RadialProjectionScanFunction() {}
};

template< typename TPI, typename TPO >
class ProjectionRadialBase : public RadialProjectionScanFunction {
   public:
      ProjectionRadialBase( Image& out, dfloat binSize, FloatArray center ) : out_( out ), binSize_( binSize ), center_( std::move( center )) {}

      virtual void Project( Image const& in, Image const& mask, dip::uint thread ) override {

         // Obtain local output image
         // The output of thread 0 is stored in out_; the output of the other threads is stored in outPerThread_
         Image& out = thread == 0 ? out_ : outPerThread_[ thread - 1 ];

         dip::uint procDim = Framework::OptimalProcessingDim( in );
         dip::uint inTensorLength = in.TensorElements();

         DIP_ASSERT( out.DataType() == DataType( TPO{} ));
         DIP_ASSERT( inTensorLength <= out.TensorElements() );
         DIP_ASSERT( out.Dimensionality() == 1 );

         TPO* pout = static_cast< TPO* >( out.Origin() );
         dip::sint stride = out.Stride( 0 );
         dip::sint tStride = out.TensorStride();
         dip::sint bins = static_cast< dip::sint >( out.Size( 0 ));

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
                     dip::sint binIndex = GetBinIndex( itPixel.Coordinate(), procDim, sqrDist );
                     // Verify that the bin is within range (for the INNERRADIUS option, not all pixels are processed)
                     if( binIndex < bins ) {
                        ProcessPixel( itPixel.begin(), { pout + binIndex * stride, tStride }, inTensorLength );
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
                  dip::sint binIndex = GetBinIndex( itPixel.Coordinate(), procDim, sqrDist );
                  // Verify that the bin is within range (for the INNERRADIUS option, not all pixels are processed)
                  if( binIndex < bins ) {
                     ProcessPixel( itPixel.begin(), { pout + binIndex * stride, tStride }, inTensorLength );
                  }
               } while( ++itPixel );
            } while( ++itLine );

         }
      }

      // Set number of threads. The output per thread is prepared here
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         // Force output image (once) and initialize
         InitializeOutputImage();
         // Allocate space for output values. Start at 1, because thread 0 is stored in image_.
         for( dip::uint ii = 1; ii < threads; ++ii ) {
            outPerThread_.emplace_back( out_.Copy() );  // makes a deep copy.
         }
         // We don't forge the images here, the Filter() function should do that so each thread allocates its own
         // data segment. This ensures there's no false sharing.
      }

   protected:
      Image& out_;   // Output image for thread 0
      ImageArray outPerThread_;  // Only non-empty when num threads > 1
      dfloat binSize_;  // Bin size of the radial statistics output
      FloatArray center_;// Convert to the following to include scaling: TransformationArray transformation_;

      // Process one pixel
      virtual void ProcessPixel( ConstSampleIterator< TPI > pIn, SampleIterator< TPO > pOut, dip::uint inTensorLength ) = 0;

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

      dip::sint GetBinIndex( dip::uint procDimCoordinate, dip::uint procDim, dfloat partialSqrDist ) const {
         dfloat dist = static_cast< dfloat >( procDimCoordinate ) - center_[ procDim ];
         dfloat radius = std::sqrt( partialSqrDist + dist * dist );
         return floor_cast( radius / binSize_ );
      }

};

// Radial sum filter
template< typename TPI, typename TPO = DoubleType< TPI >>
class ProjectionRadialSum : public ProjectionRadialBase< TPI, TPO > {
   public:
      ProjectionRadialSum( Image& out, dfloat binSize, FloatArray const& center ) :
            ProjectionRadialBase< TPI, TPO >( out, binSize, center ) {}

      virtual void InitializeOutputImage() override {
         // Initialize with zeros
         out_.Fill( 0 );
      }

      virtual void Reduce() override {
         // Take sum of all images
         for( auto const& out : outPerThread_ ) {
            out_ += out;
         }
      }
   protected:
      virtual void ProcessPixel( ConstSampleIterator< TPI > pIn, SampleIterator< TPO > pOut, dip::uint inTensorLength ) override {
         for( dip::uint iT = 0; iT < inTensorLength; ++iT, ++pIn, ++pOut ) {
            *pOut += static_cast< TPO >( *pIn );
         }
      }

      using ProjectionRadialBase< TPI, TPO >::outPerThread_;
      using ProjectionRadialBase< TPI, TPO >::out_;
};

// Radial mean filter
template< typename TPI, typename TPO = DoubleType< TPI >>
class ProjectionRadialMean : public ProjectionRadialSum< TPI, TPO > {
   public:
      ProjectionRadialMean( Image& out, dfloat binSize, FloatArray const& center ) :
            ProjectionRadialSum< TPI, TPO >( out, binSize, center ) {}

      virtual void Reduce() override {
         // Take sum of all images
         for( auto const& out : outPerThread_ ) {
            out_ += out;
         }
         // The last sample in each pixel contains the bin count.
         // Divide the other tensor elements by the last one to obtain the mean.
         ImageIterator< TPO > itOut( out_ );
         int ii = 0;
         do {
            TPO count = *( --itOut.cend() );
            ++ii;
            if( ii == 20 ) {
               ii = 0;
            }
            if( count != 0.0 ) {
               // Store the mean
               for( auto itSample = itOut.begin(); itSample != --itOut.end(); ++itSample ) {
                  *itSample /= count;
               }
            } else {
               // The bin is empty: store zero
               std::fill( itOut.begin(), --itOut.end(), 0.0 );
            }
         } while( ++itOut );
      }

   protected:
      virtual void ProcessPixel( ConstSampleIterator< TPI > pIn, SampleIterator< TPO > pOut, dip::uint inTensorLength ) override {
         for( dip::uint iT = 0; iT < inTensorLength; ++iT, ++pIn, ++pOut ) {
            *pOut += static_cast< TPO >( *pIn );
         }
         // The output pixel contains an extra tensor element to store the bin count
         *pOut += 1.0;   // If the output sample type is complex, the bin count is in the real part
      }

      using ProjectionRadialSum< TPI, TPO >::outPerThread_;
      using ProjectionRadialSum< TPI, TPO >::out_;
};

// Radial min/max filter template
// `CompareOp` is the compare operation: std::less or std::greater
template< typename TPI, typename CompareOp >
class ProjectionRadialMinMax : public ProjectionRadialBase< TPI, TPI > {
   public:
      // `limitInitVal` is the initialization value of the variable that tracks the limit value
      // For finding a minimum value, initialize with std::numeric_limits< TPI >::max(),
      // for finding a maximum value, initialize with std::numeric_limits< TPI >::lowest().
      ProjectionRadialMinMax( Image& out, dfloat binSize, FloatArray const& center, TPI limitInitVal ) :
            ProjectionRadialBase< TPI, TPI >( out, binSize, center ), limitInitVal_( limitInitVal ) {}

      virtual void InitializeOutputImage() override {
         // Initialize with limitInitVal_
         out_.Fill( limitInitVal_ );
      }

      virtual void Reduce() override {
         // Take limit of all images
         //TODO: does it help to use Supremum() and Infimum() here?
         for( dip::uint iOut = 0; iOut < outPerThread_.size(); ++iOut ) {
            JointImageIterator< TPI, TPI > itOut( { out_, outPerThread_[ iOut ] } );
            do {
               if( compareOp_( itOut.template Sample< 1 >(), itOut.template Sample< 0 >() )) {
                  itOut.template Sample< 0 >() = itOut.template Sample< 1 >();
               }
            } while( ++itOut );
         }
      }

   protected:
      virtual void ProcessPixel( ConstSampleIterator< TPI > pIn, SampleIterator< TPI > pOut, dip::uint tensorLength ) override {
         for( dip::uint iT = 0; iT < tensorLength; ++iT, ++pIn, ++pOut ) {
            if( compareOp_( *pIn, *pOut ))
               *pOut = *pIn;
         }
      }

      TPI limitInitVal_;
      CompareOp compareOp_;   // Compare functor
      using ProjectionRadialBase< TPI, TPI >::outPerThread_;
      using ProjectionRadialBase< TPI, TPI >::out_;
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

void RadialProjectionScan(
      RadialProjectionType type,
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      dfloat binSize,
      String const& maxRadius,
      FloatArray center    // taken by copy so we can modify
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );

   // TODO: handle 'process' array parameter
   // Process all dimensions until this is passed as parameter and handled properly.
   BooleanArray process( c_in.Dimensionality(), true );
   DIP_THROW_IF( process.count() <= 1, "Radial projection is not meaningful in less than 2 dimensions" );
   DIP_THROW_IF( binSize <= 0, "Bin size must be larger than 0" );

   // Prepare input references and input buffer types
   Image mask;
   if( c_mask.IsForged() ) {
      // If we have a mask, add it to the input array after possible singleton expansion
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( c_in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( c_in.Sizes() );
      DIP_END_STACK_TRACE
   }

   // Prepare center
   if( center.empty() ) {
      // Use default center
      center = c_in.GetCenter();
   } else {
      // Verify center dimensionality
      DIP_THROW_IF( center.size() != c_in.Dimensionality(), "Center has wrong dimensionality" );
      // Verify that the center is inside the image
      DIP_THROW_IF( !c_in.IsInside( center ), "Center is outside image" );
   }

   // TODO: Create support for using physical pixel sizes to compute the radius. Allows integrating over ellipses.
   //       Probably involves replacing ProjectionRadialBase::center_ by a TransformationArray member.

   // Determine radius
   dfloat radius;
   if( maxRadius == S::INNERRADIUS ) {
      radius = std::numeric_limits< dfloat >::max();
      // Find minimum size of dims to be processed
      // TODO: handle 'process' array
      for( dip::uint iDim = 0; iDim < c_in.Dimensionality(); ++iDim ) {
         // Since the filter center might not be in the image's center,
         // check both [0, center] and [center, size-1]
         radius = std::min( radius, center[ iDim ] );
         radius = std::min( radius, static_cast< dfloat >( c_in.Size( iDim ) - 1 ) - center[ iDim ] );
      }
      DIP_ASSERT( radius >= 0.0 );
   } else if ( maxRadius == S::OUTERRADIUS ) {
      // Find the maximum diagonal
      radius = 0.0;
      for( dip::uint iDim = 0; iDim < c_in.Dimensionality(); ++iDim ) {
         dfloat dimMax = std::max( center[ iDim ], static_cast< dfloat >( c_in.Size( iDim ) - 1 ) - center[ iDim ] );
         radius += dimMax * dimMax;
      }
      radius = std::sqrt( radius );
   } else {
      DIP_THROW( "Invalid maxRadius mode" );
   }
   dip::uint numBins = static_cast< dip::uint >( radius / binSize ) + 1;

   // Make copy of input image header. This separates it from the output image, so we don't change it
   // when reforging `out`.
   Image in = c_in;

   // Create output image
   DataType dt = in.DataType();
   if(( type == RadialProjectionType::sum ) || ( type == RadialProjectionType::mean )) {
      // Output type is dfloat or dcomplex.
      dt = DataType::SuggestDouble( in.DataType() );
   }
   dip::uint nTensorElements = in.TensorElements();
   if( type == RadialProjectionType::mean ) {
      // Allocate an extra tensor element to store the bin count
      ++nTensorElements;
   }
   out.ReForge( { numBins }, nTensorElements, dt );

   // Create projection scan function object, depending on the radial filter type
   std::unique_ptr< RadialProjectionScanFunction > lineFilter;
   if( type == RadialProjectionType::sum ) {
      DIP_OVL_NEW_ALL( lineFilter, ProjectionRadialSum, ( out, binSize, center ), c_in.DataType() );
   } else if( type == RadialProjectionType::mean ) {
      DIP_OVL_NEW_ALL( lineFilter, ProjectionRadialMean, ( out, binSize, center ), c_in.DataType() );
   } else if( type == RadialProjectionType::min ) {
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionRadialMin, ( out, binSize, center ), c_in.DataType() );
   } else if( type == RadialProjectionType::max ) {
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, ProjectionRadialMax, ( out, binSize, center ), c_in.DataType() );
   } else {
      DIP_THROW( "Unknown projection type" );
   }

   // Call projection scan function object
   DIP_START_STACK_TRACE
      lineFilter->SetNumberOfThreads( 1 );
      lineFilter->Project( in, mask, 0 );
      // Call Reduce() to merge the per-thread results
      lineFilter->Reduce(); // In the case of `RadialProjectionType::mean`, this does more than reduce!
   DIP_END_STACK_TRACE

   if( type == RadialProjectionType::mean ) {
      // `out` was created with a column vector tensor (default shape). Strip the last tensor element.
      out.SetTensorSizesUnsafe( in.TensorElements());
   }

   // After processing, reshape the output tensor to the input tensor shape
   out.ReshapeTensor( in.Tensor() );
   out.CopyNonDataProperties( in );
}

} // namespace

void RadialSum(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat binSize,
      String const& maxRadius,
      FloatArray const& center
) {
   RadialProjectionScan( RadialProjectionType::sum, in, mask, out, binSize, maxRadius, center );
}

void RadialMean(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat binSize,
      String const& maxRadius,
      FloatArray const& center
) {
   RadialProjectionScan( RadialProjectionType::mean, in, mask, out, binSize, maxRadius, center );
}

void RadialMinimum(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat binSize,
      String const& maxRadius,
      FloatArray const& center
) {
   RadialProjectionScan( RadialProjectionType::min, in, mask, out, binSize, maxRadius, center );
}

void RadialMaximum(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat binSize,
      String const& maxRadius,
      FloatArray const& center
) {
   RadialProjectionScan( RadialProjectionType::max, in, mask, out, binSize, maxRadius, center );
}

} // namespace dip
