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

#include "diplib/viewer/image.h"
#include "diplib/viewer/slice.h"

namespace dip { namespace viewer {

namespace {

std::unique_ptr< ViewerManager > manager_ = nullptr;
size_t count_ = 0;

String getWindowTitle( String const& title ) {
   if( !title.empty()) {
      return title;
   }
   return String( "Window " ) + std::to_string( count_ );
}

inline void Create() {
   if( !manager_ ) {
      manager_.reset( new ViewerManager() );
      count_ = 1;
   }
}

inline void Delete() {
   manager_.reset( nullptr );
}

} // namespace

SliceViewer::Ptr Show( Image const& image, String const& title, dip::uint width, dip::uint height ) {
   Create();
   SliceViewer::Ptr wdw;
   DIP_STACK_TRACE_THIS( wdw = SliceViewer::Create( image, getWindowTitle( title ), width, height ));
   DIP_STACK_TRACE_THIS( manager_->createWindow( wdw ));
   ++count_;
   
   return wdw;
}

ImageViewer::Ptr ShowSimple( Image const& image, String const& title, dip::uint width, dip::uint height ) {
   DIP_THROW_IF( image.DataType() != DT_UINT8, E::DATA_TYPE_NOT_SUPPORTED );
   Create();
   Image tmp = image.QuickCopy();
   if( image.IsScalar() ) {
      tmp.ExpandSingletonTensor( 3 );
   }
   tmp.ForceNormalStrides();
   ImageViewer::Ptr wdw;
   DIP_STACK_TRACE_THIS( wdw = ImageViewer::Create( tmp, getWindowTitle( title ), width, height ));
   DIP_STACK_TRACE_THIS( manager_->createWindow( wdw ));
   ++count_;
   
   return wdw;
}

void Spin() {
   if( !manager_ ) {
      return;
   }
   //std::this_thread::sleep_for( std::chrono::seconds( 1 )); // Wait a while to make sure the windows have finished drawing
   while( manager_->activeWindows()) {
      Draw();
      std::this_thread::sleep_for( std::chrono::microseconds( 100 ));
   }
   Delete();
}

void Draw() {
   if( !manager_ ) {
      return;
   }
   manager_->processEvents();
}

void CloseAll() {
   if( !manager_ ) {
      return;
   }
   manager_->destroyWindows();
   Spin();
}

}} // namespace dip::viewer
