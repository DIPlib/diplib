/*
 * DIPlib 3.0 viewer
 * This file contains functionality for displaying the histogram.
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
#include "diplib/statistics.h"
#include "diplib/iterators.h"
#include "diplib/overload.h"
#include "diplib/viewer/include_gl.h"
#include "diplib/viewer/histogram.h"

namespace dip { namespace viewer {

void HistogramViewPort::render()
{
  auto &o = viewer()->options();
  
  int width = width_, height = height_;
  
  if (width < 1 || height < 1)
    return;
  
  // Colorbar
  // TODO: only do this when lut changed  
  dip::Image cb { dip::UnsignedArray{ 24, (dip::uint)height }, 3, dip::DT_UINT8 };
  cb = 0;
  
  ImageIterator<dip::uint8> it(cb);
  for (size_t ii=0; ii < (dip::uint)height; ++ii)
  {
    dip::Image::Pixel val((o.range_.first + (double)ii*(o.range_.second-o.range_.first)/(double)height));
    dip::uint8 out[3] = {0, 0, 0};
    colorMap(val, out, o);
    
    if (o.lut_ == ViewingOptions::LookupTable::RGB)
    {
      for (dip::uint kk=0; kk < 3; ++kk)
        for (size_t jj=0; jj < 8; ++jj, ++it)
          it[kk] = out[kk];
    }
    else
    {
      for (size_t jj=0; jj < 24; ++jj, ++it)
        for (dip::uint kk=0; kk < 3; ++kk)
          it[kk] = out[kk];
    }
  }    
  
  colorbar_.set(cb);
  colorbar_.rebuild();
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height, 24, height);
  glOrtho(0, 24, 0, height, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  colorbar_.render();
  
  // Histogram
  // Note that no range mapping is going on below. The histogram is always
  // linear and always displayed fully; it's the colorbar that changes to
  // show what happens in a logarithmic or more limited mapping.

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_+24, viewer()->height()-y_-height, width-24, height);
  glOrtho(0, 1, 0, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW);

  // Calculate maximum value
  dip::Image copy = histogram_;
  copy.TensorToSpatial();
  dip::MinMaxAccumulator acc = MaximumAndMinimum( copy );
  GLfloat maxhist = (GLfloat)acc.Maximum();
  auto p = viewer()->image().At<GLfloat>(o.operating_point_);
  
  if (o.lut_ == ViewingOptions::LookupTable::RGB)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
  
    for (size_t ii=0; ii < 3; ++ii)
      if (o.color_elements_[ii] != -1)
      {
        // Histogram
        glColor3f(ii==0, ii==1, ii==2);
        glBegin(GL_TRIANGLE_STRIP);
        
        ImageIterator<dip::uint32> it(histogram_[(dip::uint)o.color_elements_[ii]]);
        for (size_t jj=0; jj < histogram_.Size(0); ++jj, ++it)
        {
          glVertex2f(0., (GLfloat)jj/(GLfloat)histogram_.Size(0));
          glVertex2f((GLfloat)*it/maxhist, (GLfloat)jj/(GLfloat)histogram_.Size(0));
        }
        glEnd();
      
        // Current value
        glBegin(GL_LINES);
          glVertex2f(0., (GLfloat)(((dip::dfloat)p[(dip::uint)o.color_elements_[ii]]-o.range_.first)/(o.range_.second-o.range_.first)));
          glVertex2f(1., (GLfloat)(((dip::dfloat)p[(dip::uint)o.color_elements_[ii]]-o.range_.first)/(o.range_.second-o.range_.first)));
        glEnd();
      }
    glDisable(GL_BLEND);
  }
  else
  {
    // Histogram
    glColor3f(1., 1., 1.);
    glBegin(GL_TRIANGLE_STRIP);
    
    ImageIterator<dip::uint32> it(histogram_[o.element_]);
    for (size_t ii=0; ii < histogram_.Size(0); ++ii, ++it)
    {
      glVertex2f(0., (GLfloat)ii/(GLfloat)histogram_.Size(0));
      glVertex2f((GLfloat)*it/maxhist, (GLfloat)ii/(GLfloat)histogram_.Size(0));
    }
    glEnd();
    
    // Current value
    glBegin(GL_LINES);
      glVertex2f(0., (GLfloat)(((dip::dfloat)p[o.element_]-o.range_.first)/(o.range_.second-o.range_.first)));
      glVertex2f(1., (GLfloat)(((dip::dfloat)p[o.element_]-o.range_.first)/(o.range_.second-o.range_.first)));
    glEnd();
  }
  
  // Display mapping range
  glColor3f(.5, .5, .5);
  glBegin(GL_LINES);
    glVertex2f(0., (GLfloat)((o.mapping_range_.first-o.range_.first)/(o.range_.second-o.range_.first)));
    glVertex2f(1., (GLfloat)((o.mapping_range_.first-o.range_.first)/(o.range_.second-o.range_.first)));
    glVertex2f(0., (GLfloat)((o.mapping_range_.second-o.range_.first)/(o.range_.second-o.range_.first)));
    glVertex2f(1., (GLfloat)((o.mapping_range_.second-o.range_.first)/(o.range_.second-o.range_.first)));
  glEnd();
  
  // Display range text
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_+24, viewer()->height()-y_-height, width-24, height);
  glOrtho(0, width-24, height, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);

  char buf[20];
  glColor3f(.5, .5, .5);
  glRasterPos2i(1, 11);
  sprintf(buf, "%9.2e", o.range_.second);
  viewer()->drawString(buf);
  glRasterPos2i(1, height-3);
  sprintf(buf, "%9.2e", o.range_.first);
  viewer()->drawString(buf);
}

void HistogramViewPort::click(int button, int state, int x, int y)
{
  auto &o = viewer()->options();

  if (state == 0 && button == 0)
  {
    // Left mouse button: drag mapping limits 
    double ix, iy;
  
    screenToView(x, y, &ix, &iy);
    double dlower = iy - (o.mapping_range_.first-o.range_.first)/(o.range_.second-o.range_.first),
           dupper = iy - (o.mapping_range_.second-o.range_.first)/(o.range_.second-o.range_.first);
           
    if (std::abs(dlower) < std::abs(dupper))
      drag_limit_ = 0;
    else
      drag_limit_ = 1;
      
    drag_x_ = x;
    drag_y_ = y;
  }
}

void HistogramViewPort::motion(int button, int x, int y)
{
  auto &o = viewer()->options();
  
  if (button == 0)
  {
    // Left mouse button: drag mapping limits 
    double ix, iy, dix, diy;
    screenToView(x, y, &ix, &iy);
    screenToView(drag_x_, drag_y_, &dix, &diy);
    
    double dy = (iy-diy)*(o.range_.second-o.range_.first);
    
    if (drag_limit_ == 0)
      o.mapping_range_.first += dy;
    else
      o.mapping_range_.second += dy;
    
    drag_x_ = y;
    drag_y_ = y;

    viewer()->refresh();
  }
}

void HistogramViewPort::screenToView(int x, int y, double *ix, double *iy)
{
  *ix = (x-(x_+24))/(double)(width_-24);
  *iy = 1-(y-y_)/(double)height_; // Lowest value at bottom
}

void HistogramViewPort::calculate()
{
  auto &in = viewer()->image();
  
  histogram_ = dip::Image { dip::UnsignedArray{ 100 }, in.TensorElements(), dip::DT_UINT32 };  
  histogram_ = 0;

  std::unique_ptr< dip::Framework::ScanLineFilter > scanLineFilter;
  DIP_OVL_NEW_REAL( scanLineFilter, viewer__Histogram, ( histogram_, viewer()->options().range_ ), in.DataType() );
  dip::Framework::ScanSingleInput(in, {}, in.DataType(), *scanLineFilter);
}

}} // namespace dip::viewer
