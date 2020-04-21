/*
 * This file contains an api to FFTW v3, templated in the floating point type.
 *
 * (c)2017, Erik Schuitema.
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

#ifndef FFTWAPI_H
#define FFTWAPI_H

#include <fftw3.h>

/// Struct for which several specializations are defined by macro expansion.
/// Inspired by 'fftw-wrapper' ( https://github.com/dnbaker/fftw-wrapper ).
///
/// Usage:
/// ```cpp
///     using ffttwapi = fftwapidef< FloatType >;
///     fftwapi::PlanType plan;
/// ```
template<typename FloatType>
struct fftwapidef;

#define FFTW_TEMPLATED_API_FUNC(MANGLE, FUNCNAME) static constexpr decltype( & MANGLE( FUNCNAME ) ) FUNCNAME = & MANGLE( FUNCNAME )

#define FFTW_TEMPLATED_API(T, MANGLE ) \
template<> \
struct fftwapidef<T> \
{ \
   using plan = MANGLE( plan ); \
   using complex = MANGLE( complex ); \
   using real = T; \
   using iodim = MANGLE( iodim ); \
   using r2r_kind = MANGLE( r2r_kind ); \
\
   FFTW_TEMPLATED_API_FUNC( MANGLE, cost ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, plan_guru_dft ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, plan_guru_dft_r2c ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, plan_guru_dft_c2r ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, plan_guru_r2r ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, execute ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, execute_dft ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, execute_dft_r2c ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, execute_dft_c2r ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, execute_r2r ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, init_threads ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, plan_with_nthreads ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, cleanup_threads ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, destroy_plan ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, print_plan ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, malloc ); \
   FFTW_TEMPLATED_API_FUNC( MANGLE, free ); \
} // end fftwapidef<>
// Excluded, because free() results in runtime error in debug mode: static std::string plan_to_string( plan p ) { char* pStr = MANGLE( sprint_plan )(p); std::string result( pStr ); ::free( pStr ); return result; };


/// fftwapidef struct for float
FFTW_TEMPLATED_API( float, FFTW_MANGLE_FLOAT );
/// fftwapidef struct for double
FFTW_TEMPLATED_API( double, FFTW_MANGLE_DOUBLE );
/// fftwapidef struct for long double
FFTW_TEMPLATED_API( long double, FFTW_MANGLE_LONG_DOUBLE );

#endif