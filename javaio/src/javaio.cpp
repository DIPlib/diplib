/*
 * (c)2019, Wouter Caarls.
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

#include <cstring>
#include <limits>

#include "diplib.h"
#include "diplib/javaio.h"
#include "diplib/file_io.h"

#include "image.h"
#include "fileinformation.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
#else
  #include <dlfcn.h>
  #include <libgen.h>
#endif

#include <jni.h>

namespace dip {

namespace javaio {

namespace {

String GetLibraryPath()
{
   char buf[ 4096 ] = { 0 };

#ifdef _WIN32
   HMODULE hm = nullptr;

   DIP_THROW_IF( !GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                     GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
	                                  ( LPCSTR ) &GetLibraryPath, &hm ),
	             "GetModuleHandleEx failed" );
   DIP_THROW_IF( !GetModuleFileName( hm, buf, sizeof( buf )),
	             "GetModuleHandleEx failed" );

   // Remove last path component (filename)
   dip::uint ii;
   for ( ii = strlen( buf ); ii > 0 && buf[ ii ] != '\\'; --ii );
   buf[ ii ] = '\0';

   String path = buf;
#else
   Dl_info dl_info;

   // ISO C++ forbids casting between pointer-to-function and pointer-to-object
   #ifdef __GNUC__
   __extension__
   #endif
   dladdr( reinterpret_cast< void const* >( GetLibraryPath ), &dl_info );

   strcpy( buf, dl_info.dli_fname );
   String path = dirname( buf );
#endif

   return path;
}

JNIEnv *GetEnv() {
   static JavaVM *jvm = nullptr;
   static JNIEnv *env = nullptr;

   if ( !jvm )
   {
      // Create JVM
      // NOTE: The JVM is not multi-threaded, so all calls must happen on this thread
      JavaVMInitArgs vm_args;
      JavaVMOption* options = new JavaVMOption[ 1 ];
      String classpathopt = "-Djava.class.path=" + GetLibraryPath() + "/DIPjavaio.jar";

      options[ 0 ].optionString = const_cast< char* >( classpathopt.c_str() );
      vm_args.version = JNI_VERSION_1_8;
      vm_args.nOptions = 1;
      vm_args.options = options;
      vm_args.ignoreUnrecognized = false;

      jint rc = JNI_CreateJavaVM( &jvm, reinterpret_cast< void** >( &env ), &vm_args );
      delete[] options;

      if ( rc != JNI_OK ) {
         jvm = nullptr;
         env = nullptr;
         DIP_THROW_RUNTIME( "Initializing JavaIO: cannot create JVM" );
      }

      RegisterImageNatives( env );
   }

   return env;
}

} // namespace

FileInformation ImageReadJavaIO(
      Image& out,
      String const& filename,
      String const& interface,
      dip::uint imageNumber
) {
   DIP_THROW_IF( imageNumber > static_cast< dip::uint >( std::numeric_limits< int >::max() ), "Image number parameter too large" );

   JNIEnv *env = GetEnv();

   jclass cls = env->FindClass( interface.c_str() );

   if ( env->ExceptionOccurred() ) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      DIP_THROW_RUNTIME( "Reading JavaIO file: cannot find interface class (is it supported?)" );
   }

   jmethodID mid = env->GetStaticMethodID( cls, "Read", "(Ljava/lang/String;IJ)Lorg/diplib/FileInformation;" );

   if ( env->ExceptionOccurred() ) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      DIP_THROW_RUNTIME( "Reading JavaIO file: cannot find interface class's Read method" );
   }

   // Call reader
   out.Strip();
   jobject obj = env->CallStaticObjectMethod( cls, mid, env->NewStringUTF( filename.c_str() ), static_cast< int >( imageNumber ), &out );

   if ( env->ExceptionOccurred() ) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      out.Strip();
      DIP_THROW_RUNTIME( "Reading JavaIO file: error calling interface class's Read method" );
   }

   return FileInformationFromJava( env, obj );
}

} // namespace javaio

} // namespace dip
