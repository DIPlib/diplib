/*
 * (c)2017-2025, Cris Luengo.
 * (c)2018, Erik Schuitema.
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

#include "diplib/statistics.h"

#include <functional>
#include <limits>
#include <memory>

#include "diplib.h"
#include "diplib/accumulators.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/math.h"
#include "diplib/overload.h"

#include "copy_non_nan.h"

namespace dip {

namespace {

template< typename TPI, bool ComputeMean_ >
class ProjectionSumMean : public Framework::ProjectionFunction {
      using TPO = FlexType< TPI >;
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         dip::uint n = 0;
         TPO sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
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
            do {
               sum += static_cast< TPO >( *it );
            } while( ++it );
            if( ComputeMean_ ) {
               n = in.NumberOfPixels();
            }
         }
         if( ComputeMean_ ) {
            *static_cast< TPO* >( out.Origin() ) = ( n > 0 ) ? ( sum / static_cast< FloatType< TPI >>( n ))
                                                             : ( sum );
         } else {
            *static_cast< TPO* >( out.Origin() ) = sum;
         }
      }
};

template< typename TPI >
using ProjectionSum = ProjectionSumMean< TPI, false >;

template< typename TPI >
using ProjectionMean = ProjectionSumMean< TPI, true >;

template< typename TPI >
class ProjectionMeanDirectional : public Framework::ProjectionFunction {
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         DirectionalStatisticsAccumulator acc;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() ) {
                  acc.Push( static_cast< dfloat >( it.template Sample< 0 >() ));
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            do {
               acc.Push( static_cast< dfloat >( *it ));
            } while( ++it );
         }
         *static_cast< FloatType< TPI >* >( out.Origin() ) = static_cast< FloatType< TPI >>( acc.Mean() ); // Is the same as FlexType< TPI > because TPI is not complex here.
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
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   if( mode == S::DIRECTIONAL ) {
      DIP_OVL_NEW_FLOAT( projectionFunction, ProjectionMeanDirectional, (), in.DataType() );
   } else if( mode.empty() ) {
      DIP_OVL_NEW_ALL( projectionFunction, ProjectionMean, (), in.DataType() );
   } else {
      DIP_THROW_INVALID_FLAG( mode );
   }
   Projection( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *projectionFunction );
}

void Sum(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   DIP_OVL_NEW_ALL( projectionFunction, ProjectionSum, (), in.DataType() );
   Projection( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *projectionFunction );
}

namespace {

template< typename TPI, bool ComputeMean_  >
class ProjectionProductGeomMean : public Framework::ProjectionFunction {
      using TPO = FlexType< TPI >;
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         dip::uint n = 0;
         TPO product = 1.0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
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
            do {
               product *= static_cast< TPO >( *it );
            } while( ++it );
            if( ComputeMean_ ) {
               n = in.NumberOfPixels();
            }
         }
         if( ComputeMean_ ) {
            *static_cast< TPO* >( out.Origin() ) = ( n > 0 ) ? ( std::pow( product, 1 / static_cast< FloatType< TPO >>( n )))
                                                             : ( product );
         } else {
            *static_cast< TPO* >( out.Origin() ) = product;
         }
      }
};

template< typename TPI >
using ProjectionProduct = ProjectionProductGeomMean< TPI, false >;

template< typename TPI >
using ProjectionGeometricMean = ProjectionProductGeomMean< TPI, true >;

} // // namespace

void GeometricMean( Image const& in, Image const& mask, Image& out, BooleanArray const& process ) {
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   DIP_OVL_NEW_ALL( projectionFunction, ProjectionGeometricMean, (), in.DataType() );
   Projection( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *projectionFunction );
}

void Product(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   DIP_OVL_NEW_ALL( projectionFunction, ProjectionProduct, (), in.DataType() );
   Projection( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *projectionFunction );
}

namespace {

template< typename TPI, bool ComputeMean_ >
class ProjectionSumMeanAbs : public Framework::ProjectionFunction {
      using TPO = FloatType< TPI >;
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         dip::uint n = 0;
         TPO sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() ) {
                  sum += static_cast< TPO >( std::abs( it.template Sample< 0 >() ));
                  if ( ComputeMean_ ) {
                     ++n;
                  }
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            do {
               sum += static_cast< TPO >( std::abs( *it ));
            } while( ++it );
            if( ComputeMean_ ) {
               n = in.NumberOfPixels();
            }
         }
         if( ComputeMean_ ) {
            *static_cast< TPO* >( out.Origin() ) = ( n > 0 ) ? ( sum / static_cast< TPO >( n )) : ( sum );
         } else {
            *static_cast< TPO* >( out.Origin() ) = sum;
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
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   if( in.DataType().IsUnsigned() ) {
      DIP_OVL_NEW_UNSIGNED( projectionFunction, ProjectionMean, (), in.DataType() );
   } else {
      DIP_OVL_NEW_SIGNED( projectionFunction, ProjectionMeanAbs, (), in.DataType() );
   }
   Projection( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *projectionFunction );
}

void SumAbs(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   if( in.DataType().IsUnsigned() ) {
      DIP_OVL_NEW_UNSIGNED( projectionFunction, ProjectionSum, (), in.DataType() );
   } else {
      DIP_OVL_NEW_SIGNED( projectionFunction, ProjectionSumAbs, (), in.DataType() );
   }
   Projection( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *projectionFunction );
}

namespace {

template< typename TPI, bool ComputeMean_ >
class ProjectionSumMeanSquare : public Framework::ProjectionFunction {
      using TPO = FlexType< TPI >;
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         dip::uint n = 0;
         TPO sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
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
            do {
               TPO v = static_cast< TPO >( *it );
               sum += v * v;
            } while( ++it );
            if( ComputeMean_ ) {
               n = in.NumberOfPixels();
            }
         }
         if( ComputeMean_ ) {
            *static_cast< TPO* >( out.Origin() ) = ( n > 0 ) ? ( sum / static_cast< FloatType< TPI >>( n ))
                                                             : ( sum );
         } else {
            *static_cast< TPO* >( out.Origin() ) = sum;
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
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   if( in.DataType().IsBinary() ) {
      DIP_OVL_NEW_BINARY( projectionFunction, ProjectionMean, (), DT_BIN );
   } else {
      DIP_OVL_NEW_NONBINARY( projectionFunction, ProjectionMeanSquare, (), in.DataType() );
   }
   Projection( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *projectionFunction );
}

void SumSquare(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   if( in.DataType().IsBinary() ) {
      DIP_OVL_NEW_BINARY( projectionFunction, ProjectionSum, (), DT_BIN );
   } else {
      DIP_OVL_NEW_NONBINARY( projectionFunction, ProjectionSumSquare, (), in.DataType() );
   }
   Projection( in, mask, out, DataType::SuggestFlex( in.DataType() ), process, *projectionFunction );
}

namespace {

template< typename TPI, bool ComputeMean_ >
class ProjectionSumMeanSquareModulus : public Framework::ProjectionFunction {
      // TPI is a complex type.
      using TPO = FloatType< TPI >;
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         dip::uint n = 0;
         TPO sum = 0;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
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
            *static_cast< TPO* >( out.Origin() ) = ( n > 0 ) ? ( sum / static_cast< TPO >( n ))
                                                             : ( sum );
         } else {
            *static_cast< TPO* >( out.Origin() ) = sum;
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
      std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
      DIP_OVL_NEW_COMPLEX( projectionFunction, ProjectionMeanSquareModulus, (), in.DataType() );
      Projection( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *projectionFunction );
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
      std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
      DIP_OVL_NEW_COMPLEX( projectionFunction, ProjectionSumSquareModulus, (), in.DataType() );
      Projection( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *projectionFunction );
      return;
   }
   SumSquare( in, mask, out, process );
}

namespace {

template< typename TPI, typename ACC >
class ProjectionVariance : public Framework::ProjectionFunction {
   public:
      ProjectionVariance( bool computeStD ) : computeStD_( computeStD ) {}
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         ACC acc;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() ) {
                  acc.Push( static_cast< dfloat >( it.template Sample< 0 >() ));
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            do {
               acc.Push( static_cast< dfloat >( *it ));
            } while( ++it );
         }
         *static_cast< FloatType< TPI >* >( out.Origin() ) = clamp_cast< FloatType< TPI >>(
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
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   if(( in.DataType().SizeOf() <= 2 ) && ( mode == S::STABLE )) {
      mode = S::FAST;
   }
   if( mode == S::STABLE ) {
      DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionVarianceStable, ( false ), in.DataType() );
   } else if( mode == S::FAST ) {
      DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionVarianceFast, ( false ), in.DataType() );
   } else if( mode == S::DIRECTIONAL ) {
      DIP_OVL_NEW_FLOAT( projectionFunction, ProjectionVarianceDirectional, ( false ), in.DataType() );
   } else {
      DIP_THROW_INVALID_FLAG( mode );
   }
   Projection( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *projectionFunction );
}

void StandardDeviation(
      Image const& in,
      Image const& mask,
      Image& out,
      String mode,
      BooleanArray const& process
) {
   // TODO: This exists also for complex numbers, yielding a real value
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   if(( in.DataType().SizeOf() <= 2 ) && ( mode == S::STABLE )) {
      mode = S::FAST;
   }
   if( mode == S::STABLE ) {
      DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionVarianceStable, ( true ), in.DataType() );
   } else if( mode == S::FAST ) {
      DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionVarianceFast, ( true ), in.DataType() );
   } else if( mode == S::DIRECTIONAL ) {
      DIP_OVL_NEW_FLOAT( projectionFunction, ProjectionVarianceDirectional, ( true ), in.DataType() );
   } else {
      DIP_THROW_INVALID_FLAG( mode );
   }
   Projection( in, mask, out, DataType::SuggestFloat( in.DataType() ), process, *projectionFunction );
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
class ProjectionMaxMin : public Framework::ProjectionFunction {
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         TPI res = Computer::init_value;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() ) {
                  res = Computer::compare( res, it.template Sample< 0 >() );
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            do {
               res = Computer::compare( res, *it );
            } while( ++it );
         }
         *static_cast< TPI* >( out.Origin() ) = res;
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
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   DIP_OVL_NEW_REAL( projectionFunction, ProjectionMaximum, (), in.DataType() );
   Projection( in, mask, out, in.DataType(), process, *projectionFunction );
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
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   DIP_OVL_NEW_REAL( projectionFunction, ProjectionMinimum, (), in.DataType() );
   Projection( in, mask, out, in.DataType(), process, *projectionFunction );
}

namespace {

template< typename TPI, typename Computer >
class ProjectionMaxMinAbs : public Framework::ProjectionFunction {
      using TPO = AbsType< TPI >;
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         TPO res = Computer::init_value;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() ) {
                  res = Computer::compare( res, static_cast< TPO >( std::abs( it.template Sample< 0 >() )));
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            do {
               res = Computer::compare( res, static_cast< TPO >( std::abs( *it )));
            } while( ++it );
         }
         *static_cast< TPO* >( out.Origin() ) = res;
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
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   DIP_OVL_NEW_SIGNED( projectionFunction, ProjectionMaximumAbs, (), dt );
   dt = DataType::SuggestAbs( dt );
   Projection( in, mask, out, dt, process, *projectionFunction );
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
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   DIP_OVL_NEW_SIGNED( projectionFunction, ProjectionMinimumAbs, (), dt );
   dt = DataType::SuggestAbs( dt );
   Projection( in, mask, out, dt, process, *projectionFunction );
}

namespace {

template< typename TPI >
class ProjectionPercentile : public Framework::ProjectionFunction {
   public:
      ProjectionPercentile( dfloat percentile ) : percentile_( percentile ) {}
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint thread ) override {
         CopyNonNaNValues( in, mask, buffer_[ thread ] );
         if( buffer_[ thread ].empty() ) {
            *static_cast< TPI* >( out.Origin() ) = TPI{};
         } else {
            dip::sint rank = static_cast< dip::sint >( RankFromPercentile( percentile_, buffer_[ thread ].size() ));
            auto ourGuy = buffer_[ thread ].begin() + rank;
            std::nth_element( buffer_[ thread ].begin(), ourGuy, buffer_[ thread ].end() );
            *static_cast< TPI* >( out.Origin() ) = *ourGuy;
         }
      }
      void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }
   private:
      std::vector< std::vector< TPI >> buffer_{};
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
      std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
      DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionPercentile, ( percentile ), in.DataType() );
      Projection( in, mask, out, in.DataType(), process, *projectionFunction );
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
class ProjectionAll : public Framework::ProjectionFunction {
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         bool all = true;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() && ( it.template Sample< 0 >() == TPI( 0 ))) {
                  all = false;
                  break;
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            do {
               if( *it == TPI( 0 )) {
                  all = false;
                  break;
               }
            } while( ++it );
         }
         *static_cast< bin* >( out.Origin() ) = all;
      }
};

} // namespace

void All(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   DIP_OVL_NEW_ALL( projectionFunction, ProjectionAll, (), in.DataType() );
   Projection( in, mask, out, DT_BIN, process, *projectionFunction );
}

namespace {

template< typename TPI >
class ProjectionAny : public Framework::ProjectionFunction {
   public:
      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
         bool any = false;
         if( mask.IsForged() ) {
            JointImageIterator< TPI, bin > it( { in, mask } );
            do {
               if( it.template Sample< 1 >() && ( it.template Sample< 0 >() != TPI( 0 ))) {
                  any = true;
                  break;
               }
            } while( ++it );
         } else {
            ImageIterator< TPI > it( in );
            do {
               if( *it != TPI( 0 )) {
                  any = true;
                  break;
               }
            } while( ++it );
         }
         *static_cast< bin* >( out.Origin() ) = any;
      }
};

} // namespace

void Any(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   DIP_OVL_NEW_ALL( projectionFunction, ProjectionAny, (), in.DataType() );
   Projection( in, mask, out, DT_BIN, process, *projectionFunction );
}


namespace {

// `CompareOp` is the compare operation
template< typename TPI, typename CompareOp >
class ProjectionPositionMinMax : public Framework::ProjectionFunction {
   public:
      // `limitInitVal` is the initialization value of the variable that tracks the limit value
      // For finding a minimum value, initialize with std::numeric_limits< TPI >::max(),
      // for finding a maximum value, initialize with std::numeric_limits< TPI >::lowest().
      ProjectionPositionMinMax( TPI limitInitVal ) : limitInitVal_( limitInitVal ) {}

      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint /*thread*/ ) override {
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
         *static_cast< dip::uint32* >( out.Origin() ) = clamp_cast< dip::uint32 >( limitCoords.front() );
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

   std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
   if( maximum ) {
      if( mode == S::FIRST ) {
         DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionPositionFirstMaximum, (), in.DataType() );
      } else if( mode == S::LAST ) {
         DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionPositionLastMaximum, (), in.DataType() );
      } else {
         DIP_THROW_INVALID_FLAG( mode );
      }
   } else { // minimum
      if( mode == S::FIRST ) {
         DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionPositionFirstMinimum, (), in.DataType() );
      } else if( mode == S::LAST ) {
         DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionPositionLastMinimum, (), in.DataType() );
      } else {
         DIP_THROW_INVALID_FLAG( mode );
      }
   }

   // Positions in the out image will be of type DT_UINT32
   Projection( in, mask, out, DT_UINT32, process, *projectionFunction );
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
class ProjectionPositionPercentile : public Framework::ProjectionFunction {
   public:
      ProjectionPositionPercentile( dfloat percentile, bool findFirst ) : percentile_( percentile ), findFirst_( findFirst ) {}

