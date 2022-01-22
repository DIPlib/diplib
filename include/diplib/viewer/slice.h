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

#ifndef DIP_VIEWER_SLICE_H
#define DIP_VIEWER_SLICE_H

#include <thread>

#include "diplib/color.h"

#include "diplib/viewer/manager.h"
#include "diplib/viewer/viewer.h"
#include "diplib/viewer/image.h"
#include "diplib/viewer/tensor.h"
#include "diplib/viewer/histogram.h"
#include "diplib/viewer/control.h"
#include "diplib/viewer/status.h"
#include "diplib/viewer/link.h"

/// \file
/// \brief Declares \ref dip::viewer::SliceViewer.

namespace dip { namespace viewer {

/// \addtogroup dipviewer

class DIPVIEWER_CLASS_EXPORT SliceView : public View
{
  protected:
    dip::Image projected_,       ///< Projected (2D) image.
               colored_;         ///< Colored (RGB) image.
    dip::ColorSpaceManager csm_; ///< For \ref ViewingOptions::LookupTable::ColorSpace

    dip::uint dimx_, dimy_;      ///< Indices in options.dims_.             
    unsigned int texture_;       ///< OpenGL texture identifier.
    bool dirty_;                 ///< Texture needs to be rebuilt.

  public:
    SliceView(ViewPort *viewport, dip::uint dimx, dip::uint dimy) : View(viewport), dimx_(dimx), dimy_(dimy), texture_(0), dirty_(true) { }

    DIPVIEWER_EXPORT void project();
    DIPVIEWER_EXPORT void map();
    DIPVIEWER_EXPORT void rebuild();
    DIPVIEWER_EXPORT void render();
    
    dip::uint dimx() { return dimx_; }
    dip::uint dimy() { return dimy_; }
};

class DIPVIEWER_CLASS_EXPORT SliceViewPort : public ViewPort
{
  protected:
    class SliceViewer *viewer_;
    SliceView *view_;
    int drag_x_, drag_y_, drag_mods_;
    dip::sint roi_start_, roi_end_, roi_dim_, roi_edge_;
    
  public:
    explicit SliceViewPort(class SliceViewer *viewer) : ViewPort((Viewer*)viewer), viewer_(viewer), view_(NULL) { }
    ~SliceViewPort() override { if (view_) delete view_; }
    
    void rebuild() override { view()->rebuild(); }
    DIPVIEWER_EXPORT void render() override;
    DIPVIEWER_EXPORT void click(int button, int state, int x, int y, int mods) override;
    DIPVIEWER_EXPORT void motion(int button, int x, int y) override;

    void setView(SliceView *view) { view_ = view; }
    SliceView *view() { return view_; }

  protected:    
    DIPVIEWER_EXPORT void screenToView(int x, int y, double *ix, double *iy) override;
};


/// \brief Interactive nD tensor image viewer.
class DIPVIEWER_CLASS_EXPORT SliceViewer : public Viewer
{
  public:
    /// \brief A pointer to a `SliceViewer`.
    typedef std::shared_ptr<SliceViewer> Ptr;
    
  protected:
    ViewingOptions options_;
    std::thread thread_;
    bool continue_, updated_;
    std::vector<ViewPort*> viewports_;
    SliceViewPort *main_, *left_, *top_;
    TensorViewPort *tensor_;
    HistogramViewPort *histogram_;
    ControlViewPort *control_;
    StatusViewPort *status_;
    LinkViewPort *link_;
    dip::Image original_, image_;
    
    ViewPort *drag_viewport_;
    int drag_button_;
    int refresh_seq_;
  
  public:
    /// \brief Construct a new `SliceViewer`.
    ///
    /// As the constructor is protected, this is the only way to create a `SliceViewer`.
    ///
    /// Example usage:
    ///
    /// ```cpp
    /// manager.createWindow( dip::viewer::SliceViewer::Create( image ));
    /// ```
    static Ptr Create(const dip::Image &image, std::string name="SliceViewer", dip::uint width=0, dip::uint height=0)
    {
      return Ptr(new SliceViewer(image, name, width, height));
    }
  
    ~SliceViewer() override
    {
      if (continue_)
      {
        continue_ = false;
        thread_.join();
      }
      
      for (dip::uint ii=0; ii < viewports_.size(); ++ii)
        delete viewports_[ii];
    }
    
    std::shared_ptr<SliceViewer> clone()
    {
      Ptr sv = Create(original_, name_, (dip::uint)width(), (dip::uint)height());
      
      Guard this_guard(*this);
      Guard other_guard(*sv);
      
      sv->options() = options();
      
      return sv;
    }
    
    ViewingOptions &options() override { return options_; }
    const dip::Image &image() override { return image_; }
    const dip::Image &original() override { return original_; }
    void setImage(const dip::Image &image) override { original_ = image; refresh_seq_++; }
    void refreshImage() { refresh_seq_++; }
    
    /// \brief Update linked viewers.
    DIPVIEWER_EXPORT void updateLinkedViewers();
    
    /// \brief Link this viewer to another, compatible one.
    DIPVIEWER_EXPORT void link(SliceViewer &other);
  protected:
    DIPVIEWER_EXPORT explicit SliceViewer(const dip::Image &image, std::string name="SliceViewer", dip::uint width=0, dip::uint height=0);

    DIPVIEWER_EXPORT void create() override;
    DIPVIEWER_EXPORT void reshape(int width, int height) override;
    DIPVIEWER_EXPORT void draw() override;
    DIPVIEWER_EXPORT void key(unsigned char k, int x, int y, int mods) override;
    DIPVIEWER_EXPORT void click(int button, int state, int x, int y, int mods) override;
    DIPVIEWER_EXPORT void motion(int x, int y) override;

    DIPVIEWER_EXPORT void place();
    DIPVIEWER_EXPORT ViewPort *viewport(int x, int y);
    DIPVIEWER_EXPORT void calculateTextures();
};

/// \endgroup

}} // namespace dip::viewer

#endif // DIP_VIEWER_SLICE_H
