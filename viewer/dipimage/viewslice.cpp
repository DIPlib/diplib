/*
 * DIPimage 3.0
 * This MEX-file implements the `viewslice` function
 *
 * (c)2018, Wouter Caarls.
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
#include "diplib/viewer/proxy.h"
#include "diplib/viewer/slice.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 2 );
         
      dip::Image image = dml::GetImage( prhs[ 0 ] );
         
      dip::String title = "";
      if (nrhs > 1) {
         title = dml::GetString( prhs[ 1 ] );
      }
         
      dip::viewer::WindowPtr wdw = dip::viewer::SliceViewer::Create( image, title );
      dip::viewer::ProxyManager::instance()->createWindow( wdw );
      
      mxArray *rhs[2];
      rhs[0] = mxCreateString("org.diplib.viewer.Viewer");
      rhs[1] = mxCreateUninitNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
      *(long*)mxGetData( rhs[ 1 ] ) = (long)wdw.get();
      
      mexCallMATLAB(1, plhs, 2, rhs, "javaObjectEDT");
      
      mxDestroyArray(rhs[0]);
      mxDestroyArray(rhs[1]);
   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
