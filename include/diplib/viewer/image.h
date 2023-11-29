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

#ifndef DIP_VIEWER_IMAGE_H
#define DIP_VIEWER_IMAGE_H

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "diplib.h"
#include "diplib/viewer/export.h"
#include "diplib/viewer/viewer.h"

/// \file
/// \brief Declares \ref dip::viewer::ImageViewer.

namespace dip {
namespace viewer {

/// \addtogroup dipviewer

class DIPVIEWER_CLASS_EXPORT ImageView : public View {
   protected:
      dip::Image image_; ///< 2D RGB image
      unsigned int texture_ = 0; ///< OpenGL texture identifier.

   public:
      ImageView( ViewPort* viewport ) :
         View( viewport ) {}

      ImageView( ImageView const& ) = delete;
      ImageView( ImageView&& ) = default;
      ImageView& operator=( ImageView const& ) = delete;
      ImageView& operator=( ImageView&& ) = default;
      ~ImageView() override = default;

      void set( dip::Image const& image ) {
         DIP_ASSERT( image.HasNormalStrides() );
         DIP_ASSERT( image.DataType() == DT_UINT8 );
         DIP_ASSERT( image.TensorElements() == 3 );
         image_ = image;
      }

      DIPVIEWER_EXPORT void rebuild() override;
      DIPVIEWER_EXPORT void render() override;
      dip::uint size( dip::uint ii ) override { return image_.Size( ii ); }

      class ViewPort* viewport() { return viewport_; }
      dip::Image const& image() { return image_; }
};

class DIPVIEWER_CLASS_EXPORT ImageViewPort : public ViewPort {
   protected:
      ImageView* view_ = nullptr;

   public:
      explicit ImageViewPort( Viewer* viewer ) :
         ViewPort( viewer ) {}

      ImageViewPort( ImageViewPort const& ) = delete;
      ImageViewPort( ImageViewPort&& ) = default;
      ImageViewPort& operator=( ImageViewPort const& ) = delete;
      ImageViewPort& operator=( ImageViewPort&& ) = default;

      ~ImageViewPort() override {
         delete view_;
      }

      void rebuild() override { view_->rebuild(); }
      DIPVIEWER_EXPORT void render() override;
      void setView( ImageView* view ) { view_ = view; }
      ImageView* view() { return view_; }
};


/// Non-interactive 2D RGB image viewer.
class DIPVIEWER_CLASS_EXPORT ImageViewer : public Viewer {
   public:
      /// \brief A pointer to an `ImageViewer`.
      using Ptr = std::shared_ptr< ImageViewer >;

   protected:
      ViewingOptions options_;
      ImageViewPort* viewport_;

      std::string name_;

   public:
      /// \brief Construct a new `ImageViewer`.
      ///
      /// As the constructor is protected, this is the only way to create an `ImageViewer`.
      /// Note that the ImageViewer only supports 8-bit 2D RGB images.
      ///
      /// If either `width` or `height` is 0, it is computed from the other value so as to
      /// preserve the image's aspect ratio. If both are zero, the image is displayed in its
      /// natural size (one image pixel to one screen pixel) but scaled down if otherwise the
      /// window would exceed 512 pixels along either dimension.
      ///
      /// Example usage:
      ///
      /// ```cpp
      /// manager.createWindow( dip::viewer::ImageViewer::Create( image ));
      /// ```
      static Ptr Create( const dip::Image& image, std::string name = "ImageViewer", dip::uint width = 0, dip::uint height = 0 ) {
         return Ptr( new ImageViewer( image, std::move( name ), width, height ));
      }

      ImageViewer( ImageViewer const& ) = delete;
      ImageViewer( ImageViewer&& ) = delete;
      ImageViewer& operator=( ImageViewer const& ) = delete;
      ImageViewer& operator=( ImageViewer&& ) = delete;

      ~ImageViewer() override {
         delete viewport_;
      }

      ViewingOptions& options() override { return options_; }
      dip::Image const& image() override { return viewport_->view()->image(); }
      dip::Image const& original() override { return viewport_->view()->image(); }

      void setImage( const dip::Image& image ) override {
         viewport_->view()->set( image );
         refresh();
      }

   protected:
      explicit ImageViewer( dip::Image const& image, std::string name = "ImageViewer", dip::uint width = 0, dip::uint height = 0 ) :
         Viewer( std::move( name )), options_( image ) {
         DIP_THROW_IF( !image.HasNormalStrides(), E::NO_NORMAL_STRIDE );
         DIP_THROW_IF( image.DataType() != DT_UINT8, E::DATA_TYPE_NOT_SUPPORTED );
         DIP_THROW_IF( image.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         DIP_THROW_IF( image.TensorElements() != 3, "Only defined for 3-vector images" );

         if( !width && !height ) {
            if( image.Size( 0 ) > image.Size( 1 )) {
               width = std::min< dip::uint >( 512, image.Size( 0 ));
            } else {
               height = std::min< dip::uint >( 512, image.Size( 1 ));
            }
         }
         if( !width ) {
            width = ( dip::uint )( ( double )height * ( double )image.Size( 0 ) / ( double )image.Size( 1 ));
         } else if( !height ) {
            height = ( dip::uint )( ( double )width * ( double )image.Size( 1 ) / ( double )image.Size( 0 ));
         }

         requestSize( width, height );

         viewport_ = new ImageViewPort( this );
         ImageView* view = new ImageView( viewport_ );
         view->set( image );
         viewport_->setView( view );
      }

      DIPVIEWER_EXPORT void create() override;
      DIPVIEWER_EXPORT void reshape( int /*width*/, int /*height*/ ) override;
      DIPVIEWER_EXPORT void draw() override;
};

/// \endgroup

} // namespace viewer
} // namespace dip

#endif // DIP_VIEWER_IMAGE_H
