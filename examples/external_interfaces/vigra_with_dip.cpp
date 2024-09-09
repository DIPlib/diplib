/*
 * This is a simple program that demonstrates different ways of mixing Vigra and DIPlib.
 * The first section is DIPlib code that makes a call to Vigra.
 * The second section is Vigra code that makes a call to DIPlib.
 * The difference is in which library allocates the pixel data. For both directions it is
 * possible to create an image object that encapsulates pixel data allocated by the other
 * library.
 */

#include "dip_vigra_interface.h"

#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/linear.h"

#include <vigra/multi_array.hxx>
#include <vigra/convolution.hxx>
#include <vigra/impex.hxx>

#include "dipviewer.h"

int main() {

   {
      // Part 1: This is a DIPlib program that calls a Vigra function, putting input and output data
      // into a DIPlib image object.

      std::cout << "\n -- Part 1: DIPlib program that calls Vigra function\n";

      // Create a test image in DIPlib
      dip::Random random;
      dip::TestObjectParams params;
      params.generationMethod = "fourier";
      params.objectSizes = { 250, 200 };
      params.objectAmplitude = 255 - 15 - 15;
      params.backgroundValue = 15;
      params.randomShift = true;
      params.signalNoiseRatio = 100;
      params.poissonNoise = 0;
      dip::Image input = dip::TestObject( dip::UnsignedArray{ 400, 300 }, params, random );
      input = dip::Gradient( input );
      input.Rotation90( 1 ); // Test this with non-standard strides

      // Create a Vigra object that points to the pixel data in `input`
      using TwoVector = vigra::TinyVector< dip::sfloat, 2 >;
      auto input_array = dip_vigra::DipToVigra< 2, TwoVector >( input );
      DIP_ASSERT( input.Origin() == input_array.data() ); // Verify pointers match

      // Create an output image in DIPlib, and encapsulate in Vigra object
      dip::Image output = input.Similar();
      auto output_array = dip_vigra::DipToVigra< 2, TwoVector >( output );
      DIP_ASSERT( output.Origin() == output_array.data() ); // Verify pointers match

      // Call a Vigra function
      vigra::gaussianSmoothing( input_array, output_array, 4 );
      DIP_ASSERT( output.Origin() == output_array.data() ); // Verify pointers still match

      // Now, `output` will have been modified by Vigra
      dip::viewer::Show( input, "input" );
      dip::viewer::Show( output, "output" );
      dip::viewer::Spin();
   }

   {
      // Part 2: This is a Vigra program that calls a DIPlib function, putting input and output data
      // into a Vigra image object.

      std::cout << "\n -- Part 2: Vigra program that calls DIPlib function\n";

      // Read an image from disk
      vigra::MultiArray< 2, vigra::UInt8 > input;
      vigra::importImage( DIP_EXAMPLES_DIR "/cameraman.tif", input );
      input = input.subarray( { 0, 5 }, { 255, 250 } ); // Crop the image so it is not square, we want to see proper sizes in DIPlib

      // Create a DIPlib object that points to the pixel data in `input`
      dip::Image input_dip = dip_vigra::VigraToDip( input );
      DIP_ASSERT( input_dip.Origin() == input.data() ); // Verify pointers match

      // Create an output image in Vigra, and encapsulate in DIPlib object
      vigra::MultiArray< 2, vigra::UInt8 > output( input.shape() );
      dip::Image output_dip = dip_vigra::VigraToDip( output );
      DIP_ASSERT( output_dip.Origin() == output.data() ); // Verify pointers match

      // Call a DIPlib function
      dip::Gauss( input_dip, output_dip, { 4.0 } );
      DIP_ASSERT( output_dip.Origin() == output.data() ); // Verify pointers still match

      // Now, `output` will have been modified by DIPlib. Write to file.
      vigra::exportImage( output, "test.gif" );
   }
}
