/*
 * DIPlib 3.0
 * This file contains definitions for distance transforms
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

#include "diplib.h"
#include "diplib/distance.h"

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

void VDTFast2D(
      sfloat* ox,
      sfloat* oy,
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
   sfloat* fsdy = 0;
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

   dip::sint ii, xx, yy, px, py;
   sfloat fdb, fdc;
   XYPosition* dcl, * dbl;
   XYPosition* c, * b;

   // Forward scan
   dbl = &d1[ 1 ];
   for( ii = 0; ii < nx; ii++ ) {
      dbl[ ii ] = bp;
   }

   for( yy = 0, py = 0; yy < ny; yy++, py += sy ) {
      // We split d into buffers that are swapped to read or write from
      dcl = ( yy & 1 ? d1 : d2 );
      dbl = ( yy & 1 ? d2 + 1 : d1 + 1 );

      *dcl++ = bp;
      for( xx = 0, px = py; xx < nx; xx++, dcl++, dbl++, px += sx ) {
         if( ox[ px ] != 0.0 ) {
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
                  c = dcl;
                  b = ( dcl - 1 );
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
      for( xx = 0, px = py + nx1sx; xx < nx; xx++, dcl--, px -= sx ) {
         if(( dcl->x != zero.x ) || ( dcl->y != zero.y )) {
            if((( dcl + 1 )->x == infd.x ) && (( dcl + 1 )->y == infd.y )) {
               if(( dcl->x == infd.x ) && ( dcl->y == infd.y )) {
                  ox[ px ] = 0;
                  oy[ px ] = 0;
               } else {
                  c = dcl;
                  ox[ px ] = static_cast< sfloat >( c->x );
                  oy[ px ] = static_cast< sfloat >( c->y );
               }
            } else {
               c = dcl;
               b = ( dcl + 1 );
               fdc = fsdx[ c->x ] + fsdy[ c->y ];
               fdb = fsdx[ b->x + 1 ] + fsdy[ b->y ];
               if( fdc > fdb ) {
                  dcl->x = ( dcl + 1 )->x + dx1.x;
                  dcl->y = ( dcl + 1 )->y + dx1.y;
                  ox[ px ] = static_cast< sfloat >( b->x + 1 );
                  oy[ px ] = static_cast< sfloat >( b->y );
               } else {
                  ox[ px ] = static_cast< sfloat >( c->x );
                  oy[ px ] = static_cast< sfloat >( c->y );
               }
            }
         }
            // Will be set to zero and the end
         else {
            ox[ px ] = static_cast< sfloat >( nx );
            oy[ px ] = static_cast< sfloat >( ny );
         }
      }
   }

   // Backward scan
   dbl = d1 + 1;
   for( ii = 0; ii < nx; ii++ ) {
      dbl[ ii ] = bp;
   }

   for( ii = 0, py = ny1sy; ii < ny; ii++, py -= sy ) {
      dcl = ( ii & 1 ? d1 + nx + 1 : d2 + nx + 1 );
      dbl = ( ii & 1 ? d2 + nx : d1 + nx );
      *dcl-- = bp;

      for( xx = 0, px = py + nx1sx; xx < nx; xx++, dcl--, dbl--, px -= sx ) {
         if( fsdx[ static_cast< dip::sint >( ox[ px ] ) ] + fsdy[ static_cast< dip::sint >( oy[ px ] ) ] != 0.0 ) {
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
               c = dcl;
               b = ( dcl + 1 );
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
      for( xx = 0, px = py; xx < nx; xx++, dcl++, px += sx ) {
         if( dcl->x != zero.x || dcl->y != zero.y ) {
            if(( dcl - 1 )->x == infd.x && ( dcl - 1 )->y == infd.y ) {
               if( dcl->x != infd.x || dcl->y != infd.y ) {
                  c = dcl;
                  fdc = fsdx[ c->x ] + fsdy[ c->y ];
                  if( fsdx[ static_cast< dip::sint >( ox[ px ] ) ] + fsdy[ static_cast< dip::sint >( oy[ px ] ) ] > fdc ) {
                     ox[ px ] = static_cast< sfloat >( c->x );
                     oy[ px ] = static_cast< sfloat >( c->y );
                  }
               }
            } else {
               c = dcl;
               b = ( dcl - 1 );
               fdc = fsdx[ c->x ] + fsdy[ c->y ];
               fdb = fsdx[ b->x - 1 ] + fsdy[ b->y ];
               if( fdc > fdb ) {
                  dcl->x = ( dcl - 1 )->x - dx1.x;
                  dcl->y = ( dcl - 1 )->y - dx1.y;
                  if( fsdx[ static_cast< dip::sint >( ox[ px ] ) ] + fsdy[ static_cast< dip::sint >( oy[ px ] ) ] > fdb ) {
                     ox[ px ] = static_cast< sfloat >( b->x - 1 );
                     oy[ px ] = static_cast< sfloat >( b->y );
                  }
               } else {
                  if( fsdx[ static_cast< dip::sint >( ox[ px ] ) ] + fsdy[ static_cast< dip::sint >( oy[ px ] ) ] > fdc ) {
                     ox[ px ] = static_cast< sfloat >( c->x );
                     oy[ px ] = static_cast< sfloat >( c->y );
                  }
               }
            }
         }
      }
   }

   for( yy = 0, py = 0; yy < ny; yy++, py += sy ) {
      for( xx = 0, px = py; xx < nx; xx++, px += sx ) {
         ox[ px ] = ( ox[ px ] - static_cast< sfloat >( nx )) * dx;
         oy[ px ] = ( oy[ px ] - static_cast< sfloat >( ny )) * dy;
      }
   }
}

void VDTFast3D(
      sfloat* ox,
      sfloat* oy,
      sfloat* oz,
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
   sfloat* fsdy = 0;
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
   sfloat* fsdz = 0;
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
         for( dip::uint ii = 0; ii < secondBuffer.size(); ii++ ) {
            sfloat d = static_cast< sfloat >( ii ) - static_cast< sfloat >( nz );
            fsdz[ ii ] = d * d * dzz;
         }
      }
   }
   std::vector< XYZPosition > d( static_cast< dip::uint >(( nx + 2 ) * ( ny + 2 ) * 2));

   // Initialize
   XYZPosition* d1 = d.data();
   XYZPosition* d2 = d1 + ( nx + 2 ) * ( ny + 2 );
   XYZPosition infd = { 0, 0, 0 };
   XYZPosition zero = { nx, ny, nz };
   XYZPosition bp = border ? infd : zero;

   dip::sint ii, xx, yy, zz, px, pz, py;
   XYZPosition* dcl, * dbl, * dbt;
   sfloat fdb, fdc;

   dbl = d1;
   for( ii = ( nx + 2 ) * ( ny + 2 ); --ii >= 0; ) {
      *dbl++ = bp;
   }

   for( zz = 0, pz = 0; zz < nz; zz++, pz += sz ) {

      dcl = ( zz & 1 ? d1 : d2 );
      dbl = ( zz & 1 ? d2 + nx + 3 : d1 + nx + 3 );

      for( ii = nx + 2; --ii >= 0; ) {
         *dcl++ = bp;
      }

      for( yy = 0, py = 0; yy < ny; yy++, dcl += 2 + nx, dbl += 2, py += sy ) {
         *dcl++ = bp;

         for( xx = 0, px = py + pz; xx < nx; xx++, dcl++, dbl++, px += sx ) {
            if( ox[ px ] != 0.0 ) {
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

               dbt = dcl - ( nx + 2 );

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
         for( xx = nx; --xx >= 0; dcl-- ) {
            if( dcl->x != zero.x || dcl->y != zero.y || dcl->z != zero.z ) {
               dbt = dcl + 1;

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

      for( ii = nx + 2; --ii >= 0; ) {
         *dcl++ = bp;
      }

      dcl -= nx + 4;
      for( yy = 0, py = ny1sy; yy < ny; yy++, dcl -= 2, py -= sy ) {
         for( xx = nx, px = pz + py + nx1sx; --xx >= 0; dcl--, px -= sx ) {
            if( dcl->x != zero.x || dcl->y != zero.y || dcl->z != zero.z ) {
               dbt = dcl + ( nx + 2 );

               if( dbt->x == infd.x && dbt->y == infd.y && dbt->z == infd.z ) {
                  if( dcl->x == infd.x && dcl->y == infd.y && dcl->z == infd.z ) {
                     ox[ px ] = 0.0;
                     oy[ px ] = 0.0;
                     oz[ px ] = 0.0;
                  } else {
                     ox[ px ] = static_cast< sfloat >( dcl->x );
                     oy[ px ] = static_cast< sfloat >( dcl->y );
                     oz[ px ] = static_cast< sfloat >( dcl->z );
                  }
               } else {
                  fdc = fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ];
                  fdb = fsdx[ dbt->x ] + fsdy[ dbt->y + 1 ] + fsdz[ dbt->z ];
                  if( fdc > fdb ) {
                     ox[ px ] = static_cast< sfloat >( dcl->x = dbt->x );
                     oy[ px ] = static_cast< sfloat >( dcl->y = dbt->y + 1 );
                     oz[ px ] = static_cast< sfloat >( dcl->z = dbt->z );
                  } else {
                     ox[ px ] = static_cast< sfloat >( dcl->x );
                     oy[ px ] = static_cast< sfloat >( dcl->y );
                     oz[ px ] = static_cast< sfloat >( dcl->z );
                  }
               }
            } else {
               ox[ px ] = static_cast< sfloat >( nx );
               oy[ px ] = static_cast< sfloat >( ny );
               oz[ px ] = static_cast< sfloat >( nz );
            }
         }
      }
   }

   dbl = d1;
   for( ii = ( nx + 2 ) * ( ny + 2 ); --ii >= 0; ) {
      *dbl++ = bp;
   }

   for( zz = 0, pz = nz1sz; zz < nz; zz++, pz -= sz ) {
      dcl = ( zz & 1 ? ( d1 + ( nx + 2 ) * ( ny + 2 ) - 1 ) : ( d2 + ( nx + 2 ) * ( ny + 2 ) - 1 ));
      dbl = ( zz & 1 ? ( d2 + ( nx + 2 ) * ( ny + 1 ) - 2 ) : ( d1 + ( nx + 2 ) * ( ny + 1 ) - 2 ));

      for( ii = nx + 2; --ii >= 0; ) {
         *dcl-- = bp;
      }

      for( yy = 0, py = ny1sy; yy < ny; yy++, dcl -= 2 + nx, dbl -= 2, py -= sy ) {
         *dcl-- = bp;

         for( xx = 0, px = py + pz + nx1sx; xx < nx; xx++, dcl--, dbl--, px -= sx ) {
            if( ox[ px ] != static_cast< dfloat >( nx ) ||
                oy[ px ] != static_cast< dfloat >( ny ) ||
                oz[ px ] != static_cast< dfloat >( nz )) {
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

               dbt = dcl + ( nx + 2 );

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
         for( xx = nx; --xx >= 0; dcl++ ) {
            if( dcl->x != zero.x || dcl->y != zero.y || dcl->z != zero.z ) {
               dbt = dcl - 1;

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

      for( ii = nx + 2; --ii >= 0; ) {
         *dcl-- = bp;
      }

      dcl += nx + 4;
      for( yy = 0, py = 0; yy < ny; yy++, dcl += 2, py += sy ) {
         for( xx = nx, px = pz + py; --xx >= 0; dcl++, px += sx ) {
            if( dcl->x != zero.x || dcl->y != zero.y || dcl->z != zero.z ) {
               dbt = dcl - ( nx + 2 );

               if( dbt->x == infd.x && dbt->y == infd.y && dbt->z == infd.z ) {
                  if( dcl->x != infd.x || dcl->y != infd.y || dcl->z != infd.z ) {
                     fdc = fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ];
                     if( fsdx[ static_cast< dip::sint >( ox[ px ] ) ] +
                         fsdy[ static_cast< dip::sint >( oy[ px ] ) ] +
                         fsdz[ static_cast< dip::sint >( oz[ px ] ) ] > fdc ) {
                        ox[ px ] = static_cast< sfloat >( dcl->x );
                        oy[ px ] = static_cast< sfloat >( dcl->y );
                        oz[ px ] = static_cast< sfloat >( dcl->z );
                     }
                  }
               } else {
                  fdc = fsdx[ dcl->x ] + fsdy[ dcl->y ] + fsdz[ dcl->z ];
                  fdb = fsdx[ dbt->x ] + fsdy[ dbt->y - 1 ] + fsdz[ dbt->z ];
                  if( fdc > fdb ) {
                     dcl->x = dbt->x;
                     dcl->y = dbt->y - 1;
                     dcl->z = dbt->z;
                     if( fsdx[ static_cast< dip::sint >( ox[ px ] ) ] +
                         fsdy[ static_cast< dip::sint >( oy[ px ] ) ] +
                         fsdz[ static_cast< dip::sint >( oz[ px ] ) ] > fdb ) {
                        ox[ px ] = static_cast< sfloat >( dcl->x );
                        oy[ px ] = static_cast< sfloat >( dcl->y );
                        oz[ px ] = static_cast< sfloat >( dcl->z );
                     }
                  } else {
                     if( fsdx[ static_cast< dip::sint >( ox[ px ] ) ] +
                         fsdy[ static_cast< dip::sint >( oy[ px ] ) ] +
                         fsdz[ static_cast< dip::sint >( oz[ px ] ) ] > fdc ) {
                        ox[ px ] = static_cast< sfloat >( dcl->x );
                        oy[ px ] = static_cast< sfloat >( dcl->y );
                        oz[ px ] = static_cast< sfloat >( dcl->z );
                     }
                  }
               }
            }
         }
      }
   }

   for( zz = 0, pz = 0; zz < nz; zz++, pz += sz ) {
      for( yy = 0, py = 0; yy < ny; yy++, py += sy ) {
         for( xx = 0, px = pz + py; xx < nx; xx++, px += sx ) {
            ox[ px ] = ( ox[ px ] - static_cast< sfloat >( nx )) * dx;
            oy[ px ] = ( oy[ px ] - static_cast< sfloat >( ny )) * dy;
            oz[ px ] = ( oz[ px ] - static_cast< sfloat >( nz )) * dz;
         }
      }
   }
}

dip::sint FindNeighbors2D(
      XYPosition* p,
      sfloat* mindist,
      dip::sint* minpos,
      dip::sint n,
      dip::sint nx,
      dip::sint ny,
      sfloat* fdnb,
      sfloat* fsdx,
      sfloat* fsdy,
      bool useTrue
) {
   dip::sint i, j, k, pos;
   sfloat* dnbp, min;
   XYPosition* pnbp;

   for( i = n, dnbp = fdnb, pnbp = p; --i >= 0; pnbp++ ) {
      *dnbp++ = fsdx[ pnbp->x + nx ] + fsdy[ pnbp->y + ny ];
   }

   dnbp = fdnb;
   min = *dnbp++;
   pos = 0;
   for( i = 1; i < n; i++, dnbp++ ) {
      if( *dnbp < min ) {
         min = *dnbp;
         pos = i;
      }
   }
   *mindist = min;
   *minpos = pos;

   if( useTrue ) {
      min = std::sqrt( min ) + 0.8f;
      min *= min;
   }

   dnbp = fdnb;
   for( i = 0, j = 0; i < n; i++ ) {
      if( useTrue ) {
         if( min >= *dnbp++ ) {
            if( i != j ) {
               p[ j ] = p[ i ];
            }
            j++;
         }
      } else {
         if( min == *dnbp++ ) {
            if( i != j ) {
               p[ j ] = p[ i ];
            }
            j++;
         }
      }
   }

   for( k = 0; k < j - 1; k++ ) {
      for( i = k + 1; i < j; i++ ) {
         if( p[ i ].x == p[ k ].x && p[ i ].y == p[ k ].y ) {
            if( i != --j ) {
               p[ i ] = p[ j ];
            }
            i--;
         }
      }
   }

   return j;
}

void VDTTies2D(
      sfloat* ox,
      sfloat* oy,
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
   sfloat* fsdy = 0;
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

   // Initialize
   dip::sint* nb0 = nb.data();
   dip::sint* nb1 = nb0 + nx * ( guess * dim + 2 );
   dip::sint** d1 = d.data();
   dip::sint** d2 = d1 + ( nx + 2 );
   dip::sint zero = 0;
   dip::sint* bp = ( border ? &zero : NULL );

   dip::sint py, px;
   dip::sint xx, yy, ii, jj, kk;
   dip::sint** dcl, ** dbl;
   dip::sint* nbp, * tnbp;
   dip::sint minpos;
   sfloat mindist;
   XYPosition* nbs;
   XYPosition* pnbp;

   dbl = d1 + 1;
   for( ii = nx; --ii >= 0; ) {
      *dbl++ = bp;
   }

   for( yy = 0, py = 0; yy < ny; yy++, py += sy ) {

      nbp = ( yy & 1 ? nb1 : nb0 );
      dcl = ( yy & 1 ? d1 : d2 );
      dbl = ( yy & 1 ? d2 + 1 : d1 + 1 );

      *dcl++ = bp;
      for( xx = 0, px = py; xx < nx; xx++, dcl++, dbl++, px += sx ) {
         if( ox[ px ] != 0.0 ) {
            *dcl = nbp;
            kk = 0;
            pnbp = pnb;
            tnbp = *dbl;
            if( tnbp == NULL ) {
               pnbp->x = 0;
               pnbp->y = -1;
               kk++;
               pnbp++;
            } else {
               nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x;
                  pnbp->y = nbs->y - 1;
                  kk++;
               }
            }
            tnbp = *( dcl - 1 );
            if( tnbp == NULL ) {
               pnbp->x = -1;
               pnbp->y = 0;
               kk++;
            } else {
               nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x - 1;
                  pnbp->y = nbs->y;
                  kk++;
               }
            }
            if( kk == 0 ) {
               *nbp++ = 0;
            } else {
               kk = FindNeighbors2D( pnb, &mindist, &minpos, kk, nx, ny, fdnb, fsdx, fsdy, useTrue );
               *nbp++ = kk;
               nbs = reinterpret_cast< XYPosition* >( nbp );
               pnbp = pnb;
               for( jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                  *nbs = *pnbp;
               }
               nbp += kk * dim;
            }
         } else {
            *dcl = NULL;
         }
      }

      *dcl-- = bp;
      for( ii = 0, px = py + nx1sx; ii < nx; ii++, dcl--, px -= sx ) {
         if( *dcl != NULL ) {
            kk = **dcl;
            nbs = reinterpret_cast< XYPosition* >( *dcl + 1 );
            pnbp = pnb;
            for( jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
               *pnbp = *nbs;
            }
            tnbp = *( dcl + 1 );
            if( tnbp == NULL ) {
               pnbp->x = 1;
               pnbp->y = 0;
               kk++;
            } else {
               nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x + 1;
                  pnbp->y = nbs->y;
                  kk++;
               }
            }
            *dcl = nbp;
            if( kk == 0 ) {
               *nbp++ = 0;
               ox[ px ] = static_cast< sfloat >( -nx );
               oy[ px ] = static_cast< sfloat >( -ny );
            } else {
               kk = FindNeighbors2D( pnb, &mindist, &minpos, kk, nx, ny, fdnb, fsdx, fsdy, useTrue );
               *nbp++ = kk;
               nbs = reinterpret_cast< XYPosition* >( nbp );
               pnbp = pnb;
               for( jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                  *nbs = *pnbp;
               }
               ox[ px ] = static_cast< sfloat >( pnb[ minpos ].x );
               oy[ px ] = static_cast< sfloat >( pnb[ minpos ].y );
               nbp += kk * dim;
            }
         } else {
            ox[ px ] = static_cast< sfloat >( nx );
            oy[ px ] = static_cast< sfloat >( ny );
         }
      }
   }

   dbl = d1 + 1;
   for( ii = nx; --ii >= 0; ) {
      *dbl++ = bp;
   }

   for( yy = 0, py = ny1sy; yy < ny; yy++, py -= sy ) {
      nbp = ( yy & 1 ? nb1 : nb0 );
      dcl = ( yy & 1 ? d1 + nx + 1 : d2 + nx + 1 );
      dbl = ( yy & 1 ? d2 + nx : d1 + nx );

      *dcl-- = bp;
      for( xx = 0, px = py + nx1sx; xx < nx; xx++, dcl--, dbl--, px -= sx ) {
         if( ox[ px ] != static_cast< dfloat >( nx ) &&
             oy[ px ] != static_cast< dfloat >( ny )) {
            *dcl = nbp;
            kk = 0;
            pnbp = pnb;
            tnbp = *dbl;
            if( tnbp == NULL ) {
               pnbp->x = 0;
               pnbp->y = 1;
               kk++;
               pnbp++;
            } else {
               nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x;
                  pnbp->y = nbs->y + 1;
                  kk++;
               }
            }
            tnbp = *( dcl + 1 );
            if( tnbp == NULL ) {
               pnbp->x = 1;
               pnbp->y = 0;
               kk++;
            } else {
               nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x + 1;
                  pnbp->y = nbs->y;
                  kk++;
               }
            }
            if( kk == 0 ) {
               *nbp++ = 0;
            } else {
               kk = FindNeighbors2D( pnb, &mindist, &minpos, kk, nx, ny, fdnb, fsdx, fsdy, useTrue );
               *nbp++ = kk;
               nbs = reinterpret_cast< XYPosition* >( nbp );
               pnbp = pnb;
               for( jj = kk; --jj >= 0; nbs++, pnbp++ ) {
                  *nbs = *pnbp;
               }
               nbp += kk * dim;
            }
         } else {
            *dcl = NULL;
         }
      }

      *dcl++ = bp;
      for( xx = 0, px = py; xx < nx; xx++, dcl++, px += sx ) {
         if( *dcl != NULL ) {
            kk = **dcl;
            nbs = reinterpret_cast< XYPosition* >( *dcl + 1 );
            pnbp = pnb;
            for( jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
               *pnbp = *nbs;
            }
            tnbp = *( dcl - 1 );
            if( tnbp == NULL ) {
               pnbp->x = -1;
               pnbp->y = 0;
               kk++;
            } else {
               nbs = reinterpret_cast< XYPosition* >( tnbp + 1 );
               for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                  pnbp->x = nbs->x - 1;
                  pnbp->y = nbs->y;
                  kk++;
               }
            }
            *dcl = nbp;
            if( kk == 0 ) {
               *nbp++ = 0;
               ox[ px ] *= dx;
               oy[ px ] *= dy;
            } else {
               kk = FindNeighbors2D( pnb, &mindist, &minpos, kk, nx, ny, fdnb, fsdx, fsdy, useTrue );
               *nbp++ = kk;
               nbs = reinterpret_cast< XYPosition* >( nbp );
               for( jj = kk, pnbp = pnb; --jj >= 0; nbs++, pnbp++ ) {
                  *nbs = *pnbp;
               }
               if( mindist < ( fsdx[ static_cast< dip::sint >( ox[ px ] ) + nx ] + fsdy[ static_cast< dip::sint >( oy[ px ] ) + ny ] )) {
                  ox[ px ] = static_cast< sfloat >( pnb[ minpos ].x ) * dx;
                  oy[ px ] = static_cast< sfloat >( pnb[ minpos ].y ) * dy;
               } else {
                  ox[ px ] *= dx;
                  oy[ px ] *= dy;
               }
               nbp += kk * dim;
            }
         } else {
            ox[ px ] = 0.0;
            oy[ px ] = 0.0;
         }
      }
   }
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
      sfloat* fsdx,
      sfloat* fsdy,
      sfloat* fsdz,
      bool useTrue
) {
   dip::sint i, j, k, pos;
   sfloat* dnbp, min;
   XYZPosition* pnbp;

   for( i = n, dnbp = fdnb, pnbp = p; --i >= 0; pnbp++ ) {
      *dnbp++ = fsdx[ pnbp->x + nx ] + fsdy[ pnbp->y + ny ] + fsdz[ pnbp->z + nz ];
   }

   dnbp = fdnb;
   min = *dnbp++;
   pos = 0;
   for( i = 1; i < n; i++, dnbp++ ) {
      if( *dnbp < min ) {
         min = *dnbp;
         pos = i;
      }
   }
   *mindist = min;
   *minpos = pos;

   if( useTrue ) {
      min = std::sqrt( min ) + 1.4f;
      min *= min;
   }

   dnbp = fdnb;
   for( i = 0, j = 0; i < n; i++ ) {
      if( useTrue ) {
         if( min >= *dnbp++ ) {
            if( i != j ) {
               p[ j ] = p[ i ];
            }
            j++;
         }
      } else {
         if( min == *dnbp++ ) {
            if( i != j ) {
               p[ j ] = p[ i ];
            }
            j++;
         }
      }
   }

   for( k = 0; k < j - 1; k++ ) {
      for( i = k + 1; i < j; i++ ) {
         if( p[ i ].x == p[ k ].x && p[ i ].y == p[ k ].y && p[ i ].z == p[ k ].z ) {
            if( i != --j ) {
               p[ i ] = p[ j ];
            }
            i--;
         }
      }
   }

   return j;
}

void VDTTies3D(
      sfloat* ox,
      sfloat* oy,
      sfloat* oz,
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
   sfloat* fsdy = 0;
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
   sfloat* fsdz = 0;
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
         for( dip::uint ii = 0; ii < secondBuffer.size(); ii++ ) {
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

   // Initialize
   dip::sint* nb0 = nb.data();
   dip::sint* nb1 = nb0 + nx * ny * ( guess * dim + 2 );
   dip::sint** d1 = d.data();
   dip::sint** d2 = d1 + ( nx + 2 ) * ( ny + 2 );
   dip::sint zero = 0;
   dip::sint* bp = ( border ? &zero : NULL );

   dip::sint px, pz, py;
   dip::sint xx, yy, zz, ii, jj, kk;
   dip::sint minpos;
   dip::sint** dcp, ** dbp;
   dip::sint* nbp, * tnbp;
   XYZPosition* nbs;
   XYZPosition* pnbp;
   sfloat mindist, dist;

   dbp = d1;
   for( ii = ( nx + 2 ) * ( ny + 2 ); --ii >= 0; ) {
      *dbp++ = bp;
   }

   for( zz = 0, pz = 0; zz < nz; zz++, pz += sz ) {
      nbp = ( zz & 1 ? nb1 : nb0 );
      dcp = ( zz & 1 ? d1 : d2 );
      dbp = ( zz & 1 ? d2 + nx + 3 : d1 + nx + 3 );

      for( ii = nx + 2; --ii >= 0; ) {
         *dcp++ = bp;
      }

      for( yy = 0, py = 0; yy < ny; yy++, dcp += 2 + nx, dbp += 2, py += sy ) {
         *dcp++ = bp;
         for( xx = 0, px = pz + py; xx < nx; xx++, dcp++, dbp++, px += sx ) {
            if( ox[ px ] != 0.0 ) {
               *dcp = nbp;
               kk = 0;
               pnbp = pnb;
               tnbp = *dbp;
               if( tnbp == NULL ) {
                  pnbp->x = 0;
                  pnbp->y = 0;
                  pnbp->z = -1;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z - 1;
                     kk++;
                  }
               }
               tnbp = *( dcp - nx - 2 );
               if( tnbp == NULL ) {
                  pnbp->x = 0;
                  pnbp->y = -1;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y - 1;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               tnbp = *( dcp - 1 );
               if( tnbp == NULL ) {
                  pnbp->x = -1;
                  pnbp->y = 0;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x - 1;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               if( kk == 0 ) {
                  *nbp++ = 0;
               } else {
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue );
                  *nbp++ = kk;
                  nbs = reinterpret_cast< XYZPosition* >( nbp );
                  for( jj = kk, pnbp = pnb; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
               }
            } else {
               *dcp = NULL;
            }
         }

         *dcp-- = bp;
         for( xx = 0; xx < nx; xx++, dcp-- ) {
            if( *dcp != NULL ) {
               kk = **dcp;
               nbs = reinterpret_cast< XYZPosition* >( *dcp + 1 );
               pnbp = pnb;
               for( jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
                  pnbp->x = nbs->x;
                  pnbp->y = nbs->y;
                  pnbp->z = nbs->z;
               }
               tnbp = *( dcp + 1 );
               if( tnbp == NULL ) {
                  pnbp->x = 1;
                  pnbp->y = 0;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
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
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue );
                  *nbp++ = kk;
                  nbs = reinterpret_cast< XYZPosition* >( nbp );
                  for( jj = kk, pnbp = pnb; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
               }
            }
         }
      }

      for( ii = nx + 2; --ii >= 0; ) {
         *dcp++ = bp;
      }
      dcp -= nx + 4;

      for( yy = 0, py = ny1sy; yy < ny; yy++, dcp -= 2, py -= sy ) {
         for( xx = 0, px = pz + py + nx1sx; xx < nx; xx++, dcp--, px -= sx ) {
            if( *dcp != NULL ) {
               kk = **dcp;
               nbs = reinterpret_cast< XYZPosition* >( *dcp + 1 );
               pnbp = pnb;
               for( jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
                  pnbp->x = nbs->x;
                  pnbp->y = nbs->y;
                  pnbp->z = nbs->z;
               }
               tnbp = *( dcp + nx + 2 );
               if( tnbp == NULL ) {
                  pnbp->x = 0;
                  pnbp->y = 1;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y + 1;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               *dcp = nbp;
               if( kk == 0 ) {
                  ox[ px ] = static_cast< sfloat >( -nx );
                  oy[ px ] = static_cast< sfloat >( -ny );
                  oz[ px ] = static_cast< sfloat >( -nz );
                  *nbp++ = 0;
               } else {
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue );
                  *nbp++ = kk;
                  nbs = reinterpret_cast< XYZPosition* >( nbp );
                  for( jj = kk, pnbp = pnb; --jj >= 0; nbs++, pnbp++ ) {
                     nbs->x = pnbp->x;
                     nbs->y = pnbp->y;
                     nbs->z = pnbp->z;
                  }
                  nbp += kk * dim;
                  ox[ px ] = static_cast< sfloat >( pnb[ minpos ].x );
                  oy[ px ] = static_cast< sfloat >( pnb[ minpos ].y );
                  oz[ px ] = static_cast< sfloat >( pnb[ minpos ].z );
               }
            } else {
               ox[ px ] = static_cast< sfloat >( nx );
               oy[ px ] = static_cast< sfloat >( ny );
               oz[ px ] = static_cast< sfloat >( nz );
            }
         }
      }
   }

   dbp = d1;
   for( ii = ( nx + 2 ) * ( ny + 2 ); --ii >= 0; ) {
      *dbp++ = bp;
   }

   for( zz = 0, pz = nz1sz; zz < nz; zz++, pz -= sz ) {
      nbp = ( zz & 1 ? nb1 : nb0 );
      dcp = ( zz & 1 ? d1 + ( nx + 2 ) * ( ny + 2 ) - 1 : d2 + ( nx + 2 ) * ( ny + 2 ) - 1 );
      dbp = ( zz & 1 ? d2 + ( nx + 2 ) * ( ny + 1 ) - 2 : d1 + ( nx + 2 ) * ( ny + 1 ) - 2 );

      for( ii = nx + 2; --ii >= 0; ) {
         *dcp-- = bp;
      }

      for( yy = 0, py = ny1sy; yy < ny; yy++, dcp -= 2 + nx, dbp -= 2, py -= sy ) {
         *dcp-- = bp;
         for( xx = 0, px = pz + py + nx1sx; xx < nx; xx++, dcp--, dbp--, px -= sx ) {
            if( ox[ px ] != static_cast< dfloat >( nx ) &&
                oy[ px ] != static_cast< dfloat >( ny ) &&
                oz[ px ] != static_cast< dfloat >( nz )) {
               *dcp = nbp;
               kk = 0;
               pnbp = pnb;
               tnbp = *dbp;
               if( tnbp == NULL ) {
                  pnbp->x = 0;
                  pnbp->y = 0;
                  pnbp->z = 1;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z + 1;
                     kk++;
                  }
               }
               tnbp = *( dcp + nx + 2 );
               if( tnbp == NULL ) {
                  pnbp->x = 0;
                  pnbp->y = 1;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y + 1;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               tnbp = *( dcp + 1 );
               if( tnbp == NULL ) {
                  pnbp->x = 1;
                  pnbp->y = 0;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x + 1;
                     pnbp->y = nbs->y;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               if( kk == 0 ) {
                  *nbp++ = 0;
               } else {
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue );
                  *nbp++ = kk;
                  nbs = reinterpret_cast< XYZPosition* >( nbp );
                  for( jj = kk, pnbp = pnb; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
               }
            } else {
               *dcp = NULL;
            }
         }

         *dcp++ = bp;
         for( xx = 0; xx < nx; xx++, dcp++ ) {
            if( *dcp != NULL ) {
               kk = **dcp;
               nbs = reinterpret_cast< XYZPosition* >( *dcp + 1 );
               pnbp = pnb;
               for( jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
                  *pnbp = *nbs;
               }
               tnbp = *( dcp - 1 );
               if( tnbp == NULL ) {
                  pnbp->x = -1;
                  pnbp->y = 0;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
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
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue );
                  *nbp++ = kk;
                  nbs = reinterpret_cast< XYZPosition* >( nbp );
                  for( jj = kk, pnbp = pnb; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
               }
            }
         }
      }
      for( ii = nx + 2; --ii >= 0; ) {
         *dcp-- = bp;
      }
      dcp += nx + 4;

      for( yy = 0, py = 0; yy < ny; yy++, dcp += 2, py += sy ) {
         for( xx = 0, px = pz + py; xx < nx; xx++, dcp++, px += sx ) {
            if( *dcp != NULL ) {
               kk = **dcp;
               nbs = reinterpret_cast< XYZPosition* >( *dcp + 1 );
               pnbp = pnb;
               for( jj = 0; jj < kk; jj++, nbs++, pnbp++ ) {
                  *pnbp = *nbs;
               }
               tnbp = *( dcp - nx - 2 );
               if( tnbp == NULL ) {
                  pnbp->x = 0;
                  pnbp->y = -1;
                  pnbp->z = 0;
                  kk++;
                  pnbp++;
               } else {
                  nbs = reinterpret_cast< XYZPosition* >( tnbp + 1 );
                  for( jj = *tnbp; --jj >= 0; pnbp++, nbs++ ) {
                     pnbp->x = nbs->x;
                     pnbp->y = nbs->y - 1;
                     pnbp->z = nbs->z;
                     kk++;
                  }
               }
               *dcp = nbp;
               if( kk == 0 ) {
                  *nbp++ = 0;
                  ox[ px ] *= dx;
                  oy[ px ] *= dy;
                  oz[ px ] *= dz;
               } else {
                  kk = FindNeighbors3D( pnb, &mindist, &minpos, kk, nx, ny, nz, fdnb, fsdx, fsdy, fsdz, useTrue );
                  *nbp++ = kk;
                  nbs = reinterpret_cast< XYZPosition* >( nbp );
                  for( jj = kk, pnbp = pnb; --jj >= 0; nbs++, pnbp++ ) {
                     *nbs = *pnbp;
                  }
                  nbp += kk * dim;
                  dist = fsdx[ static_cast< dip::sint >( ox[ px ] ) + nx ] +
                         fsdy[ static_cast< dip::sint >( oy[ px ] ) + ny ] +
                         fsdz[ static_cast< dip::sint >( oz[ px ] ) + nz ];
                  if( mindist < dist ) {
                     ox[ px ] = static_cast< sfloat >( pnb[ minpos ].x ) * dx;
                     oy[ px ] = static_cast< sfloat >( pnb[ minpos ].y ) * dy;
                     oz[ px ] = static_cast< sfloat >( pnb[ minpos ].z ) * dz;
                  } else {
                     ox[ px ] *= dx;
                     oy[ px ] *= dy;
                     oz[ px ] *= dz;
                  }
               }
            } else {
               ox[ px ] = 0.0;
               oy[ px ] = 0.0;
               oz[ px ] = 0.0;
            }
         }
      }
   }
}

void VDTBruteForce2D(
      sfloat* ox,
      sfloat* oy,
      UnsignedArray const& sizes,
      IntegerArray const& stride,
      FloatArray const& distance,
      bool /*border*/
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
   sfloat* fsdy = 0;
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
         if( ox[ px ] == 0.0 ) {
            if(( ox[ px - sy ] != 0.0 ) ||
               ( ox[ px - sx ] != 0.0 ) ||
               (( yy < ny - 1 ) && ( ox[ px + sy ] != 0.0 )) ||
               (( xx < nx - 1 ) && ( ox[ px + sx ] != 0.0 ))) {
               bp->x = -xx;
               bp->y = -yy;
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
         if( ox[ px ] != 0.0 ) {
            sfloat newdist = maxdist;
            bp = bord.data();
            for( dip::sint k = 0; k < nbp; k++, bp++ ) {
               sfloat dist = fsdy[ ny - yy - bp->y ] + fsdx[ nx - xx - bp->x ];
               if( dist < newdist ) {
                  newdist = dist;
                  ox[ px ] = static_cast< sfloat >( -xx - bp->x ) * dx;
                  oy[ px ] = static_cast< sfloat >( -yy - bp->y ) * dy;
               }
            }
         } else {
            ox[ px ] = 0.0;
            oy[ px ] = 0.0;
         }
      }
   }
}

