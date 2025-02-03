/*
 * (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
 * (c)2022, Cris Luengo.
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

#ifndef DIP_PYDIP_H
#define DIP_PYDIP_H

#include "diplib.h" // IWYU pragma: export
#include "diplib/random.h"
#include "diplib/file_io.h"  // for dip::FileInformation
#include "diplib/private/robin_map.h"

// IWYU pragma: begin_exports
#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>
// IWYU pragma: end_exports

#include "documentation_strings.h"

using namespace pybind11::literals;
namespace py = pybind11;

/* THINGS I'VE LEARNED SO FAR ABOUT PYBIND11:
 *
 * - Default parameter values seem to be converted to Python types, and then translated back to C++ when needed.
 *   This means that both the `load` and `cast` members of `type_caster` are very important, even if data goes
 *   only into the function.
 *
 * - `handle` references a python object without ownership, `object` references and increases the ref counter.
 *   Note when you are using which! `object::release` is your friend!
 *
 * - `py::implicitly_convertible< py::buffer, dip::Image >()` makes it so that a custom type constructor gets
 *   called to try to match up input parameter types. You cannot define a `type_caster` for a type that you
 *   have exposed to Python.
 *
 */


// Declaration of functions in other files in the interface
void init_image( py::module& m );
void init_math( py::module& m );
void init_statistics( py::module& m );
void init_filtering( py::module& m );
void init_morphology( py::module& m );
void init_analysis( py::module& m );
void init_segmentation( py::module& m );
void init_measurement( py::module& m );
void init_histogram( py::module& m );
void init_generation( py::module& m );
void init_assorted( py::module& m );

dip::Random& RandomNumberGenerator();

void OptionallyReverseDimensions( dip::Image& img );
void OptionallyReverseDimensions( dip::FileInformation& fi );

inline void ReverseDimensions( dip::FileInformation& fi ) {
   fi.sizes.reverse();
   fi.pixelSize.Reverse( fi.sizes.size() );
   fi.origin.reverse(); // let's hope this array has the right number of elements...
}


// Mapping namedtuple type name to type object. This avoids creating multiple namedtuple types with the same name.
// Multiple calls to the same function returning a namedtuple will return an object of the same type.
// It also probably speeds up stuff a tiny bit.
extern tsl::robin_map< std::string, py::object > map;

// This function creates a named tuple. This is a type in Python that behaves like a tuple but also like
// a struct with member names.
template< typename... Args >
inline py::object CreateNamedTuple( char const* tuple_name, char const* member_names, Args&& ...values ) {
   if( !map.contains( tuple_name )) {
      py::object namedtuple = py::module::import( "collections" ).attr( "namedtuple" );
      py::object type = namedtuple( tuple_name, member_names );
      map[ tuple_name ] = type;
   }
   return map[ tuple_name ]( std::forward< Args >( values )... );
}

// Note that this macro must be used inside the pybind11::detail namespace
// `diptype` is the DIPlib type without the `dip::` in front. `name` is the name for the type in Python,
// which doesn't need to be the same. We call `CreateNamedTuple( name, values, ... )`.
#define DIP_OUTPUT_TYPE_CASTER( diptype, name, values, ... ) \
template<> class type_caster< dip:: diptype > { public: \
using type = dip:: diptype; \
bool load( handle /*src*/, bool /*convert*/ ) { return false; } \
static handle cast( dip:: diptype const& src, return_value_policy /*policy*/, handle /*parent*/ ) { \
return CreateNamedTuple( name, values, __VA_ARGS__ ).release(); } \
PYBIND11_TYPE_CASTER( type, _( name )); };


namespace pybind11 {

// py::buffer type that implicitly casts to c-style dense double arrays
typedef array_t< dip::dfloat, array::c_style | array::forcecast > double_array_t;

namespace detail {

// Cast Python list types to our custom dynamic array type
template< typename Type >
struct type_caster< dip::DimensionArray< Type >> : public list_caster< dip::DimensionArray< Type >, Type > {
   using list_caster< dip::DimensionArray< Type >, Type >::value;
   using value_conv = make_caster< Type >;

