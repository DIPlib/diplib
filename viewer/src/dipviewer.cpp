/*
 * DIPlib 3.0 viewer
 * This file contains source for all classes need to use the DIPviewer.
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

#include "dipviewer.h"

#ifdef DIP__HAS_GLFW
#include "diplib/viewer/glfw.h"
using ViewerManager = dip::viewer::GLFWManager;
#else
#include "diplib/viewer/glut.h"
using ViewerManager = dip::viewer::GLUTManager;
#endif

#include "diplib/viewer/slice.h"
#include "diplib/viewer/image.h"

namespace dip { namespace viewer {

namespace {

ViewerManager *manager__ = NULL;
size_t count__ = 0;

String getWindowTitle( String const& title ) {
   if( !title.empty() ) {
      return title;
   }
   return String("Window ") + std::to_string( count__ );
}

} // namespace

void Show( Image const& image, String const& title )
{
   if (!manager__) {
      manager__ = new ViewerManager();
      count__ = 1;
   }

   Image imgCopy = image;
   if( imgCopy.DataType().IsBinary() ) {
      // Convert binary to uint8 for display (SliceViewer doesn't support binary images because of the histogram).
      // This happens in-place, because the input and output sample sizes are identical.
      imgCopy.Convert( DT_UINT8 );
   }
   manager__->createWindow( WindowPtr( new SliceViewer( imgCopy, getWindowTitle( title ))));
   ++count__;
}

void ShowSimple( Image const& image, String const& title )
{
   if (!manager__) {
      manager__ = new ViewerManager();
      count__ = 1;
   }

   manager__->createWindow( WindowPtr( new ImageViewer( image, getWindowTitle( title ))));
   ++count__;
}

void Spin( )
{
   if (!manager__)
      return;

   while( manager__->activeWindows() ) {
      Draw( );
      std::this_thread::sleep_for( std::chrono::microseconds( 100 ));
   }
   
   delete manager__;
   manager__ = NULL;
}

void Draw( )
{
   if (!manager__)
      return;
   
   manager__->processEvents();
}

}} // namespace dip::viewer
