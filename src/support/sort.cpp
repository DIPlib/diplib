/*
 * Sorting functions -- not implemented
 *
 * This files is meant to show how overloaded functions can be generated and specialized.
 * We use templates internally, but do not expose them to the user.
 */

#include "diplib.h"
#include "dip_sort.h"

using namespace dip;

template< typename T >
static void SortValues__CountingSort
(
   T* array,
   dip::uint n
)
{
   // Integer sort function: counting sort
}

template< typename T >
static void SortValues__QuickSort
(
   T* array,
   dip::uint n
)
{
   // Float sort function: quick sort
   // We'll use this also for larger integers
}

void SortValues( dip::uint8*  array, dip::uint n) {  SortValues__CountingSort( array, n ); }
void SortValues( dip::sint8*  array, dip::uint n) {  SortValues__CountingSort( array, n ); }
void SortValues( dip::uint16* array, dip::uint n) {  SortValues__CountingSort( array, n ); }
void SortValues( dip::sint16* array, dip::uint n) {  SortValues__CountingSort( array, n ); }
void SortValues( dip::uint32* array, dip::uint n) {  SortValues__QuickSort( array, n ); }
void SortValues( dip::sint32* array, dip::uint n) {  SortValues__QuickSort( array, n ); }
void SortValues( dip::sfloat* array, dip::uint n) {  SortValues__QuickSort( array, n ); }
void SortValues( dip::dfloat* array, dip::uint n) {  SortValues__QuickSort( array, n ); }

template< typename T >
static void SortIndices__CountingSort
(
   T* array,
   dip::uint* indices,
   dip::uint n
)
{
   // Integer sort function: counting sort
}

template< typename T >
static void SortIndices__QuickSort
(
   T* array,
   dip::uint* indices,
   dip::uint n
)
{
   // Float sort function: quick sort
   // We'll use this also for larger integers
}

void SortIndices( dip::uint8*  array, dip::uint* indices, dip::uint n) {  SortIndices__CountingSort( array, indices, n ); }
void SortIndices( dip::sint8*  array, dip::uint* indices, dip::uint n) {  SortIndices__CountingSort( array, indices, n ); }
void SortIndices( dip::uint16* array, dip::uint* indices, dip::uint n) {  SortIndices__CountingSort( array, indices, n ); }
void SortIndices( dip::sint16* array, dip::uint* indices, dip::uint n) {  SortIndices__CountingSort( array, indices, n ); }
void SortIndices( dip::uint32* array, dip::uint* indices, dip::uint n) {  SortIndices__QuickSort( array, indices, n ); }
void SortIndices( dip::sint32* array, dip::uint* indices, dip::uint n) {  SortIndices__QuickSort( array, indices, n ); }
void SortIndices( dip::sfloat* array, dip::uint* indices, dip::uint n) {  SortIndices__QuickSort( array, indices, n ); }
void SortIndices( dip::dfloat* array, dip::uint* indices, dip::uint n) {  SortIndices__QuickSort( array, indices, n ); }
