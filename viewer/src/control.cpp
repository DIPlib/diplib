/*
 * DIPlib 3.0 viewer
 * This file contains functionality for controlling the display options.
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

#include "diplib/viewer/include_gl.h"
#include "diplib/viewer/control.h"

using namespace dip;

void ControlViewPort::render()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height_, width_, height_);
  glOrtho(0, width(), height(), 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);

  glColor3f(1., 1., 1.);
  for (size_t ii=0; ii < lists_.size(); ++ii)
  {
    int selected = -1;
    switch (ii)
    {
      case 0:
        selected = (int)viewer()->options().lut_;
        break;
      case 1:
        selected = (int)viewer()->options().mapping_;
        break;
      case 2:
        selected = (int)viewer()->options().complex_;
        break;
      case 3:
        selected = (int)viewer()->options().projection_;
        break;
    }
  
    auto list = lists_[ii];
    for (size_t jj=0; jj < list.size(); ++jj)
    {
      if ((int)jj == selected)
        glColor3f(1., 1., 0.);
      else
        glColor3f(1., 1., 1.);
      
      glRasterPos2i((GLint)ii*width()/(GLint)lists_.size(), (GLint)(jj+1)*13);
      viewer()->drawString(list[jj].c_str());
    }
  }
}

void ControlViewPort::click(int button, int state, int x, int y)
{
  if (state == 0 && button == 0)
  {
    auto &o = viewer()->options();
    double ix, iy;
    screenToView(x, y, &ix, &iy);
    
    int list = (x-x_)*(int)lists_.size()/width();
    int opt  = (y-y_)/13;
    
    if (list < 0 || list >= (int)lists_.size())
      return;
    if (opt < 0 || opt >= (int)lists_[(dip::uint)list].size())
      return;
    
    switch (list)
    {
      case 0:
        o.lut_ = (ViewingOptions::LookupTable) opt;
        break;
      case 1:
        o.mapping_ = (ViewingOptions::Mapping) opt;
        switch (o.mapping_)
        {
          case ViewingOptions::Mapping::ZeroOne:
            o.mapping_range_ = {0, 1};
            break;
          case ViewingOptions::Mapping::Normal:
            o.mapping_range_ = {0, 255};
            break;
          case ViewingOptions::Mapping::Linear:
            o.mapping_range_ = o.range_;
            break;
          case ViewingOptions::Mapping::Symmetric:
            o.mapping_range_ = o.range_;
            if (std::abs(o.mapping_range_.first) > std::abs(o.mapping_range_.second))
              o.mapping_range_.second = -o.mapping_range_.first;
            else
              o.mapping_range_.first = -o.mapping_range_.second;
            break;
          case ViewingOptions::Mapping::Logarithmic:
            o.mapping_range_ = o.range_;
            break;
        }
        break;
      case 2:
        o.complex_ = (ViewingOptions::ComplexToReal) opt;
        break;
      case 3:
        o.projection_ = (ViewingOptions::Projection) opt;
        break;
    }
  }
  
  viewer()->refresh();
}

