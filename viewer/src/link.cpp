/*
 * DIPlib 3.0 viewer
 * This file contains functionality for the link facility.
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

#undef DIP__ENABLE_DOCTEST
#include "diplib/viewer/include_gl.h"
#include "diplib/viewer/link.h"

namespace dip { namespace viewer {

LinkViewPort *LinkViewPort::link_source_=NULL;

void LinkViewPort::render()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height_, width_, height_);
  glOrtho(0, width(), height(), 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);

  // Clear status bar text
  glColor3d(0., 0., 0.);
  glBegin(GL_POLYGON);
    glVertex2i(0, 0);
    glVertex2i(width(), 0);
    glVertex2i(width(), height_);
    glVertex2i(0, height_);
  glEnd();

  // Separate visually from histogram
  glColor3d(.5, .5, .5);
  glBegin(GL_LINES);
    glVertex2i(0, 0);
    glVertex2i(width_, 0);
  glEnd();
  
  // Highlight if this is a link source
  if (link_source_ == this)
    glColor3d(1., 1., 1.);
  else
    glColor3d(.5, .5, .5);
    
  // Show link status
  if (!links_.empty())
  {
    glRasterPos2i(width_/2-24, 12);
    viewer()->drawString("LINKED");
  }
  else
  {
    glRasterPos2i(width_/2-16, 12);
    viewer()->drawString("LINK");
  }
}

void LinkViewPort::click(int button, int state, int /*x*/, int /*y*/)
{
  if (state == 0)
  {
    if (button == 0)
    {
      // Left mouse button: add link
      if (link_source_ == this)
      {
        // Cancel
        link_source_ = NULL;
        viewer()->options().status_ = "";
      }
      else if (link_source_)
      {
        // Check compatibility
        for (size_t ii=0; ii < std::min(viewer()->image().Dimensionality(), link_source_->viewer()->image().Dimensionality()); ++ii)
          if (viewer()->image().Size(ii) != link_source_->viewer()->image().Size(ii))
          {
            viewer()->options().status_ = "Link source image size mismatch on dimension";
            return;
          }
          
        // Take settings from link source
        update(link_source_->viewer()->options());

        // Perform link
        link(link_source_);
        link_source_->link(this);
        
        link_source_ = NULL;
      }
      else
      {
        // Set link source for click in destination window
        link_source_ = this;
        
        viewer()->options().status_ = "Click link icon in destination window";
      }
    }
    
    if (button == 2)
    {
      // Right mouse button: clear links
      for (auto it=links_.begin(); it != links_.end(); ++it)
        (*it)->unlink(this);
      links_.clear();
      
      viewer()->options().status_ = "Unlinked from all viewers";
    }
  }
}

void LinkViewPort::update()
{
  for (auto it=links_.begin(); it != links_.end(); ++it)
    (*it)->update(viewer()->options());
}

void LinkViewPort::update(const ViewingOptions &options)
{
  auto old_options = viewer()->options();

  for (size_t ii=0; ii<std::min(viewer()->options().operating_point_.size(), options.operating_point_.size()); ++ii)
  {
    viewer()->options().operating_point_[ii] = options.operating_point_[ii];
    viewer()->options().zoom_[ii] = options.zoom_[ii];
    viewer()->options().origin_[ii] = options.origin_[ii];
  }

  if (viewer()->options().diff(old_options) > ViewingOptions::Diff::None)
  {
    viewer()->options().status_ = "";
    viewer()->refresh();
  }
}

void LinkViewPort::link(LinkViewPort *link)
{
  links_.insert(link);
  viewer()->options().status_ = "Linked to " + link->viewer()->name();
}

void LinkViewPort::unlink(LinkViewPort *link)
{
  links_.erase(link);
  viewer()->options().status_ = "Unlinked from " + link->viewer()->name();
}

}} // namespace dip::viewer
