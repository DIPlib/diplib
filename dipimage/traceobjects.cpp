/*
 * DIPimage 3.0
 * This MEX-file implements the `traceobjects` function
 *
 * (c)2018, Cris Luengo.
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

#include "dip_matlab_interface.h"
#include "diplib/chain_code.h"
#include "diplib/regions.h"

inline mxArray* GetArray( dip::Polygon const& in ) {
   dip::uint n = in.vertices.size();
   if( n == 0 ) {
      return mxCreateDoubleMatrix( 0, 0, mxREAL );
   }
   dip::uint ndims = 2;
   mxArray* mx = mxCreateDoubleMatrix( n, ndims, mxREAL );
   double* data = mxGetPr( mx );
   for( auto& v : in.vertices ) {
      data[ 0 ] = v.x;
      data[ n ] = v.y;
      ++data;
   }
   return mx;
}

inline mxArray* GetArray( dip::ChainCode const& in ) {
   dip::uint n = in.codes.size();
   if( n == 0 ) {
      return mxCreateNumericMatrix( 0, 0, mxUINT8_CLASS, mxREAL );
   }
   mxArray* mx = mxCreateNumericMatrix( n, 1, mxUINT8_CLASS, mxREAL );
   dip::uint8* data = static_cast< dip::uint8* >( mxGetData( mx ));
   for( auto v : in.codes ) {
      *data = ( dip::uint8 ) v; // NOTE!!! Hard cast. `Code` has only one member variable, it's a dip::uint8.
      ++data;
   }
   return mx;
}

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 4 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      int index = 1;

      dip::UnsignedArray objectIDs;
      if( nrhs > index ) {
         objectIDs = dml::GetUnsignedArray( prhs[ index ] );
         ++index;
      }
      dip::uint connectivity = 2;
      if( nrhs > index ) {
         connectivity = dml::GetUnsigned( prhs[ index ] );
         ++index;
      }
      dip::String output = "polygon";
      if( nrhs > index ) {
         output = dml::GetString( prhs[ index ] );
      }
      bool computeConvexHull = output == "convex hull";
      bool computePolygon = computeConvexHull || ( output == "polygon" );
      if( !computePolygon && ( output != "chain code" )) {
         DIP_THROW_INVALID_FLAG( output );
      }

      dip::Image const& labels = in.DataType().IsBinary() ? dip::Label( in, connectivity ) : in;
      dip::ChainCodeArray ccs = GetImageChainCodes( labels, objectIDs, connectivity );

      dip::uint n = ccs.size();
      plhs[ 0 ] = mxCreateCellMatrix( n, 1 );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         mxArray* v = nullptr;
         if( computePolygon ) {
            dip::Polygon p = ccs[ ii ].Polygon();
            if( computeConvexHull ) {
               p = p.ConvexHull().Polygon();
            }
            v = GetArray( p );
         } else {
            v = GetArray( ccs[ ii ] );
         }
         mxSetCell( plhs[ 0 ], ii, v );
      }

   } DML_CATCH
}
