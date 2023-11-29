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

#ifndef DIP_VIEWER_CONTROL_H
#define DIP_VIEWER_CONTROL_H

#include <vector>

#include "diplib.h"
#include "diplib/viewer/export.h"
#include "diplib/viewer/viewer.h"

/// \file
/// \brief Declares \ref dip::viewer::ControlViewPort.

namespace dip {
namespace viewer {

/// \addtogroup dipviewer

/// \brief Allows the user to control how the image is displayed.
class DIPVIEWER_CLASS_EXPORT ControlViewPort : public ViewPort {
   protected:
      std::vector< std::vector< dip::String >> lists_;

   public:
      explicit ControlViewPort( Viewer* viewer ) : ViewPort( viewer ) {
         lists_.push_back( { "SPA", "RGB", "GRY", "SEQ", "DIV", "CYC", "LBL" } );
         lists_.push_back( { "0-1", "ANG", "255", "LIN", "SYM", "LOG" } );
         lists_.push_back( { "REA", "IMG", "MAG", "PHA" } );
         lists_.push_back( { "SLC", "MIN", "MEA", "MAX" } );
      }

      ControlViewPort( ControlViewPort const& ) = delete;
      ControlViewPort( ControlViewPort&& ) = default;
      ControlViewPort& operator=( ControlViewPort const& ) = delete;
      ControlViewPort& operator=( ControlViewPort&& ) = default;
      ~ControlViewPort() override = default;

      DIPVIEWER_EXPORT void render() override;
      DIPVIEWER_EXPORT void click( int button, int state, int x, int y, int mods ) override;
};

/// \endgroup

} // namespace viewer
} // namespace dip

#endif // DIP_VIEWER_H
