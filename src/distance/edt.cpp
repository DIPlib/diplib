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

#include "diplib/distance.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "diplib.h"
#include "diplib/math.h"

#include "find_neighbors.h"
#include "separable_dt.h"

namespace dip {

namespace {

void EDTFast2D(
      sfloat* oi,
      UnsignedArray const& sizes,
      IntegerArray const& stride,
      FloatArray const& distance,
      bool border
) {
   dip::sint nx = static_cast< dip::sint >( sizes[ 0 ] );
   dip::sint ny = static_cast< dip::sint >( sizes[ 1 ] );
   dip::sint sx = stride[ 0 ];
   dip::sint sy = stride[ 1 ];
   sfloat dx = static_cast< sfloat >( distance[ 0 ] );
   sfloat dy = static_cast< sfloat >( distance[ 1 ] );
   dip::sint nx1sx = ( nx - 1 ) * sx;
   dip::sint ny1sy = ( ny - 1 ) * sy;

   // Allocate and initialize the necessary arrays
   std::vector< sfloat > firstBuffer( static_cast< dip::uint >( 2 * nx + 1 ));
   sfloat* fsdx = firstBuffer.data();
   sfloat dxx = dx * dx;
   for( dip::uint ii = 0; ii < firstBuffer.size(); ii++ ) {
      sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nx );
      fsdx[ ii ] = d * d * dxx;
   }
   sfloat* fsdy = nullptr;
   std::vector< sfloat> secondBuffer;
   if(( dx == dy ) && ( nx == ny )) {
      fsdy = fsdx;
   } else {
      secondBuffer.resize( static_cast< dip::uint >( 2 * ny + 1 ));
      fsdy = secondBuffer.data();
      sfloat dyy = dy * dy;
      for( dip::uint ii = 0; ii < secondBuffer.size(); ii++ ) {
         sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( ny );
         fsdy[ ii ] = d * d * dyy;
      }
   }
   std::vector< XYPosition > d( static_cast< dip::uint >(( nx + 2 ) * 2 ));
   sfloat maxDistance = fsdx[ 0 ] + fsdy[ 0 ];

   // Initialize
   XYPosition* d1 = d.data();
   XYPosition* d2 = d.data() + ( nx + 2 );
   XYPosition infd = { 0, 0 };
   XYPosition zero = { nx, ny };
   XYPosition x0y_1 = { nx, ny - 1 };
   XYPosition x0y1 = { nx, ny + 1 };
   XYPosition x_1y0 = { nx - 1, ny };
   XYPosition dx1 = { 1, 0 };
   XYPosition dy1 = { 0, 1 };
   XYPosition bp = border ? infd : zero;

   // Forward scan
   XYPosition* dbl = &d1[ 1 ];
   for( dip::sint ii = 0; ii < nx; ii++ ) {
      dbl[ ii ] = bp;
   }

   for( dip::sint yy = 0, py = 0; yy < ny; yy++, py += sy ) {
      // We split d into buffers that are swapped to read or write from
      XYPosition* dcl = ( yy & 1 ? d1 : d2 );
      XYPosition* dbl = ( yy & 1 ? d2 + 1 : d1 + 1 );

      *dcl++ = bp;
      for( dip::sint xx = 0, px = py; xx < nx; xx++, dcl++, dbl++, px += sx ) {
         if( oi[ px ] != 0.0 ) {
            if(( dbl->x == zero.x ) && ( dbl->y == zero.y )) {
               *dcl = x0y_1;
            } else {
               if(( dbl->x == infd.x ) && ( dbl->y == infd.y )) {
                  *dcl = infd;
               } else {
                  dcl->x = dbl->x - dy1.x;
                  dcl->y = dbl->y - dy1.y;
               }
            }

            if(( dcl - 1 )->x == zero.x && ( dcl - 1 )->y == zero.y ) {
               *dcl = x_1y0;
            } else {
               if((( dcl - 1 )->x != infd.x ) || (( dcl - 1 )->y != infd.y )) {
                  XYPosition* c = dcl;
                  XYPosition* b = ( dcl - 1 );
                  if(( fsdx[ c->x ] + fsdy[ c->y ] ) <
                     ( fsdx[ b->x - 1 ] + fsdy[ b->y ] )) {
                     continue;
                  }
                  dcl->x = ( dcl - 1 )->x - dx1.x;
                  dcl->y = ( dcl - 1 )->y - dx1.y;
               }
            }
         } else {
            *dcl = zero;
         }
      }

      *dcl-- = bp;
      for( dip::sint xx = 0, px = py + nx1sx; xx < nx; xx++, dcl--, px -= sx ) {
         if(( dcl->x != zero.x ) || ( dcl->y != zero.y )) {
            if((( dcl + 1 )->x == infd.x ) && (( dcl + 1 )->y == infd.y )) {
               if(( dcl->x == infd.x ) && ( dcl->y == infd.y )) {
                  oi[ px ] = maxDistance;
               } else {
                  XYPosition* c = dcl;
                  oi[ px ] = fsdx[ c->x ] + fsdy[ c->y ];
               }
            } else {
               XYPosition* c = dcl;
               XYPosition* b = ( dcl + 1 );
               sfloat fdc = fsdx[ c->x ] + fsdy[ c->y ];
               sfloat fdb = fsdx[ b->x + 1 ] + fsdy[ b->y ];
               if( fdc > fdb ) {
                  dcl->x = ( dcl + 1 )->x + dx1.x;
                  dcl->y = ( dcl + 1 )->y + dx1.y;
                  oi[ px ] = fdb;
               } else {
                  oi[ px ] = fdc;
               }
            }
         }
      }
   }

   // Backward scan
   dbl = d1 + 1;
   for( dip::sint ii = 0; ii < nx; ii++ ) {
      dbl[ ii ] = bp;
   }

