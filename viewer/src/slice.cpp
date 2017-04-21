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

#include <chrono>

#include <GL/glu.h>

#include "diplib/math.h"
#include "diplib/overload.h"
#include "diplib/viewer/slice.h"

using namespace dip;

template< typename TPI >
void viewer__ColorMap(Image const& slice, Image& out, ViewingOptions &options)
{
  auto mapping = options.mapping_;   
  auto lut = options.lut_;
  auto element = options.element_;
  auto color_elements = options.color_elements_;

  dip::uint width = slice.Size( 0 );
  dip::uint height = slice.Size( 1 );
  dip::sint sliceStride0 = slice.Stride( 0 );
  dip::sint sliceStride1 = slice.Stride( 1 );
  dip::sint outStride0 = out.Stride( 0 );
  dip::sint outStride1 = out.Stride( 1 );
  dip::sint sliceStrideT = slice.TensorStride();
   
  double offset, scale;
  if( mapping == ViewingOptions::Mapping::Logarithmic )
  {
    offset = options.mapping_range_.first-1.;
    scale = 1./std::log(options.mapping_range_.second-offset);
  }
  else
  {
    offset = options.mapping_range_.first;
    scale = 1./(options.mapping_range_.second-options.mapping_range_.first);
  }

  TPI* slicePtr = static_cast< TPI* >( slice.Origin() );
  uint8* outPtr = static_cast< uint8* >( out.Origin() );
  for( dip::uint jj = 0; jj < height; ++jj, slicePtr += sliceStride1, outPtr += outStride1 )
  {
    TPI* iPtr = slicePtr;
    uint8* oPtr = outPtr;
    dip::uint ii;
     
    switch (lut)
    {
      case ViewingOptions::LookupTable::ColorSpace:
        // TODO: Need to know color space. Is there even a per-pixel version in diplib?
        for( iPtr = slicePtr, oPtr = outPtr, ii = 0; ii < width; ++ii, iPtr += sliceStride0, oPtr += outStride0)
          out[0] = out[1] = out[2] = 0;
        break;
      case ViewingOptions::LookupTable::RGB:
        for (dip::uint kk=0; kk < 3; ++kk)
        {
          dip::sint elem = color_elements[kk];
          if (elem >= 0)
            for( iPtr = slicePtr, oPtr = outPtr, ii = 0; ii < width; ++ii, iPtr += sliceStride0, oPtr += outStride0)
              oPtr[kk] = (dip::uint8)(rangeMap((dip::sfloat)iPtr[elem*sliceStrideT], offset, scale, mapping)*255);
          else
            for( iPtr = slicePtr, oPtr = outPtr, ii = 0; ii < width; ++ii, iPtr += sliceStride0, oPtr += outStride0)
              oPtr[kk] = 0;
        }
        break;
      case ViewingOptions::LookupTable::Grey:
        for( iPtr = slicePtr, oPtr = outPtr, ii = 0; ii < width; ++ii, iPtr += sliceStride0, oPtr += outStride0)
          oPtr[0] = oPtr[1] = oPtr[2] = (dip::uint8)(rangeMap((dip::sfloat)iPtr[(dip::sint)element*sliceStrideT], offset, scale, mapping)*255);
        break;
      case ViewingOptions::LookupTable::Jet:
        for( iPtr = slicePtr, oPtr = outPtr, ii = 0; ii < width; ++ii, iPtr += sliceStride0, oPtr += outStride0)
          jet(rangeMap((dip::sfloat)iPtr[(dip::sint)element*sliceStrideT], offset, scale, mapping), oPtr);
        break;
    }
  }
}

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
  
  if (projected_.Dimensionality() == 1)
  {
    // Line data
    colored_ = Image({projected_.Size(0), 100}, 3, DT_UINT8);
    colored_.Fill(0);
    
    dip::uint width = projected_.Size( 0 );
    dip::sint stride = projected_.Stride( 0 );
    dip::sint tstride = projected_.TensorStride();
    
    dip::sfloat *ptr = (dip::sfloat*)projected_.Origin();
    for( dip::uint ii = 0; ii < width; ++ii, ptr += stride)
    {
      if (o.lut_ == ViewingOptions::LookupTable::RGB)
      {
        dip::uint8 color[3];
        colorMap(ptr, tstride, color, o);
        for (size_t kk=0; kk < 3; ++kk)
          *((dip::uint8*)colored_.Pointer(UnsignedArray{ii, 99-color[kk]*100U/256})+kk) = 255;
      }
      else
      {
        dip::uint8 color = (dip::uint8)(rangeMap(ptr[(dip::sint)o.element_*tstride], o)*255);
      
        for (size_t kk=0; kk < 3; ++kk)
          *((dip::uint8*)colored_.Pointer(UnsignedArray{ii, 99-color*100U/256})+kk) = 255;
      }
    }
    
    // For left view, show vertically.
    // TODO: Doesn't work
    if (o.dims_[dimx_] == -1)
      colored_.PermuteDimensions({1, 0});
  }
  else
  {
    // Image data
    colored_ = Image(projected_.Sizes(), 3, DT_UINT8);
    
    DIP_OVL_CALL_NONCOMPLEX( viewer__ColorMap, ( projected_, colored_, o ), projected_.DataType() );
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
  gluOrtho2D(odx, odx + (dip::dfloat)view()->size(0)/zdx,
             ody + (dip::dfloat)view()->size(1)/zdy, ody);
             
  glMatrixMode(GL_MODELVIEW);
  view()->render();
}

