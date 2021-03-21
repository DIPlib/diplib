/*
 * DIPlib 3.0
 * This file contains definitions for some basic JNI tools
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
#include "tools.h"

namespace dip {

namespace javaio {

String StringFromJava( JNIEnv *env, jstring jstr ) {
   if ( !jstr ) {
      return String();
   }
   
   const char *chars = env->GetStringUTFChars( jstr, NULL );
   String str = String( chars );
   env->ReleaseStringUTFChars( jstr, chars );
   
   return str;
}

jstring StringToJava( JNIEnv *env, String const &str ) {
   return env->NewStringUTF( str.c_str() );
}

StringArray StringArrayFromJava( JNIEnv *env, jobjectArray jarr ) {
   if ( !jarr ) {
      return StringArray();
   }

   StringArray arr( (dip::uint) env->GetArrayLength( jarr ) );
   
   for ( dip::uint ii=0; ii != arr.size(); ++ii ) {
      arr[ ii ] = StringFromJava( env, (jstring) env->GetObjectArrayElement( jarr, (jsize) ii ) );
   }
   
   return arr;
}

jobjectArray StringArrayToJava( JNIEnv *env, StringArray const &arr ) {
   jclass cls = env->FindClass( "java/lang/String" );
   jobjectArray jarr = env->NewObjectArray( (jsize) arr.size(), cls, StringToJava( env, "" ) );

   for ( dip::uint ii=0; ii != arr.size(); ++ii ) {
      env->SetObjectArrayElement( jarr, (jsize) ii, StringToJava( env, arr[ ii ] ) );
   }
   
   return jarr;
}

IntegerArray IntegerArrayFromJava( JNIEnv *env, jlongArray jarr ) {
   if ( !jarr ) {
      return IntegerArray();
   }

   IntegerArray arr( (dip::uint) env->GetArrayLength( jarr ) );

   env->GetLongArrayRegion( jarr, 0, (jsize) arr.size(), (jlong*) arr.data() );

   return arr;
}

jlongArray IntegerArrayToJava( JNIEnv *env, IntegerArray const &arr ) {
   jsize length = (jsize) arr.size();
   jlongArray jarr = env->NewLongArray( length );

   env->SetLongArrayRegion( jarr, 0, length, (jlong*) arr.data() );
   
   return jarr;
}

UnsignedArray UnsignedArrayFromJava( JNIEnv *env, jlongArray jarr ) {
   if ( !jarr ) {
      return UnsignedArray();
   }

   UnsignedArray arr( (dip::uint) env->GetArrayLength( jarr ) );

   env->GetLongArrayRegion( jarr, 0, (jsize) arr.size(), (jlong*) arr.data() );

   return arr;
}

jlongArray UnsignedArrayToJava( JNIEnv *env, UnsignedArray const &arr ) {
   jsize length = (jsize) arr.size();
   jlongArray jarr = env->NewLongArray( length );

   env->SetLongArrayRegion( jarr, 0, length, (jlong*) arr.data() );
   
   return jarr;
}

} // namespace javaio

} // namespace dip

