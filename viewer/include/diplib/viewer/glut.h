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

#ifndef DIP_VIEWER_GLUT_H_
#define DIP_VIEWER_GLUT_H_

#include "diplib/viewer/manager.h"

#include <thread>
#include <mutex>
#include <map>

/// \file
/// \brief Declares the GLUT interface of \ref viewer.

namespace dip { namespace viewer {

/// \addtogroup viewer
/// \{

/// Simple GLUT window manager
class DIPVIEWER_EXPORT GLUTManager : public Manager
{
  protected:
    std::thread thread_;
    std::mutex mutex_;
    bool continue_;
  
    typedef std::map<void*, WindowPtr> WindowMap;
    WindowMap windows_;
    
    WindowPtr new_window_, destroyed_window_;
    
    static GLUTManager *instance_;
    
  public:
    GLUTManager();
    ~GLUTManager();
  
    void createWindow(WindowPtr window);
    size_t activeWindows() { return windows_.size(); }
    void destroyWindows();
    void processEvents() { }

  protected:    
    void drawString(Window* window, const char *string);
    void swapBuffers(Window* window);
    void setWindowTitle(Window* window, const char *name);
    void refreshWindow(Window* window);

    void run();
    WindowPtr getCurrentWindow();
    
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
      {
        window->resize(width, height);
        window->reshape(width, height);
      }
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
      {
        window->close();
        instance_->windows_.erase(window->id());
      }
    }

    static void key(unsigned char k, int x, int y);
    
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

/// \}

}} // namespace dip::viewer

#endif /* DIP_VIEWER_GLUT_H_ */
