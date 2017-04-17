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
void init_math( py::module& m );
void init_linear( py::module& m );
