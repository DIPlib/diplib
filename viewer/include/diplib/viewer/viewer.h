/*
 * DIPlib 3.0 viewer
 * This file contains base definitions for the viewer.
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

#ifndef DIP_VIEWER_H
#define DIP_VIEWER_H

#include <cstdint>
#include <unistd.h>
#include <vector>

#include "diplib.h"
#include "diplib/overload.h"
#include "diplib/display.h"

#include "diplib/viewer/manager.h"

//#define GL_SAFE_CALL(x) DIP_THROW_IF((x) != 0, dip::String(#x))

namespace dip { namespace viewer {

typedef std::pair<dip::dfloat, dip::dfloat> FloatRange;
typedef dip::DimensionArray<FloatRange > FloatRangeArray;

struct DIPVIEWER_NO_EXPORT ViewingOptions
{
  enum class ComplexToReal { Real, Imaginary, Magnitude, Phase };
  enum class Mapping { ZeroOne, Angle, Normal, Linear, Symmetric, Logarithmic };
  enum class Projection { None, Min, Mean, Max };
  enum class LookupTable { ColorSpace, RGB, Grey, Sequential, Divergent, Cyclic, Label };
  enum class Diff { None, Draw, Place, Mapping, Projection, Complex };

  // Projection
  dip::IntegerArray dims_;             ///< Dimensions to visualize (MainX, MainY, LeftX, TopY).
  dip::UnsignedArray operating_point_; ///< Value of nonvisualized, nonprojected dimensions.
  ComplexToReal complex_;              ///< What to do with complex numbers.
  Projection projection_;              ///< Type of projection.
  
  // Mapping
  FloatRange range_;                   ///< value range across image (histogram limits).
  FloatRange mapping_range_;           ///< mapped value range (colorbar limits).
  Mapping mapping_;                    ///< from input to [0, 1], modifies mapping_range_.
  
  // Color
  dip::uint element_;                  ///< Tensor element to visualize.
  LookupTable lut_;                    ///< from [0, 1] to [0, 0, 0]-[255, 255, 255].
  dip::IntegerArray color_elements_;   ///< Which tensor element is R, G, and B.

  // Placement  
  dip::IntegerArray split_;            ///< Split point between projections (pixels).

  // Display
  dip::FloatArray zoom_;               ///< Zoom factor per dimension (from physical dimensions + user).
                                       ///< Also determines relative viewport sizes.
  dip::FloatArray origin_;             ///< Display origin for moving the image around.
  
  ViewingOptions() : complex_(ComplexToReal::Imaginary) // To force update
  {
  }
  
  explicit ViewingOptions(const dip::Image &image)
  {
    // Projection
    if (image.Dimensionality() == 1)
      dims_ = {0, -1, -1, -1};
    else if (image.Dimensionality() == 2)
      dims_ = {0,  1, -1, -1};
    else
      dims_ = {0,  1,  2,  2};
    
    operating_point_ = dip::UnsignedArray(image.Dimensionality(), 0);
    complex_ = ComplexToReal::Real;
    projection_ = Projection::Max;
      
    // Mapping
    if (image.DataType().IsBinary())
    {
      mapping_ = Mapping::ZeroOne;
      mapping_range_ = {0, 1};
    }
    else
    {
      mapping_ = Mapping::Normal;
      mapping_range_ = {0, 255};
    }
    
    // Color
    element_ = 0;
    if (image.TensorElements() == 1) color_elements_ = {0, -1, -1};
    else if (image.TensorElements() == 2) color_elements_ = {0, 1, -1};
    else color_elements_ = {0, 1, 2};
      
    if (image.IsColor())
      lut_ = LookupTable::ColorSpace;
    else
      lut_ = LookupTable::Grey;
    
    // Display
    zoom_ = image.AspectRatio();
    for (size_t ii=0; ii < image.Dimensionality(); ++ii)
      if (zoom_[ii] == 0)
        zoom_[ii] = 1;
    
    origin_ = dip::FloatArray(image.Dimensionality(), 0.);
    split_ = {100, 100};
  }
  
  Diff diff(const ViewingOptions &options) const
  {
    if (complex_ != options.complex_) return Diff::Complex;
  
    if (dims_ != options.dims_) return Diff::Projection;
    if (operating_point_ != options.operating_point_ &&
        projection_ == Projection::None)
      return Diff::Projection;
    
    if (projection_ != options.projection_) return Diff::Projection;
    
    if (mapping_range_ != options.mapping_range_) return Diff::Mapping;
    if (mapping_ != options.mapping_) return Diff::Mapping;
    if (element_ != options.element_) return Diff::Mapping;
    if (lut_ != options.lut_) return Diff::Mapping;
    if (color_elements_ != options.color_elements_) return Diff::Mapping;
    if (split_ != options.split_) return Diff::Place;
    if (zoom_ != options.zoom_) return Diff::Draw;
    if (origin_ != options.origin_) return Diff::Draw;
    if (split_ != options.split_) return Diff::Draw;
    
    return Diff::None;
  }
  
  bool needsReproject(const ViewingOptions &options, dip::uint dimx, dip::uint dimy) const
  {
    // Global stuff
    if (complex_ != options.complex_) return true;
    if (projection_ != options.projection_) return true;
  
    // Change of axes
    if (dims_[dimx] != options.dims_[dimx]) return true;
    if (dims_[dimy] != options.dims_[dimy]) return true;
    
    // Change of operating point in nonvisualized dimension
    if (projection_ == Projection::None)
    {
      for (size_t ii=0; ii < operating_point_.size(); ++ii)
        if ((int)ii != dims_[dimx] && (int)ii != dims_[dimy])
          if (operating_point_[ii] != options.operating_point_[ii])
            return true;
    }
    
    return false;
  }

  friend std::ostream & operator<<(std::ostream &os, const ViewingOptions& opt)
  {
    os << "Visualized dimensions: " << opt.dims_[0] << ", "
       << opt.dims_[1] << ", " << opt.dims_[2] << ", " << opt.dims_[3] << "\n";
    for (size_t ii=0; ii < opt.origin_.size(); ++ii)
    {
      os << "Dimension " << ii << ":\n";
      os << "  Origin: " << opt.origin_[ii] << "\n";
      os << "  Zoom  : " << opt.zoom_[ii] << "\n";
    }
    
    return os;
  }  
};

class DIPVIEWER_EXPORT View
{
  protected:
    class ViewPort *viewport_;

  public:
    explicit View(ViewPort *viewport) : viewport_(viewport) { }
    virtual ~View() { }

    // Set up rendering. May take a while.
    virtual void rebuild() { };
    
    // Render to screen.
    virtual void render() { };
    
    // Size in internal coordinates.
    virtual dip::uint size(dip::uint /*ii*/) { return 0; }
    
    class ViewPort *viewport() { return viewport_; }
};

