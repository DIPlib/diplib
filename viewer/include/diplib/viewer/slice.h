/*
 * DIPlib 3.0 viewer
 * This file contains definitions for the nD image slice viewer.
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

#ifndef DIP_VIEWER_SLICE_H
#define DIP_VIEWER_SLICE_H

#include <thread>
#include <mutex>

#include "diplib/viewer/manager.h"
#include "diplib/viewer/viewer.h"
#include "diplib/viewer/image.h"
#include "diplib/viewer/tensor.h"
#include "diplib/viewer/histogram.h"
#include "diplib/viewer/control.h"

class DIPVIEWER_EXPORT SliceView : public View
{
  protected:
    dip::Image projected_, ///< Projected (2D) image.
               colored_;   ///< Colored (RGB) image.

    dip::uint dimx_, dimy_;      ///< Indices in options.dims_.             
    unsigned int texture_;       ///< OpenGL texture identifier.

  public:
    SliceView(ViewPort *viewport, dip::uint dimx, dip::uint dimy) : View(viewport), dimx_(dimx), dimy_(dimy), texture_(0) { }

    void project();
    void map();
    void rebuild();
    void render();
    dip::uint size(dip::uint ii)
    {
      return colored_.Size(ii);
    }
    
    dip::uint dimx() { return dimx_; }
    dip::uint dimy() { return dimy_; }
};

class DIPVIEWER_EXPORT SliceViewPort : public ViewPort
{
  protected:
    SliceView *view_;
    int drag_x_, drag_y_;
    
  public:
    SliceViewPort(Viewer *viewer) : ViewPort(viewer), view_(NULL) { }
    ~SliceViewPort() { if (view_) delete view_; }
    
    void rebuild() { view()->rebuild(); }
    void render();
    void click(int button, int state, int x, int y);
    void motion(int button, int x, int y);

    void setView(SliceView *view) { view_ = view; }
    SliceView *view() { return view_; }

  protected:    
    void screenToView(int x, int y, double *ix, double *iy);
};

class DIPVIEWER_EXPORT SliceViewer : public Viewer
{
  protected:
    ViewingOptions options_;
    std::mutex mutex_;  
    std::thread thread_;
    bool continue_, updated_;
    std::vector<ViewPort*> viewports_;
    SliceViewPort *main_, *left_, *top_;
    TensorViewPort *tensor_;
    HistogramViewPort *histogram_;
    ControlViewPort *control_;
    dip::Image original_, image_;
    
    ViewPort *drag_viewport_;
    int drag_button_;
  
  public:
    SliceViewer(const dip::Image &image, std::string name="SliceViewer", size_t width=0, size_t height=0);
    
    ~SliceViewer()
    {
      if (continue_)
      {
        continue_ = false;
        thread_.join();
      }
      
      for (size_t ii=0; ii < viewports_.size(); ++ii)
        delete viewports_[ii];
    }
    
    ViewingOptions &options() { return options_; }
    dip::Image &image() { return image_; }
    
    void place();
  
  protected:
    void create();
    void reshape(int width, int height);
    void draw();
    void click(int button, int state, int x, int y);
    void motion(int x, int y);

    ViewPort *viewport(int x, int y);
    void calculateTextures();
};

#endif // DIP_VIEWER_SLICE_H
