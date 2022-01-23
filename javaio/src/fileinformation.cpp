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
#include "fileinformation.h"

namespace dip {

namespace javaio {

FileInformation FileInformationFromJava( JNIEnv *env, jobject obj ) {
   if ( !obj ) {
      return FileInformation();
   }

   jclass cls = env->FindClass( "org/diplib/FileInformation" );

   jfieldID nameID            = env->GetFieldID( cls, "name", "Ljava/lang/String;" );
   jfieldID fileTypeID        = env->GetFieldID( cls, "fileType", "Ljava/lang/String;" );
   jfieldID dataTypeID        = env->GetFieldID( cls, "dataType", "Ljava/lang/String;" );
   jfieldID significantBitsID = env->GetFieldID( cls, "significantBits", "J" );
   jfieldID sizesID           = env->GetFieldID( cls, "sizes", "[J" );
   jfieldID tensorElementsID  = env->GetFieldID( cls, "tensorElements", "J" );
   jfieldID colorSpaceID      = env->GetFieldID( cls, "colorSpace", "Ljava/lang/String;" );
   jfieldID pixelSizeID       = env->GetFieldID( cls, "pixelSize", "[Lorg/diplib/PhysicalQuantity;" );
   jfieldID originID          = env->GetFieldID( cls, "origin", "[Lorg/diplib/PhysicalQuantity;" );
   jfieldID numberOfImagesID  = env->GetFieldID( cls, "numberOfImages", "J" );
   jfieldID historyID         = env->GetFieldID( cls, "history", "[Ljava/lang/String;" );

   jstring name        = (jstring) env->GetObjectField( obj, nameID );
   jstring fileType    = (jstring) env->GetObjectField( obj, fileTypeID );
   jstring dataType    = (jstring) env->GetObjectField( obj, dataTypeID );
   jlong significantBits  = env->GetLongField( obj, significantBitsID );
   jlongArray sizes       = (jlongArray) env->GetObjectField( obj, sizesID );
   jlong tensorElements   = env->GetLongField( obj, tensorElementsID );
   jstring colorSpace  = (jstring) env->GetObjectField( obj, colorSpaceID );
   jobjectArray pixelSize = (jobjectArray) env->GetObjectField( obj, pixelSizeID );
   jobjectArray origin    = (jobjectArray) env->GetObjectField( obj, originID );
   jlong numberOfImages   = env->GetLongField( obj, numberOfImagesID );
   jobjectArray history   = (jobjectArray) env->GetObjectField( obj, historyID );

   FileInformation info;
   info.name            = StringFromJava( env, name );
   info.fileType        = StringFromJava( env, fileType );
   if ( dataType )
     info.dataType      = DataType( StringFromJava( env, dataType ) );
   info.significantBits = (dip::uint) significantBits;
   info.sizes           = UnsignedArrayFromJava( env, sizes );
   info.tensorElements  = (dip::uint) tensorElements;
   info.colorSpace      = StringFromJava( env, colorSpace );
   info.pixelSize       = PixelSize( PhysicalQuantityArrayFromJava( env, pixelSize ) );
   info.origin          = PhysicalQuantityArrayFromJava( env, origin );
   info.numberOfImages  = (dip::uint) numberOfImages;
   info.history         = StringArrayFromJava( env, history );

   return info;
}

} // namespace javaio

} // namespace dip
   