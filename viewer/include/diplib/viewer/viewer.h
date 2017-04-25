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

#include <stdint.h>
#include <unistd.h>

#include <vector>

#undef DIP__ENABLE_DOCTEST
#include "diplib.h"
#include "diplib/viewer/glutwm.h"

//#define GL_SAFE_CALL(x) DIP_THROW_IF((x) != 0, dip::String(#x))

typedef std::pair<dip::dfloat, dip::dfloat> FloatRange;
typedef dip::DimensionArray<FloatRange > FloatRangeArray;

struct DIP_EXPORT ViewingOptions
{
  enum class ComplexToReal { Real, Imaginary, Magnitude, Phase };
  enum class Mapping { ZeroOne, Normal, Linear, Symmetric, Logarithmic };
  enum class Projection { None, Min, Mean, Max };
  enum class LookupTable { ColorSpace, RGB, Grey, Jet };
  enum class Diff { None, Draw, Mapping, Projection, Complex };

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
  
  // Display
  dip::FloatArray zoom_;               ///< Zoom factor per dimension (from physical dimensions + user).
                                       ///< Also determines relative viewport sizes.
  dip::FloatArray origin_;             ///< Display origin for moving the image around.
  
  ViewingOptions() : complex_(ComplexToReal::Imaginary) // To force update
  {
  }
  
  ViewingOptions(const dip::Image &image)
  {
    // Projection
    dims_ = {0, 1, 2, 2}; // {0, 1, 2, 2}
    operating_point_ = dip::UnsignedArray(image.Dimensionality(), 0);
    complex_ = ComplexToReal::Real;
    projection_ = Projection::Max;
      
    // Mapping
    mapping_ = Mapping::Normal;
    mapping_range_ = {0, 255};
    
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
    dip::uint maxsz = 0;
    for (size_t ii=0; ii < image.Dimensionality(); ++ii)
      maxsz = std::max(maxsz, image.Size(ii));
      
    zoom_.resize(image.Dimensionality());
    for (size_t ii=0; ii < image.Dimensionality(); ++ii)
      zoom_[ii] = (dip::sfloat)image.Size(ii)/(dip::sfloat)maxsz;
    
    origin_ = dip::FloatArray(image.Dimensionality(), 0.);
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
    if (zoom_ != options.zoom_) return Diff::Draw;
    if (origin_ != options.origin_) return Diff::Draw;
    
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

class DIP_EXPORT View
{
  protected:
    class ViewPort *viewport_;

  public:
    View(ViewPort *viewport) : viewport_(viewport) { }
    virtual ~View() { }

    // Set up rendering. May take a while.
    virtual void rebuild() { };
    
    // Render to screen.
    virtual void render() { };
    
    // Size in internal coordinates.
    virtual dip::uint size(dip::uint /*ii*/) { return 0; }
    
    class ViewPort *viewport() { return viewport_; }
};

class DIP_EXPORT ViewPort
{
  protected:
    class Viewer *viewer_;
    int x_, y_, width_, height_;
    
  public:
    ViewPort(Viewer *viewer) : viewer_(viewer), x_(0), y_(0), width_(0), height_(0) { }
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

class DIP_EXPORT Viewer : public glutwm::Window
{
  public:
    virtual ViewingOptions &options() = 0;
    virtual dip::Image &image() = 0;

    virtual int width() = 0;
    virtual int height() = 0;
    
    virtual void setWindowTitle(const char *name) { title(name); }
};

inline void jet(dip::sfloat v, dip::uint8 *out)
{
  v = 4*v;
  out[0] = dip::clamp_cast<dip::uint8>(255*std::min(v - 1.5, -v + 4.5));
  out[1] = dip::clamp_cast<dip::uint8>(255*std::min(v - 0.5, -v + 3.5));
  out[2] = dip::clamp_cast<dip::uint8>(255*std::min(v + 0.5, -v + 2.5));
}

template<typename T>
inline dip::sfloat rangeMap(T val, double offset, double scale, ViewingOptions::Mapping mapping)
{
  if( mapping == ViewingOptions::Mapping::Logarithmic )
    return (dip::sfloat)(std::log((double)val - offset) * scale);
  else
    return (dip::sfloat)(std::min(std::max(((double)val - offset) * scale, 0.), 1.));
}

template<typename T>
inline dip::sfloat rangeMap(T val, const ViewingOptions &options)
{
  if( options.mapping_ == ViewingOptions::Mapping::Logarithmic )
    return (dip::sfloat)(std::log((double)val-options.mapping_range_.first+1.)/std::log(options.mapping_range_.second-options.mapping_range_.first+1.));
  else
    return (dip::sfloat)(std::min(std::max(((double)val-options.mapping_range_.first)/(options.mapping_range_.second-options.mapping_range_.first), 0.), 1.));
}

template<typename T>
inline void colorMap(const T *in, dip::sint tstride, dip::uint8 *out, const ViewingOptions &options)
{
  switch (options.lut_)
  {
    case ViewingOptions::LookupTable::ColorSpace:
      // TODO: Need to know color space. Is there even a per-pixel version in diplib?
      out[0] = out[1] = out[2] = 0;
      break;
    case ViewingOptions::LookupTable::RGB:
      for (size_t kk=0; kk < 3; ++kk)
      {
        dip::sint elem = options.color_elements_[kk];
        if (elem >= 0)
          out[kk] = (dip::uint8)(rangeMap(in[elem*tstride], options)*255);
        else
          out[kk] = 0;
      }
      break;
    case ViewingOptions::LookupTable::Grey:
      out[0] = out[1] = out[2] = (dip::uint8)(rangeMap(in[(dip::sint)options.element_*tstride], options)*255);
      break;
    case ViewingOptions::LookupTable::Jet:
      jet(rangeMap(in[(dip::sint)options.element_*tstride], options), out);
      break;
  }
}

#endif // DIP_VIEWER_H
