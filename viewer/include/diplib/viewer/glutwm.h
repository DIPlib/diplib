/*
 * DIPlib 3.0 viewer
 * This file contains definitions for a rudamentary GLUT window manager.
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

#ifndef GLUTWM_H_
#define GLUTWM_H_

#include <string.h>

#include <thread>
#include <mutex>
#include <iostream>
#include <map>
#include <memory>

#include "dip_export.h"

namespace glutwm
{

/// Simple GL window
class DIP_EXPORT Window
{
  friend class Manager;

  private:
    int id_;
    class Manager *manager_;
    
  public:
    Window() : id_(-1), manager_(NULL) { }
    virtual ~Window() { }

    void refresh();
    void drawString(const char *string);
  protected:
    Manager *manager() { return manager_; }
    int  id() { return id_; }  
    
    void title(const char *name);
    void swap();
    
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
    virtual void key(unsigned char /*k*/, int /*x*/, int /*y*/) { }
    
    /// Callback that is called when a mouse button is clicked.
    virtual void click(int /*button*/, int /*state*/, int /*x*/, int /*y*/) { }
    
    /// Callback that is called when the mouse is moved while a button is clicked.
    virtual void motion(int /*x*/, int /*y*/) { }
    
  private:
    void manager(Manager *_manager) { manager_ = _manager; }
    void id(int _id) { id_ = _id; }
  
};

typedef std::shared_ptr<Window> WindowPtr;

/// Simple GLUT window manager
class DIP_EXPORT Manager
{
  protected:
    std::thread thread_;
    std::mutex mutex_;
    bool continue_;
  
    typedef std::map<int, WindowPtr> WindowMap;
    WindowMap windows_;
    
    WindowPtr new_window_, destroyed_window_;
    
    static Manager *instance_;
    
  public:
    Manager();
    ~Manager();
  
    void createWindow(WindowPtr window);
    void destroyWindow(WindowPtr window);
    void refreshWindow(WindowPtr window);
    size_t activeWindows() { return windows_.size(); }
    
  protected:    
    void run();
    WindowPtr getCurrentWindow();
    void destroyWindow(WindowPtr window, bool glutDestroy);
    
    // Delegates
    static void idle()
    {
      for (WindowMap::iterator it=instance_->windows_.begin(); it != instance_->windows_.end(); ++it)
        it->second->idle();
    }
    
    static void draw()
    {
      WindowPtr window = instance_->getCurrentWindow();
      if (window)
        window->draw();
    }
    
    static void reshape(int width, int height) 
    {
      WindowPtr window = instance_->getCurrentWindow();
      if (window)
        window->reshape(width, height);
    }
    
    static void visible(int vis)
    {
      WindowPtr window = instance_->getCurrentWindow();
      if (window)
        window->visible(vis);
    }
    
    static void close()
    {
      WindowPtr window = instance_->getCurrentWindow();
      if (window)
        instance_->destroyWindow(window, false);
    }

    static void key(unsigned char k, int x, int y)
    {
      WindowPtr window = instance_->getCurrentWindow();
      if (window)
        window->key(k, x, y);
    }
    
    static void click(int button, int state, int x, int y)
    {
      WindowPtr window = instance_->getCurrentWindow();
      if (window)
        window->click(button, state, x, y);
    }

    static void motion(int x, int y)
    {
      WindowPtr window = instance_->getCurrentWindow();
      if (window)
        window->motion(x, y);
    }
};

}

#endif /* GLUTWM_H_ */