   for( dip::sint ii = 0, py = ny1sy; ii < ny; ii++, py -= sy ) {
      XYPosition* dcl = ( ii & 1 ? d1 + nx + 1 : d2 + nx + 1 );
      XYPosition* dbl = ( ii & 1 ? d2 + nx : d1 + nx );
      *dcl-- = bp;

      for( dip::sint xx = 0, px = py + nx1sx; xx < nx; xx++, dcl--, dbl--, px -= sx ) {
         if( oi[ px ] != 0.0 ) {
            if( dbl->x == zero.x && dbl->y == zero.y ) {
               *dcl = x0y1;
            } else {
               if( dbl->x == infd.x && dbl->y == infd.y ) {
                  *dcl = infd;
               } else {
                  dcl->x = dbl->x + dy1.x;
                  dcl->y = dbl->y + dy1.y;
               }
            }

            if(( dcl + 1 )->x != infd.x || ( dcl + 1 )->y != infd.y ) {
               XYPosition* c = dcl;
               XYPosition* b = ( dcl + 1 );
               if(( fsdx[ c->x ] + fsdy[ c->y ] ) < ( fsdx[ b->x + 1 ] + fsdy[ b->y ] )) {
                  continue;
               }
               dcl->x = ( dcl + 1 )->x + dx1.x;
               dcl->y = ( dcl + 1 )->y + dx1.y;
            }
         } else {
            *dcl = zero;
         }
      }

      *dcl++ = bp;
      for( dip::sint xx = 0, px = py; xx < nx; xx++, dcl++, px += sx ) {
         if( dcl->x != zero.x || dcl->y != zero.y ) {
            if(( dcl - 1 )->x == infd.x && ( dcl - 1 )->y == infd.y ) {
               if( dcl->x != infd.x || dcl->y != infd.y ) {
                  XYPosition* c = dcl;
                  sfloat fdc = fsdx[ c->x ] + fsdy[ c->y ];
                  if( oi[ px ] > fdc ) {
                     oi[ px ] = fdc;
                  }
               }
            } else {
               XYPosition* c = dcl;
               XYPosition* b = ( dcl - 1 );
               sfloat fdc = fsdx[ c->x ] + fsdy[ c->y ];
               sfloat fdb = fsdx[ b->x - 1 ] + fsdy[ b->y ];
               if( fdc > fdb ) {
                  dcl->x = ( dcl - 1 )->x - dx1.x;
                  dcl->y = ( dcl - 1 )->y - dx1.y;
                  if( oi[ px ] > fdb ) {
                     oi[ px ] = fdb;
                  }
               } else {
                  if( oi[ px ] > fdc ) {
                     oi[ px ] = fdc;
                  }
               }
            }
         }
      }
   }
}

void EDTFast3D(
      sfloat* oi,
      UnsignedArray const& sizes,
      IntegerArray const& stride,
      FloatArray const& distance,
      bool border
) {
   dip::sint nx = static_cast< dip::sint >( sizes[ 0 ] );
   dip::sint ny = static_cast< dip::sint >( sizes[ 1 ] );
   dip::sint nz = static_cast< dip::sint >( sizes[ 2 ] );
   dip::sint sx = stride[ 0 ];
   dip::sint sy = stride[ 1 ];
   dip::sint sz = stride[ 2 ];
   sfloat dx = static_cast< sfloat >( distance[ 0 ] );
   sfloat dy = static_cast< sfloat >( distance[ 1 ] );
   sfloat dz = static_cast< sfloat >( distance[ 2 ] );
   dip::sint nx1sx = ( nx - 1 ) * sx;
   dip::sint ny1sy = ( ny - 1 ) * sy;
   dip::sint nz1sz = ( nz - 1 ) * sz;

   // Allocate and initialize the necessary arrays
   std::vector< sfloat > firstBuffer( static_cast< dip::uint >( 2 * nx + 1 ));
   sfloat* fsdx = firstBuffer.data();
   sfloat dxx = dx * dx;
   for( dip::uint ii = 0; ii < firstBuffer.size(); ii++ ) {
      sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nx );
      fsdx[ ii ] = d * d * dxx;
   }
   sfloat* fsdy = nullptr;
   std::vector< sfloat> secondBuffer;
   if(( dx == dy ) && ( nx == ny )) {
      fsdy = fsdx;
   } else {
      secondBuffer.resize( static_cast< dip::uint >( 2 * ny + 1 ));
      fsdy = secondBuffer.data();
      sfloat dyy = dy * dy;
      for( dip::uint ii = 0; ii < secondBuffer.size(); ii++ ) {
         sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( ny );
         fsdy[ ii ] = d * d * dyy;
      }
   }
   sfloat* fsdz = nullptr;
   std::vector< sfloat> thirdBuffer;
   if(( dx == dz ) && ( nx == nz )) {
      fsdz = fsdx;
   } else {
      if(( dy == dz ) && ( ny == nz )) {
         fsdz = fsdy;
      } else {
         thirdBuffer.resize( static_cast< dip::uint >( 2 * nz + 1 ));
         fsdz = thirdBuffer.data();
         sfloat dzz = dz * dz;
         for( dip::uint ii = 0; ii < thirdBuffer.size(); ii++ ) {
            sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nz );
            fsdz[ ii ] = d * d * dzz;
         }
      }
   }
   std::vector< XYZPosition > d( static_cast< dip::uint >(( nx + 2 ) * ( ny + 2 ) * 2));
   sfloat maxdist = fsdx[ 0 ] + fsdy[ 0 ] + fsdz[ 0 ];

   // Initialize
   XYZPosition* d1 = d.data();
   XYZPosition* d2 = d1 + ( nx + 2 ) * ( ny + 2 );
   XYZPosition infd = { 0, 0, 0 };
   XYZPosition zero = { nx, ny, nz };
   XYZPosition bp = border ? infd : zero;

   XYZPosition* dbl = d1;
   for( dip::sint ii = ( nx + 2 ) * ( ny + 2 ); --ii >= 0; ) {
      *dbl++ = bp;
   }

   for( dip::sint zz = 0, pz = 0; zz < nz; zz++, pz += sz ) {

      XYZPosition* dcl = ( zz & 1 ? d1 : d2 );
      XYZPosition* dbl = ( zz & 1 ? d2 + nx + 3 : d1 + nx + 3 );

      for( dip::sint ii = nx + 2; --ii >= 0; ) {
         *dcl++ = bp;
      }

      for( dip::sint yy = 0, py = 0; yy < ny; yy++, dcl += 2 + nx, dbl += 2, py += sy ) {
         *dcl++ = bp;

         for( dip::sint xx = 0, px = py + pz; xx < nx; xx++, dcl++, dbl++, px += sx ) {
            if( oi[ px ] != 0.0 ) {
               if( dbl->x == zero.x && dbl->y == zero.y && dbl->z == zero.z ) {
                  dcl->x = nx;
                  dcl->y = ny;
                  dcl->z = nz - 1;
               } else {
                  if( dbl->x == infd.x && dbl->y == infd.y && dbl->z == infd.z ) {
                     *dcl = infd;
                  } else {
                     dcl->x = dbl->x;
                     dcl->y = dbl->y;
                     dcl->z = dbl->z - 1;
                  }
               }

               XYZPosition* dbt = dcl - ( nx + 2 );

               if( dbt->x != infd.x || dbt->y != infd.y || dbt->z != infd.z ) {
                  if(( fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ] ) >
                     ( fsdx[ dbt->x ] + fsdy[ dbt->y - 1 ] + fsdz[ dbt->z ] )) {
                     dcl->x = dbt->x;
                     dcl->y = dbt->y - 1;
                     dcl->z = dbt->z;
                  }
               }

               dbt = dcl - 1;

               if( dbt->x != infd.x || dbt->y != infd.y || dbt->z != infd.z ) {
                  if(( fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ] ) >
                     ( fsdx[ dbt->x - 1 ] + fsdy[ dbt->y ] + fsdz[ dbt->z ] )) {
                     dcl->x = dbt->x - 1;
                     dcl->y = dbt->y;
                     dcl->z = dbt->z;
                  }
               }
            } else {
               *dcl = zero;
            }
         }

         *dcl-- = bp;
         for( dip::sint xx = nx; --xx >= 0; dcl-- ) {
            if( dcl->x != zero.x || dcl->y != zero.y || dcl->z != zero.z ) {
               XYZPosition* dbt = dcl + 1;

               if( dbt->x != infd.x || dbt->y != infd.y || dbt->z != infd.z ) {
                  if(( fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ] ) >
                     ( fsdx[ dbt->x + 1 ] + fsdy[ dbt->y ] + fsdz[ dbt->z ] )) {
                     dcl->x = dbt->x + 1;
                     dcl->y = dbt->y;
                     dcl->z = dbt->z;
                  }
               }
            }
         }
      }

      for( dip::sint ii = nx + 2; --ii >= 0; ) {
         *dcl++ = bp;
      }

      dcl -= nx + 4;
      for( dip::sint yy = 0, py = ny1sy; yy < ny; yy++, dcl -= 2, py -= sy ) {
         for( dip::sint xx = nx, px = pz + py + nx1sx; --xx >= 0; dcl--, px -= sx ) {
            if( dcl->x != zero.x || dcl->y != zero.y || dcl->z != zero.z ) {
               XYZPosition* dbt = dcl + ( nx + 2 );

               if( dbt->x == infd.x && dbt->y == infd.y && dbt->z == infd.z ) {
                  if( dcl->x == infd.x && dcl->y == infd.y && dcl->z == infd.z ) {
                     oi[ px ] = maxdist;
                  } else {
                     oi[ px ] = fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ];
                  }
               } else {
                  sfloat fdc = fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ];
                  sfloat fdb = fsdx[ dbt->x ] + fsdy[ dbt->y + 1 ] + fsdz[ dbt->z ];
                  if( fdc > fdb ) {
                     dcl->x = dbt->x;
                     dcl->y = dbt->y + 1;
                     dcl->z = dbt->z;
                     oi[ px ] = fdb;
                  } else {
                     oi[ px ] = fdc;
                  }
               }
            } else {
               oi[ px ] = 0.0;
            }
         }
      }
   }

   dbl = d1;
   for( dip::sint ii = ( nx + 2 ) * ( ny + 2 ); --ii >= 0; ) {
      *dbl++ = bp;
   }

   for( dip::sint zz = 0, pz = nz1sz; zz < nz; zz++, pz -= sz ) {
      XYZPosition* dcl = ( zz & 1 ? ( d1 + ( nx + 2 ) * ( ny + 2 ) - 1 ) : ( d2 + ( nx + 2 ) * ( ny + 2 ) - 1 ));
      XYZPosition* dbl = ( zz & 1 ? ( d2 + ( nx + 2 ) * ( ny + 1 ) - 2 ) : ( d1 + ( nx + 2 ) * ( ny + 1 ) - 2 ));

      for( dip::sint ii = nx + 2; --ii >= 0; ) {
         *dcl-- = bp;
      }

      for( dip::sint yy = 0, py = ny1sy; yy < ny; yy++, dcl -= 2 + nx, dbl -= 2, py -= sy ) {
         *dcl-- = bp;

         for( dip::sint xx = 0, px = py + pz + nx1sx; xx < nx; xx++, dcl--, dbl--, px -= sx ) {
            if( oi[ px ] != 0.0 ) {
               if( dbl->x == zero.x && dbl->y == zero.y && dbl->z == zero.z ) {
                  dcl->x = nx;
                  dcl->y = ny;
                  dcl->z = nz + 1;
               } else {
                  if( dbl->x == infd.x && dbl->y == infd.y && dbl->z == infd.z ) {
                     dcl->x = 0;
                     dcl->y = 0;
                     dcl->z = 0;
                  } else {
                     dcl->x = dbl->x;
                     dcl->y = dbl->y;
                     dcl->z = dbl->z + 1;
                  }
               }

               XYZPosition* dbt = dcl + ( nx + 2 );

               if( dbt->x != infd.x || dbt->y != infd.y || dbt->z != infd.z ) {
                  if(( fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ] ) >
                     ( fsdx[ dbt->x ] + fsdy[ dbt->y + 1 ] + fsdz[ dbt->z ] )) {
                     dcl->x = dbt->x;
                     dcl->y = dbt->y + 1;
                     dcl->z = dbt->z;
                  }
               }

               dbt = dcl + 1;

               if( dbt->x != infd.x || dbt->y != infd.y || dbt->z != infd.z ) {
                  if(( fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ] ) >
                     ( fsdx[ dbt->x + 1 ] + fsdy[ dbt->y ] + fsdz[ dbt->z ] )) {
                     dcl->x = dbt->x + 1;
                     dcl->y = dbt->y;
                     dcl->z = dbt->z;
                  }
               }
            } else {
               *dcl = zero;
            }
         }

         *dcl++ = bp;
         for( dip::sint xx = nx; --xx >= 0; dcl++ ) {
            if( dcl->x != zero.x || dcl->y != zero.y || dcl->z != zero.z ) {
               XYZPosition* dbt = dcl - 1;

               if( dbt->x != infd.x || dbt->y != infd.y || dbt->z != infd.z ) {
                  if(( fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ] ) >
                     ( fsdx[ dbt->x - 1 ] + fsdy[ dbt->y ] + fsdz[ dbt->z ] )) {
                     dcl->x = dbt->x - 1;
                     dcl->y = dbt->y;
                     dcl->z = dbt->z;
                  }
               }
            }
         }
      }

      for( dip::sint ii = nx + 2; --ii >= 0; ) {
         *dcl-- = bp;
      }

      dcl += nx + 4;
      for( dip::sint yy = 0, py = 0; yy < ny; yy++, dcl += 2, py += sy ) {
         for( dip::sint xx = nx, px = pz + py; --xx >= 0; dcl++, px += sx ) {
            if( dcl->x != zero.x || dcl->y != zero.y || dcl->z != zero.z ) {
               XYZPosition* dbt = dcl - ( nx + 2 );

               if( dbt->x == infd.x && dbt->y == infd.y && dbt->z == infd.z ) {
                  if( dcl->x != infd.x || dcl->y != infd.y || dcl->z != infd.z ) {
                     sfloat fdc = fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ];
                     if( oi[ px ] > fdc ) {
                        oi[ px ] = fdc;
                     }
                  }
               } else {
                  sfloat fdc = fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ];
                  sfloat fdb = fsdx[ dbt->x ] + fsdy[ dbt->y - 1 ] + fsdz[ dbt->z ];
                  if( fdc > fdb ) {
                     dcl->x = dbt->x;
                     dcl->y = dbt->y - 1;
                     dcl->z = dbt->z;
                     if( oi[ px ] > fdb ) {
                        oi[ px ] = fdb;
                     }
                  } else {
                     if( oi[ px ] > fdc ) {
                        oi[ px ] = fdc;
                     }
                  }
               }
            }
         }
      }
   }
}

