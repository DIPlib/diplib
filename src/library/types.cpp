/*
 * DIPlib 3.0
 * This file contains unit tests for the basic data types.
 *
 * (c)2014-2017, Cris Luengo.
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

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/library/types.h"

DOCTEST_TEST_CASE("[DIPlib] testing the dip::bin class") {
   dip::bin A = false;
   dip::bin B = true;
   DOCTEST_CHECK( A < B );
   DOCTEST_CHECK( B > A );
   DOCTEST_CHECK( A >= A );
   DOCTEST_CHECK( A <= B );
   DOCTEST_CHECK( A == A );
   DOCTEST_CHECK( A == false );
   DOCTEST_CHECK( A == 0 );
   DOCTEST_CHECK( A != B );
   DOCTEST_CHECK( A != true );
   DOCTEST_CHECK( A != 100 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::detail::Options class") {
   enum class MyOption { clean, fresh, shine, flower, burn };
   //DIP_DECLARE_OPTIONS( MyOption, MyOptions ); // This does not work within a function
   using MyOptions = dip::detail::Options< MyOption >;
   constexpr MyOptions MyOption_freshNclean = MyOptions( MyOption::fresh ) + MyOption::clean; // because we haven't declared the operator, this is a little bit more verbose than it should be
   MyOptions opts {};
   DOCTEST_CHECK( !opts.Contains( MyOption::clean ));
   opts = MyOption::fresh;
   DOCTEST_CHECK( !opts.Contains( MyOption::clean ));
   DOCTEST_CHECK( opts.Contains( MyOption::fresh ));
   DOCTEST_CHECK( !opts.Contains( MyOptions( MyOption::fresh ) + MyOption::burn ));
   opts = MyOptions( MyOption::clean ) + MyOption::burn;
   DOCTEST_CHECK( opts.Contains( MyOption::clean ));
   DOCTEST_CHECK( opts.Contains( MyOption::burn ));
   DOCTEST_CHECK( opts.Contains( MyOptions( MyOption::burn ) + MyOption::clean ));
   DOCTEST_CHECK( !opts.Contains( MyOption::shine ));
   DOCTEST_CHECK( !opts.Contains( MyOption::fresh ));
   DOCTEST_CHECK( !opts.Contains( MyOptions( MyOption::fresh ) + MyOption::burn ));
   opts += MyOption::shine;
   DOCTEST_CHECK( opts.Contains( MyOption::clean ));
   DOCTEST_CHECK( opts.Contains( MyOption::burn ));
   DOCTEST_CHECK( opts.Contains( MyOption::shine ));
   DOCTEST_CHECK( !opts.Contains( MyOption::fresh ));
   opts = MyOption_freshNclean;
   DOCTEST_CHECK( opts.Contains( MyOption::clean ));
   DOCTEST_CHECK( opts.Contains( MyOption::fresh ));
   DOCTEST_CHECK( !opts.Contains( MyOption::shine ));
   opts -= MyOption::clean ;
   DOCTEST_CHECK( !opts.Contains( MyOption::clean ));
   DOCTEST_CHECK( opts.Contains( MyOption::fresh ));
   DOCTEST_CHECK( !opts.Contains( MyOption::shine ));

   //enum class HisOption { ugly, cheap, fast, };
   //DIP_DECLARE_OPTIONS( HisOption, HisOptions );
   //DOCTEST_CHECK( MyOptions( MyOption::clean ).Contains( HisOption::cheap )); // compiler error: comparison different types
   //HisOptions b = HisOptions( HisOption::fast ) + MyOption::flower;    // compiler error: addition different types
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::DimensionArray class") {
   dip::DimensionArray< int > a{ 1, 2, 4, 8, 16, 32 };
   DOCTEST_REQUIRE( a.size() == 6 );
   DOCTEST_REQUIRE( a.sum() == 63 );

   DOCTEST_SUBCASE("swapping") {
      dip::DimensionArray< int > b{ 5, 4, 3, 2, 1 };
      DOCTEST_REQUIRE( b.size() == 5 );
      DOCTEST_REQUIRE( b.sum() == 15 );
      a.swap( b );
      DOCTEST_CHECK( a.size() == 5 );
      DOCTEST_CHECK( a.sum() == 15 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("initialization") {
      dip::DimensionArray< int > b( 3, 1 );
      DOCTEST_CHECK( b.size() == 3 );
      DOCTEST_CHECK( b.sum() == 3 );
      b.resize( 6, 2 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 9 );
   }

   DOCTEST_SUBCASE("copy constructor I") {
      dip::DimensionArray< int > b( a );
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("copy constructor II") {
      dip::DimensionArray< int > b = a;
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("copy assignment") {
      dip::DimensionArray< int > b;
      b = a;
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("move constructor") {
      dip::DimensionArray< int > b( std::move( a ));
      DOCTEST_CHECK( a.size() == 0 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("move assignment") {
      dip::DimensionArray< int > b;
      b = std::move( a );
      DOCTEST_CHECK( a.size() == 0 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("pushing, popping") {
      a.push_back( 1 );
      DOCTEST_CHECK( a.size() == 7 );
      DOCTEST_CHECK( a.sum() == 64 );
      a.pop_back();
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      a.pop_back();
      DOCTEST_CHECK( a.size() == 5 );
      DOCTEST_CHECK( a.sum() == 31 );
   }

   DOCTEST_SUBCASE("equality") {
      dip::DimensionArray< int > b( a );
      DOCTEST_CHECK( a == b );
      b.back() = 0;
      DOCTEST_CHECK( a != b );
      b.pop_back();
      DOCTEST_CHECK( a != b );
   }

   DOCTEST_SUBCASE("insert, erase, clear") {
      a.insert( 0, 100 );
      DOCTEST_CHECK( a.size() == 7 );
      DOCTEST_CHECK( a.sum() == 163 );
      DOCTEST_CHECK( a.front() == 100 );
      a.erase( 0 );
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      DOCTEST_CHECK( a.front() == 1 );
      a.erase( 1 );
      DOCTEST_CHECK( a.size() == 5 );
      DOCTEST_CHECK( a.sum() == 61 );
      DOCTEST_CHECK( a.front() == 1 );
      a.clear();
      DOCTEST_CHECK( a.size() == 0 );
   }

   DOCTEST_SUBCASE("indexing") {
      DOCTEST_CHECK( a[ 3 ] == 8 );
      a[ 3 ] = 0;
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 55 );
   }

   DOCTEST_SUBCASE("sorting I") {
      dip::DimensionArray< int > b{ 0, 2, 4, 1, 3, 5 };
      DOCTEST_REQUIRE( b.size() == a.size() );
      b.sort( a ); // sorts b, keeps a in sync. so a should be: { 1, 8, 2, 16, 4, 32 }
      DOCTEST_CHECK( a[ 0 ] == 1 );
      DOCTEST_CHECK( a[ 1 ] == 8 );
      DOCTEST_CHECK( a[ 2 ] == 2 );
      DOCTEST_CHECK( a[ 3 ] == 16 );
      DOCTEST_CHECK( a[ 4 ] == 4 );
      DOCTEST_CHECK( a[ 5 ] == 32 );
   }

   DOCTEST_SUBCASE("sorting II") {
      dip::DimensionArray< int > b{ 0, 2, 4, 1, 3, 5 };
      dip::DimensionArray< size_t > i = b.sorted_indices();
      DOCTEST_CHECK( b.size() == i.size() );
      DOCTEST_CHECK( i[ 0 ] == 0 );
      DOCTEST_CHECK( i[ 1 ] == 3 );
      DOCTEST_CHECK( i[ 2 ] == 1 );
      DOCTEST_CHECK( i[ 3 ] == 4 );
      DOCTEST_CHECK( i[ 4 ] == 2 );
      DOCTEST_CHECK( i[ 5 ] == 5 );
   }
}

#endif // DIP__ENABLE_DOCTEST
