#ifndef DIP_SORT_H
#define DIP_SORT_H

#include "dip_support.h"

namespace dip {

void SortValues( dip::uint8*  array, dip::uint n);
void SortValues( dip::sint8*  array, dip::uint n);
void SortValues( dip::uint16* array, dip::uint n);
void SortValues( dip::sint16* array, dip::uint n);
void SortValues( dip::uint32* array, dip::uint n);
void SortValues( dip::sint32* array, dip::uint n);
void SortValues( dip::sfloat* array, dip::uint n);
void SortValues( dip::dfloat* array, dip::uint n);

void SortIndices( dip::uint8*  array, dip::uint* indices, dip::uint n);
void SortIndices( dip::sint8*  array, dip::uint* indices, dip::uint n);
void SortIndices( dip::uint16* array, dip::uint* indices, dip::uint n);
void SortIndices( dip::sint16* array, dip::uint* indices, dip::uint n);
void SortIndices( dip::uint32* array, dip::uint* indices, dip::uint n);
void SortIndices( dip::sint32* array, dip::uint* indices, dip::uint n);
void SortIndices( dip::sfloat* array, dip::uint* indices, dip::uint n);
void SortIndices( dip::dfloat* array, dip::uint* indices, dip::uint n);

} // namespace dip

#endif // DIP_SORT_H
