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

#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/generic_iterators.h"

#include "include_gl.h"
#include "diplib/viewer/control.h"
#include "diplib/viewer/histogram.h"
#include "diplib/viewer/link.h"
#include "diplib/viewer/slice.h"
#include "diplib/viewer/status.h"
#include "diplib/viewer/tensor.h"

#define CHAR_WIDTH   8
#define CHAR_HEIGHT 13

#define DIM_WIDTH   (CHAR_WIDTH+2)
#define DIM_HEIGHT  (CHAR_HEIGHT+2)

/// \file
/// \brief Defines `dip::viewer::SliceViewer`.

namespace dip { namespace viewer {

void SliceView::project()
{
  auto &o = viewport()->viewer()->options();
  Image image = viewport()->viewer()->image();

  dip::sint dx = o.dims_[dimx_], dy = o.dims_[dimy_];

  if (o.projection_ == ViewingOptions::Projection::None)
  {
    // Extraction
    RangeArray range(image.Dimensionality());

    for (dip::uint ii=0; ii < range.size(); ++ii)
      if ((int)ii != dx && (int)ii != dy)
        range[ii] = Range((dip::sint)o.operating_point_[ii]);

    projected_ = image.At(std::move(range));
  }
  else
  {
    // Projection
    BooleanArray process( image.Dimensionality(), true );
    dip::UnsignedArray ro = o.roi_origin_;
    dip::UnsignedArray rs = o.roi_sizes_;

    if (dx != -1)
    {
      process[ (dip::uint)dx ] = false;
      ro[ (dip::uint)dx ] = 0;
      rs[ (dip::uint)dx ] = image.Size((dip::uint)dx);
    }
    if (dy != -1)
    {
      process[ (dip::uint)dy ] = false;
      ro[ (dip::uint)dy ] = 0;
      rs[ (dip::uint)dy ] = image.Size((dip::uint)dy);
    }

    image = DefineROI(image, ro, rs, {});

    switch (o.projection_)
    {
      case ViewingOptions::Projection::None:
        break;
      case ViewingOptions::Projection::Min:
        Minimum( image, {}, projected_, process );
        break;
      case ViewingOptions::Projection::Mean:
        Mean( image, {}, projected_, "", process );
        break;
      case ViewingOptions::Projection::Max:
        Maximum( image, {}, projected_, process );
        break;
    }
  }

  if (dx == -1 || dy == -1)
    projected_.Squeeze();
  else
    projected_.PermuteDimensions({(unsigned int)dx, (unsigned int)dy});

  map();
}

void SliceView::map()
{
  auto &o = viewport()->viewer()->options();

  dirty_ = true;

  if (projected_.Dimensionality() == 0)
  {
    // Point data
    ApplyViewerColorMap(projected_, colored_, o);
  }
  else if (projected_.Dimensionality() == 1)
  {
    // Line data
    Image line;
    line = Image({projected_.Size(0), 100}, 3, DT_UINT8);
    line.Fill(0);

    dip::uint width = line.Size(0);
    dip::uint xstride = (dip::uint)line.Stride(0);
    dip::uint ystride = (dip::uint)line.Stride(1);
    dip::uint8 *col = (dip::uint8*)line.Origin();

    GenericImageIterator<> it(projected_);
    for( dip::uint ii = 0; ii < width; ++ii, ++it, col += xstride)
    {
      if (o.lut_ == ViewingOptions::LookupTable::RGB)
      {
        for (dip::uint kk=0; kk < 3; ++kk)
          if (o.color_elements_[kk] != -1)
          {
            dip::uint8 color = (dip::uint8)rangeMap(it[(dip::uint)o.color_elements_[kk]], o);
            dip::uint height = 99-color*100U/256;
            for (dip::uint jj=height; jj < 100; ++jj)
              col[jj*ystride+kk] = 255;
          }
      }
      else
      {
        dip::uint8 color = (dip::uint8)rangeMap(it[o.element_], o);
        dip::uint height = 99-color*100U/256;
        for (dip::uint jj=height; jj < 100; ++jj)
          for (dip::uint kk=0; kk < 3; ++kk)
            col[jj*ystride+kk] = 255;
      }
    }

    colored_ = line;

    // For left view, show vertically.
    if (o.dims_[dimx_] == -1)
    {
      colored_.PermuteDimensions({1, 0});
      colored_.ForceNormalStrides();
    }
  }
  else
  {
    // Image data
    if (o.lut_ == ViewingOptions::LookupTable::ColorSpace)
    {
      try {
        csm_.Convert(projected_, colored_, "sRGB");
      } catch (ParameterError const&) {
        // We cannot convert color space, likely it is unknown or requires a different number of tensor elements than the image has.
        colored_ = projected_;
        colored_.ResetColorSpace();
      }
      colored_.Convert(dip::DT_UINT8);
      colored_.ForceNormalStrides();
    }
    else
    {
      // Avoid overwriting shared image data
      if (colored_.IsShared())
        colored_.Strip();

      ApplyViewerColorMap(projected_, colored_, o);
    }
  }
}

void SliceView::rebuild()
{
  if (!dirty_)
    return;
  else
    dirty_ = false;

  if (!texture_)
    glGenTextures(1, &texture_);

  // Set texture
  glBindTexture( GL_TEXTURE_2D, texture_ );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  if (colored_.IsForged() && colored_.HasContiguousData())
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)colored_.Size(0), (GLsizei)colored_.Size(1), 0, GL_RGB, GL_UNSIGNED_BYTE, colored_.Origin());
}

