#ifndef DIP_SORT_H
#define DIP_SORT_H

#include "dip_support.h"

namespace dip {

void SortValues( uint8*  array, uint n);
void SortValues( sint8*  array, uint n);
void SortValues( uint16* array, uint n);
void SortValues( sint16* array, uint n);
void SortValues( uint32* array, uint n);
void SortValues( sint32* array, uint n);
void SortValues( sfloat* array, uint n);
void SortValues( dfloat* array, uint n);

void SortIndices( uint8*  array, uint* indices, uint n);
void SortIndices( sint8*  array, uint* indices, uint n);
void SortIndices( uint16* array, uint* indices, uint n);
void SortIndices( sint16* array, uint* indices, uint n);
void SortIndices( uint32* array, uint* indices, uint n);
void SortIndices( sint32* array, uint* indices, uint n);
void SortIndices( sfloat* array, uint* indices, uint n);
void SortIndices( dfloat* array, uint* indices, uint n);

} // namespace dip

#endif // DIP_SORT_H
