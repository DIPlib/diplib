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
  refresh_ = true;

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
  GLFWwindow *wdw = glfwCreateWindow(512, 512, "", NULL, NULL);
  
  glfwSetWindowRefreshCallback(wdw, refresh);
  glfwSetFramebufferSizeCallback(wdw, reshape);
  glfwSetWindowIconifyCallback(wdw, iconify);
  glfwSetWindowCloseCallback(wdw, close);
  glfwSetCharCallback(wdw, key);
  glfwSetMouseButtonCallback(wdw, click);
  glfwSetScrollCallback(wdw, scroll);
  glfwSetCursorPosCallback(wdw, motion);

  window->manager(this);
  window->id((void*)wdw);
  windows_[window->id()] = window;
  window->create();
  
  int width, height;
  glfwGetFramebufferSize(wdw, &width, &height);
  window->reshape(width, height);
  
  refresh_ = true;
}
    
void GLFWManager::destroyWindow(WindowPtr window)
{
  glfwSetWindowShouldClose((GLFWwindow*)window->id(), 1);
}

void GLFWManager::refreshWindow(WindowPtr /*window*/)
{
  refresh_ = true;
}

void GLFWManager::processEvents()
{
  glfwPollEvents();
  
  if (refresh_)
  {
    refresh_ = false;
    
    for (auto e: windows_)
    {
      makeCurrent(e.second.get());
      e.second->draw();
    }
  }
      
  for (auto it = windows_.begin(); it != windows_.end();)
  {
    if (glfwWindowShouldClose((GLFWwindow*)it->first))
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

#endif // DIP__HAS_GLFW