void SliceView::render()
{
  // Image
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture_);

  // Get width and height from texture, to avoid race condition on colored_
  int width=0, height=0;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

  if (!width || !height)
  {
    std::cerr << "Cannot draw" <<std::endl;
    return;
  }

  glBegin(GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex2i(0, 0);
    glTexCoord2d(1.0,0.0); glVertex2i(width, 0);
    glTexCoord2d(1.0,1.0); glVertex2i(width, height);
    glTexCoord2d(0.0,1.0); glVertex2i(0, height);
  glEnd();

  glDisable(GL_TEXTURE_2D);

  auto o = viewport()->viewer()->options().operating_point_;
  dip::sint dx = viewport()->viewer()->options().dims_[dimx_];
  dip::sint dy = viewport()->viewer()->options().dims_[dimy_];

  // Current position
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_LINES);
    if (dx != -1)
    {
      glVertex2f((GLfloat)o[(dip::uint)dx]+0.5f, 0.);
      glVertex2f((GLfloat)o[(dip::uint)dx]+0.5f, (GLfloat)height);
    }
    if (dy != -1)
    {
      glVertex2f(0., (GLfloat)o[(dip::uint)dy]+0.5f);
      glVertex2f((GLfloat)width, (GLfloat)o[(dip::uint)dy]+0.5f);
    }
  glEnd();

  auto ro = viewport()->viewer()->options().roi_origin_;
  auto rs = viewport()->viewer()->options().roi_sizes_;

  // ROI
  if (viewport()->viewer()->options().projection_ != ViewingOptions::Projection::None)
  {
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_LINES);
      if (dx != -1 || dy == -1)
      {
        GLfloat start=0., end=(GLfloat)height;
        glVertex2f((GLfloat)(ro[(dip::uint)dx]),                   start);
        glVertex2f((GLfloat)(ro[(dip::uint)dx]),                   end);

        glVertex2f((GLfloat)(ro[(dip::uint)dx]+rs[(dip::uint)dx]), start);
        glVertex2f((GLfloat)(ro[(dip::uint)dx]+rs[(dip::uint)dx]), end);
      }
      if (dy != -1 || dx == -1)
      {
        GLfloat start=0., end=(GLfloat)width;
        glVertex2f(start, (GLfloat)(ro[(dip::uint)dy]));
        glVertex2f(end,   (GLfloat)(ro[(dip::uint)dy]));

        glVertex2f(start, (GLfloat)(ro[(dip::uint)dy]+rs[(dip::uint)dy]));
        glVertex2f(end,   (GLfloat)(ro[(dip::uint)dy]+rs[(dip::uint)dy]));
      }
    glEnd();
  }
}