void EDTTies2D(
      sfloat* oi,
      UnsignedArray const& sizes,
      IntegerArray const& stride,
      FloatArray const& distance,
      bool border,
      bool useTrue
) {
   dip::sint dim = 2;
   dip::sint nx = static_cast< dip::sint >( sizes[ 0 ] );
   dip::sint ny = static_cast< dip::sint >( sizes[ 1 ] );
   dip::sint sx = stride[ 0 ];
   dip::sint sy = stride[ 1 ];
   sfloat dx = static_cast< sfloat >( distance[ 0 ] );
   sfloat dy = static_cast< sfloat >( distance[ 1 ] );
   sfloat delta = 0.8f * std::min( dx, dy ); // This could still go wrong if there's a large difference between dx and dy.
   dip::sint nx1sx = ( nx - 1 ) * sx;
   dip::sint ny1sy = ( ny - 1 ) * sy;
   dip::sint guess = ( dim * 2 - 1 );
   if( useTrue ) {
      guess *= 50;
   }

   // Allocate and initialize the necessary arrays
   std::vector< sfloat > firstBuffer( static_cast< dip::uint >(  2 * nx + 1 ));
   sfloat* fsdx = firstBuffer.data();
   sfloat dxx = dx * dx;
   for( dip::uint ii = 0; ii < firstBuffer.size(); ii++ ) {
      sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nx );
      fsdx[ ii ] = d * d * dxx;
   }
   sfloat* fsdy = nullptr;
   std::vector< sfloat> secondBuffer;
   if(( dx == dy ) && ( nx == ny )) {
      fsdy = fsdx;
   } else {
      secondBuffer.resize( static_cast< dip::uint >( 2 * ny + 1 ));
      fsdy = secondBuffer.data();
      sfloat dyy = dy * dy;
      for( dip::uint ii = 0; ii < secondBuffer.size(); ii++ ) {
         sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( ny );
         fsdy[ ii ] = d * d * dyy;
      }
   }
   std::vector< sfloat > fdnbBuffer( static_cast< dip::uint >( 10 * ( nx + ny )));
   sfloat* fdnb = fdnbBuffer.data();
   std::vector< dip::sint > nb( static_cast< dip::uint >( nx * 2 * ( dim * guess + 2 )));
   std::vector< XYPosition > pnbBuffer( static_cast< dip::uint >( 10 * ( nx + ny )));
   XYPosition* pnb = pnbBuffer.data();
   std::vector< dip::sint* > d( static_cast< dip::uint >(( nx + 2 ) * 2 ));
   sfloat maxdist = fsdx[ 0 ] + fsdy[ 0 ];

   // Initialize
   dip::sint* nb0 = nb.data();
   dip::sint* nb1 = nb0 + nx * ( guess * dim + 2 );
   dip::sint** d1 = d.data();
   dip::sint** d2 = d1 + ( nx + 2 );
   dip::sint zero = 0;
   dip::sint* bp = ( border ? &zero : nullptr );

   dip::sint** dbl = d1 + 1;
   for( dip::sint ii = nx; --ii >= 0; ) {
      *dbl++ = bp;
   }

   for( dip::sint yy = 0, py = 0; yy < ny; yy++, py += sy ) {

      dip::sint* nbp = ( yy & 1 ? nb1 : nb0 );
      dip::sint** dcl = ( yy & 1 ? d1 : d2 );
      dip::sint** dbl = ( yy & 1 ? d2 + 1 : d1 + 1 );

      *dcl++ = bp;
      for( dip::sint xx = 0, px = py; xx < nx; xx++, dcl++, dbl++, px += sx ) {
         if( oi[ px ] != 0.0 ) {
            *dcl = nbp;
            dip::sint kk = 0;
            XYPosition* pnbp = pnb;
            dip::sint* tnbp = *dbl;
            if( tnbp == nullptr ) {
               pnbp->x = 0;
               pnbp->y = -1;
               kk++;
               pnbp++;
            } else {
               XYPosition* nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x;
                  pnbp->y = nbs->y - 1;
                  kk++;
               }
            }
            tnbp = *( dcl - 1 );
            if( tnbp == nullptr ) {
               pnbp->x = -1;
               pnbp->y = 0;
               kk++;
            } else {
               XYPosition* nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x - 1;
                  pnbp->y = nbs->y;
                  kk++;
               }
            }
            if( kk == 0 ) {
               *nbp++ = 0;
            } else {
               sfloat mindist{};
               dip::sint minpos{};
               kk = FindNeighbors2D( pnb, &mindist, &minpos, kk, nx, ny, fdnb, fsdx, fsdy, useTrue, delta );
               *nbp++ = kk;
               XYPosition* nbs = reinterpret_cast< XYPosition* >( nbp );
               pnbp = pnb;
               for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                  *nbs = *pnbp;
               }
               nbp += kk * dim;
            }
         } else {
            *dcl = nullptr;
         }
      }

      *dcl-- = bp;
      for( dip::sint ii = 0, px = py + nx1sx; ii < nx; ii++, dcl--, px -= sx ) {
         if( *dcl != nullptr ) {
            dip::sint kk = **dcl;
            XYPosition* nbs = reinterpret_cast< XYPosition* >( *dcl + 1 );
            XYPosition* pnbp = pnb;
            for( dip::sint jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
               *pnbp = *nbs;
            }
            dip::sint* tnbp = *( dcl + 1 );
            if( tnbp == nullptr ) {
               pnbp->x = 1;
               pnbp->y = 0;
               kk++;
            } else {
               XYPosition* nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x + 1;
                  pnbp->y = nbs->y;
                  kk++;
               }
            }
            *dcl = nbp;
            if( kk == 0 ) {
               *nbp++ = 0;
               oi[ px ] = maxdist;
            } else {
               sfloat mindist{};
               dip::sint minpos{};
               kk = FindNeighbors2D( pnb, &mindist, &minpos, kk, nx, ny, fdnb, fsdx, fsdy, useTrue, delta );
               *nbp++ = kk;
               XYPosition* nbs = reinterpret_cast< XYPosition* >( nbp );
               pnbp = pnb;
               for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                  *nbs = *pnbp;
               }
               oi[ px ] = mindist;
               nbp += kk * dim;
            }
         } else {
            oi[ px ] = 0.0;
         }
      }
   }

   dbl = d1 + 1;
   for( dip::sint ii = nx; --ii >= 0; ) {
      *dbl++ = bp;
   }

   for( dip::sint yy = 0, py = ny1sy; yy < ny; yy++, py -= sy ) {
      dip::sint* nbp = ( yy & 1 ? nb1 : nb0 );
      dip::sint** dcl = ( yy & 1 ? d1 + nx + 1 : d2 + nx + 1 );
      dip::sint** dbl = ( yy & 1 ? d2 + nx : d1 + nx );

      *dcl-- = bp;
      for( dip::sint xx = 0, px = py + nx1sx; xx < nx; xx++, dcl--, dbl--, px -= sx ) {
         if( oi[ px ] != 0.0 ) {
            *dcl = nbp;
            dip::sint kk = 0;
            XYPosition* pnbp = pnb;
            dip::sint* tnbp = *dbl;
            if( tnbp == nullptr ) {
               pnbp->x = 0;
               pnbp->y = 1;
               kk++;
               pnbp++;
            } else {
               XYPosition* nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x;
                  pnbp->y = nbs->y + 1;
                  kk++;
               }
            }
            tnbp = *( dcl + 1 );
            if( tnbp == nullptr ) {
               pnbp->x = 1;
               pnbp->y = 0;
               kk++;
            } else {
               XYPosition* nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x + 1;
                  pnbp->y = nbs->y;
                  kk++;
               }
            }
            if( kk == 0 ) {
               *nbp++ = 0;
            } else {
               sfloat mindist{};
               dip::sint minpos{};
               kk = FindNeighbors2D( pnb, &mindist, &minpos, kk, nx, ny, fdnb, fsdx, fsdy, useTrue, delta );
               *nbp++ = kk;
               XYPosition* nbs = reinterpret_cast< XYPosition* >( nbp );
               pnbp = pnb;
               for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                  *nbs = *pnbp;
               }
               nbp += kk * dim;
            }
         } else {
            *dcl = nullptr;
         }
      }

      *dcl++ = bp;
      for( dip::sint xx = 0, px = py; xx < nx; xx++, dcl++, px += sx ) {
         if( *dcl != nullptr ) {
            dip::sint kk = **dcl;
            XYPosition* nbs = reinterpret_cast< XYPosition* >( *dcl + 1 );
            XYPosition* pnbp = pnb;
            for( dip::sint jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
               *pnbp = *nbs;
            }
            dip::sint* tnbp = *( dcl - 1 );
            if( tnbp == nullptr ) {
               pnbp->x = -1;
               pnbp->y = 0;
               kk++;
            } else {
               XYPosition* nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x - 1;
                  pnbp->y = nbs->y;
                  kk++;
               }
            }
            *dcl = nbp;
            if( kk == 0 ) {
               *nbp++ = 0;
               oi[ px ] = std::sqrt( oi[ px ] );
            } else {
               sfloat mindist{};
               dip::sint minpos{};
               kk = FindNeighbors2D( pnb, &mindist, &minpos, kk, nx, ny, fdnb, fsdx, fsdy, useTrue, delta );
               *nbp++ = kk;
               XYPosition* nbs = reinterpret_cast< XYPosition* >( nbp );
               pnbp = pnb;
               for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                  *nbs = *pnbp;
               }
               if( mindist < oi[ px ] ) {
                  oi[ px ] = std::sqrt( mindist );
               } else {
                  oi[ px ] = std::sqrt( oi[ px ] );
               }
               nbp += kk * dim;
            }
         }
      }
   }
}

