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

#undef DIP__ENABLE_DOCTEST
#include "diplib.h"

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>

using namespace pybind11::literals;
namespace py = pybind11;

namespace pybind11 {
namespace detail {

// Cast Python list types to our custom dynamic array type
template< typename Type >
struct type_caster< dip::DimensionArray< Type >>: list_caster< dip::DimensionArray< Type >, Type > {};

// Cast Python slice to dip::Range
template<>
class type_caster< dip::Range > {
   public:
      using type=dip::Range;
      bool load( handle src, bool ) {
         std::cout << "Executing py::type_caster<dip::Range>\n";
         if( !src || !PySlice_Check( src.ptr())) {
            //std::cout << "   Input is not a slice\n";
            return false;
         }
         auto ptr = reinterpret_cast< PySliceObject* >( src.ptr() );
         dip::sint start, stop, step;
         if( PyNone_Check( ptr->step )) {
            //std::cout << "   slice.step == None\n";
            step = 1;
         } else {
            //PyLong_Check()
            step = reinterpret_borrow< object >( ptr->step ).cast< dip::sint >();
            //std::cout << "   slice.step == " << step << std::endl;
         }
         // Start with positive step: None -> 0, otherwise -> start
         // Start with negative step: None -> -1, otherwise -> start
         if( PyNone_Check( ptr->start )) {
            //std::cout << "   slice.start == None\n";
            start = step < 0 ? -1 : 0;
         } else {
            //PyLong_Check()
            start = reinterpret_borrow< object >( ptr->start ).cast< dip::sint >();
            //std::cout << "   slice.start == " << start << std::endl;
         }
         // Stop with positive step: None -> -1, <0 -> stop-1, >0 -> stop-1, ==0 -> error
         // Stop with negative step: None -> -1, otherwise -> stop+1
         if( PyNone_Check( ptr->stop )) {
            //std::cout << "   slice.stop == None\n";
            stop = -1;
         } else {
            //PyLong_Check()
            stop = reinterpret_borrow< object >( ptr->stop ).cast< dip::sint >();
            //std::cout << "   slice.stop == " << stop << std::endl;
            stop += step < 0 ? 1 : -1;
         }
         if( step < 0 ) {
            std::swap( start, stop );
            step = -step;
         }
         //std::cout << "   value == " << start << ":" << stop << ":" << step << std::endl;
         value = dip::Range( start, stop, static_cast< dip::uint >( step ));
         // NOTE: For an originally empty range, this leads to two pixels selected, but this is
         // difficult to test for here without knowing the size of the image being indexed. The empty
         // range is not legal (or possible) in DIPlib.
         // TODO: Should we fudge with Python's definitions for slicing, and have them work like DIPlib's?
         // - If we do, we'll get complaints from people not understanding the indexing.
         // - If we don't, it'll be more difficult to translate code from Python to C++.
         return true;
      }
      static handle cast(
            const type& src,
            return_value_policy /* policy */,
            handle /* parent */
      ) {
         // TODO: This is not correct, but we don't really use it (yet).
         return slice( src.start, src.stop, static_cast< dip::sint >( src.step ));
      }
   PYBIND11_TYPE_CASTER( type, _( "slice" ));
};


/*
// Cast any Python type that exposes a raw buffer to a dip::Image
template<>
struct type_caster< dip::Image > {
   private:
      dip::Image value;
   public:
      static PYBIND11_DESCR name() { return type_descr(_("dip.Image")); }
      operator dip::Image*() { return &value; }
      operator dip::Image&() { return value; }
      bool load( handle src, bool convert ) {
         // Coerce into an array, but don't do type conversion yet; the copy below handles it.
         auto buf = py::array::ensure( src );
         if( !buf ) {
            return false;
         }
         // Allocate the new type, then build a numpy reference into it
         dip::DataType datatype;
         switch( buf.dtype().kind()) {
            case py::format_descriptor< bool >::c:
               datatype = dip::DT_BIN;
               break;
            case py::format_descriptor< dip::uint8 >::c:
               datatype = dip::DT_UINT8;
               break;
            case py::format_descriptor< dip::uint16 >::c:
               datatype = dip::DT_UINT16;
               break;
            case py::format_descriptor< dip::uint32 >::c:
               datatype = dip::DT_UINT32;
               break;
            case py::format_descriptor< dip::sint8 >::c:
               datatype = dip::DT_SINT8;
               break;
            case py::format_descriptor< dip::sint16 >::c:
               datatype = dip::DT_SINT16;
               break;
            case py::format_descriptor< dip::sint32 >::c:
               datatype = dip::DT_SINT32;
               break;
            case py::format_descriptor< dip::sfloat >::c:
               datatype = dip::DT_SFLOAT;
               break;
            case py::format_descriptor< dip::dfloat >::c:
               datatype = dip::DT_DFLOAT;
               break;
            case py::format_descriptor< dip::scomplex >::c:
               datatype = dip::DT_SCOMPLEX;
               break;
            case py::format_descriptor< dip::dcomplex >::c:
               datatype = dip::DT_DCOMPLEX;
               break;
            default:
               return false;
         }
         // Sizes
         auto nDims = buf.ndim();
         dip::UnsignedArray sizes( nDims, 1 );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            sizes[ ii ] = buf.shape()[ ii ];
         }
         // Strides
         dip::IntegerArray strides( nDims, 1 );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            dip::sint sb = buf.strides()[ ii ];
            dip::sint is = static_cast< dip::sint >( buf.itemsize());
            dip::sint s = sb / is;
            if( s * is != sb ) {
               return false;
            }
            strides[ ii ] = s;
         }
         // Create image
         value = dip::Image( nullptr, buf.ptr(), datatype, sizes, strides );

         // TODO: Copy? Or how do we mark the source as dependency?

         return true;
      }
};
*/

}
}

void init_image( py::module& m );
void init_display( py::module& m );
void init_math( py::module& m );
void init_statistics( py::module& m );
void init_linear( py::module& m );
void init_morphology( py::module& m );