void SliceViewPort::render()
{
  auto &o = viewer()->options().origin_;
  auto &z = viewer()->options().zoom_;
  auto &l = viewer()->options().labels_;

  dip::sint dx = viewer()->options().dims_[view()->dimx()];
  dip::sint dy = viewer()->options().dims_[view()->dimy()];
  dip::dfloat odx = 0, ody = 0;
  dip::dfloat zdx = 1, zdy = 1;

  if (dx != -1) { odx = o[(dip::uint)dx]; zdx = z[(dip::uint)dx]; }
  if (dy != -1) { ody = o[(dip::uint)dy]; zdy = z[(dip::uint)dy]; }

  // GLUT origin is upper left, but GL origin is lower left.
  // In the viewer we use GLUT coordinates, so here we set
  // up an inverted Y GL projection matrix.

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height_, width_, height_);
  glOrtho(0, width_, height_, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);

  int width = width_, height = height_;

  glColor3f(1., 1., 1.);
  if (view()->dimx() == 0)
  {
    char buf[] = {dy==-1?'-':l[static_cast<dip::uint>(dy)%l.size()], 0};

    glRasterPos2i((GLint)width_-(CHAR_WIDTH+1), (GLint)height_/2-(CHAR_HEIGHT/2));
    viewer()->drawString(buf);
    width -= DIM_WIDTH;
  }
  if (view()->dimy() == 1)
  {
    char buf[] = {dx==-1?'-':l[static_cast<dip::uint>(dx)%l.size()], 0};

    glRasterPos2i((GLint)width_/2-(CHAR_WIDTH/2), (GLint)height_-(CHAR_HEIGHT/2));
    viewer()->drawString(buf);
    height -= DIM_HEIGHT;
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height, width, height);
  glOrtho(odx, odx + (dip::dfloat)width/zdx,
          ody + (dip::dfloat)height/zdy, ody, -1, 1);

  glMatrixMode(GL_MODELVIEW);

  view()->render();
}

void SliceViewPort::click(int button, int state, int x, int y, int mods)
{
  dip::sint dx = viewer()->options().dims_[view()->dimx()],
            dy = viewer()->options().dims_[view()->dimy()];

  if (state == 0)
  {
    bool nearXAxis = view()->dimy() == 1 && y > y_+height_-2*DIM_HEIGHT;
    bool nearYAxis = view()->dimx() == 0 && x > x_+width_-2*DIM_WIDTH;
    double ix, iy, nix, niy;

    screenToView(x, y, &ix, &iy);

    if (button == 0)
    {
      if (mods == KEY_MOD_SHIFT)
      {
        // Shift-Left mouse button: adjust projection ROI
        dip::UnsignedArray start = viewer()->options().roi_origin_;
        dip::UnsignedArray end = start;
        end += viewer()->options().roi_sizes_;

        // Find closest edge
        roi_dim_ = dx;
        double dist = std::numeric_limits<double>::infinity();
        if (dx != -1)
        {
          double dstart = std::abs(ix - (double)start[(dip::uint)dx]), dend = std::abs(ix-(double)end[(dip::uint)dx]);
          dist = std::min(dstart, dend);
          if (dstart < dend)
            roi_edge_ = 0;
          else
            roi_edge_ = 1;
        }
        if (dy != -1)
        {
          double dstart = std::abs(iy - (double)start[(dip::uint)dy]), dend = std::abs(iy-(double)end[(dip::uint)dy]);
          if (std::min(dstart, dend) < dist)
          {
            roi_dim_ = dy;
            if (dstart < dend)
              roi_edge_ = 0;
            else
              roi_edge_ = 1;
          }
        }

        if (roi_dim_ != -1)
        {
          roi_start_ = (dip::sint)start[(dip::uint)roi_dim_];
          roi_end_   = (dip::sint)end[(dip::uint)roi_dim_];
        }
      }
      else
      {
        // Left mouse button: change operating point
        if (dx != -1)
          viewer()->options().operating_point_[(dip::uint)dx] = (dip::uint)std::min(std::max(ix-0.0, 0.), (double)viewer()->image().Size((dip::uint)dx)-1.);

        if (dy != -1)
          viewer()->options().operating_point_[(dip::uint)dy] = (dip::uint)std::min(std::max(iy-0.0, 0.), (double)viewer()->image().Size((dip::uint)dy)-1.);
      }

      viewer()->options().status_ = "";
      viewer()->refresh();
      viewer_->updateLinkedViewers();
    }

    if (button == 2)
    {
      // Right mouse button
      if (mods == KEY_MOD_SHIFT)
      {
        // Reset ROI
        viewer()->options().roi_origin_ = dip::UnsignedArray(viewer()->image().Dimensionality(), 0);
        viewer()->options().roi_sizes_ = viewer()->image().Sizes();

        viewer()->options().status_ = "Reset projection ROI";
      }
      else
      {
        // Change visualized dimension
        auto &d = viewer()->options().dims_;

        if (nearYAxis)
        {
          // Change vertical dimension
          dy++;
          if (view()->dimy() == 1)
            while (dy == d[0] || dy == d[2] || dy == d[3]) dy++;
          else
            while (dy == d[0] || dy == d[1] || dy == d[3]) dy++;

          if (dy >= (int)viewer()->options().operating_point_.size())
            dy = -1;

          d[view()->dimy()] = dy;
        }
        else if (nearXAxis)
        {
          // Change horizontal dimension
          dx++;
          if (view()->dimx() == 0)
            while (dx == d[1] || dx == d[2] || dx == d[3]) dx++;
          else
            while (dx == d[0] || dx == d[1] || dx == d[2]) dx++;

          if (dx >= (int)viewer()->options().operating_point_.size())
            dx = -1;

          d[view()->dimx()] = dx;
        }
      }

      viewer()->refresh();
    }

    if (button == 3 || button == 4)
    {
      // Mouse wheel: zoom
      double factor = std::sqrt(2.0);
      if (button == 4)
        factor = 1./factor;

      if (view()->dimy() == 1 && dx != -1 && !nearYAxis) viewer()->options().zoom_[(dip::uint)dx] *= factor;
      if (view()->dimx() == 0 && dy != -1 && !nearXAxis) viewer()->options().zoom_[(dip::uint)dy] *= factor;

      if (view()->dimx() == 0 && view()->dimy() == 1)
      {
        // In main window, zoom around current position
        screenToView(x, y, &nix, &niy);
        if (dx != -1) viewer()->options().origin_[(dip::uint)dx] += ix - nix;
        if (dy != -1) viewer()->options().origin_[(dip::uint)dy] += iy - niy;
      }
      else
      {
        // In side projections, keep origin at original place.
        if (view()->dimy() == 1 && dx != -1) viewer()->options().origin_[(dip::uint)dx] /= factor;
        if (view()->dimx() == 0 && dy != -1) viewer()->options().origin_[(dip::uint)dy] /= factor;
      }

      viewer()->options().status_ = "Zoom set to " + to_string(viewer()->options().zoom_) + ". Reset with Ctrl-1.";
      viewer()->refresh();
      viewer_->updateLinkedViewers();
    }

    drag_x_ = x;
    drag_y_ = y;
    drag_mods_ = mods;
  }
}

