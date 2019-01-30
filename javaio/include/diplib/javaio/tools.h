/*
 * DIPlib 3.0
 * This file contains declarations for some basic JNI tools
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

#ifndef DIP_JAVAIO_TOOLS_H
#define DIP_JAVAIO_TOOLS_H

#include "diplib.h"

#include <jni.h>

namespace dip {

namespace javaio {

/// C++ version of JNINativeMethod, with const char* strings.
/// To avoid "ISO C++ forbids converting a string constant to ‘char*’"
typedef struct {
   const char *name; 
   const char *signature; 
   void *fnPtr;
} JNINativeMethodCPP;

/// \brief Java string lifetime management
class JavaString {
   protected:
      JNIEnv *env_;
      jstring str_;
      char const *chars_;
      
   public:
      JavaString( JNIEnv *env, jstring str ) : env_( env ), str_( str ) {
         if ( str_ ) {
            chars_ = env_->GetStringUTFChars( str_, NULL );
         }
      }
      
      ~JavaString( ) {
         if ( str_ ) {
            env_->ReleaseStringUTFChars( str_, chars_ );
         }
      }

      operator String( ) {
         if ( str_ ) {
            return String( chars_ );
         } else {
            return String();
         }
      }
      
      String str() {
         return *this;
      }
};

/// \brief Convert java.lang.String to dip::String
JavaString StringFromJava( JNIEnv *env, jstring str );

/// \brief Convert dip::String to java.lang.String
jstring StringToJava( JNIEnv *env, String const &str );

/// \brief Convert java.lang.String[] to dip::StringArray
StringArray StringArrayFromJava( JNIEnv *env, jobjectArray jarr );

/// \brief Convert dip::StringArray to java.lang.String[]
jobjectArray StringArrayToJava( JNIEnv *env, StringArray const &arr );

/// \brief Convert long[] to dip::IntegerArray
IntegerArray IntegerArrayFromJava( JNIEnv *env, jlongArray jarr );

/// \brief Convert dip::IntegerArray to long[]
jlongArray IntegerArrayToJava( JNIEnv *env, IntegerArray const &arr );

/// \brief Convert long[] to dip::UnsignedArray
UnsignedArray UnsignedArrayFromJava( JNIEnv *env, jlongArray jarr );

/// \brief Convert dip::UnsignedArray to long[]
jlongArray UnsignedArrayToJava( JNIEnv *env, UnsignedArray const &arr );

} // namespace javaio

} // namespace dip

#endif // DIP_JAVAIO_TOOLS_H
