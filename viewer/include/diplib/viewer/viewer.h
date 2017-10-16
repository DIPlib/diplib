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
//#include <unistd.h>
#include <vector>
#include <mutex>

#include "diplib.h"
#include "diplib/overload.h"
#include "diplib/display.h"

#include "diplib/viewer/manager.h"

namespace dip { namespace viewer {

/// \brief Specifies a range of values between a lower and upper limit
typedef std::pair<dip::dfloat, dip::dfloat> FloatRange;

/// \brief Model that determines the SliceViewer's behavior
struct DIPVIEWER_NO_EXPORT ViewingOptions
{
  /// \brief Complex-to-real mapping options
  enum class ComplexToReal { Real, Imaginary, Magnitude, Phase };
  
  /// \brief Grey-value mapping options
  enum class Mapping { ZeroOne, Angle, Normal, Linear, Symmetric, Logarithmic };
  
  /// \brief Slice projection options
  enum class Projection { None, Min, Mean, Max };
  
  /// \brief Grey-value to color mapping optios
  enum class LookupTable { ColorSpace, RGB, Grey, Sequential, Divergent, Cyclic, Label };
  
  /// \brief Defines which view (parts) need to be recalculated
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
  
  // Status
  dip::String status_;                 ///< Status bar text.
  
  ViewingOptions() : complex_(ComplexToReal::Imaginary) // To force update
  {
  }
  
  /// \brief Calculate default options from an image
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
  
  /// \brief Calculates which view (parts) need to be recalculated
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
    if (operating_point_ != options.operating_point_) return Diff::Draw;
    if (status_ != options.status_) return Diff::Draw;
    
    return Diff::None;
  }
  
  /// \brief Calculates whether a particular slice projection needs to be recalculated
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

  /// \brief Returns a textual description of the current complex-to-real mapping
  dip::String getComplexDescription()
  {
    dip::String names[] = {"real part", "imaginary part", "magnitude (abs)", "phase"};
    return names[(dip::uint)complex_];
  }
  
  /// \brief Returns a textual description of the current grey-value mapping
  dip::String getMappingDescription()
  {
    dip::String names[] = {"unit", "angle", "normal", "linear", "symmetric around 0", "logaritmic"};
    return names[(dip::uint)mapping_];
  }
  
  /// \brief Returns a textual description of the current slice projection
  dip::String getProjectionDescription()
  {
    dip::String names[] = {"none (slice)", "minimum", "mean", "maximum"};
    return names[(dip::uint)projection_];
  }

