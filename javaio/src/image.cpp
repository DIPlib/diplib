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
#include "tools.h"
#include "image.h"
#include "physicalquantity.h"

extern "C" {

using namespace dip;
using namespace dip::javaio;

// dip::Image::Sizes()
JNIEXPORT jlongArray JNICALL Java_org_diplib_Image_Sizes( JNIEnv *env, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;
   
   return UnsignedArrayToJava( env, image->Sizes() );
}

// dip::Image::SetSizes( dip::UnsignedArray )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetSizes( JNIEnv *env, jobject, jlong ptr, jlongArray sizes) {
   Image *image = (Image*) ptr;
   
   image->SetSizes( UnsignedArrayFromJava( env, sizes ) );
}

// dip::Image::Strides()
JNIEXPORT jlongArray JNICALL Java_org_diplib_Image_Strides( JNIEnv *env, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;
   
   return IntegerArrayToJava( env, image->Strides() );
}

// dip::Image::SetStrides( dip::IntegerArray )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetStrides( JNIEnv *env, jobject, jlong ptr, jlongArray strides) {
   Image *image = (Image*) ptr;

   image->SetStrides( IntegerArrayFromJava( env, strides ) );
}

// dip::Image::TensorStride()
JNIEXPORT jlong JNICALL Java_org_diplib_Image_TensorStride( JNIEnv *, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;

   return (jlong) image->TensorStride();
}

// dip::Image::SetTensorStride( dip::sint )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetTensorStride( JNIEnv *, jobject, jlong ptr, jlong stride ) {
   Image *image = (Image*) ptr;

   image->SetTensorStride( (dip::sint) stride );
}

// dip::Image::TensorSizes()
JNIEXPORT jlongArray JNICALL Java_org_diplib_Image_TensorSizes( JNIEnv *env, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;
   
   return UnsignedArrayToJava( env, image->TensorSizes() );
}

// dip::Image::SetTensorSizes( dip::UnsignedArray )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetTensorSizes( JNIEnv *env, jobject, jlong ptr, jlongArray sizes) {
   Image *image = (Image*) ptr;
   
   image->SetTensorSizes( UnsignedArrayFromJava( env, sizes ) );
}

// dip::Image::DataType()
JNIEXPORT jstring JNICALL Java_org_diplib_Image_DataType( JNIEnv *env, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;
   
   return StringToJava( env, image->DataType().Name() );
}

// dip::Image::SetDataType( dip::String )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetDataType( JNIEnv *env, jobject, jlong ptr, jstring dt ) {
   Image *image = (Image*) ptr;

   image->SetDataType( DataType( StringFromJava( env, dt ) ) );
}

// dip::Image::ColorSpace()
JNIEXPORT jstring JNICALL Java_org_diplib_Image_ColorSpace( JNIEnv *env, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;
   
   return StringToJava( env, image->ColorSpace() );
}

// dip::Image::SetColorSpace( dip::String )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetColorSpace( JNIEnv *env, jobject, jlong ptr, jstring dt ) {
   Image *image = (Image*) ptr;

   image->SetColorSpace( StringFromJava( env, dt ) );
}

/// dip::Image::PixelSize( )
JNIEXPORT jobjectArray JNICALL Java_org_diplib_Image_PixelSize( JNIEnv *env, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;
   
   PixelSize ps = image->PixelSize();
   PhysicalQuantityArray arr( image->Dimensionality() );
   for (size_t ii=0; ii != arr.size(); ++ii) {
      arr[ ii ] = ps[ ii ];
   }
   
   return PhysicalQuantityArrayToJava( env, arr );
}

/// dip::Image::SetPixelSize( dip::PixelSize( dip::PhysicalQuantity ) )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetPixelSize( JNIEnv *env, jobject, jlong ptr, jobjectArray size ) {
   Image *image = (Image*) ptr;
   
   image->SetPixelSize( PhysicalQuantityArrayFromJava( env, size ) );
}

