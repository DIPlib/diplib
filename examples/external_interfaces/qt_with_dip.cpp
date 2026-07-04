/*
 * This is a simple program that demonstrates mixing Qt's QImage and DIPlib.
 * A QImage is read from disk, mapped to a dip::Image (no copy), a DIPlib
 * filter is applied, and the result (allocated as a QImage through the
 * ExternalInterface, again no copy) is displayed in a Qt window.
 */

#include "dip_qimage_interface.h"

#include "diplib.h"
#include "diplib/linear.h"

#include <QApplication>
#include <QLabel>
#include <QPixmap>

int main( int argc, char** argv ) {
   QApplication app( argc, argv );

   std::string filename = ( argc > 1 ) ? argv[ 1 ] : DIP_EXAMPLES_DIR "/DIP.tif";

   // Read the image with Qt, convert to a supported format if necessary
   QImage input = QImage( QString::fromStdString( filename ) ).convertToFormat( QImage::Format_RGB888 );
   DIP_THROW_IF( input.isNull(), "Failed reading file" );

   // Map the QImage to a dip::Image, without copying
   dip::Image input_dip = dip_qimage::QImageToDip( input );
   DIP_ASSERT( input_dip.Origin() == input.bits() ); // Verify pointers match

   // Have DIPlib allocate the output as a QImage through the ExternalInterface
   dip_qimage::ExternalInterface qtei;
   dip::Image output_dip = qtei.NewImage();
   DIP_ASSERT( !output_dip.IsForged() ); // Verify image is not forged -- there is no data segment yet

   // Call a DIPlib function; Gauss needs a floating-point type, so we convert first
   dip::Image tmp = dip::Convert( input_dip, dip::DT_SFLOAT );
   dip::Gauss( tmp, tmp, { 3.0 } );
   dip::Convert( tmp, output_dip, dip::DT_UINT8 );
   DIP_ASSERT( output_dip.IsForged() ); // Verify image is now forged

   // Get the QImage back, no copy
   QImage output = qtei.GetQImage( output_dip );
   DIP_ASSERT( output_dip.Origin() == output.bits() ); // Verify pointers match

   // Display input and output
   QLabel inputLabel;
   inputLabel.setPixmap( QPixmap::fromImage( input ) );
   inputLabel.setWindowTitle( "input" );
   inputLabel.show();

   QLabel outputLabel;
   outputLabel.setPixmap( QPixmap::fromImage( output ) );
   outputLabel.setWindowTitle( "output (Gaussian blurred)" );
   outputLabel.move( inputLabel.x() + inputLabel.width() + 20, inputLabel.y() );
   outputLabel.show();

   return app.exec();
}