void SliceViewPort::click(int button, int state, int x, int y)
{
  dip::sint dx = viewer()->options().dims_[view()->dimx()],
            dy = viewer()->options().dims_[view()->dimy()];

  if (state == 0)
  {
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
      // Right mouse button: change visualized dimensions
      auto &d = viewer()->options().dims_;
    
      if (x-x_ < 20 || x-x_ > width_-20)
      {
        dy++;
        if (view()->dimx() == 2)      // left
          while (dy == d[0] || dy == d[2]) dy++;
        else if (view()->dimy() == 3) // top
          while (dy == d[0]) dy++;
        else                          // main
          while (dy == d[0] || dy == d[2]) dy++;
      
        if (dy >= (int)viewer()->options().operating_point_.size())
          dy = -1;
          
        d[view()->dimy()] = dy;
      }
      else if (y-y_ < 20 || y-y_ > height_-20)
      {
        dx++;
        if (view()->dimx() == 2)      // left
          while (dx == d[1]) dx++;
        else if (view()->dimy() == 3) // top
          while (dx == d[1] || dx == d[3]) dx++;
        else                          // main
          while (dx == d[1] || dx == d[3]) dx++;
      
        if (dx >= (int)viewer()->options().operating_point_.size())
          dx = -1;
          
        d[view()->dimx()] = dx;
      }
    }
    if (button == 3 || button == 4)
    {
      // Mouse wheel: zoom
      if (button == 3)
      {
        for (size_t ii=0; ii < viewer()->options().zoom_.size(); ++ii)
          viewer()->options().zoom_[ii] *= 1.5;
          
        viewer()->refresh();
      }
      if (button == 4)
      {
        for (size_t ii=0; ii < viewer()->options().zoom_.size(); ++ii)
          viewer()->options().zoom_[ii] /= 1.5;

        viewer()->refresh();
      }
      // Zoom around current position
      screenToView(x, y, &nix, &niy);
      if (dx != -1) viewer()->options().origin_[(dip::uint)dx] += ix - nix;
      if (dy != -1) viewer()->options().origin_[(dip::uint)dy] += iy - niy;
    }
    
    drag_x_ = x;
    drag_y_ = y;
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
    // Middle mouse button: drag
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
    *ix = (x-x_)/viewer()->options().zoom_[(dip::uint)dx]*(double)view()->size(0)/(double)width_ + viewer()->options().origin_[(dip::uint)dx];

  if (dy != -1)
    *iy = (y-y_)/viewer()->options().zoom_[(dip::uint)dy]*(double)view()->size(1)/(double)height_ + viewer()->options().origin_[(dip::uint)dy];
}

SliceViewer::SliceViewer(const dip::Image &image) : options_(image), continue_(false), updated_(false), original_(image), image_(image)
{
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
  title("SliceViewer");
  
  continue_ = true;
  thread_ = std::thread(&SliceViewer::calculateTextures, this);
  
  // Wait for first projection
  while (!updated_) sleep(0);
  
  reshape(512, 512);
}

void SliceViewer::reshape(int width, int height)
{
  // TODO: Sizes aren't available yet on first call...
  int splitx = 100; //(width_-100)*left_.size(0)/main_.size(0);
  int splity = 100; //height_*top_.size(1)/main_.size(1);

  width_ = width;
  height_ = height;
  
  main_->place     (splitx    , splity, width_-100-splitx, height_-splity);
  left_->place     (0         , splity, splitx           , height_-splity);
  top_->place      (splitx    , 0     , width_-100-splitx, splity);
  tensor_->place   (0         , 0     , splitx           , splity);
  control_->place  (width_-100, 0     , 100              , splity);
  histogram_->place(width_-100, splity, 100              , height_-splity);
}

void SliceViewer::draw()
{
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
    while (updated_) sleep(0);
  
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
        switch (options_.complex_)
        {
          case ViewingOptions::ComplexToReal::Real:
            image_ = original_.Real();
            break;
          case ViewingOptions::ComplexToReal::Imaginary:
            image_ = original_.Imaginary();
            break;
          case ViewingOptions::ComplexToReal::Magnitude:
            // TODO
            image_ = original_.Real();
            break;
          case ViewingOptions::ComplexToReal::Phase:
            // TODO
            image_ = original_.Real();
            break;
        }      
      }
      else
        image_ = original_;
        
      // TODO: Make the whole thing datatype agnostic?
      image_.Convert(DT_SFLOAT);
      
      // Get range
      dip::Image copy = image_;
      copy.TensorToSpatial();
      dip::MinMaxAccumulator acc = GetMaximumAndMinimum(copy);
      options_.range_ = {acc.Minimum(), acc.Maximum()};
      if (options_.mapping_ == ViewingOptions::Mapping::Logarithmic)
        options_.mapping_range_ = options.range_;
        
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
    
    if (diff > ViewingOptions::Diff::Draw)
    {
      // Just redraw
      updated_ = true;
      refresh();
    }

    sleep(0);
  }
}
