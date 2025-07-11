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

#ifndef DIP_VIEWER_MANAGER_H_
#define DIP_VIEWER_MANAGER_H_

#include <memory>

#include "diplib.h"
#include "diplib/viewer/export.h"

/// \file
/// \brief Declares \ref dip::viewer::Manager.

#define KEY_MOD_SHIFT   0x01
#if defined(__APPLE__) && defined(DIP_CONFIG_HAS_GLFW)
// We want to use the command key on MacOS computers, not control. But this code only works for GLFW.
// GLUT is not supported on MacOS anyway.
#define KEY_MOD_CONTROL 0x08
#else
#define KEY_MOD_CONTROL 0x02
#endif
#define KEY_MOD_ALT     0x04

namespace dip {
namespace viewer {

/// \addtogroup dipviewer

/// Simple GL window
class DIPVIEWER_CLASS_EXPORT Window {
   friend class GLUTManager;
   friend class GLFWManager;
   friend class ProxyManager;

   private:
      void* id_ = nullptr;
      class Manager* manager_ = nullptr ;
      bool destroyed_ = false ;
      int width_ = 512 ;
      int height_ = 512 ;

   public:
      Window() = default;
      Window( Window const& ) = delete;
      Window( Window&& ) = default;
      Window& operator=( Window const& ) = delete;
      Window& operator=( Window&& ) = default;
      virtual ~Window() = default;

      /// \brief Refresh window contents.
      DIPVIEWER_EXPORT void refresh();

      /// \brief Marks the window for destruction.
      void destroy() {
         destroyed_ = true;
         release();
      }

      /// \brief Returns whether the window is marked for destruction.
      ///
      /// This is set either from a callback, or by calling \ref destroy.
      bool destroyed() const { return destroyed_; }

      /// \brief Draw a string onto the window.
      ///
      /// Must be called from a callback.
      DIPVIEWER_EXPORT dip::uint drawString( const char* string );

      /// \brief Returns the window's width.
      int width() const { return width_; }

      /// \brief Returns the window's height.
      int height() const { return height_; }

      /// \brief Set the window's screen position.
      DIPVIEWER_EXPORT void setPosition( int x, int y );

      /// \brief Set the window's size.
      DIPVIEWER_EXPORT void setSize( int width, int height );

   protected:
      /// \brief Returns the \ref dip::viewer::Manager that manages this window.
      Manager* manager() { return manager_; }

      /// \brief Returns the window's identity.
      void* id() { return id_; }

      /// \brief Sets the window's title.
      ///
      /// Must be called from a callback.
      DIPVIEWER_EXPORT void title( const char* name );

      /// \brief Swaps display buffers.
      ///
      /// Must be called from a callback.
      DIPVIEWER_EXPORT void swap();

      /// \brief Suggests a window's size.
      ///
      /// Note that this function must be called before the \ref create callback is called.
      void requestSize( dip::uint width, dip::uint height ) {
         if( !manager_ ) {
            width_ = ( int )width;
            height_ = ( int )height;
         }
      }

      /// \brief Release any resources held or referenced by this window.
      virtual void release() {}

      /// Overridable callback that draws the visualization.
      virtual void draw() {}

      /// Overridable callback that is called periodically to allow for animation.
      virtual void idle() {}

      /// Overridable callback that is called when the window shape is changed.
      virtual void reshape( int /*width*/, int /*height*/ ) {}

      /// Overridable callback that is called when the window visibility changes.
      virtual void visible( int /*vis*/ ) {}

      /// Overridable callback that is called when the window is created.
      virtual void create() {}

      /// Overridable callback that is called when the window is closed.
      virtual void close() {}

      /// Overridable callback that is called when a key is pressed.
      DIPVIEWER_EXPORT virtual void key( unsigned char k, int x, int y, int mods );

      /// Overridable callback that is called when a mouse button is clicked.
      virtual void click( int /*button*/, int /*state*/, int /*x*/, int /*y*/, int /*mods*/ ) {}

      /// Overridable callback that is called when the mouse is moved while a button is clicked.
      virtual void motion( int /*x*/, int /*y*/ ) {}

   private:
      /// \brief Sets the window's manager.
      void manager( Manager* _manager ) { manager_ = _manager; }

      /// \brief Sets the window's identity.
      ///
      /// This should be called by the window's manager in order to set a unique
      /// identifier that can be used to determine the window context.
      void id( void* _id ) { id_ = _id; }

      /// \brief Sets the window's size.
      ///
      /// Note that this does not actually resize the window!
      void resize( int width, int height ) {
         width_ = width;
         height_ = height;
      }
};


/// \brief Shared pointer to a Window
using WindowPtr = std::shared_ptr< Window >;

/// Simple window manager.
class DIPVIEWER_CLASS_EXPORT Manager {
   friend class Window;

   public:
      Manager() = default;
      Manager( Manager const& ) = delete;
      Manager( Manager&& ) = default;
      Manager& operator=( Manager const& ) = delete;
      Manager& operator=( Manager&& ) = default;
      virtual ~Manager() = default;

      /// \brief Create a window.
      ///
      /// Example usage:
      ///
      /// ```cpp
      /// manager.createWindow( dip::viewer::SliceViewer::Create( image ));
      /// ```
      virtual void createWindow( WindowPtr window ) = 0;

      /// \brief Returns the number of managed windows.
      virtual dip::uint activeWindows() = 0;

      /// \brief Destroys all windows.
      virtual void destroyWindows() = 0;

      /// \brief Processes event queue.
      ///
      /// This function must be periodically called to allow user interaction.
      /// Example usage:
      ///
      /// ```cpp
      /// while ( manager.activeWindows()) {
      ///    manager.processEvents();
      ///    std::this_thread::sleep_for( std::chrono::microseconds( 1000 ));
      /// }
      /// ```
      virtual void processEvents() = 0;

      /// \brief Returns the size of the screen in pixels.
      ///
      /// May only be called after the window manager has been initialized,
      /// such as after creating a window.
      virtual UnsignedArray screenSize() const = 0;

   protected:
      /// \brief Swap display buffers.
      ///
      /// Must be called from the specified Window's callback.
      virtual void swapBuffers( Window* window ) = 0;

      /// \brief Sets a Window's title.
      ///
      /// Must be called from the specified Window's callback.
      virtual void setWindowTitle( Window* window, const char* name ) = 0;

      /// \brief Refresh a Window's contents.
      virtual void refreshWindow( Window* window ) = 0;

      /// \brief Set a Window's screen position.
      virtual void setWindowPosition( Window* window, int x, int y ) = 0;

      /// \brief Set a Window's size.
      virtual void setWindowSize( Window* window, int width, int height ) = 0;
};

/// \endgroup

} // namespace viewer
} // namespace dip

#endif /* DIP_VIEWER_MANAGER_H_ */
