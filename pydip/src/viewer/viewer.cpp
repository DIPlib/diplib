/*
 * (c)2017, Wouter Caarls
 * (c)2024-2026, Cris Luengo
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

#include <algorithm>

#include "pydip.h"
#include "dipviewer.h"
#include "diplib/viewer/slice.h"

// IWYU pragma: no_include "pythonrun.h"

bool AreDimensionsReversed() {
   return static_cast< py::object >( py::module_::import( "diplib" ).attr( "AreDimensionsReversed" ))().cast< bool >();
}

int drawHook() {
   dip::viewer::Draw();
   return 0;
}

PYBIND11_MODULE( PyDIPviewer, m ) {
   m.doc() = "The portion of the PyDIP module that contains the DIPviewer functionality.";

   // Close all windows on exit
   py::module_::import( "atexit" ).attr( "register" )(
      py::cpp_function( []() {
         dip::viewer::CloseAll();
         if( PyOS_InputHook == &drawHook ) {
            PyOS_InputHook = nullptr;
         }
      } ));

   auto sv = py::class_< dip::viewer::SliceViewer, std::shared_ptr< dip::viewer::SliceViewer > >( m, "SliceViewer" );
   sv.def( "SetImage", &dip::viewer::SliceViewer::setImage, "Sets the image to be visualized." );
   sv.def( "Destroy", &dip::viewer::SliceViewer::destroy, "Marks the window for destruction." );
   sv.def( "RefreshImage", &dip::viewer::SliceViewer::refreshImage, "Force full redraw." );
   sv.def( "Link", &dip::viewer::SliceViewer::link, "Link this viewer to another, compatible one." );
   sv.def( "SetPosition", &dip::viewer::SliceViewer::setPosition, "Set the window's screen position." );
   sv.def( "SetSize", &dip::viewer::SliceViewer::setSize, "Set the window's size." );
   sv.def_property( "dims", &dip::viewer::SliceViewer::dims, &dip::viewer::SliceViewer::setDims,
      "Dimensions to visualize (MainX, MainY, LeftX, TopY). Use -1 to not map to any image dimension." );
   sv.def_property( "operating_point", &dip::viewer::SliceViewer::operatingPoint, &dip::viewer::SliceViewer::setOperatingPoint,
      "Coordinates of selected point, which also determines which slice is shown." );
   sv.def_property( "complex", &dip::viewer::SliceViewer::complex, &dip::viewer::SliceViewer::setComplex,
      "What to do with complex numbers. One of: 'real', 'imag', 'magnitude', 'phase'." );
   sv.def_property( "projection", &dip::viewer::SliceViewer::projection, &dip::viewer::SliceViewer::setProjection,
      "Type of projection. One of: 'none', 'min', 'mean', 'max'." );
   sv.def_property( "labels", &dip::viewer::SliceViewer::labels, &dip::viewer::SliceViewer::setLabels,
      "Labels to use for axes. A string, one character per axis." );
   sv.def_property( "mapping_range", &dip::viewer::SliceViewer::mappingRange, &dip::viewer::SliceViewer::setMappingRange,
      "Mapped value range (colorbar limits)." );
   sv.def_property( "mapping", &dip::viewer::SliceViewer::mapping, &dip::viewer::SliceViewer::setMapping,
      "Grey-value mapping options, sets mapping_range." );
   sv.def_property( "element", &dip::viewer::SliceViewer::element, &dip::viewer::SliceViewer::setElement,
      "Tensor element to visualize." );
   sv.def_property( "lut", &dip::viewer::SliceViewer::lookupTable, &dip::viewer::SliceViewer::setLookupTable,
      "Grey-value to color mapping options. One of: 'original, 'ternary', 'grey', 'sequential', 'divergent', 'periodic', 'labels'." );
   sv.def_property( "zoom", &dip::viewer::SliceViewer::zoom, &dip::viewer::SliceViewer::setZoom,
      "Zoom factor per dimension. Also determines relative viewport sizes." );
   sv.def_property( "origin", &dip::viewer::SliceViewer::origin, &dip::viewer::SliceViewer::setOrigin,
      "Display origin for moving the image around." );

   m.def( "Show", []( dip::Image const& image, dip::String const& title ) {
      if( PyOS_InputHook == nullptr ) {
         PyOS_InputHook = &drawHook;
      }
      auto h = dip::viewer::Show( image, title );
      if( !AreDimensionsReversed() ) {
         // Reverse shown dimensions
         auto& dims = h->options().dims_;
         if( image.Dimensionality() == 0 ) {
            dims = { -1, -1, -1, -1 };
         } else if( image.Dimensionality() == 1 ) {
            dims = { 0, -1, -1, -1 };
         } else if( image.Dimensionality() == 2 ) {
            dims = { 1, 0, -1, -1 };
         } else {
            dims = { 2, 1, 0, 0 };
         }
         // Reverse axis labels
         auto& labels = h->options().labels_;
         while( labels.size() < image.Dimensionality() ) {
            labels.append( labels );
         }
         std::reverse( labels.begin(), labels.begin() + static_cast< dip::sint >( image.Dimensionality() ));
      }
      return h;
   }, "in"_a, "title"_a = "", "Show an image in the slice viewer." );

   m.def( "Draw", &dip::viewer::Draw, "Process user event queue." );

   m.def( "Spin", []() {
      dip::viewer::Spin();
      if( PyOS_InputHook == &drawHook ) {
         PyOS_InputHook = nullptr;
      }
   }, "Wait until all windows are closed." );

   m.def( "CloseAll", []() {
      dip::viewer::CloseAll();
      if( PyOS_InputHook == &drawHook ) {
         PyOS_InputHook = nullptr;
      }
   }, "Close all open windows." );
}
