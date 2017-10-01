/*
 * DIPlib 3.0 viewer
 * This file contains definitions for the link facility.
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

#ifndef DIP_VIEWER_LINK_H
#define DIP_VIEWER_LINK_H

#include <set>

#include "diplib/viewer/viewer.h"

namespace dip { namespace viewer {

/// \brief Handles viewer linking
class DIPVIEWER_EXPORT LinkViewPort : public ViewPort
{
  protected:
    static LinkViewPort *link_source_;
    std::set<LinkViewPort*> links_;

  public:
    explicit LinkViewPort(Viewer *viewer) : ViewPort(viewer) { }
    ~LinkViewPort() override
    {
      for (auto it=links_.begin(); it != links_.end(); ++it)
        (*it)->unlink(this);
      links_.clear();
    }
    
    void render() override;
    void click(int button, int state, int x, int y) override;
    
    /// \brief Update linked viewers' options
    void update();
    
  protected:
  
    /// \brief Update from linked viewer's options
    void update(const ViewingOptions &options);
    
    /// \brief Add linked viewer
    void link(LinkViewPort *link);
    
    /// \brief Remove linked viewer
    void unlink(LinkViewPort *link);
};

}} // namespace dip::viewer

#endif // DIP_VIEWER_LINK_H
