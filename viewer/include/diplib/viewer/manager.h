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

#include <memory>

#include "dipviewer_export.h"

#define KEY_MOD_SHIFT   0x01
#define KEY_MOD_CONTROL 0x02
#define KEY_MOD_ALT     0x04

/// Simple GL window
class DIPVIEWER_EXPORT Window
{
  friend class GLUTManager;
  friend class GLFWManager;

  private:
    void *id_;
    class Manager *manager_;
    bool should_close_;
    
  public:
    Window() : id_(NULL), manager_(NULL), should_close_(false) { }
    virtual ~Window() { }

    void refresh();
    void drawString(const char *string);
  protected:
    Manager *manager() { return manager_; }
    void *id() { return id_; }  
    bool shouldClose() { return should_close_; }
    
    void title(const char *name);
    void swap();
    void destroy() { should_close_ = true; }
    
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
    virtual void drawString(Window* window, const char *string) = 0;
    virtual void swapBuffers(Window* window) = 0;
    virtual void setWindowTitle(Window* window, const char *name) = 0;
    virtual void refreshWindow(Window *window) = 0;
};

#endif /* DIP_VIEWER_MANAGER_H_ */
