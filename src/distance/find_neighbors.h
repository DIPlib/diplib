/*
* (c)2017-2024, Cris Luengo.
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

#ifndef DIP_FIND_NEIGHBORS_H
#define DIP_FIND_NEIGHBORS_H

namespace dip {

namespace {

struct XYPosition {
   dip::sint x;
   dip::sint y;
};

struct XYZPosition {
   dip::sint x;
   dip::sint y;
   dip::sint z;
};

inline dip::sint FindNeighbors2D(
   XYPosition* p,
   sfloat* mindist,
   dip::sint* minpos,
   dip::sint n,
   dip::sint nx,
   dip::sint ny,
   sfloat* fdnb,
   sfloat const* fsdx,
   sfloat const* fsdy,
   bool useTrue,
   sfloat delta = 0.8f
) {
   sfloat* dnbp = fdnb;
   XYPosition* pnbp = p;
   for( dip::sint ii = n; --ii >= 0; pnbp++ ) {
      *dnbp++ = fsdx[ pnbp->x + nx ] + fsdy[ pnbp->y + ny ];
   }

   dnbp = fdnb;
   sfloat min = *dnbp++;
   dip::sint pos = 0;
   for( dip::sint ii = 1; ii < n; ii++, dnbp++ ) {
      if( *dnbp < min ) {
         min = *dnbp;
         pos = ii;
      }
   }
   *mindist = min;
   *minpos = pos;

   if( useTrue ) {
      min = std::sqrt( min ) + delta;
      min *= min;
   }

   dnbp = fdnb;
   dip::sint jj = 0;
   for( dip::sint ii = 0; ii < n; ii++ ) {
      if( useTrue ) {
         if( min >= *dnbp++ ) {
            if( ii != jj ) {
               p[ jj ] = p[ ii ];
            }
            jj++;
         }
      } else {
         if( min == *dnbp++ ) {
            if( ii != jj ) {
               p[ jj ] = p[ ii ];
            }
            jj++;
         }
      }
   }

   for( dip::sint kk = 0; kk < jj - 1; kk++ ) {
      for( dip::sint ii = kk + 1; ii < jj; ii++ ) {
         if( p[ ii ].x == p[ kk ].x && p[ ii ].y == p[ kk ].y ) {
            if( ii != --jj ) {
               p[ ii ] = p[ jj ];
            }
            ii--;
         }
      }
   }

   return jj;
}

dip::sint FindNeighbors3D(
      XYZPosition* p,
      sfloat* mindist,
      dip::sint* minpos,
      dip::sint n,
      dip::sint nx,
      dip::sint ny,
      dip::sint nz,
      sfloat* fdnb,
      sfloat const* fsdx,
      sfloat const* fsdy,
      sfloat const* fsdz,
      bool useTrue,
      sfloat delta = 1.4f
) {
   sfloat* dnbp = fdnb;
   XYZPosition* pnbp = p;
   for( dip::sint ii = n; --ii >= 0; pnbp++ ) {
      *dnbp++ = fsdx[ pnbp->x + nx ] + fsdy[ pnbp->y + ny ] + fsdz[ pnbp->z + nz ];
   }

   dnbp = fdnb;
   sfloat min = *dnbp++;
   dip::sint pos = 0;
   for( dip::sint ii = 1; ii < n; ii++, dnbp++ ) {
      if( *dnbp < min ) {
         min = *dnbp;
         pos = ii;
      }
   }
   *mindist = min;
   *minpos = pos;

   if( useTrue ) {
      min = std::sqrt( min ) + delta;
      min *= min;
   }

   dnbp = fdnb;
   dip::sint jj = 0;
   for( dip::sint ii = 0; ii < n; ii++ ) {
      if( useTrue ) {
         if( min >= *dnbp++ ) {
            if( ii != jj ) {
               p[ jj ] = p[ ii ];
            }
            jj++;
         }
      } else {
         if( min == *dnbp++ ) {
            if( ii != jj ) {
               p[ jj ] = p[ ii ];
            }
            jj++;
         }
      }
   }

   for( dip::sint kk = 0; kk < jj - 1; kk++ ) {
      for( dip::sint ii = kk + 1; ii < jj; ii++ ) {
         if( p[ ii ].x == p[ kk ].x && p[ ii ].y == p[ kk ].y && p[ ii ].z == p[ kk ].z ) {
            if( ii != --jj ) {
               p[ ii ] = p[ jj ];
            }
            ii--;
         }
      }
   }

   return jj;
}

} // namespace

} // namespace dip

#endif // DIP_FIND_NEIGHBORS_H
