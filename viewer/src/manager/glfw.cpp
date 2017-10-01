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

#ifdef DIP__HAS_GLFW

#include <cstring>
#include <exception>

#include <GLFW/glfw3.h>

#undef DIP__ENABLE_DOCTEST
#include "diplib/viewer/glfw.h"
#include "fg_font_data.h"

#define EPS 0.001

namespace dip { namespace viewer {

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

  glfwInit();
}

GLFWManager::~GLFWManager()
{
  windows_.clear();
  glfwTerminate();

  instance_ = NULL;
}

void GLFWManager::createWindow(WindowPtr window)
{
  int width=window->width(), height=window->height();

  if (width  <= 0) width  = 512;
  if (height <= 0) height = width;

  GLFWwindow *wdw = glfwCreateWindow(width, height, "", NULL, NULL);
  
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
  for (auto it = windows_.begin(); it != windows_.end(); ++it)
    it->second->destroy();
}

void GLFWManager::processEvents()
{
  glfwPollEvents();
  
  refresh_lock_.lock();
  std::set<Window*> redraw = refresh_;
  refresh_.clear();
  refresh_lock_.unlock();
  
  for (auto it = windows_.begin(); it != windows_.end();)
  {
    if (redraw.count(it->second.get()))
    {
      makeCurrent(it->second.get());
      it->second.get()->draw();
    }
  
    if (it->second->shouldClose() || glfwWindowShouldClose((GLFWwindow*)it->first))
    {
      glfwDestroyWindow((GLFWwindow*)it->first);
      it = windows_.erase(it);
      break;
    }
    else
      ++it;
  }
}

WindowPtr GLFWManager::getWindow(GLFWwindow *window)
{
  WindowMap::iterator it = windows_.find((void*)window);
  if (it != windows_.end())
    return it->second;
  else
    return NULL;
}

size_t GLFWManager::drawString(Window* /*window*/, const char *string)
{
  size_t movex = 0;

  for (; *string; ++string, movex += 8)
    bitmapCharacter(*string);
    
  return movex;
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
  refresh_lock_.lock();
  refresh_.insert(window);
  refresh_lock_.unlock();
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

#endif // DIP__HAS_GLFW