   bool load( handle src, bool convert ) {
      if( isinstance< sequence >( src ) || isinstance< str >( src )) {
         return list_caster< dip::DimensionArray< Type >, Type >::load( src, convert );
      }

      // Allow scalars to be interpreted as a single-element array
      value.clear();
      value_conv conv;
      if( !conv.load( src, convert )) {
         return false;
      }
      value.push_back( cast_op< Type&& >( std::move( conv )));

      return true;
   }
};

// Cast Python string to dip::DataType
template<>
class type_caster< dip::DataType > {
   public:
      using type = dip::DataType;
      bool load( handle src, bool /*convert*/) {
         if( !src ) {
            return false;
         }
         if( PYBIND11_BYTES_CHECK( src.ptr() ) || PyUnicode_Check( src.ptr() )) {
            value = dip::DataType( src.cast< dip::String >() );
            //std::cout << "   Result: " << value << std::endl;
            return true;
         }
         return false;
      }
      static handle cast( dip::DataType const& src, return_value_policy /*policy*/, handle /*parent*/ ) {
         return py::cast( src.Name() ).release();
      }
   PYBIND11_TYPE_CASTER( type, _( "DataType" ));
};

// Cast Python string to dip::Tensor::Shape
template<>
class type_caster< dip::Tensor::Shape > {
   public:
      using type = dip::Tensor::Shape;
      bool load( handle src, bool /*convert*/ ) {
         if( !src ) {
            return false;
         }
         if( PYBIND11_BYTES_CHECK( src.ptr() ) || PyUnicode_Check( src.ptr() )) {
            value = dip::Tensor::ShapeFromString( src.cast< dip::String >() );
            //std::cout << "   Result: " << value << std::endl;
            return true;
         }
         return false;
      }
      static handle cast( dip::Tensor::Shape const& src, return_value_policy /*policy*/, handle /*parent*/ ) {
         return py::cast( dip::Tensor::ShapeToString( src )).release();
      }
   PYBIND11_TYPE_CASTER( type, _( "TensorShape" ));
};

// Cast Python slice to dip::Range
template<>
class type_caster< dip::Range > {
   public:
      using type = dip::Range;
      bool load( handle src, bool /*convert*/ ) {
         //std::cout << "Executing py::type_caster<dip::Range>::load\n";
         if( !src ) {
            //std::cout << "   Input is not\n";
            return false;
         }
         if( PySlice_Check( src.ptr() )) {
            auto* ptr = reinterpret_cast< PySliceObject* >( src.ptr() );
            dip::sint start = 0;
            dip::sint stop = 0;
            dip::sint step = 1;
            // Alternative: PySlice_Unpack( src.ptr(), &start, &stop, &step );
            if( PyNone_Check( ptr->step )) {
               step = 1;
            } else if( PyLong_Check( ptr->step )) {
               step = PyLong_AsSsize_t( ptr->step );
            } else {
               return false;
            }
            if( PyNone_Check( ptr->start )) {
               start = step < 0 ? -1 : 0;
            } else if( PyLong_Check( ptr->start )) {
               start = PyLong_AsSsize_t( ptr->start );
            } else {
               return false;
            }
            if( PyNone_Check( ptr->stop )) {
               stop = step < 0 ? 0 : -1;
            } else if( PyLong_Check( ptr->stop )) {
               stop = PyLong_AsSsize_t( ptr->stop );
            } else {
               return false;
            }
            if( step < 0 ) {
               step = -step;
            }
            //std::cout << "   value == " << start << ":" << stop << ":" << step << std::endl;
            value = dip::Range( start, stop, static_cast< dip::uint >( step ));
            return true;
         }
         if( PyLong_Check( src.ptr() )) {
            value = dip::Range( src.cast< dip::sint >() );
            return true;
         }
         return false;
      }
      static handle cast( dip::Range const& src, return_value_policy /*policy*/, handle /*parent*/ ) {
         return slice( src.start, src.stop, static_cast< dip::sint >( src.step )).release();
      }
   PYBIND11_TYPE_CASTER( type, _( "slice" ));
};

// Cast Python scalar value to dip::Image::Sample
template<>
class type_caster< dip::Image::Sample > {
   public:
      using type = dip::Image::Sample;
      bool load( handle src, bool /*convert*/ ) {
         //std::cout << "Executing py::type_caster<dip::Sample>::load\n";
         if( !src ) {
            //std::cout << "   Input is not\n";
            return false;
         }
         if( PyBool_Check( src.ptr() )) {
            //std::cout << "   Input is bool\n";
            value.swap( dip::Image::Sample( src.cast< bool >() ));
         } else if( PyLong_Check( src.ptr() )) {
            //std::cout << "   Input is int\n";
            value.swap( dip::Image::Sample( src.cast< dip::sint >() ));
         } else if( PyFloat_Check( src.ptr() )) {
            //std::cout << "   Input is float\n";
            value.swap( dip::Image::Sample( src.cast< dip::dfloat >() ));
         } else if( PyComplex_Check( src.ptr() )) {
            //std::cout << "   Input is complex\n";
            value.swap( dip::Image::Sample( src.cast< dip::dcomplex >() ));
         } else {
            //std::cout << "   Input is not a scalar type\n";
            return false;
         }
         //std::cout << "   Result: " << value << std::endl;
         return true;
      }
      static handle cast( dip::Image::Sample const& src, return_value_policy /*policy*/, handle /*parent*/ ) {
         //std::cout << "Executing py::type_caster<dip::Sample>::cast\n";
         py::object out;
         if( src.DataType().IsBinary() ) {
            //std::cout << "   Casting to bool\n";
            out = py::cast( static_cast< bool >( src ));
         } else if( src.DataType().IsComplex() ) {
            //std::cout << "   Casting to complex\n";
            out = py::cast( static_cast< dip::dcomplex >( src ));
         } else if( src.DataType().IsFloat() ) {
            //std::cout << "   Casting to float\n";
            out = py::cast( static_cast< dip::dfloat >( src ));
         } else { // IsInteger()
            //std::cout << "   Casting to int\n";
            out = py::cast( static_cast< dip::sint >( src ));
         }
         return out.release();
      }
   PYBIND11_TYPE_CASTER( type, _( "Sample" ));
};

// Cast Python list or scalar to dip::Image::Pixel
template<>
class type_caster< dip::Image::Pixel > {
   public:
      using type = dip::Image::Pixel;
      using sample_conv = make_caster< dip::Image::Sample >;
      bool load( handle src, bool convert ) {
         //std::cout << "Executing py::type_caster<dip::Pixel>::load\n";
         if( !src ) {
            //std::cout << "   Input is not\n";
            return false;
         }
         if( PyList_Check( src.ptr() )) {
            py::list list = reinterpret_borrow< py::list >( src );
            dip::uint n = list.size();
            if( n == 0 ) {
               return false;
            }
            if( PyBool_Check( list[ 0 ].ptr() )) {
               //std::cout << "   Input is bool\n";
               value.swap( dip::Image::Pixel( dip::DT_BIN, n ));
               auto it = value.begin();
               for( auto in : src ) {
                  *it = in.cast< bool >();
                  ++it;
               }
            } else if( PyLong_Check( list[ 0 ].ptr() )) {
               //std::cout << "   Input is int\n";
               value.swap( dip::Image::Pixel( dip::DT_SINT64, n ));
               auto it = value.begin();
               for( auto in : src ) {
                  *it = in.cast< dip::sint64 >();
                  ++it;
               }
            } else if( PyFloat_Check( list[ 0 ].ptr() )) {
               //std::cout << "   Input is float\n";
               value.swap( dip::Image::Pixel( dip::DT_DFLOAT, n ));
               auto it = value.begin();
               for( auto in : src ) {
                  *it = in.cast< dip::dfloat >();
                  ++it;
               }
            } else if( PyComplex_Check( list[ 0 ].ptr() )) {
               //std::cout << "   Input is complex\n";
               value.swap( dip::Image::Pixel( dip::DT_DCOMPLEX, n ));
               auto it = value.begin();
               for( auto in : src ) {
                  *it = in.cast< dip::dcomplex >();
                  ++it;
               }
            } else {
               //std::cout << "   Input is not a scalar type\n";
               return false;
            }
            //std::cout << "   Result: " << value << std::endl;
            return true;
         }
         sample_conv conv;
         if ( !conv.load( src, convert )) {
            return false;
         }
         value.swap( dip::Image::Pixel( std::move( conv )));
         return true;
      }
      static handle cast( dip::Image::Pixel const& src, return_value_policy /*policy*/, handle /*parent*/ ) {
         //std::cout << "Executing py::type_caster<dip::Pixel>::cast\n";
         py::list out;
         if( src.DataType().IsBinary() ) {
            //std::cout << "   Casting to bool\n";
            for( auto& in : src ) {
               out.append( py::cast( static_cast< bool >( in )));
            }
         } else if( src.DataType().IsComplex() ) {
            //std::cout << "   Casting to complex\n";
            for( auto& in : src ) {
               out.append( py::cast( static_cast< dip::dcomplex >( in )));
            }
         } else if( src.DataType().IsFloat() ) {
            //std::cout << "   Casting to float\n";
            for( auto& in : src ) {
               out.append( py::cast( static_cast< dip::dfloat >( in )));
            }
         } else { // IsInteger()
            //std::cout << "   Casting to int\n";
            for( auto& in : src ) {
               out.append( py::cast( static_cast< dip::sint >( in )));
            }
         }
         return out.release();
      }
   PYBIND11_TYPE_CASTER( type, _( "Pixel" ));
};

// Cast dip::FileInformation to Python dict, one way only.
template<>
class type_caster< dip::FileInformation > {
   public:
   using type = dip::FileInformation;