void VDTBruteForce3D(
      sfloat* ox,
      sfloat* oy,
      sfloat* oz,
      UnsignedArray const& sizes,
      IntegerArray const& stride,
      FloatArray const& distance,
      bool /*border*/
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
   sfloat* fsdy = 0;
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
   sfloat* fsdz = 0;
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
         for( dip::uint ii = 0; ii < secondBuffer.size(); ii++ ) {
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
            if( ox[ px ] == 0.0 ) {
               if(( ox[ px - sz ] != 0.0 ) ||
                  ( ox[ px - sy ] != 0.0 ) ||
                  ( ox[ px - sx ] != 0.0 ) ||
                  (( zz < nz - 1 ) && ( ox[ px + sz ] != 0.0 )) ||
                  (( yy < ny - 1 ) && ( ox[ px + sy ] != 0.0 )) ||
                  (( xx < nx - 1 ) && ( ox[ px + sx ] != 0.0 ))) {
                  bp->x = -xx;
                  bp->y = -yy;
                  bp->z = -zz;
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
            if( ox[ px ] != 0.0 ) {
               sfloat newdist = maxdist;
               bp = bord.data();
               for( dip::sint k = 0; k < nbp; k++, bp++ ) {
                  sfloat dist = fsdy[ ny - yy - bp->y ] + fsdx[ nx - xx - bp->x ] + fsdz[ nz - zz - bp->z ];
                  if( dist < newdist ) {
                     newdist = dist;
                     ox[ px ] = static_cast< sfloat >( -xx - bp->x ) * dx;
                     oy[ px ] = static_cast< sfloat >( -yy - bp->y ) * dy;
                     oz[ px ] = static_cast< sfloat >( -zz - bp->z ) * dz;
                  }
               }
            } else {
               ox[ px ] = 0.0;
               oy[ px ] = 0.0;
               oz[ px ] = 0.0;
            }
         }
      }
   }
}

} // namespace

