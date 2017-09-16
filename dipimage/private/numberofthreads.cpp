/*
 * DIPimage 3.0
 * This MEX-file implements the `imagedisplay` function
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

/*
 * Interface:
 *
 * n = numberofthreads
 *    Returns the current max number of threads to be used by DIPlib.
 *
 * old_n = numberofthreads(new_n)
 *    Sets the max number of threads to be used by DIPlib, and returns the old value.
 *
 * numberofthreads('unlock')
 *    Unlocks the MEX-file, so it can be cleared from memory.
 *
 */

#undef DIP__ENABLE_DOCTEST

#include "dip_matlab_interface.h"
#include "diplib/multithreading.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   if( !mexIsLocked()) {
      mexLock(); // We don't keep any data in memory, we lock the MEX-file so that DIPlib is locked in memory
   }

   try {

      if( nrhs == 1 ) {
         if( mxIsChar( prhs[ 0 ] )) {
            dip::String action = dml::GetString( prhs[ 0 ] );
            if( action == "unlock" ) {
               mexUnlock();
            } else {
               DIP_THROW( "Illegal input" );
            }
         } else {
            dip::uint new_n = dml::GetUnsigned( prhs[ 0 ] );
            dip::uint old_n = dip::GetNumberOfThreads();
            dip::SetNumberOfThreads( new_n );
            plhs[ 0 ] = dml::GetArray( old_n );
         }
      } else {
         DML_MAX_ARGS( 0 );
         dip::uint old_n = dip::GetNumberOfThreads();
         plhs[ 0 ] = dml::GetArray( old_n );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what());
   }
}
