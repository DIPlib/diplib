/*
 * This is a simple program that demonstrates different ways of mixing OpenCV and DIPlib.
 * The first section is DIPlib code that makes a call to OpenCV.
 * The second section is OpenCV code that makes a call to DIPlib.
 * The difference is in which library allocates the pixel data. For both directions it is
 * possible to create an image object that encapsulates pixel data allocated by the other
 * library.
 * Note that the assertions in the code are meant to illustrate what happens, they need
 * not be copied to your code.
 */

#include "dip_opencv_interface.h"

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
      cv::Mat input_mat = dip_opencv::DipToMat( input );
      DIP_ASSERT( input.Origin() == input_mat.ptr() ); // Verify pointers match

      // Create an output image in DIPlib, and encapsulate in OpenCV object
      dip::Image output = input.Similar();
      cv::Mat output_mat = dip_opencv::DipToMat( output );
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
      // into an OpenCV image object. There are three options here:
      //    1: Allocate an OpenCV cv::Mat object of the right sizes and type to hold the output of the
      //       DIPlib function, and encapsulate that in a dip::Image object.
      //    2: Create an uninitialized DIPlib dip::Image object with an "external interface". This will
      //       cause DIPlib to call the right OpenCV functions to allocate memory for the output image.
      //    3: Convert the DIPlib output image to an OpenCV type. This is the simplest approach at first
      //       sight, but has a DIPlib object owning data used by an OpenCV object, which could lead to
      //       difficult to spot errors.
      // The second method does not require deciding on output image sizes and types, and therefore is simper
      // in use than the first method. We recommend that you use the second method.

      cv::Mat input = cv::imread( DIP_EXAMPLES_DIR "/DIP.tif", cv::IMREAD_UNCHANGED ); // cv::IMREAD_GRAYSCALE
      DIP_THROW_IF( !input.data, "Failed reading file\n" );
      input = input.colRange( 5, 250 ); // Crop the image so it is not square, we want to see proper sizes in DIPlib

      {
         std::cout << "\n -- Part 2: OpenCV program that calls DIPlib function, method 1\n";

         // Create a DIPlib object that points to the pixel data in `input`
         dip::Image input_dip = dip_opencv::MatToDip( input );
         DIP_ASSERT( input_dip.Origin() == input.ptr()); // Verify pointers match

         // Create an output image in OpenCV, and encapsulate in DIPlib object
         cv::Mat output( input.size(), input.type());
         dip::Image output_dip = dip_opencv::MatToDip( output );
         DIP_ASSERT( output_dip.Origin() == output.ptr()); // Verify pointers match

         // Call a DIPlib function
         dip::Gauss( input_dip, output_dip, { 4.0 } );
         DIP_ASSERT( output_dip.Origin() == output.ptr()); // Verify pointers still match

         // Now, `output` will have been modified by DIPlib
         cv::imshow( "input", input );
         cv::imshow( "output", output );
         cv::waitKey();
      }

      {
         std::cout << "\n -- Part 2: OpenCV program that calls DIPlib function, method 2\n";

         // Create a DIPlib object that points to the pixel data in `input`
         dip::Image input_dip = dip_opencv::MatToDip( input );
         DIP_ASSERT( input_dip.Origin() == input.ptr()); // Verify pointers match

         // Alternative for the output: have DIPlib allocate an OpenCV object for the output data
         dip_opencv::ExternalInterface ei;
         dip::Image output_dip = ei.NewImage();
         DIP_ASSERT( !output_dip.IsForged()); // Verify image is not forged -- there is no data segment yet

         // Call a DIPlib function
         dip::Gauss( input_dip, output_dip, { 4.0 } );
         DIP_ASSERT( output_dip.IsForged()); // Verify image is now forged

         // Get the OpenCV image back
         cv::Mat output = ei.GetMat( output_dip );
         DIP_ASSERT( output_dip.Origin() == output.ptr()); // Verify pointers match

         // dip::Gauss will create a single-precision float image, which OpenCV doesn't display properly.
         // We will normalize the output to the range 0-1 for cv::imshow.
         // Note that modifying the DIPlib object modifies the OpenCV object, they are still pointing to the same data.
         output_dip /= 255;
         DIP_ASSERT( output_dip.Origin() == output.ptr()); // Verify pointers still match

         // Display to show OpenCV received the data computed by DIPlib
         cv::imshow( "input", input );
         cv::imshow( "output", output );
         cv::waitKey();
      }

      {
         std::cout << "\n -- Part 2: OpenCV program that calls DIPlib function, method 3\n";

         // Create a DIPlib object that points to the pixel data in `input`
         dip::Image input_dip = dip_opencv::MatToDip( input );
         DIP_ASSERT( input_dip.Origin() == input.ptr()); // Verify pointers match

         // Call a DIPlib function
         dip::Image output_dip = dip::Gauss( input_dip, { 4.0 } );

         // Create an OpenCV image around the DIPlib pixel data
         // Note that this could throw if the DIPlib image has data in a form that OpenCV cannot use
         cv::Mat output = dip_opencv::DipToMat( output_dip );
         DIP_ASSERT( output_dip.Origin() == output.ptr()); // Verify pointers match
         // Be careful to keep `output_dip` around for as long as you need `output`.
         // Alternatively, use this form, which is safer but more costly:
         //    cv::Mat output = dip_opencv::CopyDipToMat( output_dip );

         // Display to show OpenCV received the data computed by DIPlib
         cv::imshow( "input", input );
         cv::imshow( "output", output );
         cv::waitKey();
      }
   }
}
