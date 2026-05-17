/*
 * (c)2017, Wouter Caarls
 * (c)2026, Cris Luengo
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
      Image projected_;       ///< Projected (2D) image.
      Image colored_;         ///< Colored (RGB) image.
      ColorSpaceManager csm_; ///< For \ref ViewingOptions::LookupTable::ColorSpace

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
      Image original_, image_;

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
      static Ptr Create( const Image& image, String name = "SliceViewer", dip::uint width = 0, dip::uint height = 0 ) {
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

      /// \brief Duplicate the SliceViewer window.
      std::shared_ptr< SliceViewer > clone() {
         Ptr sv = Create( original_, name_, ( dip::uint )width(), ( dip::uint )height() );

         Guard this_guard( *this );
         Guard other_guard( *sv );

         sv->options() = options();

         return sv;
      }

      /// \brief Return the (modifyable) options struct for this image display.
      ViewingOptions& options() override { return options_; }

      /// \brief Return the image shown.
      const Image& image() override { return image_; }

      /// \brief Return the original image shown.
      const Image& original() override { return original_; }

      /// \brief Changes the image shown.
      void setImage( const Image& image ) override {
         Guard guard( *this );
         original_ = image;
         refresh_seq_++;
      }

      /// \brief Recompute the display.
      void refreshImage() {
         Guard guard( *this );
         refresh_seq_++;
      }

      /// \brief Update linked viewers.
      DIPVIEWER_EXPORT void updateLinkedViewers();

      /// \brief Link this viewer to another, compatible one.
      DIPVIEWER_EXPORT void link( SliceViewer& other );

      // Convenience functions

      /// \brief Return the visualized dimensions (MainX, MainY, LeftX, TopY). -1 means no dimension is used.
      IntegerArray dims() const {
         return options_.dims_;
      }
      /// \brief Set the dimensions to visualize (MainX, MainY, LeftX, TopY). Use -1 to not map to any image dimension.
      void setDims( IntegerArray const& dims ) {
         DIP_THROW_IF( dims.size() > 4, E::ARRAY_PARAMETER_WRONG_LENGTH );
         DIP_THROW_IF(( dims < dip::sint{ -1 } ).any() || ( dims >= static_cast< dip::sint >( image_.Dimensionality() )).any(), E::INDEX_OUT_OF_RANGE );
         // Fill unspecified dimensions with -1
         Guard guard( *this );
         IntegerArray newdims( 4, -1 );
         for( dip::uint idx = 0; idx < dims.size(); ++idx ) {
            for( dip::uint idx2 = 0; idx2 < idx; ++idx2 ) {
               DIP_THROW_IF( dims[ idx2 ] != -1 && dims[ idx2 ] == dims[ idx ], E::INDEX_OUT_OF_RANGE );
            }
            newdims[ idx ] = dims[ idx ];
         }
         // By default, both Z projections use the same axis.
         if( dims.size() == 3 ) {
            newdims[ 3 ] = dims[ 2 ];
         }
         options_.dims_ = newdims;
      }

      /// \brief Get coordinates of selected point, which also determines which slice is shown.
      UnsignedArray operatingPoint() const {
         return options_.operating_point_;
      }
      /// \brief Set coordinates of selected point, which also determines which slice is shown.
      void setOperatingPoint( UnsignedArray const& point ) {
         Guard guard( *this );
         DIP_THROW_IF( !( point < image_.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
         options_.operating_point_ = point;
         updateLinkedViewers();
      }

      /// \brief Get what to do with complex numbers. One of: `"real"`, `"imag"`, `"magnitude"`, `"phase"`.
      String complex() const {
         return ViewingOptions::ComplexToRealAsString( options_.complex_ );
      }
      /// \brief Set what to do with complex numbers. One of: `"real"`, `"imag"`, `"magnitude"`, `"phase"`.
      void setComplex( String const& complex ) {
         Guard guard( *this );
         options_.complex_ = ViewingOptions::TranslateComplexToRealString( complex );
      }

      /// \brief Get type of projection. One of: `"none"`, `"min"`, `"mean"`, `"max"`.
      String projection() const {
         return ViewingOptions::ProjectionAsString( options_.projection_ );
      }
      /// \brief Set type of projection. One of: `"none"`, `"min"`, `"mean"`, `"max"`.
      void setProjection( String const& projection ) {
         Guard guard( *this );
         options_.projection_ = ViewingOptions::TranslateProjectionString( projection );
      }

      /// \brief Get labels used for axes. A string, one character per axis.
      String labels() const {
         return options_.labels_;
      }
      /// \brief Set labels to use for axes. A string, one character per axis.
      void setLabels( String const& labels ) {
         Guard guard( *this );
         DIP_THROW_IF( labels.empty(), E::INVALID_PARAMETER );
         options_.labels_ = labels;
      }

      /// \brief Get mapped value range (colorbar limits).
      FloatRange mappingRange() const {
         return options_.mapping_range_;
      }
      /// \brief Set mapped value range (colorbar limits).
      void setMappingRange( FloatRange const& range ) {
         Guard guard( *this );
         options_.mapping_range_ = range;
      }

      /// \brief Get grey-value mapping options.
      String mapping() const {
         return ViewingOptions::MappingAsString( options_.mapping_ );
      }
      /// \brief Set grey-value mapping options, also sets mapping_range.
      void setMapping( String const& mapping ) {
         Guard guard( *this );
         options_.mapping_ = ViewingOptions::TranslateMappingString( mapping );
         options_.setMappingRange( options_.mapping_ );
      }

      /// \brief Get tensor element visualized.
      dip::uint element() const {
         return options_.element_;
      }
      /// \brief Set tensor element to visualize.
      void setElement( dip::uint element ) {
         Guard guard( *this );
         DIP_THROW_IF( element >= image_.TensorElements(), E::INDEX_OUT_OF_RANGE );
         options_.element_ = element;
      }

      /// \brief Get grey-value to color mapping option. One of: "original, "ternary", "grey", "sequential", "divergent", "periodic", "labels".
      String lookupTable() const {
         return ViewingOptions::LookupTableAsString( options_.lut_ );
      }
      /// \brief Set grey-value to color mapping option. One of: "original, "ternary", "grey", "sequential", "divergent", "periodic", "labels".
      void setLookupTable( String const& lookupTable ) {
         Guard guard( *this );
         options_.lut_ = ViewingOptions::TranslateLookupTableString( lookupTable );
      }

      /// \brief Get the zoom factor per dimension.
      FloatArray zoom() const {
         return options_.zoom_;
      }
      /// \brief Set the zoom factor per dimension. Also determines relative viewport sizes.
      void setZoom( FloatArray const& zoom ) {
         Guard guard( *this );
         DIP_THROW_IF( zoom.size() != image_.Dimensionality(), E::DIMENSIONALITIES_DONT_MATCH );
         DIP_THROW_IF( ( zoom <= 0. ).any(), E::PARAMETER_OUT_OF_RANGE );
         options_.zoom_ = zoom;
         updateLinkedViewers();
      }

      /// \brief Get the display origin.
      FloatArray origin() const {
         return options_.origin_;
      }
      /// \brief Set the display origin, for moving the image around.
      void setOrigin( FloatArray const& origin ) {
         Guard guard( *this );
         DIP_THROW_IF( origin.size() != image_.Dimensionality(), E::DIMENSIONALITIES_DONT_MATCH );
         options_.origin_ = origin;
         updateLinkedViewers();
      }

   protected:
      DIPVIEWER_EXPORT explicit SliceViewer( const Image& image, String name = "SliceViewer", dip::uint width = 0, dip::uint height = 0 );

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