void VectorDistanceTransform(
      Image const& in,
      Image& out,
      String const& border,
      String const& method
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint dim = in.Dimensionality();
   DIP_THROW_IF(( dim > 3 ) || ( dim < 2 ), E::DIMENSIONALITY_NOT_SUPPORTED );
   UnsignedArray sizes = in.Sizes();

   bool objectBorder;
   DIP_STACK_TRACE_THIS( objectBorder = BooleanFromString( border, "object", "background" ));

   // Distances to neighboring pixels
   FloatArray dist( dim, 1 );
   if( in.HasPixelSize() ) {
      for( dip::uint ii = 0; ii < dim; ++ii ) {
         dist[ ii ] = in.PixelSize( ii ).magnitude;
      }
   }

   // Convert in to out and get data pointer of out
   Image tmpIn = in.QuickCopy(); // preserve the input data, in case &in == &out
   out.ReForge( in.Sizes(), dim, DT_SFLOAT );
   out.Fill( 0 );
   out[ 0 ].Copy( in );
   IntegerArray stride = out.Strides();
   dip::sint tensorStride = out.TensorStride();
   sfloat* data = static_cast< sfloat* >( out.Origin() );

   // Call the real guts function
   if( method == "fast" ) {
      if( dim == 2 ) {
         VDTFast2D( data, data + tensorStride,
                    sizes, stride, dist, objectBorder );
      } else {
         VDTFast3D( data, data + tensorStride, data + tensorStride + tensorStride,
                    sizes, stride, dist, objectBorder );
      }
   } else if(( method == "ties" ) || ( method == "true" )) {
      bool useTrue = method == "true";
      if( dim == 2 ) {
         VDTTies2D( data, data + tensorStride,
                    sizes, stride, dist, objectBorder, useTrue );
      } else {
         VDTTies3D( data, data + tensorStride, data + tensorStride + tensorStride,
                    sizes, stride, dist, objectBorder, useTrue );
      }
   } else if( method == "brute force" ) {
      if( dim == 2 ) {
         VDTBruteForce2D( data, data + tensorStride,
                          sizes, stride, dist, objectBorder );
      } else {
         VDTBruteForce3D( data, data + tensorStride, data + tensorStride + tensorStride,
                          sizes, stride, dist, objectBorder );
      }
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
}

} // namespace dip
