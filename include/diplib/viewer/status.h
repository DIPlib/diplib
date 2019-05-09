/*
 * DIPlib 3.0 viewer
 * This file contains definitions for the status bar.
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

#ifndef DIP_VIEWER_STATUS_H
#define DIP_VIEWER_STATUS_H

#include "diplib/viewer/viewer.h"

/// \file
/// \brief Declares `dip::viewer::StatusViewPort`.

namespace dip { namespace viewer {

/// \brief Displays the Viewer's status.
class DIPVIEWER_CLASS_EXPORT StatusViewPort : public ViewPort
{
  protected:
    std::vector<std::vector<dip::String> > lists_;

  public:
    explicit StatusViewPort(Viewer *viewer) : ViewPort(viewer) { }
    ~StatusViewPort() override { }
    
    DIPVIEWER_EXPORT void render() override;
};

}} // namespace dip::viewer

#endif // DIP_VIEWER_STATUS_H
