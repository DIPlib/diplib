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

#include <GL/glu.h>

#include "diplib/math.h"
#include "diplib/viewer/histogram.h"

using namespace dip;

void HistogramViewPort::render()
{
  auto &o = viewer()->options();
  
  // Colorbar
  // TODO: only do this when lut changed  
  dip::Image cb { dip::UnsignedArray{ 24, (dip::uint)height_ }, 3, dip::DT_UINT8 };
  cb = 0;
  
  dip::uint8 *data = (dip::uint8*)cb.Origin();
  for (size_t ii=0; ii < (dip::uint)height_; ++ii)
  {
    dip::sfloat val = (dip::sfloat) (o.range_.first + (double)ii*(o.range_.second-o.range_.first)/(double)height_);
    dip::uint8 out[3] = {0, 0, 0};
    
    colorMap(&val, 0, out, o);
    if (o.lut_ == ViewingOptions::LookupTable::RGB)
    {
      for (size_t jj=0; jj < 8; ++jj)
        for (size_t kk=0; kk < 3; ++kk)
          data[(ii*24+8*kk+jj)*3+kk] = out[kk];
    }
    else
    {
      for (size_t jj=0; jj < 24; ++jj)
        for (size_t kk=0; kk < 3; ++kk)
          data[(ii*24+jj)*3+kk] = out[kk];
    }
  }    
  
  colorbar_.set(cb);
  colorbar_.rebuild();
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height_, 24, height_);
  gluOrtho2D(0, 24, 0, height_);
  glMatrixMode(GL_MODELVIEW);
  colorbar_.render();
  
  // Histogram
  // Note that no range mapping is going on below. The histogram is always
  // linear and always displayed fully; it's the colorbar that changes to
  // show what happens in a logarithmic or more limited mapping.

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_+24, viewer()->height()-y_-height_, width_-24, height_);
  gluOrtho2D(0, 1, 0, 1);
  glMatrixMode(GL_MODELVIEW);

  // Calculate maximum value
  dip::Image copy = histogram_;
  copy.TensorToSpatial();
  dip::MinMaxAccumulator acc = GetMaximumAndMinimum(copy);
  GLfloat maxhist = (GLfloat)acc.Maximum();

  dip::uint32 *hdata = static_cast< dip::uint32 * >( histogram_.Origin() );
  dip::sfloat *p = (dip::sfloat*)viewer()->image().Pointer(o.operating_point_);
  dip::sint tstride = viewer()->image().TensorStride();
  
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
        for (size_t jj=0; jj < histogram_.Size(0); ++jj)
        {
          glVertex2f(0., (GLfloat)jj/(GLfloat)histogram_.Size(0));
          glVertex2f((GLfloat)hdata[(dip::sint)jj*histogram_.Stride(0)+o.color_elements_[ii]]/maxhist, (GLfloat)jj/(GLfloat)histogram_.Size(0));
        }
        glEnd();
      
        // Current value
        double val = (double) *(p + o.color_elements_[ii]*tstride);
        glBegin(GL_LINES);
          glVertex2f(0., (GLfloat)((val-o.range_.first)/(o.range_.second-o.range_.first)));
          glVertex2f(1., (GLfloat)((val-o.range_.first)/(o.range_.second-o.range_.first)));
        glEnd();
      }
    glDisable(GL_BLEND);
  }
  else
  {
    // Histogram
    glColor3f(1., 1., 1.);
    glBegin(GL_TRIANGLE_STRIP);
    for (size_t ii=0; ii < histogram_.Size(0); ++ii)
    {
      glVertex2f(0., (GLfloat)ii/(GLfloat)histogram_.Size(0));
      glVertex2f((GLfloat)hdata[(dip::sint)ii*histogram_.Stride(0)+(dip::sint)o.element_]/maxhist, (GLfloat)ii/(GLfloat)histogram_.Size(0));
    }
    glEnd();
    
    // Current value
    double val = (GLfloat) *(p + (dip::sint)o.element_*tstride);
    glBegin(GL_LINES);
      glVertex2f(0., (GLfloat)((val-o.range_.first)/(o.range_.second-o.range_.first)));
      glVertex2f(1., (GLfloat)((val-o.range_.first)/(o.range_.second-o.range_.first)));
    glEnd();
  }
}

void HistogramViewPort::click(int button, int state, int x, int y)
{
  // TODO
  (void)button;
  (void)state;
  (void)x;
  (void)y;
}

void HistogramViewPort::motion(int button, int x, int y)
{
  // TODO
  (void)button;
  (void)x;
  (void)y;
}

void HistogramViewPort::calculate()
{
  auto &i = viewer()->image();
  
  histogram_ = dip::Image { dip::UnsignedArray{ 100 }, i.TensorElements(), dip::DT_UINT32 };  
  histogram_ = 0;
  
  viewer__Histogram<dip::sfloat> scanLineFilter(histogram_, viewer()->options().range_);
  dip::Framework::ScanSingleInput(i, {}, DT_SFLOAT, scanLineFilter);
}
