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

#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/color.h"
#include "diplib/viewer/export.h"
#include "diplib/viewer/viewer.h"

/// \file
/// \brief Declares \ref dip::viewer::SliceViewer.

namespace dip {
namespace viewer {

/// \addtogroup dipviewer

class DIPVIEWER_CLASS_EXPORT SliceView : public View {
   protected:
      dip::Image projected_;       ///< Projected (2D) image.
      dip::Image colored_;         ///< Colored (RGB) image.
      dip::ColorSpaceManager csm_; ///< For \ref ViewingOptions::LookupTable::ColorSpace

      dip::uint dimx_, dimy_;      ///< Indices in options.dims_.
      unsigned int texture_ = 0;   ///< OpenGL texture identifier.
      bool dirty_ = true;          ///< Texture needs to be rebuilt.

   public:
      SliceView( ViewPort* viewport, dip::uint dimx, dip::uint dimy ) : View( viewport ), dimx_( dimx ), dimy_( dimy ) {}

      DIPVIEWER_EXPORT void project();
      DIPVIEWER_EXPORT void map();
      DIPVIEWER_EXPORT void rebuild() override;
      DIPVIEWER_EXPORT void render() override;

      dip::uint dimx() const { return dimx_; }
      dip::uint dimy() const { return dimy_; }
};

class SliceViewPort;
class StatusViewPort;
class TensorViewPort;
class HistogramViewPort;
class ControlViewPort;
class LinkViewPort;

/// \brief Interactive nD tensor image viewer.
class DIPVIEWER_CLASS_EXPORT SliceViewer : public Viewer {
   public:
      /// \brief A pointer to a `SliceViewer`.
      using Ptr = std::shared_ptr< SliceViewer >;

   protected:
      ViewingOptions options_;
      std::thread thread_;
      bool continue_, updated_;
      std::vector< ViewPort* > viewports_;
      SliceViewPort* main_,* left_,* top_;
      TensorViewPort* tensor_;
      HistogramViewPort* histogram_;
      ControlViewPort* control_;
      StatusViewPort* status_;
      LinkViewPort* link_;
      dip::Image original_, image_;

      ViewPort* drag_viewport_;
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
      static Ptr Create( const dip::Image& image, std::string name = "SliceViewer", dip::uint width = 0, dip::uint height = 0 ) {
         return Ptr( new SliceViewer( image, std::move( name ), width, height ));
      }

      SliceViewer( SliceViewer const& ) = delete;
      SliceViewer( SliceViewer&& ) = delete;
      SliceViewer& operator=( SliceViewer const& ) = delete;
      SliceViewer& operator=( SliceViewer&& ) = delete;
      DIPVIEWER_EXPORT ~SliceViewer() override; // Not inline because of a linking error with GCC under MinGW.

      void release() override {
         if( continue_ ) {
            continue_ = false;
            thread_.join();
         }

         for( dip::uint ii = 0; ii < viewports_.size(); ++ii ) {
            delete viewports_[ ii ];
         }

         viewports_.clear();
      }

      std::shared_ptr< SliceViewer > clone() {
         Ptr sv = Create( original_, name_, ( dip::uint )width(), ( dip::uint )height() );

         Guard this_guard( *this );
         Guard other_guard( *sv );

         sv->options() = options();

         return sv;
      }

      ViewingOptions& options() override { return options_; }
      const dip::Image& image() override { return image_; }
      const dip::Image& original() override { return original_; }

      void setImage( const dip::Image& image ) override {
         Guard guard( *this );
         original_ = image;
         refresh_seq_++;
      }

      void refreshImage() {
         Guard guard( *this );
         refresh_seq_++;
      }

      /// \brief Update linked viewers.
      DIPVIEWER_EXPORT void updateLinkedViewers();

      /// \brief Link this viewer to another, compatible one.
      DIPVIEWER_EXPORT void link( SliceViewer& other );

   protected:
      DIPVIEWER_EXPORT explicit SliceViewer( const dip::Image& image, std::string name = "SliceViewer", dip::uint width = 0, dip::uint height = 0 );

      DIPVIEWER_EXPORT void create() override;
      DIPVIEWER_EXPORT void reshape( int width, int height ) override;
      DIPVIEWER_EXPORT void draw() override;
      DIPVIEWER_EXPORT void key( unsigned char k, int x, int y, int mods ) override;
      DIPVIEWER_EXPORT void click( int button, int state, int x, int y, int mods ) override;
      DIPVIEWER_EXPORT void motion( int x, int y ) override;

      DIPVIEWER_EXPORT void place();
      DIPVIEWER_EXPORT ViewPort* viewport( int x, int y );
      DIPVIEWER_EXPORT void calculateTextures();
};

class DIPVIEWER_CLASS_EXPORT SliceViewPort : public ViewPort {
   protected:
      class SliceViewer* viewer_;
      SliceView* view_ = nullptr;
      int drag_x_, drag_y_, drag_mods_;
      dip::sint roi_start_, roi_end_, roi_dim_, roi_edge_;

   public:
      explicit SliceViewPort( class SliceViewer* viewer ) : ViewPort( viewer ), viewer_( viewer ) {}
      SliceViewPort( SliceViewPort const& ) = delete;
      SliceViewPort( SliceViewPort&& ) = default;
      SliceViewPort& operator=( SliceViewPort const& ) = delete;
      SliceViewPort& operator=( SliceViewPort&& ) = default;

      ~SliceViewPort() override {
         delete view_;
      }

      void rebuild() override { view()->rebuild(); }
      DIPVIEWER_EXPORT void render() override;
      DIPVIEWER_EXPORT void click( int button, int state, int x, int y, int mods ) override;
      DIPVIEWER_EXPORT void motion( int button, int x, int y ) override;

      void setView( SliceView* view ) { view_ = view; }
      SliceView* view() { return view_; }

   protected:
      DIPVIEWER_EXPORT void screenToView( int x, int y, double* ix, double* iy ) override;
};

/// \endgroup

} // namespace viewer
} // namespace dip

#endif // DIP_VIEWER_SLICE_H