class DIPVIEWER_EXPORT ViewPort
{
  protected:
    class Viewer *viewer_;
    int x_, y_, width_, height_;
    
  public:
    explicit ViewPort(Viewer *viewer) : viewer_(viewer), x_(0), y_(0), width_(0), height_(0) { }
    virtual ~ViewPort() { }

    virtual void place(int x, int y, int width, int height)
    {
      x_ = x;
      y_ = y;
      width_ = width;
      height_ = height;
    }
    
    virtual void rebuild() { }
    virtual void render() { } 
    virtual void click(int /*button*/, int /*state*/, int /*x*/, int /*y*/) { }
    virtual void motion(int /*button*/, int /*x*/, int /*y*/) { }
    virtual void screenToView(int x, int y, double *ix, double *iy)
    {
      *ix = x-x_;
      *iy = y-y_;
    }
    
    class Viewer *viewer() { return viewer_; }
    int x() { return x_; }
    int y() { return y_; }
    int width() { return width_; }
    int height() { return height_; }
};

class DIPVIEWER_EXPORT Viewer : public Window
{
  protected:
    std::string name_;

  public:
    explicit Viewer(std::string name="Viewer") : name_(name) { }
  
    virtual ViewingOptions &options() = 0;
    virtual const dip::Image &image() = 0;

    virtual void setWindowTitle(const char *name) { title((name_ + name).c_str()); }
};

inline void jet(dip::sfloat v, dip::uint8 *out)
{
  v = 4*v;
  out[0] = dip::clamp_cast<dip::uint8>(255*std::min(v - 1.5, -v + 4.5));
  out[1] = dip::clamp_cast<dip::uint8>(255*std::min(v - 0.5, -v + 3.5));
  out[2] = dip::clamp_cast<dip::uint8>(255*std::min(v + 0.5, -v + 2.5));
}

template<typename T>
inline dip::uint8 rangeMap(T val, double offset, double scale, ViewingOptions::Mapping mapping)
{
  if( mapping == ViewingOptions::Mapping::Logarithmic )
    return (dip::uint8)(255.*std::min(std::log(std::max((double)val - offset, 1.)) * scale, 1.));
  else
    return (dip::uint8)(255.*std::min(std::max(((double)val - offset) * scale, 0.), 1.));
}

template<typename T>
inline dip::uint8 rangeMap(T val, const ViewingOptions &options)
{
  if( options.mapping_ == ViewingOptions::Mapping::Logarithmic )
    return rangeMap(val, options.mapping_range_.first-1., 1./std::log(options.mapping_range_.second-options.mapping_range_.first+1.), options.mapping_);
  else
    return rangeMap(val, options.mapping_range_.first, 1./(options.mapping_range_.second-options.mapping_range_.first), options.mapping_);
}

void DIPVIEWER_NO_EXPORT ApplyViewerColorMap(dip::Image &in, dip::Image &out, ViewingOptions &options);

}} // namespace dip::viewer

#endif // DIP_VIEWER_H
