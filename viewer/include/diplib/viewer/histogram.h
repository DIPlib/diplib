/*
 * DIPlib 3.0 viewer
 * This file contains definitions for displaying the histogram.
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

#ifndef DIP_VIEWER_HISTOGRAM_H
#define DIP_VIEWER_HISTOGRAM_H

#undef DIP__ENABLE_DOCTEST
#include <diplib/framework.h>

#include "diplib/viewer/viewer.h"
#include "diplib/viewer/image.h"

class DIPVIEWER_EXPORT HistogramViewPort : public ViewPort
{
  protected:
    ImageView colorbar_;
    dip::Image histogram_;
    
    int drag_limit_, drag_x_, drag_y_;

  public:
    HistogramViewPort(Viewer *viewer) : ViewPort(viewer), colorbar_(this)
    {
    }
    ~HistogramViewPort() { }
    
    void calculate();
    void render();
    void click(int button, int state, int x, int y);
    void motion(int button, int x, int y);
    
  protected:
    void screenToView(int x, int y, double *ix, double *iy);
};

template<class T>
class viewer__Histogram : public dip::Framework::ScanLineFilter
{
  protected:
    dip::Image &histogram_;
    FloatRange range_;

  public:
    viewer__Histogram(dip::Image &histogram, FloatRange range) : histogram_(histogram), range_(range) { }
  
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
          out[(dip::uint)(((dip::dfloat)bins-1)*((dip::dfloat)*inT-offset) * scale)*tensorLength+jj]++;
      }
    }
};

#endif // DIP_VIEWER_HISTOGRAM_H
