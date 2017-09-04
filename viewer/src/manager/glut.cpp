/*
 * DIPlib 3.0 viewer
 * This file contains functionality for a rudamentary GLUT window manager.
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

#ifdef DIP__HAS_FREEGLUT

#include <unistd.h>
#include <string.h>

#include <cstdint>
#include <exception>

#include <GL/freeglut.h>

#include "diplib/viewer/glut.h"

#define EPS 0.001

GLUTManager *GLUTManager::instance_ = NULL;

GLUTManager::GLUTManager()
{
  if (instance_)
    throw std::bad_alloc();

  instance_ = this;
  continue_ = true;
  
  mutex_.lock();
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
  mutex_.lock();
  
  new_window_ = window;
  
  while (new_window_)
  {
    mutex_.unlock();
    std::this_thread::sleep_for(std::chrono::microseconds(0));
    mutex_.lock();
  }
  
  mutex_.unlock();
}
    
void GLUTManager::destroyWindow(WindowPtr window)
{
  mutex_.lock();
  destroyWindow(window, true);
  mutex_.unlock();
}

void GLUTManager::refreshWindow(WindowPtr window)
{
  glutPostWindowRedisplay((int)(intptr_t)window->id());
}

void GLUTManager::run()
{
  int argc = 1;
  char argv1[256], *argv[]={argv1};
      
  strncpy(argv1, "GLUTManager", 256);
  
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
    
      new_window_->manager(this);
      new_window_->id((void*)(intptr_t)glutGetWindow());
      windows_[new_window_->id()] = new_window_;
      new_window_->create();

      new_window_ = NULL;
    }
    
    if (destroyed_window_)
    {
      glutDestroyWindow((int)(intptr_t)destroyed_window_->id());
      destroyed_window_ = NULL;
    }
    
    idle();
    
    mutex_.unlock();
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }
  
  windows_.clear();
  
  glutExit();
}
    
WindowPtr GLUTManager::getCurrentWindow()
{
  WindowMap::iterator it = windows_.find((void*)(intptr_t)glutGetWindow());
  if (it != windows_.end())
    return it->second;
  else
    return NULL;
}

// Must be called under lock
void GLUTManager::destroyWindow(WindowPtr window, bool glutDestroy)
{
  windows_.erase(window->id());
  
  if (glutDestroy)
  {
    destroyed_window_ = window;
    mutex_.unlock();
    while (destroyed_window_)
      std::this_thread::sleep_for(std::chrono::microseconds(0));
    mutex_.lock();
  }
}

void GLUTManager::drawString(Window* /*window*/, const char *string)
{
  for (; *string; ++string)
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *string);
}

void GLUTManager::swapBuffers(Window* /*window*/)
{
  glutSwapBuffers();
}

void GLUTManager::setWindowTitle(Window* /*window*/, const char *name)
{
  glutSetWindowTitle(name);
}

#endif // DIP__HAS_FREEGLUT