void EDTTies3D(
      sfloat* oi,
      UnsignedArray const& sizes,
      IntegerArray const& stride,
      FloatArray const& distance,
      bool border,
      bool useTrue
) {
   dip::sint dim = 3;
   dip::sint nx = static_cast< dip::sint >( sizes[ 0 ] );
   dip::sint ny = static_cast< dip::sint >( sizes[ 1 ] );
   dip::sint nz = static_cast< dip::sint >( sizes[ 2 ] );
   dip::sint sx = stride[ 0 ];
   dip::sint sy = stride[ 1 ];
   dip::sint sz = stride[ 2 ];
   sfloat dx = static_cast< sfloat >( distance[ 0 ] );
   sfloat dy = static_cast< sfloat >( distance[ 1 ] );
   sfloat dz = static_cast< sfloat >( distance[ 2 ] );
   sfloat delta = 1.4f * std::min( dx, std::min( dy, dz )); // This could still go wrong if there's a large difference between dx and dy.
   dip::sint nx1sx = ( nx - 1 ) * sx;
   dip::sint ny1sy = ( ny - 1 ) * sy;
   dip::sint nz1sz = ( nz - 1 ) * sz;
   dip::sint guess = ( dim * 2 - 1 );
   if( useTrue ) {
      guess *= 50;
   }

   // Allocate and initialize the necessary arrays
   std::vector< sfloat > firstBuffer( static_cast< dip::uint >( 2 * nx + 1 ));
   sfloat* fsdx = firstBuffer.data();
   sfloat dxx = dx * dx;
   for( dip::uint ii = 0; ii < firstBuffer.size(); ii++ ) {
      sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nx );
      fsdx[ ii ] = d * d * dxx;
   }
   sfloat* fsdy = nullptr;
   std::vector< sfloat> secondBuffer;
   if(( dx == dy ) && ( nx == ny )) {
      fsdy = fsdx;
   } else {
      secondBuffer.resize( static_cast< dip::uint >( 2 * ny + 1 ));
      fsdy = secondBuffer.data();
      sfloat dyy = dy * dy;
      for( dip::uint ii = 0; ii < secondBuffer.size(); ii++ ) {
         sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( ny );
         fsdy[ ii ] = d * d * dyy;
      }
   }
   sfloat* fsdz = nullptr;
   std::vector< sfloat> thirdBuffer;
   if(( dx == dz ) && ( nx == nz )) {
      fsdz = fsdx;
   } else {
      if(( dy == dz ) && ( ny == nz )) {
         fsdz = fsdy;
      } else {
         thirdBuffer.resize( static_cast< dip::uint >( 2 * nz + 1 ));
         fsdz = thirdBuffer.data();
         sfloat dzz = dz * dz;
         for( dip::uint ii = 0; ii < thirdBuffer.size(); ii++ ) {
            sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nz );
            fsdz[ ii ] = d * d * dzz;
         }
      }
   }
   std::vector< sfloat > fdnbBuffer( static_cast< dip::uint >( 10 * ( nx + ny + nz )));
   sfloat* fdnb = fdnbBuffer.data();
   std::vector< dip::sint > nb( static_cast< dip::uint >( ny * nx * 2 * ( dim * guess + 2 )));
   std::vector< XYZPosition > pnbBuffer( static_cast< dip::uint >( 10 * ( nx + ny + nz )));
   XYZPosition* pnb = pnbBuffer.data();
   std::vector< dip::sint* > d( static_cast< dip::uint >(( nx + 2 ) * ( ny + 2 ) * 2 ));
   sfloat maxdist = fsdx[ 0 ] + fsdy[ 0 ] + fsdz[ 0 ];

   // Initialize
   dip::sint* nb0 = nb.data();
   dip::sint* nb1 = nb0 + nx * ny * ( guess * dim + 2 );
   dip::sint** d1 = d.data();
   dip::sint** d2 = d1 + ( nx + 2 ) * ( ny + 2 );
   dip::sint zero = 0;
   dip::sint* bp = ( border ? &zero : nullptr );

   dip::sint** dbp = d1;
   for( dip::sint ii = ( nx + 2 ) * ( ny + 2 ); --ii >= 0; ) {
      *dbp++ = bp;
   }

   for( dip::sint zz = 0, pz = 0; zz < nz; zz++, pz += sz ) {
      dip::sint* nbp = ( zz & 1 ? nb1 : nb0 );
      dip::sint** dcp = ( zz & 1 ? d1 : d2 );
      dip::sint** dbp = ( zz & 1 ? d2 + nx + 3 : d1 + nx + 3 );

      for( dip::sint ii = nx + 2; --ii >= 0; ) {
         *dcp++ = bp;
      }

      for( dip::sint yy = 0, py = 0; yy < ny; yy++, dcp += 2 + nx, dbp += 2, py += sy ) {
         *dcp++ = bp;
         for( dip::sint xx = 0, px = pz + py; xx < nx; xx++, dcp++, dbp++, px += sx ) {
            if( oi[ px ] != 0.0 ) {
               *dcp = nbp;
               dip::sint kk = 0;
               XYZPosition* pnbp = pnb;
               dip::sint* tnbp = *dbp;
               if( tnbp == nullptr ) {
                  pnbp->x = 0;
                  pnbp->y = 0;
                  pnbp->z = -1;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z - 1;
                     kk++;
                  }
               }
               tnbp = *( dcp - nx - 2 );
               if( tnbp == nullptr ) {
                  pnbp->x = 0;
                  pnbp->y = -1;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y - 1;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               tnbp = *( dcp - 1 );
               if( tnbp == nullptr ) {
                  pnbp->x = -1;
                  pnbp->y = 0;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x - 1;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               if( kk == 0 ) {
                  *nbp++ = 0;
               } else {
                  sfloat mindist{};
                  dip::sint minpos{};
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue, delta );
                  *nbp++ = kk;
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( nbp );
                  pnbp = pnb;
                  for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
               }
            } else {
               *dcp = nullptr;
            }
         }

         *dcp-- = bp;
         for( dip::sint xx = 0; xx < nx; xx++, dcp-- ) {
            if( *dcp != nullptr ) {
               dip::sint kk = **dcp;
               XYZPosition* nbs = reinterpret_cast< XYZPosition* >( *dcp + 1 );
               XYZPosition* pnbp = pnb;
               for( dip::sint jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
                  pnbp->x = nbs->x;
                  pnbp->y = nbs->y;
                  pnbp->z = nbs->z;
               }
               dip::sint* tnbp = *( dcp + 1 );
               if( tnbp == nullptr ) {
                  pnbp->x = 1;
                  pnbp->y = 0;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x + 1;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               *dcp = nbp;
               if( kk == 0 ) {
                  *nbp++ = 0;
               } else {
                  sfloat mindist{};
                  dip::sint minpos{};
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue, delta );
                  *nbp++ = kk;
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( nbp );
                  pnbp = pnb;
                  for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
               }
            }
         }
      }

      for( dip::sint ii = nx + 2; --ii >= 0; ) {
         *dcp++ = bp;
      }
      dcp -= nx + 4;

      for( dip::sint yy = 0, py = ny1sy; yy < ny; yy++, dcp -= 2, py -= sy ) {
         for( dip::sint xx = 0, px = pz + py + nx1sx; xx < nx; xx++, dcp--, px -= sx ) {
            if( *dcp != nullptr ) {
               dip::sint kk = **dcp;
               XYZPosition* nbs = reinterpret_cast< XYZPosition* >( *dcp + 1 );
               XYZPosition* pnbp = pnb;
               for( dip::sint jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
                  pnbp->x = nbs->x;
                  pnbp->y = nbs->y;
                  pnbp->z = nbs->z;
               }
               dip::sint* tnbp = *( dcp + nx + 2 );
               if( tnbp == nullptr ) {
                  pnbp->x = 0;
                  pnbp->y = 1;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y + 1;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               *dcp = nbp;
               if( kk == 0 ) {
                  oi[ px ] = maxdist;
                  *nbp++ = 0;
               } else {
                  sfloat mindist{};
                  dip::sint minpos{};
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue, delta );
                  *nbp++ = kk;
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( nbp );
                  pnbp = pnb;
                  for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                     nbs->x = pnbp->x;
                     nbs->y = pnbp->y;
                     nbs->z = pnbp->z;
                  }
                  nbp += kk * dim;
                  oi[ px ] = mindist;
               }
            } else {
               oi[ px ] = 0.0;
            }
         }
      }
   }

   dbp = d1;
   for( dip::sint ii = ( nx + 2 ) * ( ny + 2 ); --ii >= 0; ) {
      *dbp++ = bp;
   }

   for( dip::sint zz = 0, pz = nz1sz; zz < nz; zz++, pz -= sz ) {
      dip::sint* nbp = ( zz & 1 ? nb1 : nb0 );
      dip::sint** dcp = ( zz & 1 ? d1 + ( nx + 2 ) * ( ny + 2 ) - 1 : d2 + ( nx + 2 ) * ( ny + 2 ) - 1 );
      dip::sint** dbp = ( zz & 1 ? d2 + ( nx + 2 ) * ( ny + 1 ) - 2 : d1 + ( nx + 2 ) * ( ny + 1 ) - 2 );

      for( dip::sint ii = nx + 2; --ii >= 0; ) {
         *dcp-- = bp;
      }

      for( dip::sint yy = 0, py = ny1sy; yy < ny; yy++, dcp -= 2 + nx, dbp -= 2, py -= sy ) {
         *dcp-- = bp;
         for( dip::sint xx = 0, px = pz + py + nx1sx; xx < nx; xx++, dcp--, dbp--, px -= sx ) {
            if( oi[ px ] != 0.0 ) {
               *dcp = nbp;
               dip::sint kk = 0;
               XYZPosition* pnbp = pnb;
               dip::sint* tnbp = *dbp;
               if( tnbp == nullptr ) {
                  pnbp->x = 0;
                  pnbp->y = 0;
                  pnbp->z = 1;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z + 1;
                     kk++;
                  }
               }
               tnbp = *( dcp + nx + 2 );
               if( tnbp == nullptr ) {
                  pnbp->x = 0;
                  pnbp->y = 1;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y + 1;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               tnbp = *( dcp + 1 );
               if( tnbp == nullptr ) {
                  pnbp->x = 1;
                  pnbp->y = 0;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x + 1;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               if( kk == 0 ) {
                  *nbp++ = 0;
               } else {
                  sfloat mindist{};
                  dip::sint minpos{};
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue, delta );
                  *nbp++ = kk;
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( nbp );
                  pnbp = pnb;
                  for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
               }
            } else {
               *dcp = nullptr;
            }
         }

         *dcp++ = bp;
         for( dip::sint xx = 0; xx < nx; xx++, dcp++ ) {
            if( *dcp != nullptr ) {
               dip::sint kk = **dcp;
               XYZPosition* nbs = reinterpret_cast< XYZPosition* >( *dcp + 1 );
               XYZPosition* pnbp = pnb;
               for( dip::sint jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
                  *pnbp = *nbs;
               }
               dip::sint* tnbp = *( dcp - 1 );
               if( tnbp == nullptr ) {
                  pnbp->x = -1;
                  pnbp->y = 0;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x - 1;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               *dcp = nbp;
               if( kk == 0 ) {
                  *nbp++ = 0;
               } else {
                  sfloat mindist{};
                  dip::sint minpos{};
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue, delta );
                  *nbp++ = kk;
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( nbp );
                  pnbp = pnb;
                  for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
               }
            }
         }
      }
      for( dip::sint ii = nx + 2; --ii >= 0; ) {
         *dcp-- = bp;
      }
      dcp += nx + 4;

      for( dip::sint yy = 0, py = 0; yy < ny; yy++, dcp += 2, py += sy ) {
         for( dip::sint xx = 0, px = pz + py; xx < nx; xx++, dcp++, px += sx ) {
            if( *dcp != nullptr ) {
               dip::sint kk = **dcp;
               XYZPosition* nbs = reinterpret_cast< XYZPosition* >( *dcp + 1 );
               XYZPosition* pnbp = pnb;
               for( dip::sint jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
                  *pnbp = *nbs;
               }
               dip::sint* tnbp = *( dcp - nx - 2 );
               if( tnbp == nullptr ) {
                  pnbp->x = 0;
                  pnbp->y = -1;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( dip::sint jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y - 1;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               *dcp = nbp;
               if( kk == 0 ) {
                  *nbp++ = 0;
                  oi[ px ] = std::sqrt( oi[ px ] );
               } else {
                  sfloat mindist{};
                  dip::sint minpos{};
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue, delta );
                  *nbp++ = kk;
                  XYZPosition* nbs = reinterpret_cast< XYZPosition* >( nbp );
                  pnbp = pnb;
                  for( dip::sint jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
                  if( mindist < oi[ px ] ) {
                     oi[ px ] = std::sqrt( mindist );
                  } else {
                     oi[ px ] = std::sqrt( oi[ px ] );
                  }
               }
            }
         }
      }
   }
}

void EDTBruteForce2D(
      sfloat* oi,
      UnsignedArray const& sizes,
      IntegerArray const& stride,
      FloatArray const& distance
) {
   dip::sint nx = static_cast< dip::sint >( sizes[ 0 ] );
   dip::sint ny = static_cast< dip::sint >( sizes[ 1 ] );
   dip::sint sx = stride[ 0 ];
   dip::sint sy = stride[ 1 ];
   sfloat dx = static_cast< sfloat >( distance[ 0 ] );
   sfloat dy = static_cast< sfloat >( distance[ 1 ] );

   // Allocate and initialize the necessary arrays
   std::vector< XYPosition > bord( static_cast< dip::uint >(( nx * ny ) / 2 + 1 ));
   std::vector< sfloat > firstBuffer( static_cast< dip::uint >( 2 * nx + 1 ));
   sfloat* fsdx = firstBuffer.data();
   sfloat dxx = dx * dx;
   for( dip::uint ii = 0; ii < firstBuffer.size(); ii++ ) {
      sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nx );
      fsdx[ ii ] = d * d * dxx;
   }
   sfloat* fsdy = nullptr;
   std::vector< sfloat> secondBuffer;
   if(( dx == dy ) && ( nx == ny )) {
      fsdy = fsdx;
   } else {
      secondBuffer.resize( static_cast< dip::uint >( 2 * ny + 1 ));
      fsdy = secondBuffer.data();
      sfloat dyy = dy * dy;
      for( dip::uint ii = 0; ii < secondBuffer.size(); ii++ ) {
         sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( ny );
         fsdy[ ii ] = d * d * dyy;
      }
   }
   sfloat maxdist = fsdx[ 0 ] + fsdy[ 0 ];

   XYPosition* bp = bord.data();
   dip::sint nbp = 0;
   dip::sint py = 0;
   for( dip::sint yy = 0; yy < ny; yy++, py += sy ) {
      dip::sint px = py;
      for( dip::sint xx = 0; xx < nx; xx++, px += sx ) {
         if( oi[ px ] == 0.0 ) {
            if((( yy > 0 )      && ( oi[ px - sy ] != 0.0 )) ||
               (( xx > 0 )      && ( oi[ px - sx ] != 0.0 )) ||
               (( yy < ny - 1 ) && ( oi[ px + sy ] != 0.0 )) ||
               (( xx < nx - 1 ) && ( oi[ px + sx ] != 0.0 ))) {
               bp->x = xx;
               bp->y = yy;
               bp++;
               nbp++;
            }
         }
      }
   }
   py = 0;
   for( dip::sint yy = 0 ; yy < ny; yy++, py += sy ) {
      dip::sint px = py;
      for( dip::sint xx = 0; xx < nx; xx++, px += sx ) {
         if( oi[ px ] != 0.0 ) {
            sfloat newdist = maxdist;
            bp = bord.data();
            for( dip::sint k = 0; k < nbp; k++, bp++ ) {
               sfloat dist = fsdy[ ny + yy - bp->y ] + fsdx[ nx + xx - bp->x ];
               if( dist < newdist ) {
                  newdist = dist;
               }
            }
            oi[ px ] = std::sqrt( newdist );
         } else {
            oi[ px ] = 0.0;
         }
      }
   }
}