  /// \brief Returns a textual description of the current grey-value to color mapping
  dip::String getLookupTableDescription()
  {
    dip::String names[] = {"image colorspace (mapping inactive)", "ternary (RGB)", "gray-value", "perceptually linear", "divergent blue-red", "cyclic", "labels"};
    return names[(dip::uint)lut_];
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

/// \brief Displays a view of the ViewingOptions model
class DIPVIEWER_EXPORT View
{
  protected:
    class ViewPort *viewport_;

  public:
    explicit View(ViewPort *viewport) : viewport_(viewport) { }
    virtual ~View() { }

    /// \brief Set up rendering. May take a while.
    virtual void rebuild() { };
    
    /// \brief Render to screen.
    virtual void render() { };
    
    /// \brief Size in internal coordinates.
    virtual dip::uint size(dip::uint /*ii*/) { return 0; }
    
    /// \brief Parent viewport
    class ViewPort *viewport() { return viewport_; }
};

/// \brief Handles interaction in a certain display area to control the ViewingOptions model
class DIPVIEWER_EXPORT ViewPort
{
  protected:
    class Viewer *viewer_;
    int x_, y_, width_, height_;
    
  public:
    explicit ViewPort(Viewer *viewer) : viewer_(viewer), x_(0), y_(0), width_(0), height_(0) { }
    virtual ~ViewPort() { }

    /// \brief Places the viewport
    virtual void place(int x, int y, int width, int height)
    {
      x_ = x;
      y_ = y;
      width_ = width;
      height_ = height;
    }
    
    /// \brief Prepares the associated view for rendering
    virtual void rebuild() { }

    /// \brief Renders the associated view
    virtual void render() { } 
    
    /// \brief Handles mouse clicking interaction
    virtual void click(int /*button*/, int /*state*/, int /*x*/, int /*y*/) { }
    
    /// \brief Handles mouse dragging interaction
    virtual void motion(int /*button*/, int /*x*/, int /*y*/) { }
    
    /// \brief Converts screen coordinates into local view coordinates
    virtual void screenToView(int x, int y, double *ix, double *iy)
    {
      *ix = x-x_;
      *iy = y-y_;
    }
    
    /// \brief Parent viewer
    class Viewer *viewer() { return viewer_; }
    
    /// \brief Screen coordinate of left edge
    int x() { return x_; }

    /// \brief Screen coordinate of bottom edge
    int y() { return y_; }

    /// \brief Viewport width
    int width() { return width_; }

    /// \brief Viewport height
    int height() { return height_; }
};

/// \brief A Window for viewing a dip::Image
class DIPVIEWER_EXPORT Viewer : public Window
{
  public:
    typedef std::lock_guard<Viewer> Guard;
    
  protected:
    std::string name_;
    std::recursive_mutex mutex_;

  public:
    explicit Viewer(std::string name="Viewer") : name_(name) { }
  
    /// \brief Returns the Viewer's model
    ///
    /// Only call or change this under lock.
    ///
    /// Example usage:
    ///
    /// ```cpp
    ///     {
    ///        dip::viewer::Viewer::Guard(*viewer);
    ///        viewer->options().mapping_range_.first = 0;
    ///     }
    /// ```
    virtual ViewingOptions &options() = 0;
    
    /// \brief Returns the dip::Image being visualized.
    ///
    /// Only call this under lock.
    ///
    /// Example usage:
    ///
    /// ```cpp
    ///     {
    ///        dip::viewer::Viewer::Guard(*viewer);
    ///        dip::Image image = viewer->image();
    ///     }
    /// ```
    virtual const dip::Image &image() = 0;
    
    /// \brief Sets the image to be visualized.
    ///
    /// Only call this under lock.
    ///
    /// Example usage:
    ///
    /// ```cpp
    ///     {
    ///        dip::viewer::Viewer::Guard(*viewer);
    ///        viewer->setImage(image);
    ///     }
    /// ```
    virtual void setImage(const dip::Image &image) = 0;
    
    /// \brief Returns the Viewer's name
    virtual const std::string &name() { return name_; }

    /// \brief Set window title, in addition to the Viewer's name
    virtual void setWindowTitle(const char *name) { title((name_ + name).c_str()); }
    
    /// \brief Lock the viewer. Necessary before making programmatic changes.
    void lock() { mutex_.lock(); }
    
    /// \brief Unlock the viewer.
    void unlock() { mutex_.unlock(); }
};

/// \brief Maps an image grey-value onto [0,255]
template<typename T>
inline dip::dfloat rangeMap(T val, double offset, double scale, ViewingOptions::Mapping mapping)
{
  if( mapping == ViewingOptions::Mapping::Logarithmic )
    return 255.*std::min(std::log(std::max((double)val - offset, 1.)) * scale, 1.);
  else
    return 255.*std::min(std::max(((double)val - offset) * scale, 0.), 1.);
}

/// \brief Maps an image grey-value onto [0,255]
template<typename T>
inline dip::dfloat rangeMap(T val, const ViewingOptions &options)
{
  if( options.mapping_ == ViewingOptions::Mapping::Logarithmic )
    return rangeMap(val, options.mapping_range_.first-1., 1./std::log(options.mapping_range_.second-options.mapping_range_.first+1.), options.mapping_);
  else
    return rangeMap(val, options.mapping_range_.first, 1./(options.mapping_range_.second-options.mapping_range_.first), options.mapping_);
}

/// \brief String conversion for dip::DimensionArray
template<typename T>
std::string to_string(dip::DimensionArray<T> array)
{
  std::ostringstream oss;
  oss << "[";
  for (size_t ii=0; ii < array.size(); ++ii)
  {
    oss << array[ii];
    if (ii < array.size()-1)
      oss << ", ";
  }
  oss << "]";
  return oss.str();
}

/// \brief Applies the colormap defined by the ViewingOptions
void DIPVIEWER_NO_EXPORT ApplyViewerColorMap(dip::Image &in, dip::Image &out, ViewingOptions &options);

}} // namespace dip::viewer

#endif // DIP_VIEWER_H