   bool load( handle /*src*/, bool /*conver*/ ) { // no conversion from Python to DIPlib
      return false;
   }

   static handle cast( dip::FileInformation const& src, return_value_policy /*policy*/, handle /*parent*/ ) {
      py::dict out;
      out[ "name" ] = py::cast( src.name );
      out[ "fileType" ] = py::cast( src.fileType );
      out[ "dataType" ] = py::cast( src.dataType );
      out[ "significantBits" ] = py::cast( src.significantBits );
      out[ "sizes" ] = py::cast( src.sizes );
      out[ "tensorElements" ] = py::cast( src.tensorElements );
      out[ "colorSpace" ] = py::cast( src.colorSpace );
      out[ "pixelSize" ] = py::cast( src.pixelSize );
      out[ "origin" ] = py::cast( src.origin );
      out[ "numberOfImages" ] = py::cast( src.numberOfImages );
      out[ "history" ] = py::cast( src.history );
      return out.release();
   }

   PYBIND11_TYPE_CASTER( type, _( "FileInformation" ));
};

} // namespace detail
} // namespace pybind11

inline dip::Image ImageOrPixel( py::object const& obj ) {
   pybind11::detail::type_caster< dip::Image > image_caster;
   if ( image_caster.load( obj, true )) {
      return image_caster;
   }
   pybind11::detail::type_caster< dip::Image::Pixel > pixel_caster;
   if ( pixel_caster.load( obj, true )) {
      return dip::Image( pixel_caster ); // static_cast< dip::Image::Pixel>( caster ));
   }
   throw dip::RunTimeError( "Cannot convert input to dip::Image" );
}

#endif // DIP_PYDIP_H
