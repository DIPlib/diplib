/*
 * DIPlib 3.0 viewer
 * This file contains functionality for displaying the tensor elements.
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
#include "diplib/viewer/tensor.h"

namespace dip { namespace viewer {

void TensorViewPort::render()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height_, width_, height_);
  glOrtho(0, width(), height(), 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);

  auto &o = viewer()->options();
  Image &image = viewer()->image();
  
  Tensor t = image.Tensor();
  auto p = image.At<dip::sfloat>(o.operating_point_);
  std::vector<dip::sint> lut = t.LookUpTable();
  
  for (size_t jj=0; jj < t.Rows(); ++jj)
    for (size_t ii=0; ii < t.Columns(); ++ii)
    {
      dip::sint idx = lut[jj*t.Columns()+ii];
      if (idx != -1)
      {
        dip::sfloat val = p[(dip::uint)idx];
        dip::uint8 rv = rangeMap(val, o);
        GLfloat cwidth = (GLfloat)width()/(GLfloat)t.Columns(),
                cheight = (GLfloat)height()/(GLfloat)t.Rows();
        
        // Tensor element value
        glColor3ub(rv, rv, rv);
        glBegin(GL_QUADS);
          glVertex2f((GLfloat)ii*cwidth+1.f,     (GLfloat)jj*cheight);
          glVertex2f((GLfloat)(ii+1)*cwidth-1.f, (GLfloat)jj*cheight);
          glVertex2f((GLfloat)(ii+1)*cwidth-1.f, (GLfloat)(jj+1)*cheight-1.f);
          glVertex2f((GLfloat)ii*cwidth+1.f,     (GLfloat)(jj+1)*cheight-1.f);
        glEnd();
        
        // Draw box around selected element(s)
        if (o.lut_ == ViewingOptions::LookupTable::RGB)
        {
          if (idx == o.color_elements_[0])
            glColor3f(1., 0., 0.);
          else if (idx == o.color_elements_[1])
            glColor3f(0., 1., 0.);
          else if (idx == o.color_elements_[2])
            glColor3f(0., 0., 1.);
          else
            glColor3f(0., 0., 0.);
        }
        else if (idx == (dip::sint)o.element_)
          glColor3f(1., 1., 1.);
        else
          glColor3f(0., 0., 0.);
        
        glBegin(GL_LINE_LOOP);
          glVertex2f((GLfloat)ii*cwidth+1.f,     (GLfloat)jj*cheight);
          glVertex2f((GLfloat)(ii+1)*cwidth-1.f, (GLfloat)jj*cheight);
          glVertex2f((GLfloat)(ii+1)*cwidth-1.f, (GLfloat)(jj+1)*cheight-1.f);
          glVertex2f((GLfloat)ii*cwidth+1.f,     (GLfloat)(jj+1)*cheight-1.f);
        glEnd();
      }
    }
}

void TensorViewPort::click(int button, int state, int x, int y)
{
  if (state == 0 && button == 0)
  {
    auto &o = viewer()->options();
    double ix, iy;
    screenToView(x, y, &ix, &iy);
    
    // Find clicked element
    Tensor t = viewer()->image().Tensor();
    std::vector<dip::sint> lut = t.LookUpTable();
    dip::sint row=y*(dip::sint)t.Rows()/(dip::sint)height();
    dip::sint col=x*(dip::sint)t.Columns()/(dip::sint)width();
    
    if (row < 0 || row >= (dip::sint)t.Rows() ||
        col < 0 || col >= (dip::sint)t.Columns())
      return;
    
    dip::sint idx = lut[(dip::uint)row*t.Columns()+(dip::uint)col];
    
    if (idx != -1)
    {
      if (o.lut_ == ViewingOptions::LookupTable::RGB)
      {
        // Select and deselect tensor elements to visualize. Clicking on a
        // selected element deselects it, while clicking on an unselected
        // element assigns the first available color from {R, G, B}.
             if (o.color_elements_[0] == idx) o.color_elements_[0] = -1;
        else if (o.color_elements_[1] == idx) o.color_elements_[1] = -1;
        else if (o.color_elements_[2] == idx) o.color_elements_[2] = -1;
        else if (o.color_elements_[0] == -1 ) o.color_elements_[0] = idx;
        else if (o.color_elements_[1] == -1 ) o.color_elements_[1] = idx;
        else if (o.color_elements_[2] == -1 ) o.color_elements_[2] = idx;
      }
      else
        o.element_ = (dip::uint)idx;
    }
  }
}

}} // namespace dip::viewer