      void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint thread ) override {
         // Create a copy of the input image line (single dimension) that can be sorted to find the percentile value
         dip::UnsignedArray percentileCoords( in.Dimensionality(), 0 ); // Coordinates of the pixel with the percentile value
         CopyNonNaNValues( in, mask, buffer_[ thread ] );
         if( buffer_[ thread ].empty() ) {
            percentileCoords.fill( 0 );
         } else {
            dip::sint rank = static_cast< dip::sint >( RankFromPercentile( percentile_, buffer_[ thread ].size() ));
            auto ourGuy = buffer_[ thread ].begin() + rank;
            std::nth_element( buffer_[ thread ].begin(), ourGuy, buffer_[ thread ].end() );
            if( mask.IsForged() ) {
               // Find the position of the ranked element within the masked pixels
               JointImageIterator< TPI, bin > it( { in, mask } );
               do {
                  if( it.template Sample< 1 >() && ( it.template Sample< 0 >() == *ourGuy ) ) {
                     percentileCoords = it.Coordinates();
                     if( findFirst_ ) {
                        break;
                     }
                  }
               } while( ++it );
            } else {
               // Find the position of the ranked element
               ImageIterator< TPI > it( in );
               do {
                  if( *it == *ourGuy ) {
                     percentileCoords = it.Coordinates();
                     if( findFirst_ ) {
                        break;
                     }
                  }
               } while( ++it );
            }
         }
         // Store coordinate.
         // Currently, only a single processing dim is supported, so only one coordinate is stored.
         *static_cast< dip::uint32* >( out.Origin() ) = clamp_cast< dip::uint32 >( percentileCoords.front() );
      }

      void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }

   protected:
      std::vector< std::vector< TPI >> buffer_{};
      dfloat percentile_;
      bool findFirst_;
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
      std::unique_ptr< Framework::ProjectionFunction > projectionFunction;
      if( mode == S::FIRST ) {
         DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionPositionPercentile, ( percentile, true ), in.DataType() );
      } else if( mode == S::LAST ) {
         DIP_OVL_NEW_NONCOMPLEX( projectionFunction, ProjectionPositionPercentile, ( percentile, false ), in.DataType() );
      } else {
         DIP_THROW_INVALID_FLAG( mode );
      }

      // Positions in the out image will be of type DT_UINT32
      Projection( in, mask, out, DT_UINT32, process, *projectionFunction );
   }
}


} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include <cmath>

