/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0
 *
 * (c)2017, Flagship Biosciences, Inc., written by Cris Luengo.
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

#include "diplib.h"

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>

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
void init_measurement( py::module& m );
void init_assorted( py::module& m );

namespace pybind11 {
namespace detail {

// Cast Python list types to our custom dynamic array type
template< typename Type >
struct type_caster< dip::DimensionArray< Type >>: list_caster< dip::DimensionArray< Type >, Type > {};

// Cast Python string to dip::DataType
template<>
class type_caster< dip::DataType > {
   public:
      using type = dip::DataType;
      bool load( handle src, bool ) {
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
      static handle cast( dip::DataType const& src, return_value_policy, handle ) {
         return py::cast( src.Name() ).release();
      }
   PYBIND11_TYPE_CASTER( type, _( "DataType" ));
};

// Cast Python string to dip::Tensor::Shape
template<>
class type_caster< dip::Tensor::Shape > {
   public:
      using type = dip::Tensor::Shape;
      bool load( handle src, bool ) {
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
      static handle cast( dip::Tensor::Shape const& src, return_value_policy, handle ) {
         return py::cast( dip::Tensor::ShapeToString( src )).release();
      }
   PYBIND11_TYPE_CASTER( type, _( "TensorShape" ));
};

// Cast Python slice to dip::Range
template<>
class type_caster< dip::Range > {
   public:
      using type = dip::Range;
      bool load( handle src, bool ) {
         //std::cout << "Executing py::type_caster<dip::Range>::load\n";
         if( !src ) {
            //std::cout << "   Input is not\n";
            return false;
         }
         if( PySlice_Check( src.ptr() )) {
            auto ptr = reinterpret_cast< PySliceObject* >( src.ptr() );
            dip::sint start, stop, step;
            // Alternative: PySlice_Unpack( src.ptr(), &start, &stop, &step );
            // Do we need to use PYBIND11_LONG_CHECK() here?
            if( PyNone_Check( ptr->step )) {
               step = 1;
            } else if( PYBIND11_LONG_CHECK( ptr->step )) {
               step = PyLong_AsSsize_t( ptr->step );
            } else {
               return false;
            }
            if( PyNone_Check( ptr->start )) {
               start = step < 0 ? -1 : 0;
            } else if( PYBIND11_LONG_CHECK( ptr->start )) {
               start = PyLong_AsSsize_t( ptr->start );
            } else {
               return false;
            }
            if( PyNone_Check( ptr->stop )) {
               stop = step < 0 ? 0 : -1;
            } else if( PYBIND11_LONG_CHECK( ptr->stop )) {
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
         if( PYBIND11_LONG_CHECK( src.ptr() )) {
            value = dip::Range( src.cast< dip::sint >() );
            return true;
         }
         return false;
      }
      static handle cast( dip::Range const& src, return_value_policy, handle ) {
         return slice( src.start, src.stop, static_cast< dip::sint >( src.step )).release();
      }
   PYBIND11_TYPE_CASTER( type, _( "slice" ));
};

// Cast Python scalar value to dip::Image::Sample
template<>
class type_caster< dip::Image::Sample > {
   public:
      using type = dip::Image::Sample;
      bool load( handle src, bool ) {
         //std::cout << "Executing py::type_caster<dip::Sample>::load" << std::endl;
         if( !src ) {
            //std::cout << "   Input is not" << std::endl;
            return false;
         }
         if( PyBool_Check( src.ptr() )) {
            //std::cout << "   Input is bool" << std::endl;
            value.swap( dip::Image::Sample( src.cast< bool >() ));
         } else if( PYBIND11_LONG_CHECK( src.ptr() )) {
            //std::cout << "   Input is int" << std::endl;
            value.swap( dip::Image::Sample( src.cast< dip::sint >() ));
         } else if( PyFloat_Check( src.ptr() )) {
            //std::cout << "   Input is float" << std::endl;
            value.swap( dip::Image::Sample( src.cast< dip::dfloat >() ));
         } else if( PyComplex_Check( src.ptr() )) {
            //std::cout << "   Input is complex" << std::endl;
            value.swap( dip::Image::Sample( src.cast< dip::dcomplex >() ));
         } else {
            //std::cout << "   Input is not a scalar type" << std::endl;
            return false;
         }
         //std::cout << "   Result: " << value << std::endl;
         return true;
      }
      static handle cast( dip::Image::Sample const& src, return_value_policy, handle ) {
         //std::cout << "Executing py::type_caster<dip::Sample>::cast" << std::endl;
         py::object out;
         if( src.DataType().IsBinary() ) {
            //std::cout << "   Casting to bool" << std::endl;
            out = py::cast( static_cast< bool >( src ));
         } else if( src.DataType().IsComplex() ) {
            //std::cout << "   Casting to complex" << std::endl;
            out = py::cast( static_cast< dip::dcomplex >( src ));
         } else if( src.DataType().IsFloat() ) {
            //std::cout << "   Casting to float" << std::endl;
            out = py::cast( static_cast< dip::dfloat >( src ));
         } else { // IsInteger()
            //std::cout << "   Casting to int" << std::endl;
            out = py::cast( static_cast< dip::sint >( src ));
         }
         return out.release();
      }
   PYBIND11_TYPE_CASTER( type, _( "Sample" ));
};

// Cast Python list to dip::Image::Pixel
template<>
class type_caster< dip::Image::Pixel > {
   public:
      using type = dip::Image::Pixel;
      bool load( handle src, bool ) {
         //std::cout << "Executing py::type_caster<dip::Pixel>::load" << std::endl;
         if( !src ) {
            //std::cout << "   Input is not" << std::endl;
            return false;
         }
         if( PyList_Check( src.ptr() )) {
            py::list list = reinterpret_borrow< py::list >( src );
            dip::uint n = list.size();
            if( n == 0 ) {
               return false;
            }
            if( PyBool_Check( list[ 0 ].ptr() )) {
               //std::cout << "   Input is bool" << std::endl;
               value.swap( dip::Image::Pixel( dip::DT_BIN, n ));
               auto it = value.begin();
               for( auto& in : src ) {
                  *it = in.cast< bool >();
                  ++it;
               }
            } else if( PYBIND11_LONG_CHECK( list[ 0 ].ptr() )) {
               //std::cout << "   Input is int" << std::endl;
               value.swap( dip::Image::Pixel( dip::DT_SINT64, n ));
               auto it = value.begin();
               for( auto& in : src ) {
                  *it = in.cast< dip::sint64 >();
                  ++it;
               }
            } else if( PyFloat_Check( list[ 0 ].ptr() )) {
               //std::cout << "   Input is float" << std::endl;
               value.swap( dip::Image::Pixel( dip::DT_DFLOAT, n ));
               auto it = value.begin();
               for( auto& in : src ) {
                  *it = in.cast< dip::dfloat >();
                  ++it;
               }
            } else if( PyComplex_Check( list[ 0 ].ptr() )) {
               //std::cout << "   Input is complex" << std::endl;
               value.swap( dip::Image::Pixel( dip::DT_DCOMPLEX, n ));
               auto it = value.begin();
               for( auto& in : src ) {
                  *it = in.cast< dip::dcomplex >();
                  ++it;
               }
            } else {
               //std::cout << "   Input is not a scalar type" << std::endl;
               return false;
            }
            //std::cout << "   Result: " << value << std::endl;
            return true;
         }
         return false;
      }
      static handle cast( dip::Image::Pixel const& src, return_value_policy, handle ) {
         //std::cout << "Executing py::type_caster<dip::Pixel>::cast" << std::endl;
         py::list out;
         if( src.DataType().IsBinary() ) {
            //std::cout << "   Casting to bool" << std::endl;
            for( auto& in : src ) {
               out.append( py::cast( static_cast< bool >( in )));
            }
         } else if( src.DataType().IsComplex() ) {
            //std::cout << "   Casting to complex" << std::endl;
            for( auto& in : src ) {
               out.append( py::cast( static_cast< dip::dcomplex >( in )));
            }
         } else if( src.DataType().IsFloat() ) {
            //std::cout << "   Casting to float" << std::endl;
            for( auto& in : src ) {
               out.append( py::cast( static_cast< dip::dfloat >( in )));
            }
         } else { // IsInteger()
            //std::cout << "   Casting to int" << std::endl;
            for( auto& in : src ) {
               out.append( py::cast( static_cast< dip::sint >( in )));
            }
         }
         return out.release();
      }
   PYBIND11_TYPE_CASTER( type, _( "Pixel" ));
};

} // nanmespace detail
} // namespace pybind11

#endif // DIP_PYDIP_H
