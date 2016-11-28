#ifndef DIP_SORT_H
#define DIP_SORT_H

#include "diplib/library/types.h"

namespace dip {

void SortValues( uint8*  array, dip::uint n);
void SortValues( sint8*  array, dip::uint n);
void SortValues( uint16* array, dip::uint n);
void SortValues( sint16* array, dip::uint n);
void SortValues( uint32* array, dip::uint n);
void SortValues( sint32* array, dip::uint n);
void SortValues( sfloat* array, dip::uint n);
void SortValues( dfloat* array, dip::uint n);

void SortIndices( uint8*  array, dip::uint* indices, dip::uint n);
void SortIndices( sint8*  array, dip::uint* indices, dip::uint n);
void SortIndices( uint16* array, dip::uint* indices, dip::uint n);
void SortIndices( sint16* array, dip::uint* indices, dip::uint n);
void SortIndices( uint32* array, dip::uint* indices, dip::uint n);
void SortIndices( sint32* array, dip::uint* indices, dip::uint n);
void SortIndices( sfloat* array, dip::uint* indices, dip::uint n);
void SortIndices( dfloat* array, dip::uint* indices, dip::uint n);

} // namespace dip

#endif // DIP_SORT_H
