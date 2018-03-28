/*
 * DIPlib 3.0 viewer
 * This file contains definitions for rudamentary proxy window manager.
 *
 * (c)2018, Wouter Caarls
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

#ifndef DIP_VIEWER_PROXY_MANAGER_H_
#define DIP_VIEWER_PROXY_MANAGER_H_

#include <map>
#include <mutex>

#include "diplib.h"
#include "diplib/viewer/export.h"
#include "diplib/viewer/manager.h"

/// \file
/// \brief Declares a proxy window manager
/// 
/// Use this to implement your own window manager in different language.
/// Just make sure the right OpenGL context is set when calling the
/// callback functions.

#define DIPVIEWER_EXTERN_EXPORT extern "C" DIPVIEWER_EXPORT

typedef dip::viewer::Window Window;

typedef void (*ProxySwapBuffersCallback)();
typedef void (*ProxySetWindowTitleCallback)(const char *);
typedef void (*ProxyRefreshWindowCallback)();
typedef void (*ProxyCreateWindowCallback)(Window *window);

// Window interaction

DIPVIEWER_EXTERN_EXPORT int proxyGetWidth(Window *window);
DIPVIEWER_EXTERN_EXPORT int proxyGetHeight(Window *window);
DIPVIEWER_EXTERN_EXPORT bool proxyGetDestroyed(Window *window);

// Manager interaction

DIPVIEWER_EXTERN_EXPORT void proxyRelease(Window *window);

// Events, called externally. Set OpenGL context first!

DIPVIEWER_EXTERN_EXPORT void proxyDrawEvent(Window *window);
DIPVIEWER_EXTERN_EXPORT void proxyIdleEvent(Window *window);
DIPVIEWER_EXTERN_EXPORT void proxyReshapeEvent(Window *window, int width, int height);
DIPVIEWER_EXTERN_EXPORT void proxyVisibleEvent(Window *window, int vis);
DIPVIEWER_EXTERN_EXPORT void proxyCreateEvent(Window *window);
DIPVIEWER_EXTERN_EXPORT void proxyCloseEvent(Window *window);
DIPVIEWER_EXTERN_EXPORT void proxyKeyEvent(Window *window, unsigned char k, int x, int y, int mods);
DIPVIEWER_EXTERN_EXPORT void proxyClickEvent(Window *window, int button, int state, int x, int y, int mods);
DIPVIEWER_EXTERN_EXPORT void proxyMotionEvent(Window *window, int x, int y);

// Callbacks, to be called internally

DIPVIEWER_EXTERN_EXPORT void proxySetSwapBuffersCallback(Window *window, ProxySwapBuffersCallback);
DIPVIEWER_EXTERN_EXPORT void proxySetWindowTitleCallback(Window *window, ProxySetWindowTitleCallback);
DIPVIEWER_EXTERN_EXPORT void proxySetRefreshWindowCallback(Window *window, ProxyRefreshWindowCallback);
DIPVIEWER_EXTERN_EXPORT void proxySetCreateWindowCallback(Window *window, ProxyCreateWindowCallback);

namespace dip { namespace viewer {

class DIPVIEWER_CLASS_EXPORT ProxyManager : public Manager
{
  protected:
    std::mutex mutex_;
    std::map<Window *, WindowPtr> windows_;
    
    std::map<Window*, ProxySwapBuffersCallback> swap_buffers_callbacks_;
    std::map<Window*, ProxySetWindowTitleCallback> set_window_title_callbacks_;
    std::map<Window*, ProxyRefreshWindowCallback> refresh_window_callbacks_;
    std::map<Window*, ProxyCreateWindowCallback> create_window_callbacks_;
    
    static ProxyManager instance_;

  public:
    static ProxyManager *instance()
    {
      return &instance_;
    }
    
    // From Manager

    virtual void createWindow(WindowPtr window)
    {
      createWindow(window, true);
    }
  
    virtual void createWindow(WindowPtr window, bool useCallback)
    {
      window->manager(this);
      window->id((void*)window.get());
      
      ProxyCreateWindowCallback cb(nullptr);
      {
        std::lock_guard<std::mutex> guard(mutex_);
        windows_[window.get()] = window;
      
        if (!create_window_callbacks_.empty())
          cb = create_window_callbacks_.begin()->second;
      }
      
      // Set useCallback to false when the creation event should not be
      // passed on to the proxy, for example when the respective window
      // will be created by the caller.
      if (cb && useCallback)
        cb(window.get());
    }
    
    virtual size_t activeWindows()
    {
      std::lock_guard<std::mutex> guard(mutex_);
      return windows_.size();
    }
    
    virtual void destroyWindows()
    {
      std::lock_guard<std::mutex> guard(mutex_);
      for (auto it = windows_.begin(); it != windows_.end(); ++it)
        it->second->destroy();
    }
    
    virtual void processEvents()
    {
    }
    
    // Window interaction

    int proxyGetWidth(Window *window)
    {
      return window->width();
    }

    int proxyGetHeight(Window *window)
    {
      return window->height();
    }
    
    bool proxyGetDestroyed(Window *window)
    {
      return window->destroyed();
    }
    
    // Manager interaction

    void release(Window *window)
    {
      WindowPtr wdw;
      {
        std::lock_guard<std::mutex> guard(mutex_);
        if (windows_.count(window))
        {
          wdw = windows_[window];
          windows_.erase(window);
        }
        swap_buffers_callbacks_.erase(window);
        set_window_title_callbacks_.erase(window);
        refresh_window_callbacks_.erase(window);
        create_window_callbacks_.erase(window);
      }
    }
    
    // Events
    
    void proxyDrawEvent(Window *window)
    {
      window->draw();
    }

    void proxyIdleEvent(Window *window)
    {
      window->idle();
    }

    void proxyReshapeEvent(Window *window, int width, int height)
    {
      window->resize(width, height);
      window->reshape(width, height);
    }

    void proxyVisibleEvent(Window *window, int vis)
    {
      window->visible(vis);
    }

    void proxyCreateEvent(Window *window)
    {
      window->create();
    }

    void proxyCloseEvent(Window *window)
    {
      window->close();
    }

    void proxyKeyEvent(Window *window, unsigned char k, int x, int y, int mods)
    {
      window->key(k, x, y, mods);
    }

    void proxyClickEvent(Window *window, int button, int state, int x, int y, int mods)
    {
      window->click(button, state, x, y, mods);
    }

    void proxyMotionEvent(Window *window, int x, int y)
    {
      window->motion(x, y);
    }
    
    // Callbacks
    
    void setSwapBuffersCallback(Window *window, ProxySwapBuffersCallback cb)
    {
      std::lock_guard<std::mutex> guard(mutex_);
      swap_buffers_callbacks_[window] = cb;
    }

    void setWindowTitleCallback(Window *window, ProxySetWindowTitleCallback cb)
    {
      std::lock_guard<std::mutex> guard(mutex_);
      set_window_title_callbacks_[window] = cb;
    }

    void setRefreshWindowCallback(Window *window, ProxyRefreshWindowCallback cb)
    {
      std::lock_guard<std::mutex> guard(mutex_);
      refresh_window_callbacks_[window] = cb;
    }
    
    void setCreateWindowCallback(Window *window, ProxyCreateWindowCallback cb)
    {
      std::lock_guard<std::mutex> guard(mutex_);
      create_window_callbacks_[window] = cb;
    }
    
  protected:
    virtual void swapBuffers(Window* window)
    {
      ProxySwapBuffersCallback cb;
      {
        std::lock_guard<std::mutex> guard(mutex_);
        if (swap_buffers_callbacks_.count(window))
          cb = swap_buffers_callbacks_[window];
        else
          return;
      }
      cb();
    }
    
    virtual void setWindowTitle(Window* window, const char *name)
    {
      ProxySetWindowTitleCallback cb;
      {
        std::lock_guard<std::mutex> guard(mutex_);
        if (set_window_title_callbacks_.count(window))
          cb = set_window_title_callbacks_[window];
        else
          return;
      }
      cb(name);
    }
    
    virtual void refreshWindow(Window *window)
    {
      ProxyRefreshWindowCallback cb;
      {
        std::lock_guard<std::mutex> guard(mutex_);
        if (refresh_window_callbacks_.count(window))
          cb = refresh_window_callbacks_[window];
        else
          return;
      }
      cb();
    }
};

}} // namespace dip::viewer

#endif /* DIP_VIEWER_PROXY_MANAGER_H_ */
