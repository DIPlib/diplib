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

#ifndef DIP_VIEWER_GLUT_H_
#define DIP_VIEWER_GLUT_H_

#include <map>
#include <mutex>
#include <thread>

#include "diplib.h"
#include "diplib/viewer/export.h"
#include "diplib/viewer/manager.h"

/// \file
/// \brief Declares the GLUT interface of \ref dipviewer.

namespace dip {
namespace viewer {

/// \addtogroup dipviewer

/// Simple GLUT window manager.
class DIPVIEWER_CLASS_EXPORT GLUTManager : public Manager {
   protected:
      using WindowMap = std::map< void*, WindowPtr >;
      using Guard = std::lock_guard< std::recursive_mutex >;

      std::thread thread_;
      std::recursive_mutex mutex_;
      bool continue_, active_;
      WindowMap windows_;
      WindowPtr new_window_;
      static GLUTManager* instance_;

   public:
      DIPVIEWER_EXPORT GLUTManager();
      DIPVIEWER_EXPORT ~GLUTManager() override;
      GLUTManager( GLUTManager const& ) = delete;
      GLUTManager( GLUTManager&& ) = delete;
      GLUTManager& operator=( GLUTManager const& ) = delete;
      GLUTManager& operator=( GLUTManager&& ) = delete;

      DIPVIEWER_EXPORT void createWindow( WindowPtr window ) override;

      dip::uint activeWindows() override {
         Guard guard( mutex_ );
         return windows_.size();
      }

      DIPVIEWER_EXPORT void destroyWindows() override;
      DIPVIEWER_EXPORT void processEvents() override {}
      DIPVIEWER_EXPORT UnsignedArray screenSize() const override;

   protected:
      DIPVIEWER_EXPORT void swapBuffers( Window* window ) override;
      DIPVIEWER_EXPORT void setWindowTitle( Window* window, const char* name ) override;
      DIPVIEWER_EXPORT void refreshWindow( Window* window ) override;
      DIPVIEWER_EXPORT void setWindowPosition( Window* window, int x, int y ) override;
      DIPVIEWER_EXPORT void setWindowSize( Window* window, int x, int y ) override;

      DIPVIEWER_EXPORT void run();
      DIPVIEWER_EXPORT WindowPtr getCurrentWindow();

      // Delegates
      static void idle() {
         for( WindowMap::iterator it = instance_->windows_.begin(); it != instance_->windows_.end(); ++it ) {
            it->second->idle();
         }
      }

      static void draw() {
         WindowPtr window = instance_->getCurrentWindow();
         if( window ) {
            window->draw();
         }
      }

      static void reshape( int width, int height ) {
         WindowPtr window = instance_->getCurrentWindow();
         if( window ) {
            window->resize( width, height );
            window->reshape( width, height );
         }
      }

      static void visible( int vis ) {
         WindowPtr window = instance_->getCurrentWindow();
         if( window ) {
            window->visible( vis );
         }
      }

      static void close() {
         WindowPtr window = instance_->getCurrentWindow();
         if( window ) {
            window->close();
            window->destroy();
            instance_->windows_.erase( window->id() );
         }
      }

      DIPVIEWER_EXPORT static void key( unsigned char k, int x, int y );

      DIPVIEWER_EXPORT static void click( int button, int state, int x, int y );

      static void motion( int x, int y ) {
         WindowPtr window = instance_->getCurrentWindow();
         if( window ) {
            window->motion( x, y );
         }
      }
};

/// \endgroup

} // namespace viewer
} // namespace dip

#endif /* DIP_VIEWER_GLUT_H_ */
