/*
 * (c)2017, Wouter Caarls
 * (c)2024, Cris Luengo
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
#include <array>

#include "pydip.h"
#include "dipviewer.h"
#include "diplib/viewer/slice.h"
#include "diplib/private/robin_map.h"

// IWYU pragma: no_include "pythonrun.h"

using namespace pybind11::literals;
namespace py = pybind11;

bool AreDimensionsReversed() {
   return static_cast< py::object >( py::module_::import( "diplib" ).attr( "AreDimensionsReversed" ))().cast< bool >();
}

// std::to_array<>() from C++20
template <typename V, typename... T>
constexpr auto to_array(T&&... t) -> std::array < V, sizeof...(T) > {
   return {{ std::forward<T>(t)... }};
}

template< dip::uint n >
dip::String toString( dip::uint idx, std::array< dip::String, n > options ) {
   DIP_THROW_IF( idx >= n, dip::E::INDEX_OUT_OF_RANGE );
   return options[ idx ];
}

template< dip::uint n >
dip::uint toIndex( dip::String const& str, std::array< dip::String, n > options ) {
   for( dip::uint idx = 0; idx < n; ++idx ) {
      if( options[ idx ] == str ) {
         return idx;
      }
   }
   DIP_THROW_INVALID_FLAG( str );
}

dip::String const& lookupAlias( dip::String const& str, tsl::robin_map< dip::String, dip::String > const& map ) {
   auto it = map.find( str );
   if( it == map.end() ) {
      return str;
   }
   return it->second;
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

   sv.def_property(
      "dims",
      []( dip::viewer::SliceViewer& self ) {
         dip::viewer::SliceViewer::Guard guard( self );
         return self.options().dims_;
      },
      []( dip::viewer::SliceViewer& self, dip::IntegerArray const& dims ) {
         dip::viewer::SliceViewer::Guard guard( self );
         DIP_THROW_IF( dims.size() > 4, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
         DIP_THROW_IF( ( dims < static_cast< dip::sint >( -1 )).any() || ( dims >= static_cast< dip::sint >( self.image().Dimensionality() )).any(), dip::E::INDEX_OUT_OF_RANGE );
         // Fill unspecified dimensions with -1
         dip::IntegerArray newdims( 4, -1 );
         for( dip::uint idx = 0; idx < dims.size(); ++idx ) {
            for( dip::uint idx2 = 0; idx2 < idx; ++idx2 ) {
               DIP_THROW_IF( dims[ idx2 ] != -1 && dims[ idx2 ] == dims[ idx ], dip::E::INDEX_OUT_OF_RANGE );
            }
            newdims[ idx ] = dims[ idx ];
         }
         // By default, both Z projections use the same axis.
         if( dims.size() == 3 ) {
            newdims[ 3 ] = dims[ 2 ];
         }
         self.options().dims_ = newdims;
      }, "Dimensions to visualize (MainX, MainY, LeftX, TopY). Use -1 to not map to any image dimension." );

   sv.def_property(
      "operating_point",
      []( dip::viewer::SliceViewer& self ) {
         dip::viewer::SliceViewer::Guard guard( self );
         return self.options().operating_point_;
      },
      []( dip::viewer::SliceViewer& self, dip::UnsignedArray const& point ) {
         dip::viewer::SliceViewer::Guard guard( self );
         DIP_THROW_IF( !( point < self.image().Sizes() ), dip::E::COORDINATES_OUT_OF_RANGE );
         self.options().operating_point_ = point;
         self.updateLinkedViewers();
      }, "Coordinates of selected point, which also determines which slice is shown." );

   auto complexToRealOpts = to_array< dip::String >( "real", "imag", "magnitude", "phase" );
   sv.def_property(
      "complex",
      [=]( dip::viewer::SliceViewer& self ) {
         return toString( static_cast< dip::uint >( self.options().complex_ ), complexToRealOpts );
      },
      [=]( dip::viewer::SliceViewer& self, dip::String const& complex ) {
         dip::viewer::SliceViewer::Guard guard( self );
         self.options().complex_ = static_cast< dip::viewer::ViewingOptions::ComplexToReal >( toIndex( complex, complexToRealOpts ));
      }, "What to do with complex numbers. One of: 'real', 'imag', 'magnitude', 'phase'." );

   auto projectionOpts = to_array< dip::String >( "none", "min", "mean", "max" );
   sv.def_property(
      "projection",
      [=]( dip::viewer::SliceViewer& self ) {
         return toString( static_cast< dip::uint >( self.options().projection_ ), projectionOpts );
      },
      [=]( dip::viewer::SliceViewer& self, dip::String const& projection ) {
         dip::viewer::SliceViewer::Guard guard( self );
         self.options().projection_ = ( projection == "slice" )
               ? dip::viewer::ViewingOptions::Projection::None
               : static_cast< dip::viewer::ViewingOptions::Projection >( toIndex( projection, projectionOpts ));
      }, "Type of projection. One of: 'none', 'min', 'mean', 'max'." );

   sv.def_property(
      "labels",
      []( dip::viewer::SliceViewer& self ) {
         dip::viewer::SliceViewer::Guard guard( self );
         return self.options().labels_;
      },
      []( dip::viewer::SliceViewer& self, dip::String const& labels ) {
         dip::viewer::SliceViewer::Guard guard( self );
         DIP_THROW_IF( labels.empty(), dip::E::INVALID_PARAMETER );
         self.options().labels_ = labels;
      }, "Labels to use for axes. A string, one character per axis." );

   sv.def_property(
      "mapping_range",
      []( dip::viewer::SliceViewer& self ) {
         dip::viewer::SliceViewer::Guard guard( self );
         return self.options().mapping_range_;
      },
      []( dip::viewer::SliceViewer& self, dip::FloatArray const& range ) {
         dip::viewer::SliceViewer::Guard guard( self );
         DIP_THROW_IF( range.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
         self.options().mapping_range_ = dip::viewer::FloatRange( range[ 0 ], range[ 1 ] );
      }, "Mapped value range (colorbar limits)." );

   auto mappingOpts = to_array< dip::String >( "unit", "angle", "8bit", "lin", "base", "log" );
   tsl::robin_map< dip::String, dip::String > mappingAliases{
      {"normal", "8bit"},
      {"linear", "lin"},
      {"all", "lin"},
      {"based", "base"},
      // Not possible: "12bit", "16bit", "s8bit", "s12bit", "s16bit", "orientation", "percentile", "modulo", "labels"
   };
   sv.def_property(
      "mapping",
      [=]( dip::viewer::SliceViewer& self ) {
         return toString( static_cast< dip::uint >( self.options().mapping_ ), mappingOpts );
      },
      [=]( dip::viewer::SliceViewer& self, dip::String const& mapping ) {
         dip::viewer::SliceViewer::Guard guard( self );
         dip::String const& newMapping = lookupAlias( mapping, mappingAliases );
         self.options().mapping_ = static_cast< dip::viewer::ViewingOptions::Mapping >( toIndex( newMapping, mappingOpts ));
         self.options().setMappingRange( self.options().mapping_ );
      }, "Grey-value mapping options, sets mapping_range." );

   sv.def_property(
      "element",
      []( dip::viewer::SliceViewer& self ) {
         dip::viewer::SliceViewer::Guard guard( self );
         return self.options().element_;
      },
      []( dip::viewer::SliceViewer& self, dip::uint element ) {
         dip::viewer::SliceViewer::Guard guard( self );
         DIP_THROW_IF( element >= self.image().TensorElements(), dip::E::INDEX_OUT_OF_RANGE );
         self.options().element_ = element;
      }, "Tensor element to visualize." );

   auto lutOpts = to_array< dip::String >( "original", "ternary", "grey", "sequential", "divergent", "periodic", "labels" );
   tsl::robin_map< dip::String, dip::String > lutAliases{
      {"linear", "sequential"},
      {"diverging", "divergent"},
      {"cyclic", "periodic"},
      {"label", "labels"},
      {"gray", "grey"},
   };
   sv.def_property(
      "lut",
      [=]( dip::viewer::SliceViewer& self ) {
         return toString( static_cast< dip::uint >( self.options().lut_ ), lutOpts );
      },
      [=]( dip::viewer::SliceViewer& self, dip::String const& lut ) {
         dip::viewer::SliceViewer::Guard guard( self );
         dip::String const& newLut = lookupAlias( lut, lutAliases );
         self.options().lut_ = static_cast< dip::viewer::ViewingOptions::LookupTable >( toIndex( newLut, lutOpts ));
      }, "Grey-value to color mapping options. One of: 'original, 'ternary', 'grey', 'sequential', 'divergent', 'periodic', 'labels'." );

   sv.def_property(
      "zoom",
      []( dip::viewer::SliceViewer& self ) {
         dip::viewer::SliceViewer::Guard guard( self );
         return self.options().zoom_;
      },
      []( dip::viewer::SliceViewer& self, dip::FloatArray const& zoom ) {
         dip::viewer::SliceViewer::Guard guard( self );
         DIP_THROW_IF( zoom.size() != self.image().Dimensionality(), dip::E::DIMENSIONALITIES_DONT_MATCH );
         DIP_THROW_IF( ( zoom <= 0. ).any(), dip::E::PARAMETER_OUT_OF_RANGE );
         self.options().zoom_ = zoom;
         self.updateLinkedViewers();
      }, "Zoom factor per dimension. Also determines relative viewport sizes." );

   sv.def_property(
      "origin",
      []( dip::viewer::SliceViewer& self ) {
         dip::viewer::SliceViewer::Guard guard( self );
         return self.options().origin_;
      },
      []( dip::viewer::SliceViewer& self, dip::FloatArray const& origin ) {
         dip::viewer::SliceViewer::Guard guard( self );
         DIP_THROW_IF( origin.size() != self.image().Dimensionality(), dip::E::DIMENSIONALITIES_DONT_MATCH );
         self.options().origin_ = origin;
         self.updateLinkedViewers();
      }, "Display origin for moving the image around." );

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
