#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>

#include "diplib/measurement.h"

// Testing dip::MeasurementTool

class FeatureTest : public dip::Feature::ImageBased {
   public:
      FeatureTest() : ImageBased( { "Test", "Test feature", false } ) {};

      virtual dip::Feature::ValueInformationArray Initialize( dip::Image const&, dip::Image const&, dip::uint ) override {
         dip::Feature::ValueInformationArray out( 1 );
         out[ 0 ].name = "testing";
         return out;
      }

      virtual void Configure( dip::String const& parameter, dip::dfloat value ) override {
         if( parameter == "Value" ) {
            value_ = value;
         } else {
            std::cout << "Unknown parameter!" << std::endl;
         }
      }

      virtual void Measure( dip::Image const&, dip::Image const&, dip::Measurement::IteratorFeature& output ) override {
         auto dst = output.FirstObject();
         do {
            dst[ 0 ] = value_;
         } while( ++dst );
      }

   private:
      dip::dfloat value_ = 0;
};

int main() {

   dip::MeasurementTool measurementTool;

   auto f = measurementTool.Features();

   measurementTool.Register( new FeatureTest );

   dip::Image label( dip::UnsignedArray{ 10, 10 }, 1, dip::DT_UINT8 );

   dip::Measurement msr = measurementTool.Measure( label, dip::Image{}, { "Test" }, { 1, 2, 10, 12 } );
   std::cout << msr;

   measurementTool.Configure( "Test", "bla", 0 );
   measurementTool.Configure( "Test", "Value", 10 );

   msr = measurementTool.Measure( label, dip::Image{}, { "Test" }, { 1, 2, 100, 18, 4 } );
   std::cout << msr;

   return 0;
}