void EDTBruteForce3D(
      sfloat* oi,
      UnsignedArray const& sizes,
      IntegerArray const& stride,
      FloatArray const& distance
) {
   dip::sint nx = static_cast< dip::sint >( sizes[ 0 ] );
   dip::sint ny = static_cast< dip::sint >( sizes[ 1 ] );
   dip::sint nz = static_cast< dip::sint >( sizes[ 2 ] );
   dip::sint sx = stride[ 0 ];
   dip::sint sy = stride[ 1 ];
   dip::sint sz = stride[ 2 ];
   sfloat dx = static_cast< sfloat >( distance[ 0 ] );
   sfloat dy = static_cast< sfloat >( distance[ 1 ] );
   sfloat dz = static_cast< sfloat >( distance[ 2 ] );

   // Allocate and initialize the necessary arrays
   std::vector< XYZPosition > bord( static_cast< dip::uint >(( nx * ny * nz ) / 2 + 1 ));
   std::vector< sfloat > firstBuffer( static_cast< dip::uint >( 2 * nx + 1 ));
   sfloat* fsdx = firstBuffer.data();
   sfloat dxx = dx * dx;
   for( dip::uint ii = 0; ii < firstBuffer.size(); ii++ ) {
      sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nx );
      fsdx[ ii ] = d * d * dxx;
   }
   sfloat* fsdy = nullptr;
   std::vector< sfloat> secondBuffer;
   if(( dx == dy ) && ( nx == ny )) {
      fsdy = fsdx;
   } else {
      secondBuffer.resize( static_cast< dip::uint >( 2 * ny + 1 ));
      fsdy = secondBuffer.data();
      sfloat dyy = dy * dy;
      for( dip::uint ii = 0; ii < secondBuffer.size(); ii++ ) {
         sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( ny );
         fsdy[ ii ] = d * d * dyy;
      }
   }
   sfloat* fsdz = nullptr;
   std::vector< sfloat> thirdBuffer;
   if(( dx == dz ) && ( nx == nz )) {
      fsdz = fsdx;
   } else {
      if(( dy == dz ) && ( ny == nz )) {
         fsdz = fsdy;
      } else {
         thirdBuffer.resize( static_cast< dip::uint >( 2 * nz + 1 ));
         fsdz = thirdBuffer.data();
         sfloat dzz = dz * dz;
         for( dip::uint ii = 0; ii < thirdBuffer.size(); ii++ ) {
            sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nz );
            fsdz[ ii ] = d * d * dzz;
         }
      }
   }
   sfloat maxdist = fsdx[ 0 ] + fsdy[ 0 ] + fsdz[ 0 ];

   XYZPosition* bp = bord.data();
   dip::sint nbp = 0;
   dip::sint pz = 0;
   for( dip::sint zz = 0; zz < nz; zz++, pz += sz ) {
      dip::sint py = 0;
      for( dip::sint yy = 0; yy < ny; yy++, py += sy ) {
         dip::sint px = pz + py;
         for( dip::sint xx = 0; xx < nx; xx++, px += sx ) {
            if( oi[ px ] == 0.0 ) {
               if((( zz > 0 )      && ( oi[ px - sz ] != 0.0 )) ||
                  (( yy > 0 )      && ( oi[ px - sy ] != 0.0 )) ||
                  (( xx > 0 )      && ( oi[ px - sx ] != 0.0 )) ||
                  (( zz < nz - 1 ) && ( oi[ px + sz ] != 0.0 )) ||
                  (( yy < ny - 1 ) && ( oi[ px + sy ] != 0.0 )) ||
                  (( xx < nx - 1 ) && ( oi[ px + sx ] != 0.0 ))) {
                  bp->x = xx;
                  bp->y = yy;
                  bp->z = zz;
                  bp++;
                  nbp++;
               }
            }
         }
      }
   }
   pz = 0;
   for( dip::sint zz = 0; zz < nz; zz++, pz += sz ) {
      dip::sint py = 0;
      for( dip::sint yy = 0; yy < ny; yy++, py += sy ) {
         dip::sint px = pz + py;
         for( dip::sint xx = 0; xx < nx; xx++, px += sx ) {
            if( oi[ px ] != 0.0 ) {
               sfloat newdist = maxdist;
               bp = bord.data();
               for( dip::sint k = 0; k < nbp; k++, bp++ ) {
                  sfloat dist = fsdy[ ny + yy - bp->y ] + fsdx[ nx + xx - bp->x ] + fsdz[ nz + zz - bp->z ];
                  if( dist < newdist ) {
                     newdist = dist;
                  }
               }
               oi[ px ] = std::sqrt( newdist );
            } else {
               oi[ px ] = 0.0;
            }
         }
      }
   }
}

} // namespace

