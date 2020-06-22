/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0 viewer
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

#include "diplib.h"
#include "dipviewer.h"
#include "diplib/viewer/slice.h"

#include "../pydip.h"

PYBIND11_MODULE( PyDIPviewer, m ) {
   auto sv = py::class_< dip::viewer::SliceViewer, std::shared_ptr< dip::viewer::SliceViewer > >( m, "SliceViewer" );
   sv.def( "SetImage", []( dip::viewer::SliceViewer &self, dip::Image const& image ) { dip::viewer::SliceViewer::Guard guard( self ); self.setImage( image ); }, "Sets the image to be visualized." );
   sv.def( "Destroy", &dip::viewer::SliceViewer::destroy, "Marks the window for destruction." );
   sv.def( "RefreshImage", []( dip::viewer::SliceViewer &self ) { dip::viewer::SliceViewer::Guard guard( self ); self.refreshImage( ); }, "Force full redraw." );
   sv.def( "Link", []( dip::viewer::SliceViewer& self, dip::viewer::SliceViewer& other ) { self.link( other ); }, "Link this viewer to another, compatible one.");
   sv.def( "SetPosition", &dip::viewer::SliceViewer::setPosition, "Set the window's screen position." );
   sv.def( "SetSize", &dip::viewer::SliceViewer::setSize, "Set the window's size." );

   sv.def_property( "operating_point", []( dip::viewer::SliceViewer& self ) {
      dip::viewer::SliceViewer::Guard guard( self );
      return self.options().operating_point_;
   }, []( dip::viewer::SliceViewer &self, dip::UnsignedArray const &point ) {
      dip::viewer::SliceViewer::Guard guard( self );
      DIP_THROW_IF( !( point < self.image().Sizes() ), dip::E::COORDINATES_OUT_OF_RANGE );
      self.options().operating_point_ = point;
      self.updateLinkedViewers();
   });

   sv.def_property( "element", []( dip::viewer::SliceViewer& self ) {
      dip::viewer::SliceViewer::Guard guard( self );
      return self.options().element_;
   }, []( dip::viewer::SliceViewer &self, int element ) {
      dip::viewer::SliceViewer::Guard guard( self );
      DIP_THROW_IF( element >= self.image().TensorElements(), dip::E::INDEX_OUT_OF_RANGE );
      self.options().element_ = element;
   });

   sv.def_property( "zoom", []( dip::viewer::SliceViewer& self ) {
      dip::viewer::SliceViewer::Guard guard( self );
      return self.options().zoom_;
   }, []( dip::viewer::SliceViewer &self, dip::FloatArray const &zoom ) {
      dip::viewer::SliceViewer::Guard guard( self );
      DIP_THROW_IF( zoom.size() != self.image().Dimensionality(), dip::E::DIMENSIONALITIES_DONT_MATCH );
      DIP_THROW_IF( ( zoom <= 0. ).any(), dip::E::PARAMETER_OUT_OF_RANGE);
      self.options().zoom_ = zoom;
      self.updateLinkedViewers();
   });

   sv.def_property( "origin", []( dip::viewer::SliceViewer& self ) {
      dip::viewer::SliceViewer::Guard guard( self );
      return self.options().origin_;
   }, []( dip::viewer::SliceViewer &self, dip::FloatArray const &origin ) {
      dip::viewer::SliceViewer::Guard guard( self );
      DIP_THROW_IF( origin.size() != self.image().Dimensionality(), dip::E::DIMENSIONALITIES_DONT_MATCH );
      self.options().origin_ = origin;
      self.updateLinkedViewers();
   });

   m.def( "Show", []( dip::Image const& image, dip::String const& title ) { return dip::viewer::Show( image, title ); }, "in"_a, "title"_a = "", "Show an image in the slice viewer." );
   m.def( "Draw", &dip::viewer::Draw, "Process user event queue." );
   m.def( "Spin", &dip::viewer::Spin, "Wait until all windows are closed." );
   m.def( "CloseAll", &dip::viewer::CloseAll, "Close all open windows." );
}
