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


/// \file
/// \brief Declares the `dip::Histogram` classe, and related functionality.


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
/// one dimension per tensor elment. The first tensor element determines the index along the first
/// dimension, the second tensor element that along the second dimension, etc.
///
/// To facilitate useage for one-dimensional histograms, all getter functions that return a value
/// for a given dimension, default to dimension 0, so can be called without arguments.
///
/// TODO: A histogram can also be constructed from a measurement feature.
class DIP_NO_EXPORT Histogram {
   public:
      /// \brief Configuration information for how the histogram is computed.
      ///
      /// Note that if `mode == Mode::COMPUTE_BINS`, `binSize` will be adjusted so that a whole number of bins
      /// fits between the bounds.
      ///
      /// Any illegal values in the configuration will silently be replaced with the defalt values.
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
      };
      using ConfigurationArray = DimensionArray< Configuration >;

      /// \brief The constructor takes an image, an optional mask, and configuration options for each histogram
      /// dimension.
      ///
      /// `configuration` should have as many elements as tensor elements in `input`. If `configuration` has only
      /// one element, it will be used for all histogram dimensions. The default configuration yields a histogram
      /// with 256 bins per dimension, and limits [0,256].
      Histogram( Image const& input, Image const& mask, ConfigurationArray configuration ) {
         DIP_THROW_IF( !input.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !input.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
         dip::uint ndims = input.TensorElements();
         DIP_START_STACK_TRACE
            ArrayUseParameter( configuration, ndims, Configuration{} );
            if( ndims == 1 ) {
               ScalarImageHistogram( input, mask, configuration[ 0 ] );
            } else {
               TensorImageHistogram( input, mask, configuration );
            }
         DIP_END_STACK_TRACE
      }

      /// \brief This version of the constructor is identical to the previous one, but with a single configuration
      /// parameter.
      explicit Histogram( Image const& input, Image const& mask = {}, Configuration configuration = {} ) {
         DIP_THROW_IF( !input.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !input.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
         dip::uint ndims = input.TensorElements();
         DIP_START_STACK_TRACE
            if( ndims == 1 ) {
               ScalarImageHistogram( input, mask, configuration );
            } else {
               ConfigurationArray newConfig( ndims, configuration );
               TensorImageHistogram( input, mask, newConfig );
            }
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
      uint32 At( dip::uint x ) const {
         DIP_THROW_IF( Dimensionality() != 1, E::ILLEGAL_DIMENSIONALITY );
         DIP_THROW_IF( x >= data_.Size( 0 ), E::INDEX_OUT_OF_RANGE );
         return *static_cast< uint32* >( data_.Pointer( static_cast< dip::sint >( x ) * data_.Stride( 0 ) ));
      }
      /// \brief Get the value at the given bin in a 2D histogram
      uint32 At( dip::uint x, dip::uint y ) const {
         DIP_THROW_IF( Dimensionality() != 2, E::ILLEGAL_DIMENSIONALITY );
         DIP_THROW_IF( x >= data_.Size( 0 ), E::INDEX_OUT_OF_RANGE );
         DIP_THROW_IF( y >= data_.Size( 1 ), E::INDEX_OUT_OF_RANGE );
         return *static_cast< uint32* >( data_.Pointer( static_cast< dip::sint >( x ) * data_.Stride( 0 ) +
                                                        static_cast< dip::sint >( y ) * data_.Stride( 1 )));
      }

      /// \brief Get the value at the given bin in a 3D histogram
      uint32 At( dip::uint x, dip::uint y, dip::uint z ) const {
         DIP_THROW_IF( Dimensionality() != 3, E::ILLEGAL_DIMENSIONALITY );
         DIP_THROW_IF( x >= data_.Size( 0 ), E::INDEX_OUT_OF_RANGE );
         DIP_THROW_IF( y >= data_.Size( 1 ), E::INDEX_OUT_OF_RANGE );
         DIP_THROW_IF( z >= data_.Size( 2 ), E::INDEX_OUT_OF_RANGE );
         return *static_cast< uint32* >( data_.Pointer( static_cast< dip::sint >( x ) * data_.Stride( 0 ) +
                                                        static_cast< dip::sint >( y ) * data_.Stride( 1 ) +
                                                        static_cast< dip::sint >( z ) * data_.Stride( 2 )));
      }

      /// \brief Get the value at the given bin
      uint32 At( UnsignedArray const& bin ) const {
         return *static_cast< uint32* >( data_.Pointer( bin )); // Does all the checking
      }

      /// \brief Get the image that holds the bin counts. The image is always scalar and of type `dip::DT_UINT32`.
      Image const& GetImage() const {
         return data_;
      }

      /// \brief Returns an iterator to the first bin
      ConstImageIterator< uint32 > begin() const {
         return { data_ };
      }

      /// \brief Returns an end iterator
      ConstImageIterator< uint32 > end() const {
         return {};
      }

      /// \brief Returns a pointer to the first bin
      uint32 const* Origin() const { return static_cast< uint32 const* >( data_.Origin() ); }

      // Functions below require stuff in math.h, we implement them in the CPP file to avoid including math.h here.

      /// \brief Returns the total number of elements in the histogram (sum of bins)
      DIP_EXPORT dip::uint Count() const;

      /// \brief Computes the mean value of the data represented by the histogram.
      ///
      /// Computing statistics through the histogram is efficient, but yields an approximation equivalent to
      /// computing the statictic on data rounded to the bin centers.
      DIP_EXPORT Image::CastPixel< dfloat > Mean() const;

      /// \brief Computes the covariance matrix of the data represented by the histogram.
      ///
      /// Computing statistics through the histogram is efficient, but yields an approximation equivalent to
      /// computing the statictic on data rounded to the bin centers.
      ///
      /// The returned pixel is a symmetric tensor, containing n*(n+1)/2 tensor elements (with n the
      /// histogram dimensionality).
      DIP_EXPORT Image::CastPixel< dfloat > Covariance() const;

      /// \brief Computes the marginal median value of the data represented by the histogram. The marginal median
      /// is a median computed independently on each pixel, and thus is not one of the input values.
      ///
      /// In the 1D histogram case (for scalar images) this function computes the approximate median (i.e. the
      /// bin containing the median value). The distinction between marginal median and median is only relevant
      /// for multivariate data (histograms from tensor images). In short, here we compute 1D medians on each
      /// of the 1D projections of the histogram.
      ///
      /// Computing statistics through the histogram is efficient, but yields an approximation equivalent to
      /// computing the statictic on data rounded to the bin centers.
      DIP_EXPORT Image::CastPixel< dfloat > MarginalMedian() const;

      /// \brief Returns the mode, the bin with the largest count. The return value is the
      ///
      /// When multiple bins have the same, largest count, the first bin encontered is returned. This is the bin
      /// with the lowest linear index, and is closest to the origin.
      DIP_EXPORT Image::CastPixel< dfloat > Mode() const;

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
      /// The maginal histogram represents the marginal intensity distribution. It is a 1D histogram determined
      /// by summing over all dimensions except `dim`, and is equivalent to the histogram for tensor element
      /// `dim`.
      DIP_EXPORT Histogram MarginalHistogram( dip::uint dim ) const;

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
      dfloat BinCenter( dip::uint bin, dip::uint dim = 0 ) const {
         return lowerBounds_[ dim ] + ( static_cast< dfloat >( bin ) + 0.5 ) * binSizes_[ dim ];
      }

      DIP_EXPORT void ScalarImageHistogram( Image const& input, Image const& mask, Configuration& configuration );
      DIP_EXPORT void TensorImageHistogram( Image const& input, Image const& mask, ConfigurationArray& configuration );
};


/// \}

} // namespace dip

#endif // DIP_HISTOGRAM_H
