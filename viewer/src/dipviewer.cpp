/*
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

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "diplib.h"
#include "dipviewer.h"

#ifdef DIP_CONFIG_HAS_FREEGLUT
#include "diplib/viewer/glut.h"
using ViewerManager = dip::viewer::GLUTManager;
#else
#ifdef DIP_CONFIG_HAS_GLFW
#include "diplib/viewer/glfw.h"
using ViewerManager = dip::viewer::GLFWManager;
#else
#include "diplib/viewer/proxy.h"
using ViewerManager = dip::viewer::ProxyManager;
#endif
#endif

#include "diplib/viewer/image.h"
#include "diplib/viewer/slice.h"

namespace dip { namespace viewer {

namespace {

std::unique_ptr< ViewerManager > manager_ = nullptr;
dip::uint count_ = 0;

String GetWindowTitle( String const& title ) {
   if( !title.empty()) {
      return title;
   }
   return String( "Window " ) + std::to_string( count_ + 1 );
}

void SetWindowPosition( Window* wdw ) {
   // 512x512 are the default window sizes. To improve on this, we'd need to keep
   // track of the windows we create, as each one could have a different size.
   constexpr dip::uint wszx = 512;
   constexpr dip::uint wszy = 512;
   auto ss = manager_->screenSize();
   dip::uint nx = ss[ 0 ] / wszx;
   dip::uint ny = ss[ 1 ] / wszy;
   if( nx > 0 && ny > 0 ) {
      dip::uint n = count_ % ( nx * ny );
      dip::uint x = ( n % nx ) * wszx;
      dip::uint y = ( n / nx ) * wszy;
      DIP_STACK_TRACE_THIS( wdw->setPosition( static_cast< int >( x ), static_cast< int >( y )));
   }
}

inline void Create() {
   if( !manager_ ) {
      manager_.reset( new ViewerManager() );
   }
   if( manager_->activeWindows() == 0 ) {
      // If we have not created any windows yet, or the user has closed all windows, reset the window count.
      count_ = 0;
   }
}

inline void Delete() {
   manager_.reset( nullptr );
}

} // namespace

SliceViewer::Ptr Show( Image const& image, String const& title, dip::uint width, dip::uint height ) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   Create();
   SliceViewer::Ptr wdw;
   DIP_STACK_TRACE_THIS( wdw = SliceViewer::Create( image, GetWindowTitle( title ), width, height ));
   DIP_STACK_TRACE_THIS( manager_->createWindow( wdw ));
   DIP_STACK_TRACE_THIS( SetWindowPosition( wdw.get() ));
   ++count_;

   return wdw;
}

ImageViewer::Ptr ShowSimple( Image const& image, String const& title, dip::uint width, dip::uint height ) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( image.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( image.DataType() != DT_UINT8, E::DATA_TYPE_NOT_SUPPORTED );
   Create();
   Image tmp = image.QuickCopy();
   if( image.IsScalar() ) {
      tmp.ExpandSingletonTensor( 3 );
   }
   tmp.ForceNormalStrides();
   ImageViewer::Ptr wdw;
   DIP_STACK_TRACE_THIS( wdw = ImageViewer::Create( tmp, GetWindowTitle( title ), width, height ));
   DIP_STACK_TRACE_THIS( manager_->createWindow( wdw ));
   DIP_STACK_TRACE_THIS( SetWindowPosition( wdw.get() ));
   ++count_;

   return wdw;
}

void Spin() {
   if( !manager_ ) {
      return;
   }
   //std::this_thread::sleep_for( std::chrono::seconds( 1 )); // Wait a while to make sure the windows have finished drawing
   while( manager_->activeWindows() ) {
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
