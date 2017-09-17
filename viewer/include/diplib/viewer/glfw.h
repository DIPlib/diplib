/*
 * DIPlib 3.0 viewer
 * This file contains definitions for a rudamentary GLFW window manager.
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

#include <thread>
#include <mutex>
#include <map>

#include "manager.h"

/// Simple GLFW window manager
class DIPVIEWER_EXPORT GLFWManager : public Manager
{
  protected:
    bool refresh_;
  
    typedef std::map<void *, WindowPtr> WindowMap;
    WindowMap windows_;
    
    static GLFWManager *instance_;
    
  public:
    GLFWManager();
    ~GLFWManager();
  
    void createWindow(WindowPtr window);
    size_t activeWindows() { return windows_.size(); }
    void destroyWindows();
    void processEvents();
    
  protected:    
    void drawString(Window* window, const char *string);
    void swapBuffers(Window* window);
    void setWindowTitle(Window* window, const char *name);
    void refreshWindow(Window* window);

    void run();
    WindowPtr getWindow(struct GLFWwindow *window);
    void getCursorPos(Window *window, int *x, int *y);
    void makeCurrent(Window *window);

    // Delegates
    static void refresh(struct GLFWwindow */*window*/)
    {
      instance_->refresh_ = true;
    }
    
    static void reshape(struct GLFWwindow *window, int width, int height)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        instance_->makeCurrent(wdw.get());
        wdw->resize(width, height);
        wdw->reshape(width, height);
      }
    }
    
    static void iconify(struct GLFWwindow *window, int iconified)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        instance_->makeCurrent(wdw.get());
        wdw->visible(!iconified);
      }
    }
    
    static void close(struct GLFWwindow *window)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        instance_->makeCurrent(wdw.get());
        wdw->close();
      }
    }

    static void key(struct GLFWwindow *window, int key, int /*scancode*/, int action, int mods)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw && action > 0)
      {
        int x, y;
        instance_->makeCurrent(wdw.get());
        instance_->getCursorPos(wdw.get(), &x, &y);
        wdw->key((unsigned char)key, x, y, mods);
      }
    }
    
    static void click(struct GLFWwindow *window, int button, int state, int /*mods*/)
    {
      WindowPtr wdw = instance_->getWindow(window);
      if (wdw)
      {
        int x, y;
        instance_->makeCurrent(wdw.get());
        instance_->getCursorPos(wdw.get(), &x, &y);
        wdw->click(button==1?2:button==2?1:0, state==0, x, y);
      }
    }

    static void scroll(struct GLFWwindow *window, double /*xoffset*/, double yoffset)
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
          wdw->click(button, 1, x, y);
          wdw->click(button, 0, x, y);
        }
      }
    }

    static void motion(struct GLFWwindow *window, double /*x*/, double /*y*/)
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

#endif /* DIP_VIEWER_GLFW_H_ */
