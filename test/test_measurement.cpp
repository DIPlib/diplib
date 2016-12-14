#include <iostream>
#include <vector>

#include "diplib/measurement.h"

// Testing dip::Measurement

int main() {

   // Create measurement structure

   dip::Measurement msr;

   // msr.Data(); // throws exception

   {
      dip::Feature::ValueInformationArray values( 2 );
      values[ 0 ].name = "Dim1";
      values[ 0 ].units = dip::Units::Meter();
      values[ 1 ].name = "Dim2";
      values[ 1 ].units = dip::Units::Hertz();
      msr.AddFeature( "Feature1", values );

      values.resize( 1 );
      values[ 0 ].name = "Bla";
      values[ 0 ].units = dip::Units::SquareMeter();
      msr.AddFeature( "Feature2", values );

      values.resize( 3 );
      values[ 0 ].name = "Foo";
      values[ 0 ].units = dip::Units::SquareMeter();
      values[ 1 ].name = "Bar";
      values[ 1 ].units = dip::Units::CubicMeter();
      values[ 2 ].name = "Ska";
      values[ 2 ].units = dip::Units::Meter();
      msr.AddFeature( "Feature3", values );

      dip::UnsignedArray ids;
      for( dip::uint ii = 10; ii < 30; ++ii ) {
         ids.push_back( ii );
      }
      msr.AddObjectIDs( ids );

      msr.Forge();
   }

   // Write data

   dip::Measurement::ValueType* v = msr.Data();
   dip::uint nValues = msr.NumberOfValues();
   dip::uint nObjects = msr.NumberOfObjects();
   for( dip::uint ii = 0; ii < nValues * nObjects; ++ii ) {
      *v = ii;
      ++v;
   }

   // Print data row-wise

   {
      auto& features = msr.Features();
      for( auto& feature : features ) {
         std::cout << " -- " << feature.name << " (" << feature.numberValues << ", start = " << feature.startColumn
                   << ")";
      }
      std::cout << std::endl;
      for( auto const& feature : features ) {
         auto values = msr.Values( feature.name ); // could be done more efficiently, this is for testing...
         std::cout << " --";
         for( auto& value : values ) {
            std::cout << " " << value.name << " (" << value.units << ")";
         }
      }
      std::cout << std::endl;
      auto row = msr.FirstObject();
      do {
         std::cout << row.ObjectID();
         auto column = row.FirstFeature();
         do {
            std::cout << " --";
            for( auto& value : column ) {
               std::cout << " " << value;
            }
         } while( ++column );
         std::cout << std::endl;
      } while( ++row );
   }

   /*
   std::cout << std::endl << std::endl;
   auto values = msr.Values();
   for( auto& value : values ) {
      std::cout << " " << value.name << " (" << value.units << ")";
   }
   std::cout << std::endl;
   */

   // Print data column-wise

   {
      auto column = msr.FirstFeature();
      do {
         std::cout << column.Name();
         auto row = column.FirstObject();
         do {
            std::cout << " --";
            for( auto& value : row ) {
               std::cout << " " << value;
            }
         } while( ++row );
         std::cout << std::endl;
      } while( ++column );
   }

   // msr.AddFeature( "no", {} ); // throws exception

   return 0;
}
