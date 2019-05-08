/*
 * DIPlib 3.0 viewer
 * This file contains definitions for displaying tensor elements.
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

#ifndef DIP_VIEWER_TENSOR_H
#define DIP_VIEWER_TENSOR_H

#include "diplib/viewer/viewer.h"

/// \file
/// \brief Declares `dip::viewer::TensorViewPort`.

namespace dip { namespace viewer {

/// \brief Allows the user to control which tensor elements are visualized.
class DIPVIEWER_CLASS_EXPORT TensorViewPort : public ViewPort
{
  public:
    TensorViewPort(Viewer *viewer) : ViewPort(viewer) { }
    ~TensorViewPort() { }
    
    DIPVIEWER_EXPORT void render();
    DIPVIEWER_EXPORT void click(int button, int state, int x, int y, int mods);
};

}} // namespace dip::viewer

#endif // DIP_VIEWER_TENSOR_H
