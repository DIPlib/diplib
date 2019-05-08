/*
 * DIPlib 3.0
 * This file contains declarations for the random number generator.
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

#ifndef DIP_RANDOM_H
#define DIP_RANDOM_H


#include <random>

#include "diplib.h"
#include "diplib/private/pcg_random.hpp"


/// \file
/// \brief Random number generators
/// \see random


namespace dip {


/// \defgroup random Random
/// \ingroup infrastructure
/// \brief Pseudo-random generator and probability distributions.
/// \{


/// \brief A pseudo-random number generator with excellent statistical properties, and it's also fast.
///
/// The `operator()` method returns the next random integer in the sequence.
///
/// The default `%Random` is initialized using `std::random_device`, but it is also possible to
/// use a `dip::uint` seed value when creating the generator to be able to replicate the same pseudo-random
/// sequence. In multi-threaded code, algorithms can use `dip::Random::Split` to split off separate
/// streams. This causes those algorithms to not replicate the same sequence when run with a different number
/// of threads. Thus, even if seeded with the same value, the same algorithm can yield different results
/// when run on a different computer with a different number of cores. To guarantee exact replicability,
/// run your code single-threaded.
///
/// `%Random` has a 128-bit internal state, and produces 64-bit output with a period of 2<sup>128</sup>.
/// On architectures where 128-bit integers are not natively supported, this changes to have a 64-bit internal state,
/// and produce 32-bit output with a period of 2<sup>64</sup>. This lesser PRNG still has very good statistical
/// properties. Defining `DIP_ALWAYS_128_PRNG` causes the better 128-bit PRNG engine to be used, using emulated
/// 128-bit arithmetic. Note that, if *DIPlib* is compiled with this flag, code that links to it must also be
/// compiled with this flag, or bad things will happen.
///
/// \see dip::UniformRandomGenerator, dip::GaussianRandomGenerator, dip::PoissonRandomGenerator, dip::BinaryRandomGenerator.
class DIP_NO_EXPORT Random {
#if defined(__SIZEOF_INT128__) || defined(DIP__ALWAYS_128_PRNG)
      using Engine = pcg64;
#else
      using Engine = pcg32;
#endif

   public:
      using result_type = Engine::result_type; ///< The type of the integer returned by the generator.
      using state_type = Engine::state_type;   ///< The type of the internal state of the generator.
      static constexpr result_type min() { return Engine::min(); }
      static constexpr result_type max() { return Engine::max(); }

      /// The default random generator is initialized using `std::random_device`.
      Random() {
         Seed();
      }

      /// Provide a seed to create a random generator that gives the same sequence every time.
      Random( dip::uint seed ) {
         Seed( seed );
      }

      /// Reseed the random generator using `std::random_device`.
      void Seed() {
         pcg_extras::seed_seq_from< std::random_device > seed_source;
         engine_.seed( seed_source );
      }

      /// Reseed the random generator using `seed`.
      void Seed( dip::uint seed ) {
         engine_.seed( static_cast< state_type >( seed ));
      }

      /// Get the next random value.
      result_type operator()() {
         return engine_();
      }

      /// Advance the generator `n` steps without producing output, takes log(`n`) time.
      void Advance( dip::uint n ) {
         engine_.advance( n );
      }

      /// Set the stream for the generator using a random value from the generator itself.
      void SetStream() {
         engine_.set_stream( engine_() );
      }

      /// Set the stream for the generator to `n`.
      void SetStream( state_type n ) {
         engine_.set_stream( n );
      }

      /// \brief Create a copy of the random generator, and set it to a random stream. Used by
      /// parallel algorithms to provide a different random generator to each thread.
      Random Split() {
         Random out( *this );
         out.SetStream( engine_() );
         return out;
      }

   private:
      Engine engine_;
};


/// \brief Generates random floating-point values taken from a uniform distribution.
///
/// The `operator()` method returns the next random value in the sequence. It takes two
/// parameters, the lower and upper bound of the distribution. The values are taken from
/// the half-open interval [`lowerBound`, `upperBound`).
///
/// The constructor takes and stores a reference to a `dip::Random` object, which is used
/// to produce the randomness. That object needs to exist for as long as this one exists.
///
/// \see dip::GaussianRandomGenerator, dip::PoissonRandomGenerator, dip::BinaryRandomGenerator.
class DIP_NO_EXPORT UniformRandomGenerator {
      using Distribution = std::uniform_real_distribution< dfloat >;
   public:

      /// \brief Constructor
      explicit UniformRandomGenerator( Random& generator ) : generator_( generator ) {}

      /// \brief Get the next random value, using the given parameters.
      dfloat operator()( dfloat lowerBound, dfloat upperBound ) {
         return distribution_( generator_, Distribution::param_type( lowerBound, upperBound ));
      }

   private:
      Random& generator_;
      Distribution distribution_;
};

/// \brief Generates random floating-point values taken from a normal distribution.
///
/// The `operator()` method returns the next random value in the sequence. It takes two
/// parameters, the mean and standard deviation of the distribution.
///
/// The constructor takes and stores a reference to a `dip::Random` object, which is used
/// to produce the randomness. That object needs to exist for as long as this one exists.
///
/// \see dip::UniformRandomGenerator, dip::PoissonRandomGenerator, dip::BinaryRandomGenerator.
class DIP_NO_EXPORT GaussianRandomGenerator {
      using Distribution = std::normal_distribution< dfloat >;
   public:

      /// \brief Constructor
      explicit GaussianRandomGenerator( Random& generator ) : generator_( generator ) {}

      /// \brief Get the next random value, using the given parameters.
      dfloat operator()( dfloat mean, dfloat standardDeviation ) {
         return distribution_( generator_, Distribution::param_type( mean, standardDeviation ));
      }

   private:
      Random& generator_;
      Distribution distribution_;
};

/// \brief Generates random integer values taken from a poisson distribution.
///
/// The `operator()` method returns the next random value in the sequence. It takes one
/// parameter, the mean of the distribution.
///
/// The constructor takes and stores a reference to a `dip::Random` object, which is used
/// to produce the randomness. That object needs to exist for as long as this one exists.
///
/// \see dip::UniformRandomGenerator, dip::GaussianRandomGenerator, dip::BinaryRandomGenerator.
class DIP_NO_EXPORT PoissonRandomGenerator {
      using Distribution = std::poisson_distribution< dip::uint >;
   public:

      /// \brief Constructor
      explicit PoissonRandomGenerator( Random& generator ) : generator_( generator ) {}

      /// \brief Get the next random value, using the given parameter.
      dip::uint operator()( dfloat mean ) {
         return distribution_( generator_, Distribution::param_type( mean ));
      }

   private:
      Random& generator_;
      Distribution distribution_;
};

/// \brief Generates random binary values.
///
/// The `operator()` method returns the next random value in the sequence. It takes one
/// parameter, the probability of drawing `true`.
///
/// The constructor takes and stores a reference to a `dip::Random` object, which is used
/// to produce the randomness. That object needs to exist for as long as this one exists.
///
/// \see dip::UniformRandomGenerator, dip::GaussianRandomGenerator, dip::PoissonRandomGenerator.
class DIP_NO_EXPORT BinaryRandomGenerator {
   public:

      /// \brief Constructor
      explicit BinaryRandomGenerator( Random& generator ) : generator_( generator ) {}

      /// \brief Get the next random value, using the given parameter.
      bin operator()( dfloat p ) {
         if( p <= 0.0 ) { return false; }
         if( p >= 1.0 ) { return true; }
         return std::generate_canonical< dfloat, std::numeric_limits< dfloat >::digits, Random >( generator_ ) < p;
         // The above is slow, potentially requires multiple random numbers to be generated. Instead, we could do
         // the following, though the casts loose precision and therefore probabilities might not be as requested:
         // return static_cast< long double >( generator_() ) < ( static_cast< long double >( Random::max() ) + 1 ) * p;
      }

   private:
      Random& generator_;
};


/// \}


} // namespace dip

#endif // DIP_RANDOM_H
