/*
 * DIPlib 3.0 viewer
 * This file contains functionality for the status bar.
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

#undef DIP__ENABLE_DOCTEST
#include "diplib/viewer/include_gl.h"
#include "diplib/viewer/status.h"

namespace dip { namespace viewer {

void StatusViewPort::render()
{
  auto &o = viewer()->options();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height_, width_, height_);
  glOrtho(0, width(), height(), 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);

  glColor3f(.5, .5, .5);
  glBegin(GL_LINES);
    glVertex2f(0.f, 0.f);
    glVertex2f((GLfloat)width(), 0.f);
  glEnd();
  
  glColor3f(1., 1., 1.);
  glRasterPos2i(1, 12);
  
  size_t rx = 1;
  
  if (o.status_ != "")
  {
    viewer()->drawString(o.status_.c_str());
  }
  else
  {
    // Describe operating point
    auto op = o.operating_point_;
    auto te = (int)viewer()->image().TensorElements();
    
    rx += viewer()->drawString("(");
    for (dip::uint ii=0; ii < op.size(); ++ii)
    {
      rx += viewer()->drawString(std::to_string(op[ii]).c_str());
      if (ii < op.size()-1)
        rx += viewer()->drawString(", ");
    }
    rx += viewer()->drawString("): ");
    if (te > 1)
      rx += viewer()->drawString("[");
    
    for (int ii=0; ii < te; ++ii)
    {
      if (o.lut_ == ViewingOptions::LookupTable::RGB)
      {
        if (ii == o.color_elements_[0])
          glColor3d(0.9, 0.17, 0.);
        else if (ii == o.color_elements_[1])
          glColor3d(0.0, 0.50, 0.);
        else if (ii == o.color_elements_[2])
          glColor3d(0.1, 0.33, 1.);
        else
          glColor3f(.5, .5, .5);
      }
      else
      {
        if ((size_t)ii == o.element_)
          glColor3f(1., 1., 1.);
        else
          glColor3f(.5, .5, .5);
      }
      
      glRasterPos2i((GLint)rx, 12);
      rx += viewer()->drawString(std::to_string((dip::sfloat)viewer()->image().At(op)[(size_t)ii]).c_str());
        
      glColor3f(1., 1., 1.);
      glRasterPos2i((GLint)rx, 12);
      
      if (ii < te-1)
        rx += viewer()->drawString(", ");
    }
    if (te > 1)
      rx += viewer()->drawString("]");
  }
}

}} // namespace dip::viewer
