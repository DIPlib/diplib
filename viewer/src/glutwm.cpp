/*
 * DIPlib 3.0 viewer
 * This file contains functionality for a redamentary GLUT window manager.
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

#include <unistd.h>
#include <exception>

#include <GL/freeglut.h>

#include "diplib/viewer/glutwm.h"

#define EPS 0.001

using namespace glutwm;

Manager *Manager::instance_ = NULL;

void Window::title(const char *name)
{
  glutSetWindowTitle(name);
}

void Window::swap()
{
  glutSwapBuffers();
}

void Window::drawString(const char *string)
{
  for (; *string; ++string)
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *string);
}

void Window::refresh()
{
  glutPostWindowRedisplay(id());
}

Manager::Manager()
{
  if (instance_)
    throw std::bad_alloc();

  instance_ = this;
  continue_ = true;
  
  mutex_.lock();
  thread_ = std::thread(&Manager::run, this);
}

Manager::~Manager()
{
  if (continue_)
  {
    continue_ = false;
    thread_.join();
  }
  
  instance_ = NULL;
}

void Manager::createWindow(WindowPtr window)
{
  mutex_.lock();
  
  new_window_ = window;
  
  while (new_window_)
  {
    mutex_.unlock();
    usleep(0);
    mutex_.lock();
  }
  
  mutex_.unlock();
}
    
void Manager::destroyWindow(WindowPtr window)
{
  mutex_.lock();
  destroyWindow(window, true);
  mutex_.unlock();
}

void Manager::refreshWindow(WindowPtr window)
{
  glutPostWindowRedisplay(window->id());
}

void Manager::run()
{
  int argc = 1;
  char argv1[256], *argv[]={argv1};
      
  strncpy(argv1, "Manager", 256);
  
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
  glutIdleFunc(idle);
  
  mutex_.unlock();
      
  while (continue_)
  {
    mutex_.lock();

    glutMainLoopEvent();
    
    if (new_window_)
    {
      glutCreateWindow("");
      glutReshapeWindow(512, 512);
      glutPositionWindow(((glutGetWindow()-1)%2)*512, (((int)(glutGetWindow()-1))/2)*512);
     
      glutDisplayFunc(draw);
      glutReshapeFunc(reshape);
      glutVisibilityFunc(visible);  
      glutCloseFunc(close);
      glutKeyboardFunc(key);
      glutMouseFunc(click);
      glutMotionFunc(motion);
    
      new_window_->id(glutGetWindow());
      windows_[new_window_->id()] = new_window_;
      new_window_->create();

      new_window_ = NULL;
    }
    
    if (destroyed_window_)
    {
      glutDestroyWindow(destroyed_window_->id());
      destroyed_window_ = NULL;
    }
    
    idle();
    
    mutex_.unlock();
    usleep(1000);
  }
  
  windows_.clear();
  
  glutExit();
}
    
WindowPtr Manager::getCurrentWindow()
{
  WindowMap::iterator it = windows_.find(glutGetWindow());
  if (it != windows_.end())
    return it->second;
  else
    return NULL;
}

// Must be called under lock
void Manager::destroyWindow(WindowPtr window, bool glutDestroy)
{
  windows_.erase(window->id());
  
  if (glutDestroy)
  {
    destroyed_window_ = window;
    mutex_.unlock();
    while (destroyed_window_)
      usleep(0);
    mutex_.lock();
  }
}
