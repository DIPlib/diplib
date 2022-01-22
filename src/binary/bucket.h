/*
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
struct DIP_NO_EXPORT Node {
   uint8* pim;
   uint8 dirc;
};

struct DIP_NO_EXPORT Chunk {
   bool used;                       /* true if chunk currently used */
   struct Chunk* bnext;             /* pointer to next chunk of current bucket */
   struct Chunk* lnext;             /* pointer to next allocated chunk */
   std::vector< Node > nodes;       /* contains Bucket.chunksize elements */
};

struct DIP_NO_EXPORT Bucket {
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
   std::vector< std::unique_ptr< Chunk >> pextra; /* Extra chunks allocated */

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
         pextra.emplace_back( new Chunk ); // allocate a new chunk
         newchunk = pextra.back().get();
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

   //
   // Macros, display and storing and recalling from buckets
   //

   Node* pnr;  // where to read in bucket
   Node* pnw;  // where to write in bucket
   Node* pnre; // where to end reading
   Node* pnwe; // where to end writing
   bool go;    // TRUE if still nodes to be read

   void STR( uint8* pointer, uint8 direction ) {
      pnw->pim = pointer;
      pnw++->dirc = direction;
      if( pnw == pnwe ) {
         nextwrite();
      }
   }

   void STRP( uint8* pointer ) {
      pnw++->pim = pointer;
      if( pnw == pnwe ) {
         nextwrite();
      }
   }

   void RCL( uint8*& pointer, uint8& direction ) {
      pointer = pnr->pim;
      direction = pnr++->dirc;
      if( pnr == pnre ) {
         if( pnr == plastnode[ rbuck ] ) {
            go = false;
         } else {
            nextread();
         }
      }
   }

   void RCLP( uint8*& pointer ) {
      pointer = pnr++->pim;
      if( pnr == pnre ) {
         if( pnr == plastnode[ rbuck ] ) {
            go = false;
         } else {
            nextread();
         }
      }
   }

   void startwrite( dip::sint bucknr ) {
      wbuck = static_cast< dip::uint >( bucknr ) & andmask;
      pwritechunk = &( pchunk1[ wbuck ] );
      pnw = &( pwritechunk->nodes[ 0 ] );
      pnwe = pnw + chunksize;
   }

   void nextwrite() {
      pwritechunk->bnext = GetChunk();
      pwritechunk = pwritechunk->bnext;
      /*DIP_THROW_IF((pwritechunk == nullptr), E::NO_MEMORY;)*/
      pnw = &( pwritechunk->nodes[ 0 ] );
      pnwe = pnw + chunksize;
   }

   void closewrite() {
      pwritechunk->bnext = nullptr;
      plastnode[ wbuck ] = pnw;
   }

   void startread( dip::sint bucknr ) {
      rbuck = static_cast< dip::uint >( bucknr ) & andmask;
      preadchunk = &( pchunk1[ rbuck ] );
      pnr = &( preadchunk->nodes[ 0 ] );
      go = true;
      if( preadchunk->bnext == nullptr ) {
         pnre = plastnode[ rbuck ];
         if( pnr == pnre ) {
            go = false;
         }
      } else {
         pnre = pnr + chunksize;
      }
   }

   void nextread() {
      preadchunk = preadchunk->bnext;
      pnr = &( preadchunk->nodes[ 0 ] );
      go = true;
      if( preadchunk->bnext == nullptr ) {
         pnre = plastnode[ rbuck ];
         if( pnr == pnre ) {
            go = false;
         }
      } else {
         pnre = pnr + chunksize;
      }
   }

};

} // namespace dip

#endif // DIP_BUCKET_H
