/*
 * This is a simple program that demonstrates different ways of mixing OpenCV and DIPlib.
 * The first section is DIPlib code that makes a call to OpenCV.
 * The second section is OpenCV code that makes a call to DIPlib.
 * The difference is in which library allocates the pixel data. For both directions it is
 * possible to create an image object that encapsulates pixel data allocated by the other
 * library.
 */

#include "dip_opencv_interface.h"
namespace dcv = dip_opencv;

#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/linear.h"

#include <opencv2/opencv.hpp>

#include "dipviewer.h"

int main() {

   {
      // Part 1: This is a DIPlib program that calls an OpenCV function, putting input and output data
      // into a DIPlib image object.

      std::cout << "\n -- Part 1: DIPlib program that calls OpenCV function\n";

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

      // Create an OpenCV object that points to the pixel data in `input`
      cv::Mat input_mat = dcv::DipToMat( input );
      DIP_ASSERT( input.Origin() == input_mat.ptr() ); // Verify pointers match

      // Create an output image in DIPlib, and encapsulate in OpenCV object
      dip::Image output = input.Similar();
      cv::Mat output_mat = dcv::DipToMat( output );
      DIP_ASSERT( output.Origin() == output_mat.ptr() ); // Verify pointers match

      // Call an OpenCV function
      cv::GaussianBlur( input_mat, output_mat, { 0, 0 }, 4 );
      DIP_ASSERT( output.Origin() == output_mat.ptr() ); // Verify pointers still match

      // Now, `output` will have been modified by OpenCV
      dip::viewer::Show( input, "input" );
      dip::viewer::Show( output, "output" );
      dip::viewer::Spin();
   }

   {
      // Part 2: This is an OpenCV program that calls a DIPlib function, putting input and output data
      // into an OpenCV image object. There are two options here:
      //    1: Allocate an OpenCV cv::Mat object of the right sizes and type to hold the output of the
      //       DIPlib function, and encapsulate that in a dip::Image object.
      //    2: Create an uninitialized DIPlib dip::Image object with an "external interface". This will
      //       cause DIPlib to call the right OpenCV functions to allocate memory for the output image.
      // The second mode is simper in use, and should be preferred, but won't allow to control the data
      // type of the output image.

      std::cout << "\n -- Part 2: OpenCV program that calls DIPlib function, method 1\n";

      cv::Mat input = cv::imread( DIP_EXAMPLES_DIR "/DIP.tif", cv::IMREAD_UNCHANGED ); // cv::IMREAD_GRAYSCALE
      DIP_THROW_IF( !input.data, "Failed reading file\n" );
      input = input.colRange( 5, 250 ); // Crop the image so it is not square, we want to see proper sizes in DIPlib

      // Create a DIPlib object that points to the pixel data in `input`
      dip::Image input_dip = dcv::MatToDip( input );
      DIP_ASSERT( input_dip.Origin() == input.ptr() ); // Verify pointers match

      // Create an output image in OpenCV, and encapsulate in DIPlib object
      cv::Mat output( input.size(), input.type() );
      dip::Image output_dip = dcv::MatToDip( output );
      DIP_ASSERT( output_dip.Origin() == output.ptr() ); // Verify pointers match

      // Call a DIPlib function
      dip::Gauss( input_dip, output_dip, { 4.0 } );
      DIP_ASSERT( output_dip.Origin() == output.ptr() ); // Verify pointers still match

      // Now, `output` will have been modified by DIPlib:
      cv::imshow( "input", input );
      cv::imshow( "output", output );
      cv::waitKey();

      std::cout << "\n -- Part 2: OpenCV program that calls DIPlib function, method 2\n";

      // Alternative for the output: have DIPlib allocate an OpenCV object for the output data
      dcv::ExternalInterface ei;
      dip::Image output2_dip = ei.NewImage();
      DIP_ASSERT( !output2_dip.IsForged() ); // Verify image is not forged -- there is no data segment yet

      // Call a DIPlib function
      dip::Gauss( input_dip, output2_dip, { 4.0 } );
      DIP_ASSERT( output2_dip.IsForged() ); // Verify image is now forged

      // Get the OpenCV image back
      cv::Mat output2 = ei.GetMat( output2_dip );
      DIP_ASSERT( output2_dip.Origin() == output2.ptr() ); // Verify pointers match

      // dip::Gauss will create a single-precision float image, which OpenCV doesn't display properly.
      // We will normalize the output to the range 0-1 for cv::imshow.
      // Note that modifying the DIPlib object modifies the OpenCV object, they are still pointing to the same data.
      output2_dip /= 255;

      // Display to show OpenCV received the data computed by DIPlib
      cv::imshow( "input", input );
      cv::imshow( "output", output2 );
      cv::waitKey();
   }
}
