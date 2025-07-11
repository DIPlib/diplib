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

#ifdef DIP_CONFIG_HAS_GLFW

#include <cstring>

#include <GLFW/glfw3.h>

#include "diplib/viewer/glfw.h"

#if (GLFW_VERSION_MAJOR >= 4) || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3)
  #define GLFW_THROW_IF(x, _str) \
    do { \
      if (x) \
      { \
        std::string str(_str); \
        const char *glfwstr; \
        glfwGetError(&glfwstr); \
        DIP_THROW(str + ": " + glfwstr); \
      } \
    } while (false)
#else
  #define GLFW_THROW_IF(x, str) DIP_THROW_IF(x, str)
#endif

#define EPS 0.001

/// \file
/// \brief Defines the GLFW interface of \ref viewer.

namespace dip { namespace viewer {

GLFWManager *GLFWManager::instance_ = NULL;

GLFWManager::GLFWManager()
{
  Guard guard(mutex_);

  if (instance_)
    throw std::bad_alloc();

  instance_ = this;

#ifdef GLFW_COCOA_CHDIR_RESOURCES
  glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GL_FALSE);
#endif
  GLFW_THROW_IF(glfwInit() != GL_TRUE, "Failed to initialize GLFW");
}

GLFWManager::~GLFWManager()
{
  Guard guard(mutex_);

  destroyWindows();
  windows_.clear();
  glfwTerminate();

  instance_ = NULL;
}

void GLFWManager::createWindow(WindowPtr window)
{
  Guard guard(mutex_);

  int width=window->width(), height=window->height();

  if (width  <= 0) width  = 512;
  if (height <= 0) height = width;

  GLFWwindow *wdw = glfwCreateWindow(width, height, "", NULL, NULL);
  GLFW_THROW_IF(wdw == nullptr, "Failed to create window");

  glfwSetWindowRefreshCallback(wdw, refresh);
  glfwSetFramebufferSizeCallback(wdw, reshape);
  glfwSetWindowIconifyCallback(wdw, iconify);
  glfwSetWindowCloseCallback(wdw, close);
  glfwSetKeyCallback(wdw, key);
  glfwSetMouseButtonCallback(wdw, click);
  glfwSetScrollCallback(wdw, scroll);
  glfwSetCursorPosCallback(wdw, motion);

  window->manager(this);
  window->id((void*)wdw);
  windows_[window->id()] = window;
  window->create();

  glfwGetFramebufferSize(wdw, &width, &height);
  window->resize(width, height);
  window->reshape(width, height);
  window->refresh();
}

void GLFWManager::destroyWindows()
{
  Guard guard(mutex_);

  for (auto it = windows_.begin(); it != windows_.end(); ++it)
    it->second.wdw->destroy();
}

void GLFWManager::processEvents()
{
  Guard guard(mutex_);

  glfwPollEvents();

  for (auto it = windows_.begin(); it != windows_.end();)
  {
    if (it->second.refresh)
    {
      it->second.refresh = false;
      makeCurrent(it->second.wdw.get());
      it->second.wdw->draw();
    }

    if (it->second.wdw->destroyed() || glfwWindowShouldClose((GLFWwindow*)it->first))
    {
      it->second.wdw->destroy();
      glfwDestroyWindow((GLFWwindow*)it->first);
      it = windows_.erase(it);
    }
    else
      ++it;
  }
}

UnsignedArray GLFWManager::screenSize() const
{
  auto const* data = glfwGetVideoMode(glfwGetPrimaryMonitor());
  
  return { static_cast< dip::uint >( std::max( data->width, 0 )),
           static_cast< dip::uint >( std::max( data->height, 0 )) };
}

WindowPtr GLFWManager::getWindow(GLFWwindow *window)
{
  WindowMap::iterator it = windows_.find((void*)window);
  if (it != windows_.end() && !it->second.wdw->destroyed())
    return it->second.wdw;
  else
    return NULL;
}

void GLFWManager::swapBuffers(Window* window)
{
  glfwSwapInterval(0);
  glfwSwapBuffers((GLFWwindow*)window->id());
}

void GLFWManager::setWindowTitle(Window* window, const char *name)
{
  glfwSetWindowTitle((GLFWwindow*)window->id(), name);
}

void GLFWManager::refreshWindow(Window* window)
{
  for (auto it = windows_.begin(); it != windows_.end();++it)
    if (it->second.wdw.get() == window)
      it->second.refresh = true;
}

void GLFWManager::setWindowPosition(Window* window, int x, int y)
{
  Guard guard(mutex_);
  glfwSetWindowPos((GLFWwindow*)window->id(), x, y);
}

void GLFWManager::setWindowSize(Window* window, int width, int height)
{
  Guard guard(mutex_);
  glfwSetWindowSize((GLFWwindow*)window->id(), width, height);
}

void GLFWManager::makeCurrent(Window *window)
{
  glfwMakeContextCurrent((GLFWwindow*)window->id());
}

/// Get cursor position in framebuffer coordinates.
void GLFWManager::getCursorPos(Window *window, int *x, int *y)
{
  int fb_width, fb_height, wdw_width, wdw_height;

  glfwGetWindowSize((GLFWwindow*)window->id(), &wdw_width, &wdw_height);
  glfwGetFramebufferSize((GLFWwindow*)window->id(), &fb_width, &fb_height);

  double wdw_x, wdw_y;
  glfwGetCursorPos((GLFWwindow*)window->id(), &wdw_x, &wdw_y);

  *x = (int)(wdw_x * (double)fb_width/(double)wdw_width);
  *y = (int)(wdw_y * (double)fb_height/(double)wdw_height);
}

}} // namespace dip::viewer

#endif // DIP_CONFIG_HAS_GLFW
