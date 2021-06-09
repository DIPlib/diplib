/*
 * DIPlib 3.0
 * This file contains unit tests for the iterators.
 *
 * (c)2016-2018, Cris Luengo.
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

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include <doctest/doctest.h>
#include "diplib/generic_iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing ImageIterator and GenericImageIterator") {
   dip::Image img{ dip::UnsignedArray{ 3, 2, 4 }, 1, dip::DT_UINT16 };
   DOCTEST_REQUIRE( img.DataType() == dip::DT_UINT16 );
   {
      dip::ImageIterator< dip::uint16 > it( img );
      dip::uint16 counter = 0;
      do {
         *it = static_cast< dip::uint16 >( counter++ );
      } while( ++it );
      DOCTEST_CHECK( !it.HasProcessingDimension() );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( !it.HasProcessingDimension() );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1 } );
   }
   {
      dip::ImageIterator< dip::uint16 > it( img, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3 } );
   }
   img.Rotation90( 1 ); // rotates over dims 0 and 1.
   {
      dip::ImageIterator< dip::uint16 > it( img, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
   }
   {
      dip::ImageIterator< dip::uint16 > it( img, 1 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3 } );
   }
   {
      dip::ImageIterator< dip::uint16 > it( img, 2 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 2 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3*2 } );
   }

   img.StandardizeStrides(); // returns strides to normal.
   {
      dip::GenericImageIterator< dip::sint32 > it( img );
      dip::sint32 counter = 0;
      do {
         DOCTEST_CHECK( *it == counter++ );
      } while( ++it );
      DOCTEST_CHECK( !it.HasProcessingDimension() );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1 } );
   }
   {
      dip::GenericImageIterator<> it( img, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3 * 2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2 * 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3 } );
   }
   img.Rotation90( 1 ); // rotates over dims 0 and 1.
   {
      dip::GenericImageIterator<> it( img, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
   }
   {
      dip::GenericImageIterator<> it( img, 1 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3 } );
   }
   {
      dip::GenericImageIterator<> it( img, 2 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 2 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3*2 } );
   }

   dip::Image img2{ dip::UnsignedArray{ 3, 4 }, 3, dip::DT_SINT32 };
   DOCTEST_REQUIRE( img2.DataType() == dip::DT_SINT32 );
   {
      dip::ImageIterator< dip::sint32 > it( img2 );
      dip::sint32 counter = 0;
      do {
         it[ 0 ] = static_cast< dip::sint32 >( counter );
         it[ 1 ] = static_cast< dip::sint32 >( counter * 1000 );
         it[ 2 ] = static_cast< dip::sint32 >( counter * -10000 );
         ++counter;
      } while( ++it );
   }
   {
      dip::GenericImageIterator< dip::sint32 > it( img2 );
      dip::sint32 counter = 0;
      do {
         DOCTEST_CHECK( it[ 0 ] == counter );
         DOCTEST_CHECK( it[ 1 ] == counter * 1000 );
         DOCTEST_CHECK( it[ 2 ] == counter * -10000 );
         ++counter;
      } while( ++it );
   }
   {
      dip::GenericImageIterator< dip::sint32 > it( img2 );
      ++it;
      auto iit = it.begin();
      DOCTEST_CHECK( *iit == 1 );
      ++iit;
      DOCTEST_CHECK( *iit == 1000 );
      ++iit;
      DOCTEST_CHECK( *iit == -10000 );
      ++iit;
      DOCTEST_CHECK( iit == it.end() );
   }
}

DOCTEST_TEST_CASE("[DIPlib] testing JointImageIterator and GenericJointImageIterator") {
   dip::Image imgA{ dip::UnsignedArray{ 3, 2, 4 }, 1, dip::DT_UINT16 };
   dip::Image imgB{ dip::UnsignedArray{ 3, 2, 4 }, 1, dip::DT_SINT8 };
   DOCTEST_REQUIRE( imgA.DataType() == dip::DT_UINT16 );
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB } );
      dip::uint16 counter = 0;
      do {
         it.Sample< 0 >() = static_cast< dip::uint16 >( counter++ );
      } while( ++it );
      DOCTEST_CHECK( !it.HasProcessingDimension() );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( !it.HasProcessingDimension() );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1 } );
   }
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB }, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3 } );
   }
   imgA.Rotation90( 1 ); // rotates over dims 0 and 1.
   imgB.Rotation90( 1 ); // rotates over dims 0 and 1.
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB }, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
   }
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB }, 1 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3 } );
   }
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB }, 2 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 2 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3*2 } );
   }

   imgA.StandardizeStrides(); // returns strides to normal.
   imgB.StandardizeStrides(); // returns strides to normal.
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB } );
      dip::sint32 counter = 0;
      do {
         DOCTEST_CHECK( (dip::sint32)(it.Sample< 0 >() ) == counter++ );
      } while( ++it );
      DOCTEST_CHECK( !it.HasProcessingDimension() );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1 } );
   }
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB }, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3 * 2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3 * 2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2 * 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3 } );
   }
   imgA.Rotation90( 1 ); // rotates over dims 0 and 1.
   imgB.Rotation90( 1 ); // rotates over dims 0 and 1.
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB }, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
   }
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB }, 1 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3 } );
   }
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB }, 2 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 2 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3*2 } );
   }
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
