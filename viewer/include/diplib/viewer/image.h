/*
 * DIPlib 3.0 viewer
 * This file contains definitions for the simple 2D RGB image viewer.
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

#ifndef DIP_VIEWER_IMAGE_H
#define DIP_VIEWER_IMAGE_H

#include "diplib/viewer/viewer.h"

/// \file
/// \brief Declares `dip::viewer::ImageViewer`.

namespace dip { namespace viewer {

/// \addtogroup viewer
/// \{

class DIPVIEWER_EXPORT ImageView : public View
{
  protected:
    class ViewPort *viewport_;
    dip::Image image_;     ///< 2D RGB image
    unsigned int texture_;       ///< OpenGL texture identifier.

  public:
    ImageView(ViewPort *viewport) : View(viewport), texture_(0) { }
    ~ImageView() override { }

    void set(const dip::Image &image) {
       DIP_ASSERT( image.HasNormalStrides() );
       DIP_ASSERT( image.DataType() == DT_UINT8 );
       DIP_ASSERT( image.TensorElements() == 3 );
       image_ = image;
    }
    void rebuild() override;
    void render() override;
    virtual dip::uint size(dip::uint ii) override { return image_.Size(ii); }
    
    class ViewPort *viewport() { return viewport_; }
    const dip::Image &image() { return image_; }
};

class DIPVIEWER_EXPORT ImageViewPort : public ViewPort
{
  protected:
    ImageView *view_;
    
  public:
    explicit ImageViewPort(Viewer *viewer) : ViewPort(viewer), view_(NULL) { }
    ~ImageViewPort() override
    {
      if (view_)
        delete view_;
    }
    
    void rebuild() override { view_->rebuild(); }
    void render() override;
    void setView(ImageView *view) { view_ = view; }
    ImageView *view() { return view_; }
};

/// Non-interactive 2D image viewer
class DIPVIEWER_EXPORT ImageViewer : public Viewer
{
  protected:
    ViewingOptions options_;
    ImageViewPort *viewport_;
    
    std::string name_;
  
  public:
    explicit ImageViewer(const dip::Image &image, std::string name="ImageViewer", size_t width=0, size_t height=0) : Viewer(name), options_(image)
    {
      DIP_THROW_IF( !image.HasNormalStrides(), E::NO_NORMAL_STRIDE );
      DIP_THROW_IF( image.DataType() != DT_UINT8, "ImageViewer requires an image of type DT_UINT8" );
      DIP_THROW_IF( image.TensorElements() != 3, "ImageViewer requires an image with three tensor elements" );

      if (!width || !height)
      {
        if (image.Size(0) > image.Size(1))
        {
          width = std::max(512UL, image.Size(0));
          height = (size_t)((double)width * (double)image.Size(1)/(double)image.Size(0));
        }
        else
        {
          height = std::max(512UL, image.Size(1));
          width = (size_t)((double)height * (double)image.Size(0)/(double)image.Size(1));
        }
      }

      requestSize(width, height);

      viewport_ = new ImageViewPort(this);
      ImageView *view = new ImageView(viewport_);
      view->set(image);
      viewport_->setView(view);
    }
    
    ~ImageViewer() override
    {
      if (viewport_)
        delete viewport_; 
    }

    ViewingOptions &options() override { return options_; }
    const dip::Image &image() override { return viewport_->view()->image(); }

  protected:
    void create() override;
    void reshape(int /*width*/, int /*height*/) override
    {
      viewport_->place(0, 0, width(), height());
      refresh();
    }
    
    void draw() override
    {
      viewport_->rebuild();
      viewport_->render();
      swap();
    }
};

/// \}

}} // namespace dip::viewer

#endif // DIP_VIEWER_IMAGE_H