DOCTEST_TEST_CASE("[DIPlib] testing the projection function mechanics") {
   // Testing that the Projection framework works appropriately.
   dip::Image img{ dip::UnsignedArray{ 30, 40, 200 }, 3, dip::DT_UINT8 };
   img = { 1, 1, 1 };
   img.At( 10, 20, 100 ) = { 2, 3, 4 };

   // Project over all dimensions except the tensor dimension
   dip::Image out = dip::Maximum( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_UINT8 );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 1 );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 2, 3, 4 } ));

   // Idem except we write in an image of a different type
   out.Strip();
   out.SetDataType( dip::DT_SINT32 );
   out.Protect();
   dip::Maximum( img, {}, out );
   DOCTEST_CHECK( out.DataType() == dip::DT_SINT32 );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 1 );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 2, 3, 4 } ));
   out.Protect( false );

   // Project over two dimensions
   out = dip::Maximum( img, {}, { false, true, true } );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 30 );
   DOCTEST_CHECK( out.Size( 0 ) == 30 );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 10, 0, 0 ) == dip::Image::Pixel( { 2, 3, 4 } ));
   DOCTEST_CHECK( out.At( 20, 0, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));

   // Project over another two dimensions
   out = dip::Maximum( img, {}, { true, false, true } );
   DOCTEST_CHECK( out.Dimensionality() == 3 );
   DOCTEST_CHECK( out.NumberOfPixels() == 40 );
   DOCTEST_CHECK( out.Size( 1 ) == 40 );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 0, 10, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 0, 20, 0 ) == dip::Image::Pixel( { 2, 3, 4 } ));
   DOCTEST_CHECK( out.At( 0, 30, 0 ) == dip::Image::Pixel( { 1, 1, 1 } ));

   // Project over no dimensions -- square must still be applied
   out = dip::MeanSquare( img, {}, { false, false, false } );
   DOCTEST_CHECK( out.Sizes() == img.Sizes() );
   DOCTEST_CHECK( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.At( 0, 20, 100 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 10, 20, 100 ) == dip::Image::Pixel( { 4, 9, 16 } ));
   DOCTEST_CHECK( out.At( 20, 20, 100 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 10, 0, 100 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 10, 10, 100 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 10, 30, 100 ) == dip::Image::Pixel( { 1, 1, 1 } ));
   DOCTEST_CHECK( out.At( 10, 20, 101 ) == dip::Image::Pixel( { 1, 1, 1 } ));

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
   DOCTEST_CHECK( out.As< dip::dfloat >() == doctest::Approx( std::atan2( std::sin( 1 ), std::cos( 1 ) + ( 3 * 4 * 2 - 1 ))));

   // Using a mask
   img = dip::Image{ dip::UnsignedArray{ 3, 4, 2 }, 3, dip::DT_UINT8 };
   img = { 1, 1, 1 };
   img.At( 0, 0, 0 ) = { 2, 3, 4 };
   img.At( 0, 1, 0 ) = { 3, 2, 2 };
   img.At( 0, 0, 1 ) = { 4, 2, 3 };
   img.At( 1, 0, 0 ) = { 4, 2, 1 };
   dip::Image mask{ img.Sizes(), 1, dip::DT_BIN };
   mask = 1;
   mask.At( 0, 0, 0 ) = 0;
   out = dip::Maximum( img, mask, { true, true, false } );
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 4, 2, 2 } )); // not {4,3,4}
   DOCTEST_CHECK( out.At( 0, 0, 1 ) == dip::Image::Pixel( { 4, 2, 3 } ));

   // Using a view
   out = dip::Maximum( img.At( mask ));
   DOCTEST_CHECK( out.At( 0, 0, 0 ) == dip::Image::Pixel( { 4, 2, 3 } )); // not {4,3,4}

   // Over an image with weird strides, and a similar mask
   img = dip::Image{ dip::UnsignedArray{ 5, 4 }, 1, dip::DT_UINT8 };
   img = 1;
   img.At( 0, 0 ) = 2;
   img.At( 0, 2 ) = 3;
   img.At( 3, 0 ) = 4;
   img.At( 3, 2 ) = 5;
   mask = dip::Image{ img.Sizes(), 1, dip::DT_BIN };
   mask = 1;
   mask.At( 3, 2 ) = 0;
   img.Rotation90();
   mask.Rotation90();
   out = dip::Maximum( img, mask, { true, false } );
   DOCTEST_REQUIRE( out.Sizes() == dip::UnsignedArray{ 1, 5 } );
   DOCTEST_CHECK( out.At( 0, 0 ) == 3 );
   DOCTEST_CHECK( out.At( 0, 1 ) == 1 );
   DOCTEST_CHECK( out.At( 0, 2 ) == 1 );
   DOCTEST_CHECK( out.At( 0, 3 ) == 4 ); // 5 is masked out
   DOCTEST_CHECK( out.At( 0, 4 ) == 1 );

   // Over an image with weird strides, and a mask with normal strides
   mask = dip::Image{ img.Sizes(), 1, dip::DT_BIN };
   mask = 1;
   mask.At( 1, 3 ) = 0;
   out = dip::Maximum( img, mask, { true, false } );
   DOCTEST_REQUIRE( out.Sizes() == dip::UnsignedArray{ 1, 5 } );
   DOCTEST_CHECK( out.At( 0, 0 ) == 3 );
   DOCTEST_CHECK( out.At( 0, 1 ) == 1 );
   DOCTEST_CHECK( out.At( 0, 2 ) == 1 );
   DOCTEST_CHECK( out.At( 0, 3 ) == 4 ); // 5 is masked out
   DOCTEST_CHECK( out.At( 0, 4 ) == 1 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the projection function computations") {
   // Testing each of the projection functions to verify they do the right thing
   dip::Image img( { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }, dip::DT_UINT8 );
   img.TensorToSpatial();  // The line above creates a single-pixel tensor image.
   DOCTEST_REQUIRE( img.Sizes() == dip::UnsignedArray{ 10 } );
   DOCTEST_CHECK( dip::Mean( img ).As< double >() == doctest::Approx( 5.5 ));
   DOCTEST_CHECK( dip::Sum( img ).As< double >() == 55 );
   DOCTEST_CHECK( dip::GeometricMean( img ).As< double >() == doctest::Approx( 4.5287 ));
   DOCTEST_CHECK( dip::Product( img ).As< double >() == doctest::Approx( 3628800 ));
   DOCTEST_CHECK( dip::MeanSquare( img ).As< double >() == doctest::Approx( 38.5 ));
   DOCTEST_CHECK( dip::SumSquare( img ).As< double >() == doctest::Approx( 385 ));
   DOCTEST_CHECK( dip::Variance( img ).As< double >() == doctest::Approx( 9.1667 ));
   DOCTEST_CHECK( dip::StandardDeviation( img ).As< double >() == doctest::Approx( 3.02765 ));
   DOCTEST_CHECK( dip::Maximum( img ).As< double >() == 10 );
   DOCTEST_CHECK( dip::Minimum( img ).As< double >() == 1 );
   DOCTEST_CHECK( dip::Percentile( img, {}, 70 ).As< double >() == 7 );

   dip::Image out = dip::MeanAbs( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( out.As< double >() == doctest::Approx( 5.5 ));
   DOCTEST_CHECK( dip::SumAbs( img ).As< double >() == 55 );
   DOCTEST_CHECK( dip::MeanSquareModulus( img ).As< double >() == doctest::Approx( 38.5 ));
   DOCTEST_CHECK( dip::SumSquareModulus( img ).As< double >() == doctest::Approx( 385 ));
   out = dip::MaximumAbs( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_UINT8 );
   DOCTEST_CHECK( out.As< double >() == 10 );
   DOCTEST_CHECK( dip::MinimumAbs( img ).As< double >() == 1 );

   img.Convert( dip::DT_SINT8 );
   dip::Invert( img, img );
   out = dip::MeanAbs( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( out.As< double >() == doctest::Approx( 5.5 ));
   DOCTEST_CHECK( dip::SumAbs( img ).As< double >() == 55 );
   DOCTEST_CHECK( dip::MeanSquareModulus( img ).As< double >() == doctest::Approx( 38.5 ));
   DOCTEST_CHECK( dip::SumSquareModulus( img ).As< double >() == doctest::Approx( 385 ));
   out = dip::MaximumAbs( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_UINT8 );
   DOCTEST_CHECK( out.As< double >() == 10 );
   DOCTEST_CHECK( dip::MinimumAbs( img ).As< double >() == 1 );

   img.Convert( dip::DT_DFLOAT );
   out = dip::MeanAbs( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_DFLOAT );
   DOCTEST_CHECK( out.As< double >() == doctest::Approx( 5.5 ));
   DOCTEST_CHECK( dip::SumAbs( img ).As< double >() == 55 );
   DOCTEST_CHECK( dip::MeanSquareModulus( img ).As< double >() == doctest::Approx( 38.5 ));
   DOCTEST_CHECK( dip::SumSquareModulus( img ).As< double >() == doctest::Approx( 385 ));
   out = dip::MaximumAbs( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_DFLOAT );
   DOCTEST_CHECK( out.As< double >() == 10 );
   DOCTEST_CHECK( dip::MinimumAbs( img ).As< double >() == 1 );

   img.Convert( dip::DT_SCOMPLEX );
   out = dip::MeanAbs( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( out.As< double >() == doctest::Approx( 5.5 ));
   DOCTEST_CHECK( dip::SumAbs( img ).As< double >() == doctest::Approx( 55 ));
   DOCTEST_CHECK( dip::MeanSquareModulus( img ).As< double >() == doctest::Approx( 38.5 ));
   DOCTEST_CHECK( dip::SumSquareModulus( img ).As< double >() == doctest::Approx( 385 ));
   out = dip::MaximumAbs( img );
   DOCTEST_CHECK( out.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( out.As< double >() == 10 );
   DOCTEST_CHECK( dip::MinimumAbs( img ).As< double >() == 1 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the Percentile and PositionPercentile functions dealing with NaNs") {
   // Testing each of the projection functions to verify they do the right thing
   dip::Image img( { dip::nan, 10.0, 2.0, dip::nan, dip::nan, 5.0, dip::nan, 6.0, 3.0, 7.0, 4.0, dip::nan, dip::nan }, dip::DT_DFLOAT );
   img.TensorToSpatial();  // The line above creates a single-pixel tensor image.
   DOCTEST_CHECK( dip::Percentile( img, {}, 0.0 ).As< double >() == 2.0 );
   DOCTEST_CHECK( dip::Percentile( img, {}, 100.0 / 7.0 ).As< double >() == 3.0 );
   DOCTEST_CHECK( dip::Percentile( img, {}, 200.0 / 7.0 ).As< double >() == 4.0 );
   DOCTEST_CHECK( dip::Percentile( img, {}, 100.0 ).As< double >() == 10.0 );
   DOCTEST_CHECK( dip::PositionPercentile( img, {}, 0.0 ).As< dip::uint >() == 2u );
   DOCTEST_CHECK( dip::PositionPercentile( img, {}, 100.0 / 7.0 ).As< dip::uint >() == 8u );
   DOCTEST_CHECK( dip::PositionPercentile( img, {}, 200.0 / 7.0 ).As< dip::uint >() == 10u );
   DOCTEST_CHECK( dip::PositionPercentile( img, {}, 100.0 ).As< dip::uint >() == 1u );

   img.Fill( dip::nan );
   DOCTEST_CHECK( dip::Percentile( img, {}, 30.0 ).As< double >() == 0.0 );
   DOCTEST_CHECK( dip::PositionPercentile( img, {}, 30.0 ).As< dip::uint >() == 0u );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
