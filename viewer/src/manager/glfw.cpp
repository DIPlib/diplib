/*
 * DIPlib 3.0 viewer
 * This file contains functionality for a rudamentary GLFW window manager.
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
#include <string.h>

#include <exception>

#include <GLFW/glfw3.h>

#include "diplib/viewer/glfw.h"
#include "fg_font_data.h"

#define EPS 0.001

/*
 * This function is 
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 16 1999
 * 
 * See fg_font_data.h for license information.
 */
void bitmapCharacter( int character )
{
    const GLubyte* face;
    const SFG_Font* font = &fgFontFixed8x13;

    /*
     * Find the character we want to draw (???)
     */
    face = font->Characters[ character ];

    glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );
    glPixelStorei( GL_UNPACK_SWAP_BYTES,  GL_FALSE );
    glPixelStorei( GL_UNPACK_LSB_FIRST,   GL_FALSE );
    glPixelStorei( GL_UNPACK_ROW_LENGTH,  0        );
    glPixelStorei( GL_UNPACK_SKIP_ROWS,   0        );
    glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0        );
    glPixelStorei( GL_UNPACK_ALIGNMENT,   1        );
    glBitmap(
        face[ 0 ], font->Height,      /* The bitmap's width and height  */
        font->xorig, font->yorig,     /* The origin in the font glyph   */
        ( float )( face[ 0 ] ), 0.0,  /* The raster advance -- inc. x,y */
        ( face + 1 )                  /* The packed bitmap data...      */
    );
    glPopClientAttrib( );
}

GLFWManager *GLFWManager::instance_ = NULL;

GLFWManager::GLFWManager()
{
  if (instance_)
    throw std::bad_alloc();

  instance_ = this;
  continue_ = true;
  refresh_ = true;
  
  mutex_.lock();
  thread_ = std::thread(&GLFWManager::run, this);
}

GLFWManager::~GLFWManager()
{
  if (continue_)
  {
    continue_ = false;
    thread_.join();
  }
  
  instance_ = NULL;
}

void GLFWManager::createWindow(WindowPtr window)
{
  mutex_.lock();
  
  new_window_ = window;
  
  while (new_window_)
  {
    mutex_.unlock();
    usleep(1000);
    mutex_.lock();
  }
  
  mutex_.unlock();
}
    
void GLFWManager::destroyWindow(WindowPtr window)
{
  mutex_.lock();
  destroyWindow(window, true);
  mutex_.unlock();
}

void GLFWManager::refreshWindow(WindowPtr /*window*/)
{
  refresh_ = true;
}

void GLFWManager::run()
{
  glfwInit();
  
  mutex_.unlock();
      
  while (continue_)
  {
    mutex_.lock();

    glfwPollEvents();
    
    if (new_window_)
    {
      GLFWwindow *window = glfwCreateWindow(512, 512, "", NULL, NULL);
      
      glfwSetWindowRefreshCallback(window, refresh);
      glfwSetFramebufferSizeCallback(window, reshape);
      glfwSetWindowIconifyCallback(window, iconify);
      glfwSetWindowCloseCallback(window, close);
      glfwSetCharCallback(window, key);
      glfwSetMouseButtonCallback(window, click);
      glfwSetScrollCallback(window, scroll);
      glfwSetCursorPosCallback(window, motion);
    
      new_window_->manager(this);
      new_window_->id((void*)window);
      windows_[new_window_->id()] = new_window_;
      new_window_->create();
      new_window_->reshape(512, 512);
      refresh_ = true;
      
      new_window_ = NULL;
    }
    
    if (destroyed_window_)
    {
      glfwDestroyWindow((GLFWwindow*)destroyed_window_->id());
      destroyed_window_ = NULL;
    }
    
    if (refresh_)
    {
      refresh_ = false;
      
      for (auto e: windows_)
      {
        makeCurrent(e.second.get());
        e.second->draw();
      }
    }
    
    mutex_.unlock();
    usleep(1000);
  }
  
  windows_.clear();
  
  glfwTerminate();
}

WindowPtr GLFWManager::getWindow(GLFWwindow *window)
{
  WindowMap::iterator it = windows_.find((void*)window);
  if (it != windows_.end())
    return it->second;
  else
    return NULL;
}

// Must be called under lock
void GLFWManager::destroyWindow(WindowPtr window, bool external)
{
  windows_.erase(window->id());
  destroyed_window_ = window;
  
  if (external)
  {
    mutex_.unlock();
    while (destroyed_window_)
      usleep(1000);
    mutex_.lock();
  }
}

void GLFWManager::drawString(Window* /*window*/, const char *string)
{
  for (; *string; ++string)
    bitmapCharacter(*string);
}

void GLFWManager::swapBuffers(Window* window)
{
  glfwSwapBuffers((GLFWwindow*)window->id());
}

void GLFWManager::setWindowTitle(Window* window, const char *name)
{
  glfwSetWindowTitle((GLFWwindow*)window->id(), name);
}

void GLFWManager::makeCurrent(Window *window)
{
  glfwMakeContextCurrent((GLFWwindow*)window->id());
}

void GLFWManager::getCursorPos(Window *window, double *x, double *y)
{
  glfwGetCursorPos((GLFWwindow*)window->id(), x, y);
}
