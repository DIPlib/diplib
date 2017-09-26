/*
 * DIPlib 3.0 viewer
 * This file contains functionality for the nD image slice viewer.
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
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/generic_iterators.h"
#include "diplib/overload.h"
#include "diplib/viewer/include_gl.h"
#include "diplib/viewer/slice.h"

#define CHAR_WIDTH   8
#define CHAR_HEIGHT 13

#define DIM_WIDTH   (CHAR_WIDTH+2)
#define DIM_HEIGHT  (CHAR_HEIGHT+2)

namespace dip { namespace viewer {

void SliceView::project()
{
  auto &o = viewport()->viewer()->options();
  Image &image = viewport()->viewer()->image();
  
  dip::sint dx = o.dims_[dimx_], dy = o.dims_[dimy_];
  
  // For projection  
  BooleanArray process( image.Dimensionality(), true );
  if (dx != -1) process[ (dip::uint)dx ] = false;
  if (dy != -1) process[ (dip::uint)dy ] = false;
  
  // For extraction
  RangeArray range(image.Dimensionality());
  for (size_t ii=0; ii < range.size(); ++ii)
    if ((int)ii != dx && (int)ii != dy)
      range[ii] = Range((dip::sint)o.operating_point_[ii]);
  
  switch (o.projection_)
  {
    case ViewingOptions::Projection::None:
      projected_ = image.At(range);
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
  
  if (dx == -1 || dy == -1)
    projected_.Squeeze();
  else
    projected_.PermuteDimensions({(unsigned int)dx, (unsigned int)dy});
    
  map();
}

void SliceView::map()
{
  auto &o = viewport()->viewer()->options();
  
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
    
    dip::uint width = projected_.Size( 0 );
    
    GenericImageIterator<> it(projected_);
    for( dip::uint ii = 0; ii < width; ++ii, ++it)
    {
      if (o.lut_ == ViewingOptions::LookupTable::RGB)
      {
        for (size_t kk=0; kk < 3; ++kk)
          if (o.color_elements_[kk] != -1)
          {
            dip::uint8 color = rangeMap(it[(dip::uint)o.color_elements_[kk]], o);
            line.At<dip::uint8>({ii, 99-color*100U/256})[kk] = 255;
          }
      }
      else
      {
        dip::uint8 color = (dip::uint8)(rangeMap(it[o.element_], o)*255);
      
        auto p = line.At<dip::uint8>({ii, 99-color*100U/256});
        for (size_t kk=0; kk < 3; ++kk)
          p[kk] = 255;
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
      csm_.Convert(projected_, colored_, "RGB");
      colored_.Convert(dip::DT_UINT8);
      colored_.ForceNormalStrides();
    } 
    else
      ApplyViewerColorMap(projected_, colored_, o);
  }
}

void SliceView::rebuild()
{
  if (!texture_)
    glGenTextures(1, &texture_);
    
  // Set texture
  glBindTexture( GL_TEXTURE_2D, texture_ );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  
  if (colored_.IsForged() && colored_.HasContiguousData())
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)colored_.Size(0), (GLsizei)colored_.Size(1), 0, GL_RGB, GL_UNSIGNED_BYTE, colored_.Origin());
}

void SliceView::render()
{
  // Image
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture_);
  
  glBegin(GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex2i(0, 0);
    glTexCoord2d(1.0,0.0); glVertex2i((GLint)colored_.Size(0), 0);
    glTexCoord2d(1.0,1.0); glVertex2i((GLint)colored_.Size(0), (GLint)colored_.Size(1));
    glTexCoord2d(0.0,1.0); glVertex2i(0, (GLint)colored_.Size(1));
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
      glVertex2f((GLfloat)o[(dip::uint)dx]+0.5f, (GLfloat)colored_.Size(1));
    }
    if (dy != -1)
    {
      glVertex2f(0., (GLfloat)o[(dip::uint)dy]+0.5f);
      glVertex2f((GLfloat)colored_.Size(0), (GLfloat)o[(dip::uint)dy]+0.5f);
    }
  glEnd();
}

void SliceViewPort::render()
{
  auto &o = viewer()->options().origin_;
  auto &z = viewer()->options().zoom_;
  
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
  char dimchars[] = "xyzw56789)!@#$%%^&*()";
  
  glColor3f(1., 1., 1.);
  if (view()->dimx() == 0)
  {
    char buf[] = {dy==-1?'-':dimchars[dy], 0};

    glRasterPos2i((GLint)width_-(CHAR_WIDTH+1), (GLint)height_/2-(CHAR_HEIGHT/2));
    viewer()->drawString(buf);
    width -= DIM_WIDTH;
  }
  if (view()->dimy() == 1)
  {
    char buf[] = {dx==-1?'-':dimchars[dx], 0};
    
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

void SliceViewPort::click(int button, int state, int x, int y)
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
      // Left mouse button: change operating point
      if (dx != -1)
        viewer()->options().operating_point_[(dip::uint)dx] = (dip::uint)std::min(std::max(ix-0.0, 0.), (double)view()->size(0)-1.);

      if (dy != -1)
        viewer()->options().operating_point_[(dip::uint)dy] = (dip::uint)std::min(std::max(iy-0.0, 0.), (double)view()->size(1)-1.);

      viewer()->refresh();
    }
    
    if (button == 2)
    {
      // Right mouse button: change visualized dimension
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

      viewer()->refresh();
    }
    
    if (button == 3 || button == 4)
    {
      // Mouse wheel: zoom
      double factor = 1.5;
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

      viewer()->refresh();
    }
    
    drag_x_ = x;
    drag_y_ = y;
  }
  
  if (state == 1)
  {
    if (button == 0)
    {
      // Set title to current position and value. We do it here because
      // somehow calling glutSetWindowTitle is very slow and we can't do
      // it every draw.
      auto op = viewer()->options().operating_point_;
      auto te = viewer()->image().TensorElements();
      
      std::ostringstream oss;
      oss << " (";
      for (dip::uint ii=0; ii < op.size(); ++ii)
      {
        oss << op[ii];
        if (ii < op.size()-1)
          oss << ", ";
      }
      oss << "): ";
      if (te > 1) oss << "[";
      for (dip::uint ii=0; ii < te; ++ii)
      {
        oss << (dip::dfloat) viewer()->image().At(op)[ii];
        if (ii < te-1)
          oss << ", ";
      }
      if (te > 1) oss << "]";

      viewer()->setWindowTitle(oss.str().c_str());
    }
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
    // Left mouse button: change operating point
    if (dx != -1)
      viewer()->options().operating_point_[(dip::uint)dx] = (dip::uint)std::min(std::max(ix-0.0, 0.), (double)view()->size(0)-1.);
    if (dy != -1)
      viewer()->options().operating_point_[(dip::uint)dy] = (dip::uint)std::min(std::max(iy-0.0, 0.), (double)view()->size(1)-1.);

    viewer()->refresh();
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

SliceViewer::SliceViewer(const dip::Image &image, std::string name, size_t width, size_t height) : Viewer(name), options_(image), continue_(false), updated_(false), original_(image), image_(image), drag_viewport_(NULL)
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
}

void SliceViewer::create()
{
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

  main_->place     (splitx     , splity, width()-100-splitx, height()-splity);
  left_->place     (0          , splity, splitx            , height()-splity);
  top_->place      (splitx     , 0     , width()-100-splitx, splity);
  tensor_->place   (0          , 0     , splitx            , splity);
  control_->place  (width()-100, 0     , 100               , splity);
  histogram_->place(width()-100, splity, 100               , height()-splity);
}

void SliceViewer::reshape(int /*width*/, int /*height*/)
{
  place();
}

