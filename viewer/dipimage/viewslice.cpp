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

constexpr char const* ViewerClassName = "org.diplib.viewer.Viewer";
constexpr char const* MexFileName = "viewslice";
constexpr char const* JarFileName = "Viewer.jar";

void JavaAddPath( bool add = true ) {
   // The code below does:
   //    path = fullfile(fileparts(which('viewslice')),'Viewer.jar')
   mxArray* fname = mxCreateString( MexFileName );
   mxArray* path1;
   mexCallMATLAB( 1, &path1, 1, &fname, "which" );
   mxArray* path2;
   mexCallMATLAB( 1, &path2, 1, &path1, "fileparts" ); // Is this easier than implementing it in C++?
   mxArray* path;
   mxArray* args[ 2 ];
   args[ 0 ] = path2;
   args[ 1 ] = mxCreateString( JarFileName );
   mexCallMATLAB( 1, &path, 2, args, "fullfile" ); // Is this easier than implementing it in C++?
   // Add the found path to the Java Path:
   mexCallMATLABWithTrap( 0, nullptr, 1, &path, add ? "javaaddpath" : "javarmpath" ); // Ignore any errors generated
   //mexPrintf( add ? "Added to the Java path\n" : "Removed from the Java path\n" );
}

bool HasViewerClass() {
   static bool hasViewerClass = false;
   if( !hasViewerClass ) {
      //mexPrintf( "Testing for the Viewer java class\n" );
      mxArray* rhs[ 2 ];
      rhs[ 0 ] = mxCreateString( ViewerClassName );
      rhs[ 1 ] = mxCreateString( "class" );
      mxArray* lhs;
      mexCallMATLAB(1, &lhs, 2, rhs, "exist");
      dip::uint result = dml::GetUnsigned( lhs );
      if( result == 8 ) {
         hasViewerClass = true;
         //mexPrintf( "   - Tested true\n" );
      }
   }
   return hasViewerClass;
}

void EnsureViewerJarIsOnPath() {
   if( !HasViewerClass() ) {
      JavaAddPath();
      if( !HasViewerClass() ) {
         JavaAddPath( false );
         mexErrMsgTxt( "Cannot load library Viewer.jar.\nPossible sources of this error:\n"
                          " - Viewer.jar is not in the expected location.\n"
                          " - Viewer.jar is not compatible with this version of MATLAB.\n"
                          " - MATLAB's JVM is disabled." );
      }
   }
}

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   EnsureViewerJarIsOnPath();

   try {
      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 2 );

      dip::Image image = dml::GetImage( prhs[ 0 ], dml::GetImageMode::SHARED_COPY );

      dip::String title;
      if( nrhs > 1 ) {
         title = dml::GetString( prhs[ 1 ] );
      }

      dip::viewer::WindowPtr wdw = dip::viewer::SliceViewer::Create( image, title );
      dip::viewer::ProxyManager::instance()->createWindow( wdw );

      mxArray* obj;
      mxArray* rhs[2];
      rhs[ 0 ] = mxCreateString( ViewerClassName );
      static_assert( sizeof( void* ) == 8, "viewslice requires a 64-bit environment" );
      rhs[ 1 ] = mxCreateUninitNumericMatrix( 1, 1, mxINT64_CLASS, mxREAL );
      *static_cast< void** >( mxGetData( rhs[ 1 ] )) = wdw.get();
      mexCallMATLAB( 1, &obj, 2, rhs, "javaObjectEDT" );

      if( nlhs > 0 ) {
         plhs[ 0 ] = obj;
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what());
   }
}
