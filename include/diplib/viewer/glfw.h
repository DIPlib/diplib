/*
 * DIPlib 3.0 viewer
 * This file contains definitions for a rudimentary GLFW window manager.
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

#ifndef DIP_VIEWER_GLFW_H_
#define DIP_VIEWER_GLFW_H_

#include "diplib/viewer/manager.h"

struct GLFWwindow;

#include <thread>
#include <mutex>
#include <map>
#include <set>

/// \file
/// \brief Declares the GLFW interface of \ref viewer.

namespace dip { namespace viewer {

/// \addtogroup viewer
/// \{

/// Simple GLFW window manager.
class DIPVIEWER_CLASS_EXPORT GLFWManager : public Manager
{
  protected:
    struct WindowInfo
    {
      WindowPtr wdw;
      bool refresh;
      
      WindowInfo(WindowPtr _wdw=WindowPtr(), bool _refresh=false) : wdw(_wdw), refresh(_refresh) { }
    };
  
    typedef std::map<void *, WindowInfo> WindowMap;
    typedef std::lock_guard<std::recursive_mutex> Guard;
    
  protected:
    std::recursive_mutex mutex_;

    WindowMap windows_;

    static GLFWManager *instance_;
    
  public:
    DIPVIEWER_EXPORT GLFWManager();
    DIPVIEWER_EXPORT ~GLFWManager() override;
  
    DIPVIEWER_EXPORT void createWindow(WindowPtr window) override;
    size_t activeWindows() override { Guard guard(mutex_); return windows_.size(); }
    DIPVIEWER_EXPORT void destroyWindows() override;
    DIPVIEWER_EXPORT void processEvents() override;
    
  protected:    
    DIPVIEWER_EXPORT void swapBuffers(Window* window) override;
    DIPVIEWER_EXPORT void setWindowTitle(Window* window, const char *name) override;
    DIPVIEWER_EXPORT void refreshWindow(Window* window) override;
    DIPVIEWER_EXPORT void setWindowPosition(Window* window, int x, int y) override;
    DIPVIEWER_EXPORT void setWindowSize(Window* window, int x, int y) override;

    DIPVIEWER_EXPORT void run();
    DIPVIEWER_EXPORT WindowPtr getWindow(GLFWwindow *window);
    DIPVIEWER_EXPORT void getCursorPos(Window *window, int *x, int *y);
    DIPVIEWER_EXPORT void makeCurrent(Window *window);

    // Delegates
    static void refresh(GLFWwindow *window)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        instance_->makeCurrent(wdw.get());
        wdw->draw();
      }
    }
    
    static void reshape(GLFWwindow *window, int width, int height)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        instance_->makeCurrent(wdw.get());
        wdw->resize(width, height);
        wdw->reshape(width, height);
      }
    }
    
    static void iconify(GLFWwindow *window, int iconified)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        instance_->makeCurrent(wdw.get());
        wdw->visible(!iconified);
      }
    }
    
    static void close(GLFWwindow *window)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        instance_->makeCurrent(wdw.get());
        wdw->close();
      }
    }

    static void key(GLFWwindow *window, int key, int /*scancode*/, int action, int mods)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw && action > 0 && key < 128)
      {
        int x, y;
        instance_->makeCurrent(wdw.get());
        instance_->getCursorPos(wdw.get(), &x, &y);
        wdw->key((unsigned char)key, x, y, mods);
      }
    }
    
    static void click(GLFWwindow *window, int button, int state, int mods)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        int x, y;
        instance_->makeCurrent(wdw.get());
        instance_->getCursorPos(wdw.get(), &x, &y);
        wdw->click(button==1?2:button==2?1:0, state==0, x, y, mods);
      }
    }

    static void scroll(GLFWwindow *window, double /*xoffset*/, double yoffset)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        int x, y;
        instance_->makeCurrent(wdw.get());
        instance_->getCursorPos(wdw.get(), &x, &y);
                
        int button = 3 + (yoffset < 0);
        if (yoffset != 0)
        {
          wdw->click(button, 1, x, y, 0);
          wdw->click(button, 0, x, y, 0);
        }
      }
    }

    static void motion(GLFWwindow *window, double /*x*/, double /*y*/)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        int x, y;
        instance_->makeCurrent(wdw.get());
        instance_->getCursorPos(wdw.get(), &x, &y);
        wdw->motion(x, y);
      }
    }
};

/// \}

}} // namespace dip::viewer

#endif /* DIP_VIEWER_GLFW_H_ */
