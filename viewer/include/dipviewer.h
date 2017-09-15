/*
 * DIPlib 3.0 viewer
 * This file contains definitions for all classes need to use the DIPviewer.
 *
 * (c)2017, Wouter Caarls
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

#ifndef DIPVIEWER_H
#define DIPVIEWER_H


// TODO: All of this needs to be in a `dip::viewer::` a `dipviewer::` namespace.

// TODO: Document usage:
// ```cpp
//     #include "diplib.h"
//     #include "dipviewer.h"
//     int main() {
//        dip::Image grey( { 300, 200 }, 1, dip::DT_UINT16 );
//        dip::FillRadiusCoordinate( grey );
//        dip::Image result = dip::Dilation( grey );
//        ViewerManager manager;
//        ShowImage( manager, grey );
//        ShowImage( manager, result );
//        WaitForWindows( manager );
//        return 0;
//     }
// ```


#ifdef DIP__HAS_GLFW
#include "diplib/viewer/glfw.h"
   using ViewerManager = GLFWManager;
#else
#include "diplib/viewer/glut.h"
using ViewerManager = GLUTManager;
#endif

#include "diplib/viewer/slice.h"


void ShowImage( ViewerManager& manager, dip::Image const& image ) {
   dip::Image imgCopy = image;
   if( imgCopy.DataType().IsBinary() ) {
      // Convert binary to uint8 for display (SliceViewer doesn't support binary images because of the histogram).
      // This happens in-place, because the input and output sample sizes are identical.
      // However, it will touch each of the samples in `image`, without changing any values.
      imgCopy.Convert( dip::DT_UINT8 );
   }
   manager.createWindow( WindowPtr( new SliceViewer( imgCopy )));
}

void WaitForWindows( ViewerManager& manager ) {
   while( manager.activeWindows()) {
      manager.processEvents(); // Only necessary for GLFW
      std::this_thread::sleep_for( std::chrono::microseconds( 100 ));
   }
}


#endif // DIPVIEWER_H
