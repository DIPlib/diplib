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
  dip::Image values { dip::UnsignedArray{ 3, (dip::uint)height }, viewer()->image().TensorElements(), dip::DT_DFLOAT };
  values = 0.;
  dip::dfloat* it = static_cast< dip::dfloat* >( values.Origin() );
  for (size_t ii=0; ii < size_t(height); ++ii)
  {
    double val = o.range_.first + (double)ii*(o.range_.second-o.range_.first)/(double)height;
    
    for (size_t jj=0; jj < 3; ++jj)
      for (size_t kk=0; kk < viewer()->image().TensorElements(); ++kk, ++it)
        *it = val;
  }
  
  dip::Image cb { dip::UnsignedArray{ 3, (dip::uint)height }, 3, dip::DT_UINT8 };
  if (o.lut_ == ViewingOptions::LookupTable::RGB)
  {
    // Make sure channels do not bleed into each other
    dip::Image cbsingle = cb;
    cb = 0;
    for (size_t jj=0; jj < 3; ++jj)
    {
      auto osingle = o;
      osingle.color_elements_[(jj+1)%3] = -1;
      osingle.color_elements_[(jj+2)%3] = -1;
      ApplyViewerColorMap(values, cbsingle, osingle);
      cbsingle.At({((dip::sint)jj+1)%3,((dip::sint)jj+1)%3},{}) = 0;
      cbsingle.At({((dip::sint)jj+2)%3,((dip::sint)jj+2)%3},{}) = 0;
      cb = cb + cbsingle;
    }
    cb.Convert(dip::DT_UINT8);
  }
  else
    ApplyViewerColorMap(values, cb, o);

  colorbar_.set(cb);
  colorbar_.rebuild();
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height, 24, height);
  glOrtho(0, 3, 0, height, -1, 1);
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
  mutex_.lock();
  dip::Image copy = histogram_;
  copy.TensorToSpatial();
  dip::MinMaxAccumulator acc = MaximumAndMinimum( copy );
  copy.Strip();
  GLfloat maxhist = (GLfloat)acc.Maximum();
  auto p = viewer()->image().At<GLfloat>(o.operating_point_);
  
  // Only draw histogram if it has been updated to correspond to current image.
  if (histogram_.TensorElements() == viewer()->image().TensorElements())
  {
    if (o.lut_ == ViewingOptions::LookupTable::RGB)
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE);
    
      for (size_t ii=0; ii < 3; ++ii)
        if (o.color_elements_[ii] != -1)
        {
          // Histogram
          if (ii == 0)
            glColor3d(0.9, 0.17, 0.);
          else if (ii == 1)
            glColor3d(0.0, 0.50, 0.);
          else if (ii == 2)
            glColor3d(0.1, 0.33, 1.);
          
          glBegin(GL_TRIANGLE_STRIP);
          
          ImageIterator<dip::uint32> it(histogram_[o.color_elements_[ii]]);
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
      
      ImageIterator<dip::uint32> it(histogram_[(dip::sint)o.element_]);
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
  }
  mutex_.unlock();
  
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

void HistogramViewPort::click(int button, int state, int x, int y, int /*mods*/)
{
  auto &o = viewer()->options();

  if (state == 0 && button == 0)
  {
    // Left mouse button: drag mapping limits 
    double ix, iy;
  
    screenToView(x, y, &ix, &iy);
    double dlower = iy - clamp((o.mapping_range_.first-o.range_.first)/(o.range_.second-o.range_.first), 0., 1.),
           dupper = iy - clamp((o.mapping_range_.second-o.range_.first)/(o.range_.second-o.range_.first), 0., 1.);

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
    {
      o.mapping_range_.first += dy;
      if (o.mapping_ == ViewingOptions::Mapping::Symmetric)
        o.mapping_range_.second = -o.mapping_range_.first;
    }
    else
    {
      o.mapping_range_.second += dy;
      if (o.mapping_ == ViewingOptions::Mapping::Symmetric)
        o.mapping_range_.first = -o.mapping_range_.second;
    }
    
    drag_x_ = y;
    drag_y_ = y;
    
    // Show we're not in a fixed mapping anymore
    switch (o.mapping_)
    {
      case ViewingOptions::Mapping::ZeroOne:
      case ViewingOptions::Mapping::Angle:
      case ViewingOptions::Mapping::Normal:
        o.mapping_ = ViewingOptions::Mapping::Linear;
        break;
      case ViewingOptions::Mapping::Linear:
      case ViewingOptions::Mapping::Symmetric:
      case ViewingOptions::Mapping::Logarithmic:
        // Nothing to do
        break;
    }
    
    o.status_ = "Mapping range set to [" + std::to_string(o.mapping_range_.first) + ", " + std::to_string(o.mapping_range_.second) + "]";

    viewer()->refresh();
  }
}

void HistogramViewPort::screenToView(int x, int y, double *ix, double *iy)
{
  *ix = (x-(x_+24))/(double)(width_-24);
  *iy = 1-(y-y_)/(double)height_; // Lowest value at bottom
}

namespace {

template<class T>
class HistogramLineFilter : public dip::Framework::ScanLineFilter
{
   protected:
      dip::Image &histogram_;
      FloatRange range_;

   public:
      HistogramLineFilter( dip::Image &histogram, FloatRange range) : histogram_( histogram), range_( range) { }

      virtual void Filter( dip::Framework::ScanLineFilterParameters const& params ) override
      {
        T const* in = static_cast< T const* >( params.inBuffer[ 0 ].buffer );
        dip::uint32 * out = static_cast< dip::uint32 * >( histogram_.Origin() );

        auto bufferLength = params.bufferLength;
        auto inStride = params.inBuffer[ 0 ].stride;
        auto tensorStride = params.inBuffer[ 0 ].tensorStride;
        auto tensorLength = params.inBuffer[ 0 ].tensorLength;
        auto offset = range_.first;
        auto scale = 1.f/(range_.second-range_.first);
        auto bins = histogram_.Size(0);

        for( dip::uint ii = 0; ii < bufferLength; ++ii, in += inStride )
        {
          T const* inT = in;

          for( dip::uint jj = 0; jj < tensorLength; ++jj, inT += tensorStride )
            out[dip::clamp((dip::uint)(((dip::dfloat)bins-1)*((dip::dfloat)*inT-offset) * scale), (dip::uint)0, (dip::uint)bins-1)*tensorLength+jj]++;
        }
      }
};

} // namespace

void HistogramViewPort::calculate()
{
  viewer()->lock();
  dip::Image in = viewer()->image();
  viewer()->unlock();

  dip::Image histogram = dip::Image { dip::UnsignedArray{ 100 }, in.TensorElements(), dip::DT_UINT32 };  
  histogram = 0;

  std::unique_ptr< dip::Framework::ScanLineFilter > scanLineFilter;
  DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, HistogramLineFilter, ( histogram, viewer()->options().range_ ), in.DataType() );
  dip::Framework::ScanSingleInput(in, {}, in.DataType(), *scanLineFilter);
  
  mutex_.lock();
  histogram_ = histogram;
  mutex_.unlock();
}

}} // namespace dip::viewer
