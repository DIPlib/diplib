/*
 * This example program demonstrates how to register a new measurement feature in dip::MeasurementTool.
 * The new class FeatureTest implements an image-based measurement feature (though it doesn't really do anything,
 * it just outputs a constant). The feature can be configured, and doing so changes the constant that is output
 * by the feature.
 */

#include <iostream>
#include "diplib/measurement.h"

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
            std::cout << "Unknown parameter!\n";
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
   measurementTool.Register( new FeatureTest );

   dip::Image label( dip::UnsignedArray{ 10, 10 }, 1, dip::DT_UINT8 );

   dip::Measurement msr = measurementTool.Measure( label, {}, { "Test" }, { 1, 2, 10, 12 } );
   std::cout << msr;

   measurementTool.Configure( "Test", "bla", 0 ); // Writes "Unknown parameter!" to stdout.
   measurementTool.Configure( "Test", "Value", 10 );

   msr = measurementTool.Measure( label, {}, { "Test" }, { 1, 2, 100, 18, 4 } );
   std::cout << msr;
}
