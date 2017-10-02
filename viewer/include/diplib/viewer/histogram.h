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

#include "diplib/framework.h"

#include "diplib/viewer/viewer.h"
#include "diplib/viewer/image.h"

namespace dip { namespace viewer {

/// \brief Controls grey-value mapping range and shows color mapping.
class DIPVIEWER_EXPORT HistogramViewPort : public ViewPort
{
  protected:
    ImageView colorbar_;
    dip::Image histogram_;
    
    int drag_limit_, drag_x_, drag_y_;

  public:
    explicit HistogramViewPort(Viewer *viewer) : ViewPort(viewer), colorbar_(this)
    {
    }
    ~HistogramViewPort() override { }
    
    void calculate();
    void render() override;
    void click(int button, int state, int x, int y) override;
    void motion(int button, int x, int y) override;
    
  protected:
    void screenToView(int x, int y, double *ix, double *iy) override;
};

}} // namespace dip::viewer

#endif // DIP_VIEWER_HISTOGRAM_H
