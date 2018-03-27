/*
 * DIPlib 3.0
 * This file contains code for loop unrolling at compile time with a constexpr loop variable.
 *
 * (c)2018, Erik Schuitema.
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
 
#ifndef CONSTFOR_H_INCLUDED
#define CONSTFOR_H_INCLUDED 1

#include <utility>
#include <type_traits>

// Loop unrolling by using templates and lambda functions,
// making the loop variable available as constexpr.
// In this way, the loop variable can be used as, e.g., template parameter.
// From: https://stackoverflow.com/questions/42005229/why-for-loop-isnt-a-compile-time-expression-and-extended-constexpr-allows-for-l
// Usage: define the loop body as lambda function, capturing by reference:
//    constexpr numIterations = 10;
//    const_for< numIterations >( [&]( auto i ) {
//       some_templated_function< i.value >();  // Usually no need to add `.value`, unless loop variable is captured by another nested lambda in MSVC
//    }

template <std::size_t I> using const_int = std::integral_constant<std::size_t, I>;

template<typename F, std::size_t... S>
constexpr void const_for( F&& function, std::index_sequence<S...> ) {
   int unpack[] = { 0,
      (function( const_int<S>{} ), void(), 0)...
   };

   (void)unpack;
}

template<std::size_t iterations, typename F>
constexpr void const_for( F&& function ) {
   const_for( function, std::make_index_sequence<iterations>() );
   // Original code line (but slower in MSVC): const_for( std::forward<F>( function ), std::make_index_sequence<iterations>() );
}


// Other links worth considering:
// http://spraetor.github.io/2015/12/26/compile-time-loops.html
// https://www.codeproject.com/Articles/75423/Loop-Unrolling-over-Template-Arguments

#endif