void SliceViewPort::motion(int button, int x, int y)
{
  double ix, iy;
  screenToView(x, y, &ix, &iy);
  dip::sint dx = viewer()->options().dims_[view()->dimx()],
            dy = viewer()->options().dims_[view()->dimy()];

  if (button == 0)
  {
    if (drag_mods_ == KEY_MOD_SHIFT)
    {
      // Change ROI
      if (roi_dim_ != -1 && viewer()->options().projection_ != ViewingOptions::Projection::None)
      {
        double dix, diy;
        screenToView(drag_x_, drag_y_, &dix, &diy);

        int d = (int)((roi_dim_==dx)?(ix-dix):(iy-diy));

        dip::uint start = std::min((dip::uint)std::max< dip::sint >(roi_start_+d*(1-roi_edge_), 0), viewer()->image().Size((dip::uint)roi_dim_));
        dip::uint end   = std::min((dip::uint)std::max< dip::sint >(roi_end_  +d*(  roi_edge_), 0), viewer()->image().Size((dip::uint)roi_dim_));

        if (start == end)
        {
          if (end == viewer()->image().Size((dip::uint)roi_dim_))
            start--;
          else
            end++;
        }

        viewer()->options().roi_origin_[(dip::uint)roi_dim_] = std::min(start, end);
        viewer()->options().roi_sizes_[(dip::uint)roi_dim_] = (dip::uint)std::abs((dip::sint)start-(dip::sint)end);

        std::ostringstream oss;
        oss << "Projection ROI set to " << viewer()->options().roi_origin_ << "+" << viewer()->options().roi_sizes_ << ". Reset with Ctrl-R.";
        viewer()->options().status_ = oss.str();
      }
    }
    else
    {
      // Left mouse button: change operating point
      if (dx != -1)
        viewer()->options().operating_point_[(dip::uint)dx] = (dip::uint)std::min(std::max(ix-0.0, 0.), (double)viewer()->image().Size((dip::uint)dx)-1.);
      if (dy != -1)
        viewer()->options().operating_point_[(dip::uint)dy] = (dip::uint)std::min(std::max(iy-0.0, 0.), (double)viewer()->image().Size((dip::uint)dy)-1.);
    }

    viewer()->refresh();
    viewer_->updateLinkedViewers();
  }

  if (button == 1)
  {
    // Middle mouse button: change split
    int dx = x-drag_x_, dy = y-drag_y_;

    viewer()->options().split_[0] = std::min(std::max((int)viewer()->options().split_[0] + dx, 100), viewer()->width()-200);
    viewer()->options().split_[1] = std::min(std::max((int)viewer()->options().split_[1] + dy, 100), viewer()->height()-100);

    drag_x_ = x;
    drag_y_ = y;
  }

  if (button == 2)
  {
    // Right mouse button: drag
    double dix, diy;
    screenToView(drag_x_, drag_y_, &dix, &diy);

    if (dx != -1) viewer()->options().origin_[(dip::uint)dx] += (dix-ix);
    if (dy != -1) viewer()->options().origin_[(dip::uint)dy] += (diy-iy);

    drag_x_ = x;
    drag_y_ = y;

    viewer()->refresh();
    viewer_->updateLinkedViewers();
  }
}

