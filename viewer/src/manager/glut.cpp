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

#ifdef DIP_CONFIG_HAS_FREEGLUT

#include <cstring>
#include <cstdint>
#include <exception>

#include <GL/freeglut.h>

#include "diplib/viewer/glut.h"

#define EPS 0.001

/// \file
/// \brief Defines the GLUT interface of \ref viewer.

namespace dip { namespace viewer {

GLUTManager *GLUTManager::instance_ = NULL;

GLUTManager::GLUTManager()
{
  Guard guard(mutex_);

  if (instance_)
    throw std::bad_alloc();

  instance_ = this;
  continue_ = true;
  active_ = false;

  thread_ = std::thread(&GLUTManager::run, this);
}

GLUTManager::~GLUTManager()
{
  if (continue_)
  {
    continue_ = false;
    thread_.join();
  }

  instance_ = NULL;
}

void GLUTManager::createWindow(WindowPtr window)
{
  while (1)
  {
    mutex_.lock();
    if (!new_window_)
      break;
    mutex_.unlock();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }

  new_window_ = window;

  // If called from event handler, don't wait for window to be created.
  // Note that this means only one window can be created per
  // glutMainLoopEvent() call.
  if (active_)
  {
    mutex_.unlock();
    return;
  }

  mutex_.unlock();

  while (new_window_)
    std::this_thread::sleep_for(std::chrono::microseconds(100));
}

void GLUTManager::destroyWindows()
{
  Guard guard(mutex_);

  for (auto it = windows_.begin(); it != windows_.end(); ++it)
    it->second->destroy();
}

UnsignedArray GLUTManager::screenSize() const
{
  return { static_cast< dip::uint >( std::max( glutGet(GLUT_SCREEN_WIDTH), 0 )),
           static_cast< dip::uint >( std::max( glutGet(GLUT_SCREEN_HEIGHT), 0 )) };
}

void GLUTManager::run()
{
  int argc = 1;
  char argv1[256], *argv[]={argv1};

  std::strncpy(argv1, "GLUTManager", 256);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
  glutIdleFunc(idle);

  while (continue_)
  {
    mutex_.lock();

    active_ = true;

    glutMainLoopEvent();

    if (new_window_)
    {
      int width=new_window_->width(), height=new_window_->height();

      if (width  <= 0) width  = 512;
      if (height <= 0) height = width;

      glutCreateWindow("");
      glutReshapeWindow(width, height);
      int id = (int)(glutGetWindow() - 1);
      glutPositionWindow((id%2)*512+(id%16)/4*16, ((id%4)/2)*512+(id % 16) / 4 * 16);

      glutDisplayFunc(draw);
      glutReshapeFunc(reshape);
      glutVisibilityFunc(visible);
      glutCloseFunc(close);
      glutKeyboardFunc(key);
      glutMouseFunc(click);
      glutMotionFunc(motion);

      new_window_->manager(this);
      new_window_->id((void*)(intptr_t)glutGetWindow());
      windows_[new_window_->id()] = new_window_;
      new_window_->create();

      new_window_ = NULL;
    }

    for (auto it = windows_.begin(); it != windows_.end();)
    {
      if (it->second->destroyed())
      {
        glutDestroyWindow((int)(intptr_t)it->first);
        it = windows_.erase(it);
        break;
      }
      else
        ++it;
    }

    idle();

    active_ = false;

    mutex_.unlock();
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }

  Guard guard(mutex_);

  destroyWindows();
  windows_.clear();

  glutExit();
}

WindowPtr GLUTManager::getCurrentWindow()
{
  WindowMap::iterator it = windows_.find((void*)(intptr_t)glutGetWindow());
  if (it != windows_.end() && !it->second->destroyed())
    return it->second;
  else
    return NULL;
}

void GLUTManager::swapBuffers(Window* /*window*/)
{
  glutSwapBuffers();
}

void GLUTManager::setWindowTitle(Window* /*window*/, const char *name)
{
  glutSetWindowTitle(name);
}

void GLUTManager::refreshWindow(Window *window)
{
  glutPostWindowRedisplay((int)(intptr_t)window->id());
}

void GLUTManager::setWindowPosition(Window* window, int x, int y)
{
  mutex_.lock();
  int id = glutGetWindow();
  glutSetWindow((int)(intptr_t)window->id());
  glutPositionWindow(x, y);
  glutSetWindow(id);
  mutex_.unlock();
}

void GLUTManager::setWindowSize(Window* window, int width, int height)
{
  mutex_.lock();
  int id = glutGetWindow();
  glutSetWindow((int)(intptr_t)window->id());
  glutReshapeWindow(width, height);
  glutSetWindow(id);
  mutex_.unlock();
}

void GLUTManager::key(unsigned char k, int x, int y)
{
  WindowPtr window = instance_->getCurrentWindow();
  if (window)
  {
    if (k > 0 && k < 27)
    {
      k = (unsigned char)(k + 'A' - 1);
    }
    else if (k >= 'a' && k <= 'z')
    {
      k = (unsigned char)(k - 'a' + 'A');
    }

    window->key(k, x, y, glutGetModifiers());
  }
}

void GLUTManager::click(int button, int state, int x, int y)
{
  WindowPtr window = instance_->getCurrentWindow();
  if (window)
    window->click(button, state, x, y, glutGetModifiers());
}

}} // namespace dip::viewer

#endif // DIP_CONFIG_HAS_FREEGLUT
