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

#include "diplib.h"
#include "tools.h"
#include "physicalquantity.h"

namespace dip {

namespace javaio {

PhysicalQuantity PhysicalQuantityFromJava( JNIEnv *env, jobject obj ) {
   if ( !obj ) {
      return PhysicalQuantity();
   }
   
   jclass cls = env->FindClass( "org/diplib/PhysicalQuantity" );
   jfieldID magnitudeID = env->GetFieldID( cls, "magnitude", "D" );
   jfieldID unitsID = env->GetFieldID( cls, "units", "Ljava/lang/String;" );
   double magnitude = env->GetDoubleField( obj, magnitudeID );
   jstring units = (jstring) env->GetObjectField( obj, unitsID );
   String unitsStr = StringFromJava( env, units );
   
   dip::Units dipunits;
   try {
     dipunits = Units( unitsStr );
   } catch (dip::Error const& e) {
     std::cout << e.what() << " while converting `" << unitsStr << "'" << std::endl;
   }

   return PhysicalQuantity( magnitude, dipunits );
}

jobject PhysicalQuantityToJava( JNIEnv *env, const PhysicalQuantity &quantity ) {
   jclass cls = env->FindClass( "org/diplib/PhysicalQuantity" );
   jmethodID constructor = env->GetMethodID( cls, "<init>", "(DLjava/lang/String;)V" );
   
   return env->NewObject( cls, constructor, quantity.magnitude, StringToJava( env, quantity.units.String().c_str() ) );
}

PhysicalQuantityArray PhysicalQuantityArrayFromJava( JNIEnv *env, jobjectArray jarr ) {
   if ( !jarr ) {
      return PhysicalQuantityArray();
   }
   
   PhysicalQuantityArray arr( (dip::uint) env->GetArrayLength( jarr ) );
   
   for ( dip::uint ii=0; ii != arr.size(); ++ii ) {
      arr[ ii ] = PhysicalQuantityFromJava( env, env->GetObjectArrayElement( jarr, (jsize) ii ) );
   }

   return arr;
}

jobjectArray PhysicalQuantityArrayToJava( JNIEnv *env, PhysicalQuantityArray const &arr ) {
   jclass cls = env->FindClass( "org/diplib/PhysicalQuantity" );
   jobjectArray jarr = env->NewObjectArray( (jsize) arr.size(), cls, PhysicalQuantityToJava( env, PhysicalQuantity() ) );

   for ( dip::uint ii=0; ii != arr.size(); ++ii ) {
      env->SetObjectArrayElement( jarr, (jsize) ii, PhysicalQuantityToJava( env, arr[ ii ] ) );
   }
   
   return jarr;
}

} // namespace javaio

} // namespace dip