void SliceViewPort::screenToView(int x, int y, double *ix, double *iy)
{
  dip::sint dx = viewer()->options().dims_[view()->dimx()];
  dip::sint dy = viewer()->options().dims_[view()->dimy()];

  *ix = *iy = 0;

  if (dx != -1)
    *ix = (x-x_)/viewer()->options().zoom_[(dip::uint)dx] + viewer()->options().origin_[(dip::uint)dx];
  if (dy != -1)
    *iy = (y-y_)/viewer()->options().zoom_[(dip::uint)dy] + viewer()->options().origin_[(dip::uint)dy];
}

SliceViewer::SliceViewer(const dip::Image &image, std::string name, dip::uint width, dip::uint height)
  : Viewer(name), options_(image), continue_(false), updated_(false), original_(image), drag_viewport_(NULL), refresh_seq_(0)
{
  if (width && height)
    requestSize(width, height);

  main_ = new SliceViewPort(this);
  main_->setView(new SliceView(main_, 0, 1));
  viewports_.push_back(main_);

  left_ = new SliceViewPort(this);
  left_->setView(new SliceView(left_, 2, 1));
  viewports_.push_back(left_);

  top_ = new SliceViewPort(this);
  top_->setView(new SliceView(top_, 0, 3));
  viewports_.push_back(top_);

  tensor_ = new TensorViewPort(this);
  viewports_.push_back(tensor_);

  control_ = new ControlViewPort(this);
  viewports_.push_back(control_);

  histogram_ = new HistogramViewPort(this);
  viewports_.push_back(histogram_);

  status_ = new StatusViewPort(this);
  viewports_.push_back(status_);

  link_ = new LinkViewPort(this);
  viewports_.push_back(link_);
}