void SliceViewer::draw()
{
  // Actual drawing
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  if (updated_)
  {
    for (size_t ii=0; ii < viewports_.size(); ++ii)
      viewports_[ii]->rebuild();
    updated_ = false;
  }
  
  for (size_t ii=0; ii < viewports_.size(); ++ii)
    viewports_[ii]->render();
  swap();
}

void SliceViewer::key(unsigned char k, int x, int y, int mods)
{
  Viewer::key(k, x, y, mods);
  
  auto &dims = options_.dims_;
  auto &zoom = options_.zoom_;

  if (!mods)
  {
    if (k == 'N' && image_.Dimensionality() > 2 && options_.operating_point_[2] < image_.Size(2)-1)
      options_.operating_point_[2]++;
    if (k == 'P' && image_.Dimensionality() > 2 && options_.operating_point_[2] > 0)
      options_.operating_point_[2]--;
    if (k == 'F' && image_.Dimensionality() > 3 && options_.operating_point_[3] < image_.Size(3)-1)
      options_.operating_point_[3]++;
    if (k == 'B' && image_.Dimensionality() > 3 && options_.operating_point_[3] > 0)
      options_.operating_point_[3]--;
      
    refresh();
  }
  
  if (mods == KEY_MOD_CONTROL)
  {
    if (k == '1')
    {
      // ^1: 1:1 zoom
      zoom = image_.AspectRatio();
      
      for (size_t ii=0; ii < image_.Dimensionality(); ++ii)
      {
        options_.origin_[ii] = 0;
        if (zoom[ii] == 0)
          zoom[ii] = 1;
      }
      
      refresh();
    }
    
    if (k == 'F')
    {
      // ^F: fit window
      for (size_t ii=0; ii < image_.Dimensionality(); ++ii)
      {
        options_.origin_[ii] = 0;
        zoom[ii] = std::numeric_limits<dip::dfloat>::max();
      }
      
      for (size_t ii=0; ii < 4; ++ii)
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
      
      for (size_t ii=0; ii < image_.Dimensionality(); ++ii)
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
      
      refresh();
    }
    
    if (k == 'L')
    {
      // ^L: linear stretch
      options_.mapping_ = ViewingOptions::Mapping::Linear;
      options_.mapping_range_ = options_.range_;
      refresh();
    }
  }
}

