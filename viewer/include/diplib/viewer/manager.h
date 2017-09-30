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

/// \addtogroup viewer
/// \{

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

    void refresh();
    size_t drawString(const char *string);
    int width() { return width_; }
    int height() { return height_; }
  protected:
    Manager *manager() { return manager_; }
    void *id() { return id_; }  
    bool shouldClose() { return should_close_; }
    
    void title(const char *name);
    void swap();
    void destroy() { should_close_ = true; }
    void requestSize(size_t width, size_t height)
    {
      if (!manager_)
      {
        width_ = (int)width;
        height_ = (int)height;
      }
    }
    
    /// Callback that draws the visualization.
    virtual void draw() { }
    
    /// Callback that is called periodically to allow for animation.
    virtual void idle() { }
    
    /// Callback that is called when the window shape is changed.
    virtual void reshape(int /*width*/, int /*height*/) { }
    
    /// Callback that is called when the window visibility changes.
    virtual void visible(int /*vis*/) { }
    
    /// Callback that is called when the window is created.
    virtual void create() { }
    
    /// Callback that is called when the window is closed.
    virtual void close() { }

    /// Callback that is called when a key is pressed.
    virtual void key(unsigned char k, int x, int y, int mods);
    
    /// Callback that is called when a mouse button is clicked.
    virtual void click(int /*button*/, int /*state*/, int /*x*/, int /*y*/) { }
    
    /// Callback that is called when the mouse is moved while a button is clicked.
    virtual void motion(int /*x*/, int /*y*/) { }
    
  private:
    void manager(Manager *_manager) { manager_ = _manager; }
    void id(void *_id) { id_ = _id; }
    void resize(int width, int height) { width_ = width; height_ = height; }
};

typedef std::shared_ptr<Window> WindowPtr;

/// Simple window manager
class DIPVIEWER_EXPORT Manager
{
  friend class Window;

  public:
    virtual ~Manager() { }
  
    virtual void createWindow(WindowPtr window) = 0;
    
    virtual size_t activeWindows() = 0;
    virtual void destroyWindows() = 0;
    
  protected:
    virtual size_t drawString(Window* window, const char *string) = 0;
    virtual void swapBuffers(Window* window) = 0;
    virtual void setWindowTitle(Window* window, const char *name) = 0;
    virtual void refreshWindow(Window *window) = 0;
};

/// \}

}} // namespace dip::viewer

#endif /* DIP_VIEWER_MANAGER_H_ */
