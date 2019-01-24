/*
 * DIPlib 3.0
 * This file contains definitions for a rudimentary Java dip::Image wrapper
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
#include "diplib/javaio/image.h"

extern "C" {

// dip::Image::Sizes()
JNIEXPORT jlongArray JNICALL Java_org_diplib_Image_Sizes( JNIEnv *env, jobject, jlong ptr ) {
   dip::Image *image = (dip::Image*) ptr;
   dip::UnsignedArray szarr = image->Sizes();
   jlongArray sizes = env->NewLongArray( (jsize) szarr.size() );

   env->SetLongArrayRegion( sizes, 0, (jsize) szarr.size(), (jlong*) szarr.data() );
   return sizes;
}

// dip::Image::SetSizes( dip::UnsignedArray )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetSizes( JNIEnv *env, jobject, jlong ptr, jlongArray sizes) {
   dip::Image *image = (dip::Image*) ptr;
   jsize length = env->GetArrayLength( sizes );
   dip::UnsignedArray szarr( (dip::uint) length );

   env->GetLongArrayRegion( sizes, 0, length, (jlong*) szarr.data() );
   image->SetSizes( szarr );
}

// dip::Image::Strides()
JNIEXPORT jlongArray JNICALL Java_org_diplib_Image_Strides( JNIEnv *env, jobject, jlong ptr ) {
   dip::Image *image = (dip::Image*) ptr;
   dip::IntegerArray starr = image->Strides();
   jlongArray strides = env->NewLongArray( (jsize) starr.size() );

   env->SetLongArrayRegion( strides, 0, (jsize) starr.size(), (jlong*) starr.data() );
   return strides;
}

// dip::Image::SetStrides( dip::IntegerArray )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetStrides( JNIEnv *env, jobject, jlong ptr, jlongArray strides) {
   dip::Image *image = (dip::Image*) ptr;
   jsize length = env->GetArrayLength( strides );
   dip::IntegerArray starr( (dip::uint) length );

   env->GetLongArrayRegion( strides, 0, length, (jlong*) starr.data() );
   image->SetStrides( starr );
}

// dip::Image::TensorStride()
JNIEXPORT jlong JNICALL Java_org_diplib_Image_TensorStride( JNIEnv *, jobject, jlong ptr ) {
   dip::Image *image = (dip::Image*) ptr;

   return (jlong) image->TensorStride();
}

// dip::Image::SetTensorStride( dip::sint )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetTensorStride( JNIEnv *, jobject, jlong ptr, jlong stride ) {
   dip::Image *image = (dip::Image*) ptr;

   image->SetTensorStride( (dip::sint) stride );
}

// dip::Image::TensorSizes()
JNIEXPORT jlongArray JNICALL Java_org_diplib_Image_TensorSizes( JNIEnv *env, jobject, jlong ptr ) {
   dip::Image *image = (dip::Image*) ptr;
   dip::UnsignedArray szarr = image->TensorSizes();
   jlongArray sizes = env->NewLongArray( (jsize) szarr.size() );
   
   env->SetLongArrayRegion( sizes, 0, (jsize) szarr.size(), (jlong*) szarr.data() );
   return sizes;
}

// dip::Image::SetTensorSizes( dip::UnsignedArray )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetTensorSizes( JNIEnv *env, jobject, jlong ptr, jlongArray sizes) {
   dip::Image *image = (dip::Image*) ptr;
   jsize length = env->GetArrayLength( sizes );
   dip::UnsignedArray szarr( (dip::uint) length );

   env->GetLongArrayRegion( sizes, 0, length, (jlong*) szarr.data() );
   image->SetTensorSizes( szarr );
}

// dip::Image::DataType()
JNIEXPORT jstring JNICALL Java_org_diplib_Image_DataType( JNIEnv *env, jobject, jlong ptr ) {
   dip::Image *image = (dip::Image*) ptr;
   
   return env->NewStringUTF( image->DataType().Name() );
}

// dip::Image::SetDataType( dip::String )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetDataType( JNIEnv *env, jobject, jlong ptr, jstring dt ) {
   dip::Image *image = (dip::Image*) ptr;
   const char *str = env->GetStringUTFChars( dt, NULL );

   image->SetDataType( dip::DataType( dip::String( str ) ) );
   env->ReleaseStringUTFChars( dt, str );
}

// dip::Image::Forge()
JNIEXPORT void JNICALL Java_org_diplib_Image_Forge( JNIEnv *, jobject, jlong ptr ) {
   dip::Image *image = (dip::Image*) ptr;

   image->Forge();
}

// dip::Image::Strip()
JNIEXPORT void JNICALL Java_org_diplib_Image_Strip( JNIEnv *, jobject, jlong ptr ) {
   dip::Image *image = (dip::Image*) ptr;

   image->Strip();
}

// dip::Image::Origin()
JNIEXPORT jobject JNICALL Java_org_diplib_Image_Origin( JNIEnv *env, jobject, jlong ptr ) {
   dip::Image *image = (dip::Image*) ptr;
   dip::uint n = 1;

   for( auto sz : image->Sizes() ) {
      n *= sz;
   }
   n *= image->TensorElements();
   n *= image->DataType().SizeOf();
   
   return env->NewDirectByteBuffer( image->Origin(), (long) n );
}

// dip::Image::Image( dip::UnsignedArray, dip::uint, dip::DataType )
JNIEXPORT jlong JNICALL Java_org_diplib_Image_Constructor( JNIEnv *env, jobject, jlongArray sizes, jlong nelems, jstring dt) {
   const char *str = env->GetStringUTFChars( dt, NULL );
   jsize length = env->GetArrayLength( sizes );
   dip::UnsignedArray szarr( (dip::uint) length );

   env->GetLongArrayRegion( sizes, 0, length, (jlong*) szarr.data() );
      
   dip::Image *image = new dip::Image( szarr, (dip::uint) nelems, dip::DataType( dip::String( str ) ) );

   env->ReleaseStringUTFChars( dt, str );
   
   return (jlong) image;
}

// dip::Image::~Image()
JNIEXPORT void JNICALL Java_org_diplib_Image_Destructor( JNIEnv *, jobject, jlong ptr ) {
   dip::Image *image = (dip::Image*) ptr;

   delete image;
}

} // extern "C"
