/*
 * DIPlib 3.0 viewer
 * This file contains definitions for rudamentary window management.
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

#ifndef DIP_VIEWER_MANAGER_H_
#define DIP_VIEWER_MANAGER_H_

#include "diplib.h"
#include "dipviewer_export.h"

#include <memory>

/// \file
/// \brief Declares `dip::viewer::Manager`.

#define KEY_MOD_SHIFT   0x01
#if defined(__APPLE__) && defined(DIP__HAS_GLFW)
// We want to use the command key on MacOS computers, not control. But this code only works for GLFW.
// GLUT is not supported on MacOS anyway.
#define KEY_MOD_CONTROL 0x08
#else
#define KEY_MOD_CONTROL 0x02
#endif
#define KEY_MOD_ALT     0x04

namespace dip { namespace viewer {

/// Simple GL window
class DIPVIEWER_EXPORT Window
{
  friend class GLUTManager;
  friend class GLFWManager;

  private:
    void *id_;
    class Manager *manager_;
    bool should_close_;
    int width_, height_;
    
  public:
    Window() : id_(NULL), manager_(NULL), should_close_(false), width_(512), height_(512) { }
    virtual ~Window() { }

    /// \brief Refresh window contents.
    void refresh();
    
    /// \brief Marks the window for destruction.
    void destroy() { should_close_ = true; }
    
    /// \brief Draw a string onto the window.
    ///
    /// Must be called from a callback.
    size_t drawString(const char *string);
    
    /// \brief Returns the window's width.
    int width() { return width_; }

    /// \brief Returns the window's height.
    int height() { return height_; }
  protected:
    /// \brief Returns the dip::viewer::Manager that manages this window.
    Manager *manager() { return manager_; }
    
    /// \brief Returns the window's identity.
    void *id() { return id_; }
    
    /// \brief Returns whether the window is marked for destruction.
    ///
    /// This is set either from a callback, or by calling destroy().
    bool shouldClose() { return should_close_; }
    
    /// \brief Sets the window's title.
    ///
    /// Must be called from a callback.
    void title(const char *name);
    
    /// \brief Swaps display buffers.
    ///
    /// Must be called from a callback.
    void swap();
    
    /// \brief Suggests a window's size.
    ///
    /// *** Note *** that this function must be called before the create() callback is called.
    void requestSize(size_t width, size_t height)
    {
      if (!manager_)
      {
        width_ = (int)width;
        height_ = (int)height;
      }
    }
    
    /// Overridable callback that draws the visualization.
    virtual void draw() { }
    
    /// Overridable callback that is called periodically to allow for animation.
    virtual void idle() { }
    
    /// Overridable callback that is called when the window shape is changed.
    virtual void reshape(int /*width*/, int /*height*/) { }
    
    /// Overridable callback that is called when the window visibility changes.
    virtual void visible(int /*vis*/) { }
    
    /// Overridable callback that is called when the window is created.
    virtual void create() { }
    
    /// Overridable callback that is called when the window is closed.
    virtual void close() { }

    /// Overridable callback that is called when a key is pressed.
    virtual void key(unsigned char k, int x, int y, int mods);
    
    /// Overridable callback that is called when a mouse button is clicked.
    virtual void click(int /*button*/, int /*state*/, int /*x*/, int /*y*/) { }
    
    /// Overridable callback that is called when the mouse is moved while a button is clicked.
    virtual void motion(int /*x*/, int /*y*/) { }
    
  private:
    /// \brief Sets the window's manager.
    void manager(Manager *_manager) { manager_ = _manager; }
    
    /// \brief Sets the window's identity.
    ///
    /// This should be called by the window's manager in order to set a unique
    /// identifier that can be used to determine the window context.
    void id(void *_id) { id_ = _id; }
    
    /// \brief Sets the window's size.
    ///
    /// *** Note *** that this does not actually resize the window!
    void resize(int width, int height) { width_ = width; height_ = height; }
};

/// \addtogroup viewer
/// \{

/// \brief Shared pointer to a Window
typedef std::shared_ptr<Window> WindowPtr;

/// Simple window manager.
class DIPVIEWER_EXPORT Manager
{
  friend class Window;

  public:
    virtual ~Manager() { }
  
    /// \brief Create a window.
    ///
    /// Example usage:
    ///
    /// ```cpp
    ///     manager.createWindow( dip::viewer::WindowPtr( new dip::viewer::SliceViewer( image )));
    /// ```
    virtual void createWindow(WindowPtr window) = 0;
    
    /// \brief Returns the number of managed windows.
    virtual size_t activeWindows() = 0;
    
    /// \brief Destroys all windows.
    virtual void destroyWindows() = 0;
    
    /// \brief Processes event queue.
    ///
    /// This function must be periodically called to allow user interaction.
    /// Example usage:
    ///
    /// ```cpp
    ///     while ( manager.activeWindows()) {
    ///        manager.processEvents();
    ///        std::this_thread::sleep_for( std::chrono::microseconds( 1000 ) );
    ///     }
    /// ```
    virtual void processEvents() = 0;
    
  protected:
    /// \brief Draw a string onto a window.
    ///
    /// Must be called from the specified Window's callback.
    virtual size_t drawString(Window* window, const char *string) = 0;

    /// \brief Swap display buffers.
    ///
    /// Must be called from the specified Window's callback.
    virtual void swapBuffers(Window* window) = 0;

    /// \brief Sets a Window's title.
    ///
    /// Must be called from the specified Window's callback.
    virtual void setWindowTitle(Window* window, const char *name) = 0;

    /// \brief Refresh a Window's contents.
    virtual void refreshWindow(Window *window) = 0;
};

/// \}

}} // namespace dip::viewer

#endif /* DIP_VIEWER_MANAGER_H_ */
