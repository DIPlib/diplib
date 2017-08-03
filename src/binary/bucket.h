/*
 * DIPlib 3.0
 * This file contains the area opening and related functions.
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#ifndef DIP_BUCKET_H
#define DIP_BUCKET_H

#include "diplib.h"

namespace dip {

/* Typedefs */
struct Node {
   uint8* pim;
   uint8 dirc;
};

struct Chunk {
   bool used;                       /* true if chunk currently used */
   struct Chunk* bnext;             /* pointer to next chunk of current bucket */
   struct Chunk* lnext;             /* pointer to next allocated chunk */
   std::vector< Node > nodes;       /* contains Bucket.chunksize elements */
};

struct Bucket {
   dip::uint nbuckets;              /* Number of buckets */
   dip::uint chunksize;             /* Size of chunk (#nodes) */
   //int sizeofchunk;               /* Size of chunk (in bytes, total structure) */
   dip::uint andmask;               /* Andmask for modulo operation */
   dip::uint rbuck;                 /* Number of bucket which is being read */
   dip::uint wbuck;                 /* Number of bucket in which nodes are being put */
   Chunk* pwritechunk;              /* Pointer to chunk being written in */
   Chunk* preadchunk;               /* Pointer to chunk being read */
   std::vector< Node* > plastnode;  /* Array pointers to last nodes+1 of each bucket */
   std::vector< Chunk > pchunk1;    /* Array pointers to first chunk of each bucket */
   Chunk* firstchunk;               /* First allocated chunk */
   Chunk* lastchunk;                /* Last allocated chunk */
   Chunk* freechunk;                /* Last freed chunk */
   dip::uint freecount;             /* Number of free chunks */
   //dip::uint allocated;           /* Number of chunks allocated extra */
   std::vector< Chunk > pextra;     /* Extra chunks allocated */

   Bucket( dip::uint nb, dip::uint cs ) {
      nbuckets = nb;
      chunksize = cs;
      //sizeofchunk = sizeof( Chunk )
      andmask = nbuckets - 1;

      /* Allocate the first chunk of each bucket and set up an array of
          pointers to these first chunks. */
      pchunk1.resize( nbuckets );

      /* Allocate an array with pointers for the last nodes written */
      plastnode.resize( nbuckets );
      for( dip::uint ii = 0; ii < nbuckets; ii++ ) {
         pchunk1[ ii ].nodes.resize( chunksize );
         plastnode[ ii ] = &( pchunk1[ ii ].nodes[ 0 ] );
         pchunk1[ ii ].bnext = nullptr;
      }

      /* Set up free allocation variables */
      firstchunk = nullptr;
      lastchunk = nullptr;
      freechunk = nullptr;
      freecount = 0;
   }

   void Free( dip::sint index ) {
      dip::uint ii = static_cast< dip::uint >( index );
      plastnode[ ii & andmask ] = &( pchunk1[ ii & andmask ].nodes[ 0 ] );
      Chunk* pc = pchunk1[ ii & andmask ].bnext;
      if( pc != nullptr ) {
         freechunk = pc;
      }
      while( pc != nullptr ) {
         pc->used = false;
         freecount++;
         pc = pc->bnext;
      }
   }

   bool Empty() const {
      for( dip::uint ii = 0; ii < nbuckets; ii++ ) {
         if( plastnode[ ii ] != &( pchunk1[ ii ].nodes[ 0 ] )) {
            return false;
         }
      }
      return true;
   }

   Chunk* GetChunk() {
      Chunk* newchunk;
      if( freecount > 0 ) {
         newchunk = freechunk;
         while( newchunk->used ) {
            newchunk = newchunk->lnext;
         }
         freecount--;
         freechunk = freechunk->lnext;
      } else {
         pextra.emplace_back(); // allocate a new chunk
         newchunk = &( pextra.back() );
         newchunk->nodes.resize( chunksize );
         if( firstchunk == nullptr ) {
            firstchunk = newchunk;
         } else {
            lastchunk->lnext = newchunk;
         }
         lastchunk = newchunk;
         newchunk->lnext = firstchunk;
      }
      newchunk->used = true;
      return newchunk;
   }

/* Macros, display and storing and recalling from buckets */
/* These all expect the following defines in the code that uses them:
   Node* pnr;  // where to read in bucket
   Node* pnw;  // where to write in bucket
   Node* pnre; // where to end reading
   Node* pnwe; // where to end writing
*/
#define STR( b, pointer, direction ) { \
   pnw->pim = ( pointer ); pnw++->dirc = uint8( direction );\
   if( pnw == pnwe ) nextwrite( b ); }

#define STRP( b, pointer ) { \
   pnw++->pim = ( pointer ); \
   if( pnw == pnwe ) nextwrite( b ); }

#define RCL( b, pointer, direction ) { \
   ( pointer ) = pnr->pim; ( direction ) = pnr++->dirc; \
   if( pnr == pnre ) { \
      if( pnr == ( b ).plastnode[ ( b ).rbuck ] ) go = false; \
      else nextread( b ); }}

#define RCLP( b, pointer ) { \
   ( pointer ) = pnr++->pim; \
   if( pnr == pnre ) { \
      if( pnr == ( b ).plastnode[ ( b ).rbuck ] ) go = false; \
      else nextread( b ); }}

#define startwrite( b, bucknr ) { \
   ( b ).wbuck = static_cast< dip::uint >( bucknr ) & ( b ).andmask; \
   ( b ).pwritechunk = &(( b ).pchunk1[ ( b ).wbuck ] ); \
   pnw = &(( b ).pwritechunk->nodes[ 0 ] ); \
   pnwe = pnw + ( b ).chunksize; }

#define nextwrite( b ) { \
   ( b ).pwritechunk->bnext = ( b ).GetChunk(); \
   ( b ).pwritechunk = ( b ).pwritechunk->bnext; \
   /*DIP_THROW_IF((( b ).pwritechunk == nullptr), E::NO_MEMORY;)*/ \
   pnw = &(( b ).pwritechunk->nodes[ 0 ] ); \
   pnwe = pnw + ( b ).chunksize; }

#define closewrite( b, pnode ) { \
   ( b ).pwritechunk->bnext = nullptr; \
   ( b ).plastnode[ ( b ).wbuck ] = (pnode); }

#define startread( b, bucknr ) { \
   ( b ).rbuck = static_cast< dip::uint >( bucknr ) & ( b ).andmask; \
   ( b ).preadchunk = &(( b ).pchunk1[ ( b ).rbuck ] ); \
   pnr = &(( b ).preadchunk->nodes[ 0 ] ); \
   go = true; \
   if(( b ).preadchunk->bnext == nullptr) { \
      pnre = ( b ).plastnode[ ( b ).rbuck ]; \
      if( pnr == pnre ) go = false; \
   } else pnre = pnr + ( b ).chunksize; }

#define nextread( b ) { \
   ( b ).preadchunk = ( b ).preadchunk->bnext; \
   pnr = &(( b ).preadchunk->nodes[ 0 ] ); \
   go = true; \
   if(( b ).preadchunk->bnext == nullptr ) { \
      pnre = ( b ).plastnode[ ( b ).rbuck ]; \
      if( pnr == pnre ) go = false; \
   } else pnre = pnr + b.chunksize; }

};

} // namespace dip

#endif // DIP_BUCKET_H