// dip::Image::Forge()
JNIEXPORT void JNICALL Java_org_diplib_Image_Forge( JNIEnv *, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;

   image->Forge();
}

// dip::Image::Strip()
JNIEXPORT void JNICALL Java_org_diplib_Image_Strip( JNIEnv *, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;

   image->Strip();
}

// dip::Image::Origin()
JNIEXPORT jobject JNICALL Java_org_diplib_Image_Origin( JNIEnv *env, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;
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
   jsize length = env->GetArrayLength( sizes );
   UnsignedArray szarr( (dip::uint) length );

   env->GetLongArrayRegion( sizes, 0, length, (jlong*) szarr.data() );

   return (jlong) new Image( szarr, (dip::uint) nelems, DataType( StringFromJava( env, dt ) ) );
}

// dip::Image::~Image()
JNIEXPORT void JNICALL Java_org_diplib_Image_Destructor( JNIEnv *, jobject, jlong ptr ) {
   Image *image = (Image*) ptr;

   delete image;
}

} // extern "C"

namespace dip {

namespace javaio {

static JNINativeMethodCPP image_natives_[] = {
   {"Sizes",           "(J)[J",                    (void*)Java_org_diplib_Image_Sizes },
   {"SetSizes",        "(J[J)V",                   (void*)Java_org_diplib_Image_SetSizes },
   {"Strides",         "(J)[J",                    (void*)Java_org_diplib_Image_Strides },
   {"SetStrides",      "(J[J)V",                   (void*)Java_org_diplib_Image_SetStrides },
   {"TensorStride",    "(J)J",                     (void*)Java_org_diplib_Image_TensorStride },
   {"SetTensorStride", "(JJ)V",                    (void*)Java_org_diplib_Image_SetTensorStride },
   {"TensorSizes",     "(J)[J",                    (void*)Java_org_diplib_Image_TensorSizes },
   {"SetTensorSizes",  "(J[J)V",                   (void*)Java_org_diplib_Image_SetTensorSizes },
   {"DataType",        "(J)Ljava/lang/String;",    (void*)Java_org_diplib_Image_DataType },
   {"SetDataType",     "(JLjava/lang/String;)V",   (void*)Java_org_diplib_Image_SetDataType },
   {"ColorSpace",      "(J)Ljava/lang/String;",    (void*)Java_org_diplib_Image_ColorSpace },
   {"SetColorSpace",   "(JLjava/lang/String;)V",   (void*)Java_org_diplib_Image_SetColorSpace },
   {"PixelSize",       "(J)[Lorg/diplib/PhysicalQuantity;",  (void*)Java_org_diplib_Image_PixelSize },
   {"SetPixelSize",    "(J[Lorg/diplib/PhysicalQuantity;)V", (void*)Java_org_diplib_Image_SetPixelSize },
   {"Forge",           "(J)V",                     (void*)Java_org_diplib_Image_Forge },
   {"Strip",           "(J)V",                     (void*)Java_org_diplib_Image_Strip },
   {"Origin",          "(J)Ljava/nio/ByteBuffer;", (void*)Java_org_diplib_Image_Origin },
   {"Constructor",     "([JJLjava/lang/String;)J", (void*)Java_org_diplib_Image_Constructor },
   {"Destructor",      "(J)V",                     (void*)Java_org_diplib_Image_Destructor } };

void RegisterImageNatives( JNIEnv *env ) {
   jclass cls = env->FindClass( "org/diplib/Image" );
   
   if ( env->ExceptionOccurred() ) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      DIP_THROW_RUNTIME( "Registering native functions: cannot find org.diplib.Image" );
   }   
   
   if ( env->RegisterNatives(cls, (JNINativeMethod*) image_natives_, 19) < 0 ) {
      DIP_THROW_RUNTIME( "Failed to register native functions for org.diplib.Image" );
   }   
}

} // namespace javaio

} // namespace dip