void SliceViewer::create()
{
  if (continue_)
  {
    // Deal with recreating OpenGL context
    continue_ = false;
    thread_.join();
    updated_ = false;
  }

  setWindowTitle("");

  continue_ = true;
  thread_ = std::thread(&SliceViewer::calculateTextures, this);

  // Wait for first projection
  while (!updated_)
     std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

void SliceViewer::place()
{
  options_.split_[0] = std::min(std::max((int)options_.split_[0], 100), width()-200);
  options_.split_[1] = std::min(std::max((int)options_.split_[1], 100), height()-100);

  int splitx = (int)options_.split_[0];
  int splity = (int)options_.split_[1];

  main_->place     (splitx     , splity             , width()-100-splitx, height()-splity-DIM_HEIGHT);
  left_->place     (0          , splity             , splitx            , height()-splity-DIM_HEIGHT);
  top_->place      (splitx     , 0                  , width()-100-splitx, splity);
  tensor_->place   (0          , 0                  , splitx            , splity);
  control_->place  (width()-100, 0                  , 100               , splity);
  histogram_->place(width()-100, splity             , 100               , height()-splity-DIM_HEIGHT);
  status_->place   (0          , height()-DIM_HEIGHT, width()-100       , DIM_HEIGHT);
  link_->place     (width()-100, height()-DIM_HEIGHT, 100               , DIM_HEIGHT);
}

void SliceViewer::reshape(int /*width*/, int /*height*/)
{
  Guard guard(*this);
  place();
}

void SliceViewer::draw()
{
  Guard guard(*this);
  // Actual drawing
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (updated_)
  {
    for (dip::uint ii=0; ii < viewports_.size(); ++ii)
      viewports_[ii]->rebuild();
    updated_ = false;
  }

  for (dip::uint ii=0; ii < viewports_.size(); ++ii)
    viewports_[ii]->render();
  swap();
}

void SliceViewer::key(unsigned char k, int x, int y, int mods)
{
  Guard guard(*this);
  Viewer::key(k, x, y, mods);

  auto &dims = options_.dims_;
  auto &zoom = options_.zoom_;

  if (!mods)
  {
    if (k >= '1' && k <= '9')
    {
      dip::sint idx = k - '1';

      if (image_.TensorElements() > (dip::uint) idx)
      {
        if (options_.lut_ == ViewingOptions::LookupTable::RGB)
        {
          auto &c = options_.color_elements_;

          // Select and deselect tensor elements to visualize. Clicking on a
          // selected element deselects it, while clicking on an unselected
          // element assigns the first available color from {R, G, B}.
               if (c[0] == idx) c[0] = -1;
          else if (c[1] == idx) c[1] = -1;
          else if (c[2] == idx) c[2] = -1;
          else if (c[0] == -1 ) c[0] = idx;
          else if (c[1] == -1 ) c[1] = idx;
          else if (c[2] == -1 ) c[2] = idx;
        }
        else
          options_.element_ = (dip::uint) idx;
      }
    }

    if (k == 'D' && image_.Dimensionality() > 0 && options_.operating_point_[0] < image_.Size(0)-1)
      options_.operating_point_[0]++;
    if (k == 'A' && image_.Dimensionality() > 0 && options_.operating_point_[0] > 0)
      options_.operating_point_[0]--;
    if (k == 'S' && image_.Dimensionality() > 1 && options_.operating_point_[1] < image_.Size(1)-1)
      options_.operating_point_[1]++;
    if (k == 'W' && image_.Dimensionality() > 1 && options_.operating_point_[1] > 0)
      options_.operating_point_[1]--;
    if (k == 'N' && image_.Dimensionality() > 2 && options_.operating_point_[2] < image_.Size(2)-1)
      options_.operating_point_[2]++;
    if (k == 'P' && image_.Dimensionality() > 2 && options_.operating_point_[2] > 0)
      options_.operating_point_[2]--;
    if (k == 'F' && image_.Dimensionality() > 3 && options_.operating_point_[3] < image_.Size(3)-1)
      options_.operating_point_[3]++;
    if (k == 'B' && image_.Dimensionality() > 3 && options_.operating_point_[3] > 0)
      options_.operating_point_[3]--;

    options_.status_ = "";
    refresh();
    updateLinkedViewers();
  }

  if (mods == KEY_MOD_CONTROL)
  {
    if (k == '1')
    {
      // ^1: 1:1 zoom
      zoom = image_.AspectRatio();

      for (dip::uint ii=0; ii < image_.Dimensionality(); ++ii)
      {
        options_.origin_[ii] = 0;
        if (zoom[ii] == 0)
          zoom[ii] = 1;
      }

      options_.status_ = "Zoom reset to 1:1";
      refresh();
      updateLinkedViewers();
    }

    if (k == 'F')
    {
      // ^F: fit window
      for (dip::uint ii=0; ii < image_.Dimensionality(); ++ii)
      {
        options_.origin_[ii] = 0;
        zoom[ii] = std::numeric_limits<dip::dfloat>::max();
      }

      for (dip::uint ii=0; ii < 4; ++ii)
      {
        int sz;
        switch (ii)
        {
          case 0: sz = main_->width()-DIM_WIDTH; break;
          case 1: sz = main_->height()-DIM_HEIGHT; break;
          case 2: sz = left_->width(); break;
          default: // avoid compiler warning
          case 3: sz = top_->height(); break;
        }

        if (dims[ii] != -1)
          zoom[(dip::uint)dims[ii]] = std::min(zoom[(dip::uint)dims[ii]], (dip::dfloat)sz/(dip::dfloat)image_.Size((dip::uint)dims[ii]));
      }

      for (dip::uint ii=0; ii < image_.Dimensionality(); ++ii)
        if (zoom[ii] == std::numeric_limits<dip::dfloat>::max())
          zoom[ii] = 1.;

      // Keep XY aspect ratio
      dip::sint dx = dims[0], dy = dims[1];
      if (dx != -1 && dy != -1)
      {
        dip::dfloat aspect_image = (dip::dfloat)image_.Size((dip::uint)dx)/(dip::dfloat)image_.Size((dip::uint)dy),
                    aspect_viewport = (dip::dfloat)(main_->width()-DIM_WIDTH)/(dip::dfloat)(main_->height()-DIM_HEIGHT);

        if (aspect_image > aspect_viewport)
          zoom[(dip::uint)dy] = zoom[(dip::uint)dx];
        else
          zoom[(dip::uint)dx] = zoom[(dip::uint)dy];
      }

      options_.status_ = "Zoom set to fit window: " + to_string(options_.zoom_) + ". Reset with Ctrl-1.";
      refresh();
      updateLinkedViewers();
    }

    if (k == 'L')
    {
      // ^L: linear stretch
      options_.mapping_ = ViewingOptions::Mapping::Linear;
      options_.setAutomaticRange();

      options_.status_ = "Mapping set to " + options_.getMappingDescription() + ": range [" + std::to_string(options_.mapping_range_.first) + ", " + std::to_string(options_.mapping_range_.second) + "]";
      refresh();
    }

    if (k == 'R')
    {
      // ^R: reset ROI
      options_.roi_origin_ = dip::UnsignedArray(image_.Dimensionality(), 0);
      options_.roi_sizes_ = image_.Sizes();

      options_.status_ = "Reset projection ROI";
      refresh();
    }

    if (k == 'N')
    {
      // ^N: Clone
      Ptr sv = clone();

      link_->link(sv->link_);
      sv->link_->link(link_);

      manager()->createWindow(sv);
    }
  }
}

void SliceViewer::click(int button, int state, int x, int y, int mods)
{
  Guard guard(*this);
  drag_viewport_ = viewport(x, y);

  if (state == 0)
    drag_button_ = button;
  else
    drag_button_ = -1;

  if (drag_viewport_)
    drag_viewport_->click(button, state, x, y, mods);
}

void SliceViewer::motion(int x, int y)
{
  Guard guard(*this);
  if (drag_viewport_)
    drag_viewport_->motion(drag_button_, x, y);
}

ViewPort *SliceViewer::viewport(int x, int y)
{
  for (dip::uint ii=0; ii < viewports_.size(); ++ii)
  {
    ViewPort *v = viewports_[ii];

    if (x >= v->x() && x < v->x() + v->width() &&
        y >= v->y() && y < v->y() + v->height())
      return v;
  }

  return NULL;
}

void SliceViewer::calculateTextures()
{
  ViewingOptions options, old_options;
  int seq = -1;

  while (continue_)
  {
    // Make sure we don't lose updates
    while (updated_ && continue_)
       std::this_thread::sleep_for(std::chrono::microseconds(1000));

    if (!continue_)
      break;

    old_options = options;
    lock();
    ViewingOptions::Diff diff = options.diff(options_);
    if (seq != refresh_seq_)
      diff = ViewingOptions::Diff::Complex;
    options = options_;
    seq = refresh_seq_;
    unlock();

    // Calculate textures
    if (diff >= ViewingOptions::Diff::Complex)
    {
      lock();
      dip::Image original = original_, image;
      unlock();

      // Deal with complex numbers
      if (original.DataType().IsComplex())
      {
        switch (options.complex_)
        {
          case ViewingOptions::ComplexToReal::Real:
            image = original.Real();
            break;
          case ViewingOptions::ComplexToReal::Imaginary:
            image = original.Imaginary();
            break;
          case ViewingOptions::ComplexToReal::Magnitude:
            image = Abs(original);
            break;
          case ViewingOptions::ComplexToReal::Phase:
            image = Phase(original);
            break;
        }
      }
      else
        image = original;

      // Get ranges
      FloatRange range = { std::numeric_limits<dip::dfloat>::infinity(),
                          -std::numeric_limits<dip::dfloat>::infinity()};
      FloatRangeArray tensor_range(image.TensorElements());

      for (dip::uint ii=0; ii != image.TensorElements(); ++ii)
      {
        dip::MinMaxAccumulator acc = MaximumAndMinimum( image[ii], IsFinite(image[ii]) );
        tensor_range[ii] = {acc.Minimum(), acc.Maximum()};
        range = {std::min(range.first, tensor_range[ii].first),
                 std::max(range.second, tensor_range[ii].second)};
      }

      lock();
      image_ = image;
      options_.range_ = range;
      options_.tensor_range_ = tensor_range;

      if (options.mapping_ == ViewingOptions::Mapping::Linear ||
          options.mapping_ == ViewingOptions::Mapping::Symmetric ||
          options.mapping_ == ViewingOptions::Mapping::Logarithmic)
      {
        // If we're on some automatic mapping more, adjust it.
        options_.setAutomaticRange();
      }

      // Adjust model in case image properties changed
      options_.operating_point_.resize(image.Dimensionality(), 0);
      options_.roi_origin_.resize(image.Dimensionality(), 0);
      options_.roi_sizes_.resize(image.Dimensionality(), 0);
      options_.zoom_.resize(image.Dimensionality(), 0);
      options_.origin_.resize(image.Dimensionality(), 0.);
      options_.offset_ = dip::PhysicalQuantityArray(image.Dimensionality());

      for (dip::uint ii=0; ii != 4; ++ii)
        if (options_.dims_[ii] >= (dip::sint)image.Dimensionality())
          options_.dims_[ii] = -1;

      for (dip::uint ii=0; ii != image.Dimensionality(); ++ii)
      {
        if (options_.operating_point_[ii] >= image.Size(ii))
          options_.operating_point_[ii] = image.Size(ii)-1;

        if (options_.roi_origin_[ii] >= image.Size(ii))
          options_.roi_origin_[ii] = image.Size(ii)-1;

        if (options_.roi_sizes_[ii] == 0)
        {
          options_.roi_origin_[ii] = 0;
          options_.roi_sizes_[ii] = image.Size(ii);
        }

        if (options_.roi_origin_[ii] + options_.roi_sizes_[ii] > image.Size(ii))
          options_.roi_sizes_[ii] = image.Size(ii) - options_.roi_origin_[ii];

        if (options_.zoom_[ii] == 0)
          options_.zoom_[ii] = 1;

        options_.offset_[ii] = 0 * image.PixelSize(ii);
      }

      if (options_.element_ >= image.TensorElements())
        options_.element_ = 0;

      for (dip::uint ii=0; ii != 3; ++ii)
        if (options_.color_elements_[ii] >= (dip::sint)image.TensorElements())
          options_.color_elements_[ii] = -1;

      unlock();

      // Recalculate histogram
      histogram_->calculate();
    }

    if (diff >= ViewingOptions::Diff::Projection)
    {
      // Need to reproject
      if (old_options.needsReproject(options, main_->view()->dimx(), main_->view()->dimy()) || diff >= ViewingOptions::Diff::Complex)
        main_->view()->project();
      if (old_options.needsReproject(options, left_->view()->dimx(), left_->view()->dimy()) || diff >= ViewingOptions::Diff::Complex)
        left_->view()->project();
      if (old_options.needsReproject(options, top_->view()->dimx(), top_->view()->dimy()) || diff >= ViewingOptions::Diff::Complex)
        top_->view()->project();
    }

    if (diff == ViewingOptions::Diff::Mapping)
    {
      // Need to remap
      main_->view()->map();
      left_->view()->map();
      top_->view()->map();
    }

    if (diff >= ViewingOptions::Diff::Place)
    {
      // Need to replace viewports
      place();
    }

    if (diff >= ViewingOptions::Diff::Draw)
    {
      // Just redraw
      updated_ = true;
    }

    if (diff != ViewingOptions::Diff::None)
      refresh();

    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }
}

SliceViewer::~SliceViewer() = default;

void SliceViewer::updateLinkedViewers()
{
  Guard guard(*this);
  link_->update();
}

void SliceViewer::link(SliceViewer &other)
{
  Guard guard(*this);
  Guard guard2(other);

  DIP_THROW_IF( other.image().Sizes() != image().Sizes(), E::DIMENSIONALITIES_DONT_MATCH );

  // Take settings from link source
  link_->update(other.options());

  // Perform link
  link_->link(other.link_);
  other.link_->link(link_);
}

}} // namespace dip::viewer
