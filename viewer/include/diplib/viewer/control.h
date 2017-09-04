/*
 * DIPlib 3.0 viewer
 * This file contains definitions for controlling the display options.
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

#ifndef DIP_VIEWER_CONTROL_H
#define DIP_VIEWER_CONTROL_H

#include "diplib/viewer/viewer.h"

class DIPVIEWER_EXPORT ControlViewPort : public ViewPort
{
  protected:
    std::vector<std::vector<dip::String> > lists_;

  public:
    ControlViewPort(Viewer *viewer) : ViewPort(viewer)
    {
      lists_.push_back({"SPA", "RGB", "GRY", "JET"});
      lists_.push_back({"0-1", "255", "LIN", "SYM", "LOG"});
      lists_.push_back({"REA", "IMG", "MAG", "PHA"});
      lists_.push_back({"SLC", "MIN", "MEA", "MAX"});
    }
    ~ControlViewPort() { }
    
    void render();
    void click(int button, int state, int x, int y);
};

#endif // DIP_VIEWER_H
