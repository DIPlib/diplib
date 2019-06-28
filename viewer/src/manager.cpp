/*
 * DIPlib 3.0 viewer
 * This file contains functionality for a rudimentary window manager.
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

#include "diplib/viewer/manager.h"

#include "fg_font_data.h"

/// \file
/// \brief Defines `dip::viewer::Manager`.

namespace dip { namespace viewer {

void Window::title(const char *name)
{
  manager()->setWindowTitle(this, name);
}

void Window::swap()
{
  manager()->swapBuffers(this);
}

size_t Window::drawString(const char *string)
{
  size_t movex = 0;

  for (; *string; ++string, movex += 8)
    bitmapCharacter(*string);
    
  return movex;
}

void Window::refresh()
{
  manager()->refreshWindow(this);
}

void Window::key(unsigned char k, int /*x*/, int /*y*/, int mods)
{
  if (k == 'W')
  {
    if (mods == KEY_MOD_CONTROL)
      destroy();
    else if (mods == (KEY_MOD_CONTROL | KEY_MOD_SHIFT))
      manager()->destroyWindows();
  }
}

}} // namespace dip::viewer
