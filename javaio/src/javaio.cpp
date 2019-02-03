/*
 * DIPlib 3.0
 * This file contains definitions for reading images using Java.
 *
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

#include "diplib.h"
#include "diplib/javaio.h"
#include "image.h"
#include "fileinformation.h"

#include <jni.h>
#include <dlfcn.h>
#include <libgen.h>

namespace dip {

namespace javaio {

namespace {

String GetLibraryPath()
{
   Dl_info dl_info;
  
   // ISO C++ forbids casting between pointer-to-function and pointer-to-object
   #ifdef __GNUC__
   __extension__
   #endif
   dladdr( (void *) GetLibraryPath, &dl_info );

   char buf[ 4096 ] = { 0 };
   strcpy( buf, dl_info.dli_fname );

   String path = dirname( buf );
  
   return path;
}

JNIEnv *GetEnv() {
   static JavaVM *jvm = NULL;
   static JNIEnv *env = NULL;
   
   if ( !jvm )
   {
     // Create JVM
     // NOTE: The JVM is not multi-threaded, so all calls must happen on this thread
     JavaVMInitArgs vm_args;
     JavaVMOption* options = new JavaVMOption[ 1 ];
     String classpathopt = "-Djava.class.path=" + GetLibraryPath() + "/DIPjavaio.jar";
   
     options[ 0 ].optionString = (char*) classpathopt.c_str();
     vm_args.version = JNI_VERSION_1_8;
     vm_args.nOptions = 1;
     vm_args.options = options;
     vm_args.ignoreUnrecognized = false;
   
     jint rc = JNI_CreateJavaVM( &jvm, (void**) &env, &vm_args );
     delete[] options;
   
     if ( rc != JNI_OK ) {
        jvm = NULL;
        env = NULL;
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
      String const& interface
) {
   JNIEnv *env = GetEnv();
   
   jclass cls = env->FindClass( interface.c_str() );
   
   if ( env->ExceptionOccurred() ) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      DIP_THROW_RUNTIME( "Reading JavaIO file: cannot find interface class (is it supported?)" );
   }   
   
   jmethodID mid = env->GetStaticMethodID( cls, "Read", "(Ljava/lang/String;J)Lorg/diplib/FileInformation;" );

   if ( env->ExceptionOccurred() ) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      DIP_THROW_RUNTIME( "Reading JavaIO file: cannot find interface class's Read method" );
   }   

   // Call reader
   out.Strip();
   jobject obj = env->CallStaticObjectMethod( cls, mid, env->NewStringUTF( filename.c_str() ), &out );

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