void EuclideanDistanceTransform(
      Image const& in,
      Image& out,
      String const& border,
      String const& method
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );

   bool objectBorder{};
   DIP_STACK_TRACE_THIS( objectBorder = BooleanFromString( border, S::OBJECT, S::BACKGROUND ));

   // Distances to neighboring pixels
   dip::uint dim = in.Dimensionality();
   FloatArray dist( dim, 1 );
   if( in.HasPixelSize() ) {
      for( dip::uint ii = 0; ii < dim; ++ii ) {
         dist[ ii ] = in.PixelSize( ii ).magnitude;
      }
   }

   if( method == S::SEPARABLE ) {

      SeparableDistanceTransform( in, out, dist, objectBorder, false );

   } else if( method == S::SQUARE ) {

      SeparableDistanceTransform( in, out, dist, objectBorder, true );

   } else {
      DIP_THROW_IF(( dim > 3 ) || ( dim < 2 ), E::DIMENSIONALITY_NOT_SUPPORTED );

      // Convert in to out and get data pointer of out
      Convert( in, out, DT_SFLOAT );
      UnsignedArray const& sizes = out.Sizes();
      IntegerArray const& stride = out.Strides();
      sfloat* data = static_cast< sfloat* >( out.Origin() );

      // Call the real guts function
      if( method == S::FAST ) {
         if( dim == 2 ) {
            EDTFast2D( data, sizes, stride, dist, objectBorder );
         } else {
            EDTFast3D( data, sizes, stride, dist, objectBorder );
         }
         Sqrt( out, out );
      } else if(( method == S::TIES ) || ( method == S::TRUE )) {
         bool useTrue = method == S::TRUE;
         if( dim == 2 ) {
            EDTTies2D( data, sizes, stride, dist, objectBorder, useTrue );
         } else {
            EDTTies3D( data, sizes, stride, dist, objectBorder, useTrue );
         }
      } else if( method == S::BRUTE_FORCE ) {
         DIP_THROW_IF( !objectBorder, "The \"brute force\" method doesn't handle \"background\" for border" );
         if( dim == 2 ) {
            EDTBruteForce2D( data, sizes, stride, dist );
         } else {
            EDTBruteForce3D( data, sizes, stride, dist );
         }
      } else {
         DIP_THROW_INVALID_FLAG( method );
      }
   }
}

} // namespace dip
