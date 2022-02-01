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

#include "include_gl.h"
#include "diplib/viewer/status.h"

namespace dip { namespace viewer {

namespace {
std::string to_string(dcomplex value) {
   std::stringstream str;
   str << value.real();
   if (value.imag() >= 0) {
      str << '+';
   }
   str << value.imag() << 'i';
   return str.str();
}
}

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

  dip::uint rx = 1;
  dim_starts_.clear();

  if (o.status_ != "")
  {
    viewer()->drawString(o.status_.c_str());
  }
  else
  {
    // Describe operating point
    auto op = o.operating_point_;
    
    // Bail if options do not match original. This can happen
    // after the original is changed, but before it is processed
    // and copied to the viewer's image.
    if (op.size() != viewer()->original().Dimensionality())
      return;
    for (dip::uint ii=0; ii < op.size(); ++ii)
      if (op[ii] >= viewer()->original().Size(ii))
        return;
        
    auto te = (int)viewer()->image().TensorElements();
    auto opp = viewer()->image().PixelsToPhysical((FloatArray)op);
    
    rx += viewer()->drawString("(");
    for (dip::uint ii=0; ii < op.size(); ++ii)
    {
      dim_starts_.push_back(rx);
      rx += viewer()->drawString(std::to_string(op[ii]).c_str());
      if (viewer()->image().PixelSize(ii) != PhysicalQuantity::Pixel() || o.offset_[ii])
      {
        std::ostringstream oss;
        PhysicalQuantity p = opp[ii] + o.offset_[ii];
        p.Normalize();
        oss << "=" << p.magnitude << p.units.String();
        rx += viewer()->drawString(oss.str().c_str());
      }
        
      if (ii < op.size()-1)
        rx += viewer()->drawString(", ");
    }
    dim_starts_.push_back(rx);

    rx += viewer()->drawString("): ");
    if (te > 1)
      rx += viewer()->drawString("[");

    auto pixel = viewer()->original().At(op);
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
        if ((dip::uint)ii == o.element_)
          glColor3f(1., 1., 1.);
        else
          glColor3f(.5, .5, .5);
      }
      
      glRasterPos2i((GLint)rx, 12);
      auto value = pixel[(dip::uint)ii];
      if (value.DataType().IsUnsigned()) {
         rx += viewer()->drawString(std::to_string(value.As<dip::uint>()).c_str());
      } else if (value.DataType().IsSInt()) {
         rx += viewer()->drawString(std::to_string(value.As<dip::sint>()).c_str());
      } else if (value.DataType().IsComplex()) {
         rx += viewer()->drawString(to_string(value.As<dcomplex>()).c_str());
      } else {
         rx += viewer()->drawString(std::to_string(value.As<dfloat>()).c_str());
      }
        
      glColor3f(1., 1., 1.);
      glRasterPos2i((GLint)rx, 12);
      
      if (ii < te-1)
        rx += viewer()->drawString(", ");
    }
    if (te > 1)
      rx += viewer()->drawString("]");
  }
}

void StatusViewPort::click(int button, int state, int x, int y, int /*mods*/)
{
  auto& o = viewer()->options();
  auto& op = o.operating_point_;

  double ix, iy;
  screenToView(x, y, &ix, &iy);

  if (state == 0)
  {
    if (button == 0)
    {
      o.status_ = "";
      viewer()->refresh();
    }

    if ((button == 3 || button == 4) && dim_starts_.size() == op.size() + 1)
    {
      // Find which dimension was clicked
      dip::sint dim = -1;
      for (dip::uint idx = 0; idx != op.size(); ++idx)
        if (x > dim_starts_[idx] && x < dim_starts_[idx + 1])
          dim = idx;

      if (dim != -1)
      {
        auto sz = viewer()->original().Sizes();

        dip::sint diff = (button == 3) ? -1 : 1;
        op[dim] = std::min(std::max((int)(op[dim] + diff), 0), (int)(sz[dim] - 1));
      }
    }
  }
}

}} // namespace dip::viewer