void SliceViewer::click(int button, int state, int x, int y)
{
  drag_viewport_ = viewport(x, y);
  
  if (state == 0)
    drag_button_ = button;
  else
    drag_button_ = -1;
  
  if (drag_viewport_)
    drag_viewport_->click(button, state, x, y);
}

void SliceViewer::motion(int x, int y)
{
  if (drag_viewport_)
    drag_viewport_->motion(drag_button_, x, y);
}

ViewPort *SliceViewer::viewport(int x, int y)
{
  for (size_t ii=0; ii < viewports_.size(); ++ii)
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

  while (continue_)
  {
    // Make sure we don't lose updates
    while (updated_)
       std::this_thread::sleep_for(std::chrono::microseconds(1000));
  
    mutex_.lock();
    ViewingOptions::Diff diff = options.diff(options_);
    old_options = options;
    options = options_;
    mutex_.unlock();
    
    // Calculate textures
    if (diff >= ViewingOptions::Diff::Complex)
    {
      // Deal with complex numbers
      if (original_.DataType().IsComplex())
      {
        switch (options.complex_)
        {
          case ViewingOptions::ComplexToReal::Real:
            image_ = original_.Real();
            break;
          case ViewingOptions::ComplexToReal::Imaginary:
            image_ = original_.Imaginary();
            break;
          case ViewingOptions::ComplexToReal::Magnitude:
            image_ = Abs(original_);
            break;
          case ViewingOptions::ComplexToReal::Phase:
            image_ = Phase(original_);
            break;
        }      
      }
      else
        image_ = original_;
        
      // Get range
      dip::Image copy = image_;
      copy.TensorToSpatial();
      dip::MinMaxAccumulator acc = MaximumAndMinimum( copy );
      options_.range_ = {acc.Minimum(), acc.Maximum()};
      if (options.mapping_ == ViewingOptions::Mapping::Linear ||
          options.mapping_ == ViewingOptions::Mapping::Symmetric || 
          options.mapping_ == ViewingOptions::Mapping::Logarithmic)
      {
        // If we're on some automatic mapping more, adjust it.
        options_.mapping_range_ = options_.range_;
        
        if (options.mapping_ == ViewingOptions::Mapping::Symmetric)
        {
          if (std::abs(options_.mapping_range_.first) > std::abs(options_.mapping_range_.second))
            options_.mapping_range_.second = -options_.mapping_range_.first;
          else
            options_.mapping_range_.first = -options_.mapping_range_.second;
        }
      }
        
      // Recalculate histogram
      histogram_->calculate();
    }
    
    if (diff >= ViewingOptions::Diff::Projection)
    {
      // Need to reproject
      if (old_options.needsReproject(options, main_->view()->dimx(), main_->view()->dimy()))
        main_->view()->project();
      if (old_options.needsReproject(options, left_->view()->dimx(), left_->view()->dimy()))
        left_->view()->project();
      if (old_options.needsReproject(options, top_->view()->dimx(), top_->view()->dimy()))
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

}} // namespace dip::viewer
