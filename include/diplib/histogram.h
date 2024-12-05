/*
 * (c)2017-2024, Cris Luengo.
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

#include <cmath>
#include <ostream>
#include <utility>

#include "diplib.h"
#include "diplib/iterators.h"
#include "diplib/lookup_table.h"
#include "diplib/measurement.h"


/// \file
/// \brief Histograms and related functionality.
/// See \ref histograms.


namespace dip {


/// \group histograms Histograms
/// \brief Histograms and related functionality.
/// \addtogroup

namespace detail {
inline dip::sint FindBin( dfloat value, dfloat lowerBound, dfloat binSize, dip::uint nBins ) {
   return static_cast< dip::sint >( clamp(( value - lowerBound ) / binSize, 0.0, static_cast< dfloat >( nBins - 1 )));
   // the cast does implicit floor because it's always non-negative
}
}


/// \brief Computes and holds histograms.
///
/// A histogram is computed by the constructor. A default-constructed `Histogram` is empty and can
/// only be assigned to.
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
      /// \brief Type of histogram bins. See \ref dip::DT_COUNT.
      using CountType = uint64;

      /// \brief Configuration information for how the histogram is computed.
      ///
      /// Constructors exist to set different subsets of the configuration; write upper bound and bin size
      /// as a floating point number so the right constructor can be selected (they differ in the location
      /// of the integer value). For example, note the difference between the following constructor calls,
      /// where `10` indicates 10 bins, `100.0` indicates the upper bound, and `1.0` indicates the bin size:
      ///
      /// ```cpp
      /// dip::Histogram::Configuration conf1( 0.0, 100.0, 1.0 );
      /// dip::Histogram::Configuration conf1( 0.0, 100.0, 10 );
      /// dip::Histogram::Configuration conf2( 0.0, 10, 1.0 );
      /// ```
      ///
      /// An additional constructor takes a \ref dip::DataType, and selects appropriate values for an image of the
      /// given data type; see \ref Configuration(DataType).
      ///
      /// The functions \ref OptimalConfiguration and \ref OptimalConfigurationWithFullRange create configurations
      /// that are expected to be robust for arbitrary data. They choose the bin size based on the Freedman--Diaconis
      /// rule. The former chooses histogram bounds to exclude only extreme outliers, the latter always includes the
      /// full range. Note that including the full range can potentially lead to an extremely large histogram.
      ///
      /// Here are the rules followed to complete the configuration given:
      ///
      /// - Any illegal values in the configuration will silently be replaced with the default values.
      ///
      /// - For integer images, the bin size and bounds will be forced to integer values.
      ///
      /// - For integer images, if `mode == Mode::COMPUTE_BINSIZE`, the upper bound will be adjusted so that a whole
      ///   number of integer-sized bins fit within the bounds.
      ///
      /// - If `mode == Mode::COMPUTE_BINS`, the bin size is adjusted to that a whole number of bins fit within
      ///   the given bounds. Except for integer images, where the bin size must be an integer as well. In this case,
      ///   the upper bound is adjusted instead.
      ///
      /// - If `mode == Mode::COMPUTE_BINS`, `binSize` was set to zero or a negative value, and the input image is of
      ///   an integer type, then `binSize` will be computed to be an integer power of two, and such that there
      ///   are no more than 256 bins in the histogram.
      ///
      /// - For integer images, if the bin centers are not whole numbers, the bounds are shifted down by half
      ///   to make the bin centers whole numbers. This should not affect the computed histogram, but make the display
      ///   prettier.
      struct Configuration {
         dfloat lowerBound = 0.0;      ///< Lower bound for this dimension, corresponds to the lower bound of the first bin.
         dfloat upperBound = 256.0;    ///< Upper bound for this dimension, corresponds to the upper bound of the last bin.
         dip::uint nBins = 256;        ///< Number of bins for this dimension.
         dfloat binSize = 1.0;         ///< Size of each bin for this dimension.
         /// How to complete the configuration
         enum class Mode : uint8 {
               COMPUTE_BINSIZE,     ///< Compute `binSize` from the other three values
               COMPUTE_BINS,        ///< Compute `nBins` from the other three values
               COMPUTE_LOWER,       ///< Compute `lowerBound` from the other three values
               COMPUTE_UPPER,       ///< Compute `upperBound` from the other three values
               ESTIMATE_BINSIZE,    ///< Choose `binSize` using the Freedman--Diaconis rule, then compute `nBins`.
                                    /// If the data is not available to estimate `binSize`, 256 bins will be made.
               ESTIMATE_BINSIZE_AND_LIMITS,///< Like `ESTIMATE_BINSIZE`, but also determines the lower and upper limits
                                           /// to exclude outliers, defined as samples below three interquartile ranges
                                           /// from the lower quartile, and above three interquartile ranges above the
                                           /// upper quartile. Ignores all configuration values.
               IS_COMPLETE          ///< The configuration values will be taken as-is. `lowerIsPercentile` and
                                    /// `upperIsPercentile` will be ignored. Weird things will happen if the configuration
                                    /// is not correct, we might even end up writing out of bounds!
         };
         Mode mode = Mode::COMPUTE_BINSIZE;     ///< The given value is ignored and replaced by the computed value.
         bool lowerIsPercentile = false;        ///< If set, `lowerBound` is replaced by the given percentile pixel value.
         bool upperIsPercentile = false;        ///< If set, `upperBound` is replaced by the given percentile pixel value.
         bool excludeOutOfBoundValues = false;  ///< If set, pixels outside of the histogram bounds are not counted.

         /// \brief Default-constructed configuration defines 256 bins in the range [0,256].
         Configuration() = default;
         /// \brief A constructor takes a lower and upper bounds, and the bin size. The number of bins are computed.
         Configuration( dfloat lowerBound, dfloat upperBound, dfloat binSize ) :
               lowerBound( lowerBound ), upperBound( upperBound ), binSize( binSize ), mode( Mode::COMPUTE_BINS ) {}
         /// \brief A constructor takes a lower and upper bounds, and the number of bins. The bin size is computed.
         Configuration( dfloat lowerBound, dfloat upperBound, dip::uint nBins = 256 ) :
               lowerBound( lowerBound ), upperBound( upperBound ), nBins( nBins ) {}
         /// \brief A constructor takes a lower and upper bounds, and the number of bins. The bin size is computed.
         Configuration( dfloat lowerBound, dfloat upperBound, int nBins ) :
               Configuration( lowerBound, upperBound, clamp_cast< dip::uint >( nBins )) {}
         /// \brief A constructor takes a lower bound, the number of bins and the bin size. The upper bound is computed.
         Configuration( dfloat lowerBound, dip::uint nBins, dfloat binSize ) :
               lowerBound( lowerBound ), nBins( nBins ), binSize( binSize ), mode( Mode::COMPUTE_UPPER ) {}
         /// \brief A constructor takes a lower bound, the number of bins and the bin size. The upper bound is computed.
         Configuration( dfloat lowerBound, int nBins, dfloat binSize ) :
               Configuration( lowerBound, clamp_cast< dip::uint >( nBins ), binSize ) {}
         /// \brief A constructor takes an image data type, yielding a default histogram configuration for that data type.
         ///
         /// - For 8-bit images, the histogram has 256 bins, one for each possible input value.
         /// - For other integer-valued images, the histogram has up to 256 bins, stretching from the lowest value
         ///   in the image to the highest, and with bin size a power of two. This is the simplest way of correctly
         ///   handling data from 10-bit, 12-bit and 16-bit sensors that can put data in the lower or the upper bits
         ///   of the 16-bit words, and will handle other integer data correctly as well.
         /// - For floating-point images, the histogram always has 256 bins, stretching from the lowest value in the
         ///   image to the highest.
         explicit Configuration( DataType dataType ) {
            if( dataType == DT_UINT8 ) {
               // 256 bins between 0 and 256, this is the default.
            } else if ( dataType == DT_SINT8 ) {
               // 256 bins between -128 and 128.
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

         /// Returns true if the value should not be included in the histogram.
         bool IsOutOfRange( dfloat value ) const {
            return( excludeOutOfBoundValues && (( value < lowerBound ) || ( value >= upperBound )));
         }

         /// \brief Returns the bin that the value belongs in, assuming `!IsOutOfRange(value)`.
         dip::sint FindBin( dfloat value ) const {
            return detail::FindBin( value, lowerBound, binSize, nBins );
         }

         // The functions below are not part of the public interface, though I need them to be available
         // to functions in the library outside of the dip::Histogram class. Therefore they are public
         // functions, but they are not documented publicly.

         // Complete the configuration, computing the value given by `mode`. Percentiles will not be
         // computed. For integer images, bin sizes and bin centers are forced to be integer.
         DIP_EXPORT void Complete( bool isInteger );

         // Complete the configuration, computing the value given by `mode`, as well as percentiles if
         // required. For integer images, bin sizes and bin centers are forced to be integer.
         DIP_EXPORT void Complete( Image const& input, Image const& mask = {} );

         // Complete the configuration, computing the value given by `mode`, as well as percentiles if
         // required.
         DIP_EXPORT void Complete( Measurement::IteratorFeature const& featureValues ) {
            Complete( featureValues.AsScalarImage() );
         }
      };

      /// \brief Creates a \ref Configuration that uses the optimal bin size according to the Freedman--Diaconis rule.
      ///
      /// The Freedman--Diaconis rule sets the bin size to $2 \text{IQR} / \sqrt[3]{n}$, where IQR is the
      /// interquartile range (the distance from the lower to the upper quartile), and $n$ is the number of samples
      /// over which the histogram will be computed.
      ///
      /// The histogram limits are chosen to avoid extremely large histograms, by ignoring values 50 IQRs below the
      /// lower quartile or above the upper quartile.
      static Configuration OptimalConfiguration() {
         Configuration conf;
         conf.mode = Configuration::Mode::ESTIMATE_BINSIZE_AND_LIMITS;
         conf.excludeOutOfBoundValues = true;
         return conf;
      };

      /// \brief Like \ref OptimalConfiguration, but includes the full data range (min to max); note that this can
      /// potentially lead to extremely large histograms.
      static Configuration OptimalConfigurationWithFullRange() {
         Configuration conf( 0.0, 100.0 );
         conf.lowerIsPercentile = true;
         conf.upperIsPercentile = true;
         conf.mode = Configuration::Mode::ESTIMATE_BINSIZE;
         return conf;
      };

      /// \brief An array of \ref Configuration objects, one per histogram dimension.
      using ConfigurationArray = DimensionArray< Configuration >;

      /// \brief The constructor takes an image, an optional mask, and configuration options for each histogram
      /// dimension.
      ///
      /// `configuration` should have as many elements as tensor elements in `input`. If `configuration` has only
      /// one element, it will be used for all histogram dimensions. If it is an empty array, appropriate configuration
      /// values for `input` are chosen based on its data type (see \ref Configuration::Configuration(DataType)).
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
      explicit Histogram( Image::View const& input, ConfigurationArray configuration = {} ) {
         if( input.Offsets().empty() ) {
            // This code works if either the view is regular or has a mask.
            *this = Histogram( input.Reference(), input.Mask(), std::move( configuration ));
         } else {
            // When the view uses indices, we copy the data over to a new image, it's not worth while writing separate code for this case.
            *this = Histogram( Image( input ), {}, std::move( configuration ));
         }
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
      Histogram( Image::View const& input, Configuration configuration ) {
         if( input.Offsets().empty() ) {
            // This code works if either the view is regular or has a mask.
            *this = Histogram( input.Reference(), input.Mask(), configuration );
         } else {
            // When the view uses indices, we copy the data over to a new image, it's not worth while writing separate code for this case.
            *this = Histogram( Image( input ), {}, configuration );
         }
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

      /// \brief The constructor takes an `IteratorFeature` of a \ref dip::Measurement object, and configuration
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

      /// \brief An empty histogram with the given configuration. Histogram bins are initialized to 0.
      ///
      /// The array must not be empty. The histogram will have `configuration->size()` dimensions.
      /// `configuration[ii].lowerIsPercentile` and `configuration[ii].upperIsPercentile` must all be false (since
      /// there is no way of determining these percentiles).
      ///
      /// The `GetImage` method returns a const reference to the histogram bins (in the form of an image),
      /// but it is possible to modify the values in the bins (modify the pixel values of this image).
      explicit Histogram( ConfigurationArray configuration ) {
         DIP_THROW_IF( configuration.empty(), E::ARRAY_PARAMETER_WRONG_LENGTH );
         DIP_STACK_TRACE_THIS( EmptyHistogram( std::move( configuration )));
      }

      /// \brief An empty histogram with the given configuration. Histogram bins are initialized to 0.
      ///
      /// The histogram will have one dimension.
      /// `configuration.lowerIsPercentile` and `configuration.upperIsPercentile` must both be false (since
      /// there is no way of determining these percentiles).
      ///
      /// The `GetImage` method returns a const reference to the histogram bins (in the form of an image),
      /// but it is possible to modify the values in the bins (modify the pixel values of this image).
      explicit Histogram( Configuration const& configuration ) {
         DIP_STACK_TRACE_THIS( EmptyHistogram( ConfigurationArray{ configuration } ));
      }

      /// \brief Create a 1D histogram around existing data. No ownership is transferred.
      ///
      /// `data` is a raw pointer to the data that will be encapsulated by the output histogram. Data must
      /// be contiguous and of type \ref dip::Histogram::CountType. `configuration` determines how many
      /// bins are pointed to by `data`.
      ///
      /// Even though the data pointer is declared `const` here, it is possible to obtain a non-const
      /// pointer to the data later.
      Histogram( CountType const* data, Configuration const& configuration ) {
         DIP_STACK_TRACE_THIS( HistogramFromDataPointer( data, configuration ));
      }

      /// \brief The default-initialized histogram is empty and can only be assigned to.
      Histogram() = default;

      /// \brief Swaps `this` and `other`.
      void swap( Histogram& other ) noexcept {
         using std::swap;
         swap( data_, other.data_ );
         swap( lowerBounds_, other.lowerBounds_ );
         swap( binSizes_, other.binSizes_ );
      }

      /// \brief Returns false for a default-initialized histogram.
      bool IsInitialized() const {
         return data_.IsForged();
      }

      /// \brief Deep copy, returns a copy of `this` with its own data segment.
      ///
      /// When making a copy of a histogram, the data segment is shared:
      ///
      /// ```cpp
      /// Histogram second = first;
      /// second.Smooth(); // modifies `first` also!
      /// ```
      ///
      /// In contrast, this function returns a deep copy of `this`, with its own data segment:
      ///
      /// ```cpp
      /// Histogram second = first.Copy();
      /// second.Smooth(); // OK
      /// ```
      Histogram Copy() const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         Histogram out( *this );
         out.data_ = data_.Copy();
         return out;
      }

      /// \brief Performs a reverse histogram lookup, yielding an image "painted" with the bin values.
      ///
      /// The bin corresponding to each pixel in the input image is found in the histogram, and this bin's
      /// value is written to `output`.
      ///
      /// `input` must be similar to the image used to generate the histogram (at least have the same number
      /// of tensor elements, which corresponds to the dimensionality of the histogram). The lookup occurs
      /// in the same way as when generating the histogram. The `excludeOutOfBoundValues` parameter for each
      /// dimension indicates if an edge bin is found or no bin is found (yielding a 0 output) if the pixel
      /// is not represented in the histogram.
      ///
      /// `output` is a scalar image with data type \ref dip::Histogram::CountType, containing the histogram
      /// bin values.
      ///
      /// This function is particularly interesting when applied to a histogram resulting from clustering
      /// algorithms such as \ref dip::KMeansClustering( dip::Histogram const&, dip::uint ) or
      /// \ref dip::MinimumVariancePartitioning( dip::Histogram const&, dip::uint ).
      DIP_EXPORT void ReverseLookup( Image const& input, Image& out, BooleanArray excludeOutOfBoundValues = { false } );
      DIP_NODISCARD Image ReverseLookup( Image const& input, BooleanArray excludeOutOfBoundValues = { false } ) {
         Image out;
         ReverseLookup( input, out, std::move( excludeOutOfBoundValues ));
         return out;
      }

      /// \brief Adds a histogram to *this. `other` must have identical properties.
      ///
      /// Adding multiple histograms together can be useful, for example, when accumulating pixel values
      /// from multiple images, or in multiple threads.
      Histogram& operator+=( Histogram const& other ) {
         DIP_THROW_IF( !IsInitialized() || !other.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF(( data_.Sizes() != other.data_.Sizes() ||
                      ( lowerBounds_ != other.lowerBounds_ ) ||
                      ( binSizes_ != other.binSizes_ )), "Histograms don't match" );
         data_ += other.data_;
         return *this;
      }

      /// \brief Subtracts a histogram from *this, using saturated subtraction. `other` must have identical properties.
      Histogram& operator-=( Histogram const& other ) {
         DIP_THROW_IF( !IsInitialized() || !other.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF(( data_.Sizes() != other.data_.Sizes() ||
                      ( lowerBounds_ != other.lowerBounds_ ) ||
                      ( binSizes_ != other.binSizes_ )), "Histograms don't match" );
         data_ -= other.data_;
         return *this;
      }

      /// \brief Returns the histogram dimensionality.
      dip::uint Dimensionality() const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         return data_.Dimensionality();
      }

      /// \brief Returns the number of bins along dimension `dim`
      dip::uint Bins( dip::uint dim = 0 ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( dim >= Dimensionality(), E::INVALID_PARAMETER );
         return data_.Size( dim );
      }

      /// \brief Returns the size of the bins along dimension `dim`
      dfloat BinSize( dip::uint dim = 0 ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( dim >= Dimensionality(), E::INVALID_PARAMETER );
         return binSizes_[ dim ];
      }

      /// \brief Returns the lower bound of the histogram for dimension `dim`
      dfloat LowerBound( dip::uint dim = 0 ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( dim >= Dimensionality(), E::INVALID_PARAMETER );
         return lowerBounds_[ dim ];
      }

      /// \brief Returns the upper bound of the histogram for dimension `dim`
      dfloat UpperBound( dip::uint dim = 0 ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( dim >= Dimensionality(), E::INVALID_PARAMETER );
         return lowerBounds_[ dim ] + static_cast< dfloat >( data_.Size( dim )) * binSizes_[ dim ];
      }

      /// \brief Returns the bin boundaries along dimension `dim` (`Bins(dim)+1` values).
      FloatArray BinBoundaries( dip::uint dim = 0 ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( dim >= Dimensionality(), E::INVALID_PARAMETER );
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
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( dim >= Dimensionality(), E::INVALID_PARAMETER );
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
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( dim >= Dimensionality(), E::INVALID_PARAMETER );
         return lowerBounds_[ dim ] + ( static_cast< dfloat >( bin ) + 0.5 ) * binSizes_[ dim ];
      }

      /// \brief Gets the bin for `value` in a 1D histogram
      dip::uint Bin( dfloat value ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( Dimensionality() != 1, E::ILLEGAL_DIMENSIONALITY );
         return FindClampedBin( value, 0 );
      }

      /// \brief Gets the bin for {`x_value`, `y_value`} in a 2D histogram
      UnsignedArray Bin( dfloat x_value, dfloat y_value ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( Dimensionality() != 2, E::ILLEGAL_DIMENSIONALITY );
         return { FindClampedBin( x_value, 0 ),
                  FindClampedBin( y_value, 1 ) };
      }

      /// \brief Gets the bin for {`x_value`, `y_value`, `z_value`} in a 3D histogram
      UnsignedArray Bin( dfloat x_value, dfloat y_value, dfloat z_value ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( Dimensionality() != 3, E::ILLEGAL_DIMENSIONALITY );
         return { FindClampedBin( x_value, 0 ),
                  FindClampedBin( y_value, 1 ),
                  FindClampedBin( z_value, 2 ) };
      }

      /// \brief Gets the bin for `value` in an nD histogram
      UnsignedArray Bin( FloatArray const& value ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( Dimensionality() != value.size(), E::ARRAY_PARAMETER_WRONG_LENGTH );
         UnsignedArray out( value.size() );
         for( dip::uint ii = 0; ii < value.size(); ++ii ) {
            out[ ii ] = FindClampedBin( value[ ii ], ii );
         }
         return out;
      }

      /// \brief Get the value at the given bin in a 1D histogram
      CountType At( dip::uint x ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( Dimensionality() != 1, E::ILLEGAL_DIMENSIONALITY );
         DIP_THROW_IF( x >= data_.Size( 0 ), E::INDEX_OUT_OF_RANGE );
         return *static_cast< CountType* >( data_.Pointer( static_cast< dip::sint >( x ) * data_.Stride( 0 ) ));
      }
      /// \brief Get the value at the given bin in a 2D histogram
      CountType At( dip::uint x, dip::uint y ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         DIP_THROW_IF( Dimensionality() != 2, E::ILLEGAL_DIMENSIONALITY );
         DIP_THROW_IF( x >= data_.Size( 0 ), E::INDEX_OUT_OF_RANGE );
         DIP_THROW_IF( y >= data_.Size( 1 ), E::INDEX_OUT_OF_RANGE );
         return *static_cast< CountType* >( data_.Pointer( static_cast< dip::sint >( x ) * data_.Stride( 0 ) +
                                                           static_cast< dip::sint >( y ) * data_.Stride( 1 )));
      }

      /// \brief Get the value at the given bin in a 3D histogram
      CountType At( dip::uint x, dip::uint y, dip::uint z ) const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
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
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         return *static_cast< CountType* >( data_.Pointer( bin )); // Does all the checking
      }

      /// \brief Get the image that holds the bin counts. The image is always scalar and of type \ref dip::DT_COUNT.
      Image const& GetImage() const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         return data_;
      }

      /// \brief Returns an iterator to the first bin
      ConstImageIterator< CountType > begin() const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         return ConstImageIterator< CountType >( data_ );
      }

      /// \brief Returns an end iterator
      static ConstImageIterator< CountType > end() {
         return {};
      }

      /// \brief Returns a pointer to the first bin
      CountType const* Origin() const {
         DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
         return static_cast< CountType const* >( data_.Origin() );
      }

      /// \brief Returns the total number of elements in the histogram (sum of bins)
      DIP_EXPORT dip::uint Count() const;

      /// \brief Converts the histogram to a cumulative histogram. For each bin, it will contain the sum of that
      /// bin with all the previous ones.
      ///
      /// The cumulative histogram has `this->Count()` as the right-most bin. The `Count` method applied to the
      /// cumulative histogram is meaningless, as are `Mean` and the other statistics functions.
      ///
      /// For a multi-dimensional histogram, the cumulative histogram has `bin(i,j,k)` equal to the sum of all bins
      /// with indices equal or smaller to `i`, `j` and `k`: `bin(1..i,1..j,1..k)`. It is computed through the
      /// \ref dip::CumulativeSum function.
      DIP_EXPORT Histogram& Cumulative();

      /// \brief Returns the marginal histogram for dimension `dim`.
      ///
      /// The marginal histogram represents the marginal intensity distribution. It is a 1D histogram determined
      /// by summing over all dimensions except `dim`, and is equivalent to the histogram for tensor element
      /// `dim`.
      DIP_EXPORT Histogram GetMarginal( dip::uint dim ) const;

      /// \brief Smooths the histogram, using Gaussian smoothing with parameters `sigma`.
      ///
      /// Set a single sigma value, or one value per histogram dimension. The value is in bins, yielding a Gaussian
      /// kernel of size `2 * std::ceil( 3 * sigma ) + 1` bins. See \ref dip::GaussFIR for information on the smoothing
      /// operation applied. `sigma` defaults to 1.
      ///
      /// The histogram is extended by `std::ceil( 3 * sigma )` below and above the original bounds, to prevent the
      /// histogram count to change.
      DIP_EXPORT Histogram& Smooth( FloatArray sigma );
      Histogram& Smooth( dfloat sigma = 1 ) {
         return Smooth( FloatArray{ sigma } );
      }

   private:
      Image data_;             // This is where the bins are stored. Always scalar and DT_COUNT.
      FloatArray lowerBounds_; // These are the lower bounds of the histogram along each dimension.
      FloatArray binSizes_;    // These are the sizes of the bins along each dimension.
      // Compute the upper bound by : lowerBounds_[ii] + binSizes_[ii]*data_.Size(ii).
      // data_.Dimensionality() == lowerBounds_.size() == binSizes_.size()
      // Lower and upper bounds are not bin centers!

      dip::uint FindClampedBin( dfloat value, dip::uint dim ) const {
         return static_cast< dip::uint >( detail::FindBin( value, lowerBounds_[ dim ], binSizes_[ dim ], data_.Size( dim )));
      }

      DIP_EXPORT void ScalarImageHistogram( Image const& input, Image const& mask, Configuration& configuration );
      DIP_EXPORT void TensorImageHistogram( Image const& input, Image const& mask, ConfigurationArray& configuration );
      DIP_EXPORT void JointImageHistogram( Image const& input1, Image const& input2, Image const& mask, ConfigurationArray& configuration );
      DIP_EXPORT void MeasurementFeatureHistogram( Measurement::IteratorFeature const& featureValues, ConfigurationArray& configuration );
      DIP_EXPORT void EmptyHistogram( ConfigurationArray configuration );
      DIP_EXPORT void HistogramFromDataPointer( CountType const* data, Configuration const& configuration );
};

/// \brief Data type of histogram bins. See \ref dip::Histogram::CountType.
/// \ingroup pixeltypes
/// \relates dip::Histogram
constexpr DataType DT_COUNT{ Histogram::CountType{} };


//
// Operators
//

inline void swap( Histogram& v1, Histogram& v2 ) noexcept {
   v1.swap( v2 );
}

/// \brief Adds two histograms.
/// \relates dip::Histogram
inline Histogram operator+( Histogram const& lhs, Histogram const& rhs ) {
   Histogram out = lhs.Copy();
   out += rhs;
   return out;
}

/// \brief Subtracts two histograms.
/// \relates dip::Histogram
inline Histogram operator-( Histogram const& lhs, Histogram const& rhs ) {
   Histogram out = lhs.Copy();
   out -= rhs;
   return out;
}

/// \brief You can output a \ref dip::Histogram to `std::cout` or any other stream. Some
/// information about the histogram is printed.
/// \relates dip::Distribution
inline std::ostream& operator<<(
      std::ostream& os,
      Histogram const& histogram
) {
   auto print_dim_info = [ & ]( dip::uint ii ) {
      os << histogram.Bins( ii ) << " bins"
                           << ", lower bound: " << histogram.LowerBound( ii )
                           << ", upper bound: " << histogram.UpperBound( ii )
                           << ", bin size: " << histogram.BinSize( ii );
   };

   if( histogram.IsInitialized()) {
      dip::uint nd = histogram.Dimensionality(); // Must be nd >= 1
      os << nd << "D histogram:";
      if( nd == 1 ) {
         os << ' ';
         print_dim_info( 0 );
         os << '\n';
      } else {
         os << '\n';
         for( dip::uint ii = 0; ii < nd; ++ii ) {
            os << "    dimension " << ii << ": ";
            print_dim_info( ii );
            os << '\n';
         }
      }
   } else {
      os << "Uninitialized histogram\n";
   }
   return os;
}


//
// Creating modified histograms
//

/// \brief Computes a cumulative histogram from `in`. See \ref dip::Histogram::Cumulative.
inline Histogram CumulativeHistogram( Histogram const& in ) {
   DIP_THROW_IF( !in.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   Histogram out = in.Copy();
   out.Cumulative();
   return out;
}

/// \brief Returns a smoothed version of the histogram `in`. See \ref dip::Histogram::Smooth.
inline Histogram Smooth( Histogram const& in, FloatArray const& sigma ) {
   DIP_THROW_IF( !in.IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   Histogram out = in.Copy();
   out.Smooth( sigma );
   return out;
}

/// \brief Returns a smoothed version of the histogram `in`. See \ref dip::Histogram::Smooth.
inline Histogram Smooth( Histogram const& in, dfloat sigma = 1 ) {
   return Smooth( in, FloatArray{ sigma } );
}


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
/// elements are stored in a symmetric tensor image (see \ref dip::Tensor::Shape). That is, there are $\frac{1}{2}n(n+1)$
/// elements (with $n$ the histogram dimensionality), with the diagonal matrix elements stored first, and the
/// off-diagonal elements after. For a 2D histogram, the three elements are *xx*, *yy*, and *xy*.
DIP_EXPORT FloatArray Covariance( Histogram const& in );

/// \brief Computes the marginal percentile value of the data represented by the histogram. The marginal percentile
/// is a percentile computed independently on each dimension, and thus is not one of the input values.
///
/// In the 1D histogram case (for scalar images) this function computes the approximate percentile (i.e. the
/// bin containing the percentile value). The distinction between marginal percentile and percentile is only relevant
/// for multivariate data (histograms from tensor images). In short, here we compute 1D percentile on each
/// of the 1D projections of the histogram.
///
/// The `percentile` must be a value between 0 (minimum) and 100 (maximum).
///
/// Computing statistics through the histogram is efficient, but yields an approximation equivalent to
/// computing the statistic on data rounded to the bin centers.
DIP_EXPORT FloatArray MarginalPercentile( Histogram const& in, dfloat percentile = 50 );

/// \brief Computes the marginal median value of the data represented by the histogram. The median is the 50th
/// percentile, see `dip::MarginalPercentile` for details.
inline FloatArray MarginalMedian( Histogram const& in ) {
   return MarginalPercentile( in, 50 );
}

/// \brief Returns the mode, the bin with the largest count.
///
/// When multiple bins have the same, largest count, the first bin encountered is returned. This is the bin
/// with the lowest linear index.
DIP_EXPORT FloatArray Mode( Histogram const& in );

/// \brief Computes the Pearson correlation coefficient between two images from their joint histogram `in`.
///
/// `in` must be a 2D histogram. The number of bins along each axis determines the precision for the result.
inline dfloat PearsonCorrelation( Histogram const& in ) {
   DIP_THROW_IF( in.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   FloatArray cov = Covariance( in );
   DIP_ASSERT( cov.size() == 3 );
   dfloat denom = cov[ 0 ] * cov[ 1 ];
   return ( denom != 0.0 ) ? ( cov[ 2 ] / std::sqrt( denom )) : ( 0.0 );
}

/// \brief Fits a line through the histogram. Returns the slope and intercept of the regression line.
///
/// `in` must be a 2D histogram. The number of bins along each axis determines the precision for the result.
DIP_EXPORT RegressionParameters Regression( Histogram const& in );

/// \brief Calculates the mutual information, in bits, between two images from their joint histogram `in`.
///
/// `in` must be a 2D histogram. The number of bins along each axis determines the precision for the result.
DIP_EXPORT dfloat MutualInformation( Histogram const& in );

/// \brief Calculates the entropy, in bits, of an image from its histogram `in`.
///
/// `in` must be a 1D histogram. The number of bins determines the precision for the result.
DIP_EXPORT dfloat Entropy( Histogram const& in );

/// \brief Determines the parameters for a Gaussian Mixture Model fitted to the histogram `in`.
///
/// `numberOfGaussians` Gaussians will be fitted to the histogram using the Expectation Maximization (EM) procedure.
///
/// The parameters are initialized deterministically, the means are distributed equally over the domain,
/// the sigma are all set to the distance between means, and the amplitude are set to 1.
///
/// `maxIter` sets how many iterations are run. There is currently no other stopping criterion.
///
/// The output is sorted by amplitude, most important component first.
///
/// \see dip::GaussianMixtureModel(dip::ConstSampleIterator, dip::SampleIterator, dip::uint, dip::uint, dip::uint, dip::Option::Periodicity)
DIP_EXPORT std::vector< GaussianParameters > GaussianMixtureModel(
      Histogram const& in,
      dip::uint numberOfGaussians,
      dip::uint maxIter = 20
);

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
/// !!! literature
///     - T.W. Ridler, and S. Calvard, "Picture Thresholding Using an Iterative Selection Method", IEEE Transactions on Systems, Man, and Cybernetics 8(8):630-632, 1978.
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
/// !!! literature
///     - N. Otsu, "A threshold selection method from gray-level histograms", IEEE Transactions on Systems, Man, and Cybernetics 9(1):62-66, 1979.
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
/// !!! literature
///     - J. Kittler, and J. Illingworth, "Minimum error thresholding", Pattern Recognition 19(1):41-47, 1986.
DIP_EXPORT dfloat MinimumErrorThreshold(
      Histogram const& in
);

/// \brief Determines a set of `nThresholds` thresholds by modeling the histogram with a Gaussian Mixture Model,
/// and choosing the optimal Bayes thresholds.
///
/// The algorithm fits a mixture of `nThresholds + 1` Gaussians to the 1D histogram, and returns the thresholds
/// in between the fitted Gaussians that minimize the Bayes error (if possible). Note that the sum of a narrow
/// Gaussian and an overlapping broad Gaussian would typically yield two thresholds (dividing space into three
/// regions, the middle one belonging to the narrow Gaussian and the other two to the broad Gaussian). This
/// routine instead always returns a single threshold in between each of the Gaussian means.
///
/// \see dip::GaussianMixtureModel(Histogram const&, dip::uint, dip::uint)
DIP_EXPORT FloatArray GaussianMixtureModelThreshold(
      Histogram const& in,
      dip::uint nThresholds = 1
);

/// \brief Determines a threshold using the using the chord method (a.k.a. skewed bi-modality, maximum distance
/// to triangle), and the image's histogram `in`.
///
/// This method finds the point along the intensity distribution that is furthest from the line between the peak and
/// either of the histogram ends. This typically coincides or is close to the inflection point of a unimodal distribution
/// where the background forms the large peak, and the foreground contributes a small amount to the histogram and is
/// spread out. For example, small fluorescent dots typically yield such a distribution, as does any thin line drawing.
///
/// To robustly detect and characterize the background peak, smoothing is necessary. This function applies
/// a Gaussian filter with `sigma`, in samples (i.e. this value is independent of the bin width).
/// See \ref dip::Histogram::Smooth. Do note that smoothing also broadens the distribution.
///
/// !!! literature
///     - G.W. Zack, W.E. Rogers, and S.A. Latt, "Automatic measurement of sister chromatid exchange frequency", Journal of Histochemistry and Cytochemistry 25(7):741-753, 1977.
///     - P.L. Rosin, "Unimodal thresholding", Pattern Recognition 34(11):2083-2096, 2001.
DIP_EXPORT dfloat TriangleThreshold(
      Histogram const& in,
      dfloat sigma = 4.0
);

/// \brief Determines a threshold using the unimodal background-symmetry method, and the image's histogram `in`.
///
/// The method finds the peak in the intensity distribution, characterizes its half width at half maximum, then sets
/// the threshold at `distance` times the half width.
///
/// This method assumes a unimodal distribution, where the background forms the large peak, and the foreground
/// contributes a small amount to the histogram and is spread out. For example, small fluorescent dots typically
/// yield such a distribution, as does any thin line drawing. The background peak can be at either end of the
/// histogram. However, it is important that the peak is not clipped too much, for example when too many background
/// pixels in a fluorescence image are underexposed.
///
/// To robustly detect and characterize the background peak, smoothing is necessary. This function applies
/// a Gaussian filter with `sigma`, in samples (i.e. this value is independent of the bin width).
/// See \ref dip::Histogram::Smooth. Do note that smoothing also broadens the distribution; even though this
/// broadening is taken into account when computing the peak width, too much smoothing will be detrimental.
DIP_EXPORT dfloat BackgroundThreshold(
      Histogram const& in,
      dfloat distance = 2.0,
      dfloat sigma = 4.0
);


//
// Multi-dimensional histogram partitioning
//

/// \brief Partitions a (multi-dimensional) histogram into `nClusters` partitions using k-means clustering.
///
/// K-means clustering partitions the histogram into compact, similarly-weighted segments. The algorithm
/// uses a random initialization, so multiple runs might yield different results.
///
/// For 1D histograms, \ref dip::IsodataThreshold( Histogram const&, dip::uint ) is more efficient, and deterministic.
DIP_EXPORT Histogram KMeansClustering(
      Histogram const& in,
      dip::uint nClusters = 2
);

/// \brief Partitions a (multi-dimensional) histogram into `nClusters` partitions iteratively using Otsu
/// thresholding along individual dimensions.
///
/// Minimum variance partitioning builds a k-d tree of the histogram, where, for each node, the marginal histogram
/// with the largest variance is split using Otsu thresholding.
///
/// For two clusters in a 1D histogram, use \ref dip::OtsuThreshold( Histogram const& ).
DIP_EXPORT Histogram MinimumVariancePartitioning(
      Histogram const& in,
      dip::uint nClusters = 2
);


//
// Computing lookup tables from the histogram
//

/// \brief Computes a lookup table that, when applied to an image with the histogram `in`, yields an image with a
/// flat histogram (or rather a histogram that is as flat as possible).
///
/// The lookup table will be of type \ref dip::DT_DFLOAT, meaning that applying it to an image will yield an image
/// of that type. Convert the lookup table to a different type using \ref dip::LookupTable::Convert.
///
/// The lookup table will produce an output in the range [0,255].
///
/// `in` must be a 1D histogram.
DIP_EXPORT LookupTable EqualizationLookupTable(
      Histogram const& in
);

/// \brief Computes a lookup table that, when applied to an image with the histogram `in`, yields an image with a
/// histogram as similar as possible to `example`.
///
/// The lookup table will be of type \ref dip::DT_DFLOAT, meaning that applying it to an image will yield an image
/// of that type. Convert the lookup table to a different type using \ref dip::LookupTable::Convert.
///
/// The lookup table will produce an output in the range [`example.LowerBound()`,`example.UpperBound()`].
///
/// `in` and `example` must be 1D histograms.
DIP_EXPORT LookupTable MatchingLookupTable(
      Histogram const& in,
      Histogram const& example
);


class DIP_NO_EXPORT Distribution;

/// \brief Computes a histogram of grey values in `grey` for each object in `label`.
///
/// `label` is a labelled image. For each object, the corresponding pixels in `grey` are
/// used to build a histogram. `mask` can optionally be used to further constrain which
/// pixels are used. `grey` must be a real-valued image, but does not need to be scalar.
///
/// `configuration` describes the histogram. The same configuration is applied to each
/// of the histograms, they all use the same bins. Percentiles are computed over all tensor
/// components of `grey` (and masked by `mask`).
///
/// `mode` can be `"fraction"` (the default) or `"count"`. The former yields normalized
/// distributions, whereas the latter yields an integer pixel count per bin.
///
/// If `background` is `"include"`, the label ID 0 will be included in the result if present in the image.
/// Otherwise, `background` is `"exclude"`, and the label ID 0 will be ignored.
///
/// The output \ref dip::Distribution has bin centers as the *x* values, and one *y* value
/// per object and per tensor component. These are accessed as
/// `distribution[bin].Y(objectID, tensor)`, where `objectID` is the pixel values in `label`,
/// but subtract one if `background` is `"exclude"`.
///
/// Note that you will need to also include `<diplib/distribution.h>` to use this function.
DIP_EXPORT Distribution PerObjectHistogram(
      Image const& grey,
      Image const& label,
      Image const& mask = {},
      Histogram::Configuration configuration = {},
      String const& mode = S::FRACTION,
      String const& background = S::EXCLUDE
);


/// \endgroup

} // namespace dip

#endif // DIP_HISTOGRAM_H
