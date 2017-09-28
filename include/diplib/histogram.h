/*
 * DIPlib 3.0
 * This file contains declarations for histograms and related functionality
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

#ifndef DIP_HISTOGRAM_H
#define DIP_HISTOGRAM_H

#include "diplib.h"
#include "diplib/iterators.h"
#include "diplib/measurement.h"


/// \file
/// \brief Histograms and related functionality.
/// \see histograms


namespace dip {


/// \defgroup histograms Histograms
/// \brief Histograms and related functionality.
/// \{

/// \brief Computes and holds histograms.
///
/// A histogram is computed by the constructor. There is no default-constructed `%Histogram`.
///
/// A histogram can have multiple dimensions. In general, a scalar image will yield a classical
/// one-dimensional histogram, and a tensor image will yield a multi-dimensional histogram, with
/// one dimension per tensor element. The first tensor element determines the index along the first
/// dimension, the second tensor element that along the second dimension, etc.
///
/// To facilitate usage for one-dimensional histograms, all getter functions that return a value
/// for a given dimension, default to dimension 0, so can be called without arguments.
class DIP_NO_EXPORT Histogram {
   public:
      using CountType = uint32;

      /// \brief Configuration information for how the histogram is computed.
      ///
      /// Note that if `mode == Mode::COMPUTE_BINS`, `binSize` will be adjusted so that a whole number of bins
      /// fits between the bounds. If `binSize` was set to zero or a negative value, and the input image is of
      /// any integer type, then `binSize` will be computed to be an integer power of two, and such that there
      /// are no more than 256 bins in the histogram.
      ///
      /// Any illegal values in the configuration will silently be replaced with the default values.
      ///
      /// Constructors exist to set different subsets of the configuration; write upper bound and bin size
      /// as a floating point number so the right constructor can be selected (they differ in the location
      /// of the integer value). For example, note the difference between the following constructor calls,
      /// where `10` indicates 10 bins, `10.0` indicates the upper bound, and `1.0` indicates the bin size:
      ///
      /// ```cpp
      ///     dip::Histogram::Configuration conf1( 0.0, 10.0, 1.0 );
      ///     dip::Histogram::Configuration conf1( 0.0, 10.0, 10 );
      ///     dip::Histogram::Configuration conf2( 0.0, 10, 1.0 );
      /// ```
      ///
      /// An additional constructor takes a `dip::DataType`, and selects appropriate values for an image of the
      /// given data type:
      ///  - For 8-bit images, the histogram has 256 bins, one for each possible input value.
      ///  - For other integer-valued images, the histogram has up to 256 bins, stretching from the lowest value
      ///    in the image to the highest, and with bin size a power of two. This is the simplest way of correctly
      ///    handling data from 10-bit, 12-bit and 16-bit sensors that can put data in the lower or the upper bits
      ///    of the 16-bit words, and will handle other integer data correctly as well.
      ///  - For floating-point images, the histogram always has 256 bins, stretching from the lowest value in the
      ///    image to the highest.
      struct Configuration {
         dfloat lowerBound = 0.0;      ///< Lower bound for this dimension, corresponds to the lower bound of the first bin.
         dfloat upperBound = 256.0;    ///< Upper bound for this dimension, corresponds to the upper bound of the last bin.
         dip::uint nBins = 256;        ///< Number of bins for this dimension.
         dfloat binSize = 1.0;         ///< Size of each bin for this dimension.
         enum class Mode {
               COMPUTE_BINSIZE,
               COMPUTE_BINS,
               COMPUTE_LOWER,
               COMPUTE_UPPER
         } mode = Mode::COMPUTE_BINSIZE; ///< The given value is ignored and replaced by the computed value.
         bool lowerIsPercentile = false; ///< If set, `lowerBound` is replaced by the given percentile pixel value.
         bool upperIsPercentile = false; ///< If set, `upperBound` is replaced by the given percentile pixel value.
         bool excludeOutOfBoundValues = false; ///< If set, pixels outside of the histogram bounds are not counted.

         /// \brief Default-constructed configuration defines 256 bins in the range [0,256].
         Configuration() {}
         /// \brief A constructor takes a lower and upper bounds, and the bin size. The numbr of bins are computed.
         Configuration( dfloat lowerBound, dfloat upperBound, dfloat binSize ) :
               lowerBound( lowerBound ), upperBound( upperBound ), binSize( binSize ), mode( Mode::COMPUTE_BINS ) {}
         /// \brief A constructor takes a lower and upper bounds, and the number of bins. The bin size is computed.
         Configuration( dfloat lowerBound, dfloat upperBound, int nBins = 256 ) :
               lowerBound( lowerBound ), upperBound( upperBound ), nBins( static_cast< dip::uint >( nBins )), mode( Mode::COMPUTE_BINSIZE ) {}
         /// \brief A constructor takes a lower bound, the number of bins and the bin size. The upper bound is computed.
         Configuration( dfloat lowerBound, int nBins, dfloat binSize ) :
               lowerBound( lowerBound ), nBins( static_cast< dip::uint >( nBins )), binSize( binSize ), mode( Mode::COMPUTE_UPPER ) {}
         /// \brief A constructor takes an image data type, yielding a default histogram configuration for that data type.
         Configuration( DataType dataType ) {
            if( dataType == DT_UINT8 ) {
               // 256 bins between 0 and 255, this is the default:
            } else if ( dataType == DT_SINT8 ) {
               // 256 bins between -128 and 128:
               lowerBound = -128.0;
               upperBound = 128.0;
            } else if( dataType.IsInteger() ) {
               // Other integer types: the complicated one.
               lowerBound = 0.0;
               upperBound = 100.0;
               lowerIsPercentile = true;
               upperIsPercentile = true;
               binSize = 0.0; // binSize==0.0 indicates to compute bins as an integer power of two.
               mode = Mode::COMPUTE_BINS;
            } else {
               // Floating-point types: 256 bins, stretched between max and min.
               lowerBound = 0.0;
               upperBound = 100.0;
               lowerIsPercentile = true;
               upperIsPercentile = true;
            }
         }
      };
      using ConfigurationArray = DimensionArray< Configuration >;

      /// \brief The constructor takes an image, an optional mask, and configuration options for each histogram
      /// dimension.
      ///
      /// `configuration` should have as many elements as tensor elements in `input`. If `configuration` has only
      /// one element, it will be used for all histogram dimensions. If it is an empty array, appropriate configuration
      /// values for `input` are chosen based on its data type (see `dip::Histogram::Configuration`).
      explicit Histogram( Image const& input, Image const& mask = {}, ConfigurationArray configuration = {} ) {
         DIP_THROW_IF( !input.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !input.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
         DIP_START_STACK_TRACE
            ArrayUseParameter( configuration, input.TensorElements(), Configuration( input.DataType() ) );
            if( input.IsScalar() ) {
               ScalarImageHistogram( input, mask, configuration[ 0 ] );
            } else {
               TensorImageHistogram( input, mask, configuration );
            }
         DIP_END_STACK_TRACE
      }

      /// \brief This version of the constructor is identical to the previous one, but with a single configuration
      /// parameter instead of an array.
      Histogram( Image const& input, Image const& mask, Configuration configuration ) {
         DIP_THROW_IF( !input.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !input.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
         DIP_START_STACK_TRACE
            if( input.IsScalar() ) {
               ScalarImageHistogram( input, mask, configuration );
            } else {
               ConfigurationArray newConfig( input.TensorElements(), configuration );
               TensorImageHistogram( input, mask, newConfig );
            }
         DIP_END_STACK_TRACE
      }

      /// \brief A version of the constructor that takes two scalar input images, and constructs their joint histogram
      /// (a 2D histogram, equal to the one obtained if the two images were the two channels of a tensor image).
      Histogram( Image const& input1, Image const& input2, Image const& mask, ConfigurationArray configuration = {} ) {
         // Note: `mask` cannot have a default value, it must always be given to distinguish this constructor from
         // the first one.
         DIP_THROW_IF( !input1.IsForged() || !input2.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !input1.IsScalar() || !input2.IsScalar(), E::IMAGE_NOT_SCALAR );
         DIP_THROW_IF( !input1.DataType().IsReal() || !input2.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
         DIP_START_STACK_TRACE
            if( configuration.empty() ) {
               configuration.resize( 2 );
               configuration[ 0 ] = Configuration( input1.DataType() );
               configuration[ 1 ] = Configuration( input2.DataType() );
            } else {
               ArrayUseParameter( configuration, 2, Configuration{} );
            }
            JointImageHistogram( input1, input2, mask, configuration );
         DIP_END_STACK_TRACE
      }

      /// \brief The constructor takes an `%IteratorFeature` of a `dip::Measurement` object, and configuration
      /// options for each histogram dimension.
      ///
      /// `configuration` should have as many elements as values in `featureValues`. If `configuration` has only
      /// one element, it will be used for all histogram dimensions. In the default configuration, the histogram
      /// will stretch from lowest to highest value, in 100 bins.
      explicit Histogram( Measurement::IteratorFeature const& featureValues, ConfigurationArray configuration = {} ) {
         Configuration defaultConf( 0.0, 100.0, 100 ); // nBins==100
         defaultConf.lowerIsPercentile = true;
         defaultConf.upperIsPercentile = true;
         DIP_START_STACK_TRACE
            ArrayUseParameter( configuration, featureValues.NumberOfValues(), defaultConf );
            MeasurementFeatureHistogram( featureValues, configuration );
         DIP_END_STACK_TRACE
      }

      /// \brief Adds a histogram to *this. `other` must have identical properties.
      ///
      /// Adding multiple histograms together can be useful, for example, when accumulating pixel values
      /// from multiple images, or in multiple threads.
      Histogram& operator+=( Histogram const& other ) {
         DIP_THROW_IF(( data_.Sizes() != other.data_.Sizes() ||
                      ( lowerBounds_ != other.lowerBounds_ ) ||
                      ( binSizes_ != other.binSizes_ )), "Histograms don't match" );
         data_ += other.data_;
         return *this;
      }

      /// \brief Returns the histogram dimensionality.
      dip::uint Dimensionality() const { return data_.Dimensionality(); }

      /// \brief Returns the number of bins along dimension `dim`
      dip::uint Bins( dip::uint dim = 0 ) const {
         DIP_THROW_IF( dim >= Dimensionality(), E::PARAMETER_OUT_OF_RANGE );
         return data_.Size( dim );
      }

      /// \brief Returns the size of the bins along dimension `dim`
      dfloat BinSize( dip::uint dim = 0 ) const {
         DIP_THROW_IF( dim >= Dimensionality(), E::PARAMETER_OUT_OF_RANGE );
         return binSizes_[ dim ];
      }

      /// \brief Returns the lower bound of the histogram for dimension `dim`
      dfloat LowerBound( dip::uint dim = 0 ) const {
         DIP_THROW_IF( dim >= Dimensionality(), E::PARAMETER_OUT_OF_RANGE );
         return lowerBounds_[ dim ];
      }

      /// \brief Returns the upper bound of the histogram for dimension `dim`
      dfloat UpperBound( dip::uint dim = 0 ) const {
         DIP_THROW_IF( dim >= Dimensionality(), E::PARAMETER_OUT_OF_RANGE );
         return lowerBounds_[ dim ] + static_cast< dfloat >( data_.Size( dim )) * binSizes_[ dim ];
      }

      /// \brief Returns the bin boundaries along dimension `dim` (`Bins(dim)+1` values).
      FloatArray BinBoundaries( dip::uint dim = 0 ) const {
         DIP_THROW_IF( dim >= Dimensionality(), E::PARAMETER_OUT_OF_RANGE );
         FloatArray boundaries( data_.Size( dim ) + 1 );
         dfloat offset = lowerBounds_[ dim ];
         dfloat scale = binSizes_[ dim ];
         for( dip::uint ii = 0; ii < boundaries.size(); ++ii ) {
            boundaries[ ii ] = offset + static_cast< dfloat >( ii ) * scale;
            // NOTE: this is safer than `boundaries[ii]=boundaries[ii-1]+scale`, because of numerical inaccuracy.
         }
         return boundaries;
      }

      /// \brief Returns the bin centers along dimension `dim`
      FloatArray BinCenters( dip::uint dim = 0 ) const {
         DIP_THROW_IF( dim >= Dimensionality(), E::PARAMETER_OUT_OF_RANGE );
         FloatArray centers( data_.Size( dim ) );
         dfloat scale = binSizes_[ dim ];
         dfloat offset = lowerBounds_[ dim ] + scale / 2;
         for( dip::uint ii = 0; ii < centers.size(); ++ii ) {
            centers[ ii ] = offset + static_cast< dfloat >( ii ) * scale;
         }
         return centers;
      }

      /// \brief Returns the bin center for the given `bin` along dimension `dim`
      dfloat BinCenter( dip::uint bin, dip::uint dim = 0 ) const {
         DIP_THROW_IF( dim >= Dimensionality(), E::PARAMETER_OUT_OF_RANGE );
         return lowerBounds_[ dim ] + ( static_cast< dfloat >( bin ) + 0.5 ) * binSizes_[ dim ];
      }

      /// \brief Gets the bin for `value` in a 1D histogram
      dip::uint Bin( dfloat value ) const {
         DIP_THROW_IF( Dimensionality() != 1, E::ILLEGAL_DIMENSIONALITY );
         return FindClampedBin( value, 0 );
      }

      /// \brief Gets the bin for {`x_value`, `y_value`} in a 2D histogram
      UnsignedArray Bin( dfloat x_value, dfloat y_value ) const {
         DIP_THROW_IF( Dimensionality() != 2, E::ILLEGAL_DIMENSIONALITY );
         return { FindClampedBin( x_value, 0 ),
                  FindClampedBin( y_value, 1 ) };
      }

      /// \brief Gets the bin for {`x_value`, `y_value`, `z_value`} in a 3D histogram
      UnsignedArray Bin( dfloat x_value, dfloat y_value, dfloat z_value ) const {
         DIP_THROW_IF( Dimensionality() != 3, E::ILLEGAL_DIMENSIONALITY );
         return { FindClampedBin( x_value, 0 ),
                  FindClampedBin( y_value, 1 ),
                  FindClampedBin( z_value, 2 ) };
      }

      /// \brief Gets the bin for `value` in an nD histogram
      UnsignedArray Bin( FloatArray value ) const {
         DIP_THROW_IF( Dimensionality() != value.size(), E::ARRAY_ILLEGAL_SIZE );
         UnsignedArray out( value.size() );
         for( dip::uint ii = 0; ii < value.size(); ++ii ) {
            out[ ii ] = FindClampedBin( value[ ii ], ii );
         }
         return out;
      }

      /// \brief Get the value at the given bin in a 1D histogram
      CountType At( dip::uint x ) const {
         DIP_THROW_IF( Dimensionality() != 1, E::ILLEGAL_DIMENSIONALITY );
         DIP_THROW_IF( x >= data_.Size( 0 ), E::INDEX_OUT_OF_RANGE );
         return *static_cast< CountType* >( data_.Pointer( static_cast< dip::sint >( x ) * data_.Stride( 0 ) ));
      }
      /// \brief Get the value at the given bin in a 2D histogram
      CountType At( dip::uint x, dip::uint y ) const {
         DIP_THROW_IF( Dimensionality() != 2, E::ILLEGAL_DIMENSIONALITY );
         DIP_THROW_IF( x >= data_.Size( 0 ), E::INDEX_OUT_OF_RANGE );
         DIP_THROW_IF( y >= data_.Size( 1 ), E::INDEX_OUT_OF_RANGE );
         return *static_cast< CountType* >( data_.Pointer( static_cast< dip::sint >( x ) * data_.Stride( 0 ) +
                                                           static_cast< dip::sint >( y ) * data_.Stride( 1 )));
      }

      /// \brief Get the value at the given bin in a 3D histogram
      CountType At( dip::uint x, dip::uint y, dip::uint z ) const {
         DIP_THROW_IF( Dimensionality() != 3, E::ILLEGAL_DIMENSIONALITY );
         DIP_THROW_IF( x >= data_.Size( 0 ), E::INDEX_OUT_OF_RANGE );
         DIP_THROW_IF( y >= data_.Size( 1 ), E::INDEX_OUT_OF_RANGE );
         DIP_THROW_IF( z >= data_.Size( 2 ), E::INDEX_OUT_OF_RANGE );
         return *static_cast< CountType* >( data_.Pointer( static_cast< dip::sint >( x ) * data_.Stride( 0 ) +
                                                           static_cast< dip::sint >( y ) * data_.Stride( 1 ) +
                                                           static_cast< dip::sint >( z ) * data_.Stride( 2 )));
      }

      /// \brief Get the value at the given bin
      CountType At( UnsignedArray const& bin ) const {
         return *static_cast< CountType* >( data_.Pointer( bin )); // Does all the checking
      }

      /// \brief Get the image that holds the bin counts. The image is always scalar and of type `dip::DT_UINT32`.
      Image const& GetImage() const {
         return data_;
      }

      /// \brief Returns an iterator to the first bin
      ConstImageIterator< CountType > begin() const {
         return { data_ };
      }

      /// \brief Returns an end iterator
      ConstImageIterator< CountType > end() const {
         return {};
      }

      /// \brief Returns a pointer to the first bin
      CountType const* Origin() const { return static_cast< CountType const* >( data_.Origin() ); }

      /// \brief Returns the total number of elements in the histogram (sum of bins)
      DIP_EXPORT dip::uint Count() const;

      /// \brief Returns a new histogram containing, for each bin, the sum of that bin with all the previous ones.
      ///
      /// The cumulative histogram has `this->Count()` as the right-most bin. The `Count` method applied to the
      /// cumulative histogram is meaningless, as are `Mean` and the other statistics functions.
      ///
      /// For a multi-dimensional histogram, the cumulative histogram has bin(i,j,k) equal to the sum of all bins
      /// with indices equal or smaller to i, j and k: bin(1..i,1..j,1..k). It is computed through the
      /// `dip::CumulativeSum` function.
      DIP_EXPORT Histogram Cumulative() const;

      /// \brief Returns the marginal histogram for dimension `dim`.
      ///
      /// The marginal histogram represents the marginal intensity distribution. It is a 1D histogram determined
      /// by summing over all dimensions except `dim`, and is equivalent to the histogram for tensor element
      /// `dim`.
      DIP_EXPORT Histogram Marginal( dip::uint dim ) const;

      /// \brief Returns a smoothed version of the histogram, using Gaussian smoothing with parameters `sigma`.
      ///
      /// Set a single sigma value, or one value per image dimension. The value is in bins, yielding a Gaussian kernel
      /// of size `2 * std::ceil( 3 * sigma ) + 1` bins. See `dip::GaussFIR` for information on the smoothing operation applied.
      /// `sigma` defaults to 1.
      ///
      /// The output histogram is larger than the input histogram: it is extended by `std::ceil( 3 * sigma )` below and
      /// above the input bounds.
      DIP_EXPORT Histogram Smooth( FloatArray sigma ) const;
      Histogram Smooth( dfloat sigma = 1 ) const {
         return Smooth( FloatArray{ sigma } );
      }

   private:
      Image data_;             // This is where the bins are stored. Always scalar and DT_UINT32.
      FloatArray lowerBounds_; // These are the lower bounds of the histogram along each dimension.
      FloatArray binSizes_;    // These are the sizes of the bins along each dimension.
      // Compute the upper bound by : lowerBounds_[ii] + binSizes_[ii]*data_.Sizes(ii).
      // data_.Dimensionality() == lowerBounds_.size() == binSizes_.size()
      // Lower and upper bounds are not bin centers!

      dfloat FindBin( dfloat value, dip::uint dim ) const {
         return std::floor(( value - lowerBounds_[ dim ] ) / binSizes_[ dim ] );
      }
      dip::uint FindClampedBin( dfloat value, dip::uint dim ) const {
         dfloat bin = clamp( FindBin( value, dim ), 0.0, static_cast< dfloat >( data_.Size( dim ) - 1 ));
         return static_cast< dip::uint >( bin );
      }

      DIP_EXPORT void ScalarImageHistogram( Image const& input, Image const& mask, Configuration& configuration );
      DIP_EXPORT void TensorImageHistogram( Image const& input, Image const& mask, ConfigurationArray& configuration );
      DIP_EXPORT void JointImageHistogram( Image const& input1, Image const& input2, Image const& mask, ConfigurationArray& configuration );
      DIP_EXPORT void MeasurementFeatureHistogram( Measurement::IteratorFeature const& featureValues, ConfigurationArray& configuration );
};

//
// Computing image statistics from the histogram
//

/// \brief Computes the mean value of the data represented by the histogram.
///
/// Computing statistics through the histogram is efficient, but yields an approximation equivalent to
/// computing the statistic on data rounded to the bin centers.
DIP_EXPORT FloatArray Mean( Histogram const& in );

/// \brief Computes the covariance matrix of the data represented by the histogram.
///
/// Computing statistics through the histogram is efficient, but yields an approximation equivalent to
/// computing the statistic on data rounded to the bin centers.
///
/// The returned array contains the elements of the symmetric covariance matrix in the same order as tensor
/// elements are stored in a symmetric tensor image (see `dip::Tensor::Shape`). That is, there are `n`*(`n`+1)/2
/// elements (with `n` the histogram dimensionality), with the diagonal matrix elements stored first, and the
/// off-diagonal elements after. For a 2D histogram, the three elements are *xx*, *yy*, and *xy*.
DIP_EXPORT FloatArray Covariance( Histogram const& in );

/// \brief Computes the marginal median value of the data represented by the histogram. The marginal median
/// is a median computed independently on each pixel, and thus is not one of the input values.
///
/// In the 1D histogram case (for scalar images) this function computes the approximate median (i.e. the
/// bin containing the median value). The distinction between marginal median and median is only relevant
/// for multivariate data (histograms from tensor images). In short, here we compute 1D medians on each
/// of the 1D projections of the histogram.
///
/// Computing statistics through the histogram is efficient, but yields an approximation equivalent to
/// computing the statistic on data rounded to the bin centers.
DIP_EXPORT FloatArray MarginalMedian( Histogram const& in );

/// \brief Returns the mode, the bin with the largest count. The return value is the
///
/// When multiple bins have the same, largest count, the first bin encountered is returned. This is the bin
/// with the lowest linear index, and is closest to the origin.
DIP_EXPORT FloatArray Mode( Histogram const& in );

/// \brief Calculates the mutual information, in bits, between two images from their joint histogram `in`.
///
/// `in` must be a 2D histogram. The number of bins along each axis determines the precision for the result.
DIP_EXPORT dfloat MutualInformation( Histogram const& in );

/// \brief Calculates the entropy, in bits, of an image from its histogram `in`.
///
/// `in` must be a 1D histogram. The number of bins determines the precision for the result.
DIP_EXPORT dfloat Entropy( Histogram const& in );

//
// Computing image thresholds from the histogram
//

/// \brief Determines a set of `nThresholds` thresholds using the Isodata algorithm (k-means clustering),
/// and the image's histogram `in`.
///
/// The algorithm uses k-means clustering (with k set to `nThresholds + 1`) to separate the histogram into
/// compact, similarly-weighted segments. This means that, for each class, the histogram should have a clearly
/// visible mode, and all modes should be similarly sized. A class that has many fewer pixels than another class
/// will likely not be segmented correctly.
///
/// The implementation here uses initial seeds distributed evenly over the histogram range, rather than
/// the more common random seeds. This fixed initialization makes this a deterministic algorithm.
///
/// Note that the original Isodata algorithm (referenced below) does not use the image histogram, but instead
/// works directly on the image. 2-means clustering on the histogram yields an identical result to the original
/// Isodata algorithm, but is much more efficient. The implementation here generalizes to multiple thresholds
/// because k-means clustering allows any number of thresholds.
///
/// **Literature**
/// - T.W. Ridler, and S. Calvard, "Picture Thresholding Using an Iterative Selection Method", IEEE Transactions on Systems, Man, and Cybernetics 8(8):630-632, 1978.
DIP_EXPORT FloatArray IsodataThreshold(
      Histogram const& in,
      dip::uint nThresholds = 1
);

/// \brief Determines a threshold using the maximal inter-class variance method by Otsu, and the image's histogram `in`.
///
/// This method assumes a bimodal distribution. It finds the threshold that maximizes the inter-class variance, which
/// is equivalent to minimizing the inter-class variances. That is, the two parts of the histogram generated when
/// splitting at the threshold value are as compact as possible.
///
/// **Literature**
/// - N. Otsu, "A threshold selection method from gray-level histograms", IEEE Transactions on Systems, Man, and Cybernetics 9(1):62-66, 1979.
DIP_EXPORT dfloat OtsuThreshold(
      Histogram const& in
);

/// \brief Determines a threshold using the minimal error method method, and the image's histogram `in`.
///
/// This method assumes a bimodal distribution, composed of two Gaussian distributions with (potentially) different
/// variances, and finds the threshold that minimizes the classification error. The algorithm, however, doesn't try
/// to fit two Gaussians to the data, instead uses an error measure that depends on the second order central moment
/// for the two regions of the histogram obtained by dividing it at a given threshold value. The threshold with the
/// lowest error measure is returned.
///
/// **Literature**
/// - J. Kittler, and J. Illingworth, "Minimum error thresholding", Pattern Recognition 19(1):41-47, 1986.
DIP_EXPORT dfloat MinimumErrorThreshold(
      Histogram const& in
);

/// \brief Determines a threshold using the using the chord method (a.k.a. skewed bi-modality, maximum distance
/// to triangle), and the image's histogram `in`.
///
/// This method finds the point along the intensity distribution that is furthest from the line between the peak and
/// either of the histogram ends. This typically coincides or is close to the inflection point of a unimodal distribution
/// where the background forms the large peak, and the foreground contributes a small amount to the histogram and is
/// spread out. For example, small fluorescent dots typically yield such a distribution, as does any thin line drawing.
///
/// **Literature**
/// - G.W. Zack, W.E. Rogers, and S.A. Latt, "Automatic measurement of sister chromatid exchange frequency", Journal of Histochemistry and Cytochemistry 25(7):741-753, 1977.
/// - P.L. Rosin, "Unimodal thresholding", Pattern Recognition 34(11):2083-2096, 2001.
DIP_EXPORT dfloat TriangleThreshold(
      Histogram const& in
);

/// \brief Determines a threshold using the unimodal background-symmetry method, and the image's histogram `in`.
///
/// The method finds the peak in the intensity distribution, characterizes its half-width at half-maximum, then sets
/// the threshold at `distance` times the half-width.
///
/// This method assumes a unimodal distribution, where the background forms the large peak, and the foreground
/// contributes a small amount to the histogram and is spread out. For example, small fluorescent dots typically
/// yield such a distribution, as does any thin line drawing. The background peak can be at either end of the
/// histogram.
DIP_EXPORT dfloat BackgroundThreshold(
      Histogram const& in,
      dfloat distance = 2.0
);


/// \}

} // namespace dip

#endif // DIP_HISTOGRAM_H
