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

#ifndef DIP_VIEWER_HISTOGRAM_H
#define DIP_VIEWER_HISTOGRAM_H

#include <mutex>

#include "diplib.h"
#include "diplib/viewer/export.h"
#include "diplib/viewer/image.h"
#include "diplib/viewer/viewer.h"

/// \file
/// \brief Declares \ref dip::viewer::HistogramViewPort.

namespace dip {
namespace viewer {

/// \addtogroup dipviewer

/// \brief Controls grey-value mapping range and shows color mapping.
class DIPVIEWER_CLASS_EXPORT HistogramViewPort : public ViewPort {
   protected:
      ImageView colorbar_;
      dip::Image histogram_;
      std::mutex mutex_;

      int drag_limit_, drag_x_, drag_y_;

   public:
      explicit HistogramViewPort( Viewer* viewer ) :
         ViewPort( viewer ), colorbar_( this ) {}

      HistogramViewPort( HistogramViewPort const& ) = delete;
      HistogramViewPort( HistogramViewPort&& ) = delete;
      HistogramViewPort& operator=( HistogramViewPort const& ) = delete;
      HistogramViewPort& operator=( HistogramViewPort&& ) = delete;
      ~HistogramViewPort() override = default;

      DIPVIEWER_EXPORT void calculate();
      DIPVIEWER_EXPORT void render() override;
      DIPVIEWER_EXPORT void click( int button, int state, int x, int y, int mods ) override;
      DIPVIEWER_EXPORT void motion( int button, int x, int y ) override;

   protected:
      DIPVIEWER_EXPORT void screenToView( int x, int y, double* ix, double* iy ) override;
};

/// \endgroup

} // namespace viewer
} // namespace dip

#endif // DIP_VIEWER_HISTOGRAM_H
