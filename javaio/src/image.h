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

#ifndef DIP_JAVAIO_IMAGE_H
#define DIP_JAVAIO_IMAGE_H

#include <jni.h>

extern "C" {

/// dip::Image::Sizes()
JNIEXPORT jlongArray JNICALL Java_org_diplib_Image_Sizes( JNIEnv *, jobject, jlong );

/// dip::Image::SetSizes( dip::UnsignedArray )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetSizes( JNIEnv *, jobject, jlong, jlongArray );

/// dip::Image::Strides()
JNIEXPORT jlongArray JNICALL Java_org_diplib_Image_Strides( JNIEnv *, jobject, jlong );

/// dip::Image::SetStrides( dip::IntegerArray )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetStrides( JNIEnv *, jobject, jlong, jlongArray );

/// dip::Image::TensorStride()
JNIEXPORT jlong JNICALL Java_org_diplib_Image_TensorStride( JNIEnv *, jobject, jlong );

/// dip::Image::SetTensorStride( dip::sint )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetTensorStride( JNIEnv *, jobject, jlong, jlong );

/// dip::Image::TensorSizes()
JNIEXPORT jlongArray JNICALL Java_org_diplib_Image_TensorSizes( JNIEnv *, jobject, jlong );

/// dip::Image::SetTensorSizes( dip::UnsignedArray )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetTensorSizes( JNIEnv *, jobject, jlong, jlongArray );

/// dip::Image::DataType()
JNIEXPORT jstring JNICALL Java_org_diplib_Image_DataType( JNIEnv *, jobject, jlong );

/// dip::Image::SetDataType( dip::String )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetDataType( JNIEnv *, jobject, jlong, jstring );

/// dip::Image::ColorSpace()
JNIEXPORT jstring JNICALL Java_org_diplib_Image_ColorSpace( JNIEnv *, jobject, jlong );

/// dip::Image::SetColorSpace( dip::String )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetColorSpace( JNIEnv *, jobject, jlong, jstring );

/// dip::Image::PixelSize()
JNIEXPORT jobjectArray JNICALL Java_org_diplib_Image_PixelSize( JNIEnv *, jobject, jlong );

/// dip::Image::SetPixelSize( dip::PixelSize( dip::PhysicalQuantityArray ) )
JNIEXPORT void JNICALL Java_org_diplib_Image_SetPixelSize( JNIEnv *, jobject, jlong, jobjectArray );

/// dip::Image::Forge()
JNIEXPORT void JNICALL Java_org_diplib_Image_Forge( JNIEnv *, jobject, jlong );

/// dip::Image::Strip()
JNIEXPORT void JNICALL Java_org_diplib_Image_Strip( JNIEnv *, jobject, jlong );

/// dip::Image::Origin()
JNIEXPORT jobject JNICALL Java_org_diplib_Image_Origin( JNIEnv *, jobject, jlong, jlong );

/// dip::Image::Image( dip::UnsignedArray, dip::uint, dip::DataType )
JNIEXPORT jlong JNICALL Java_org_diplib_Image_Constructor( JNIEnv *, jobject, jlongArray, jlong, jstring );

/// dip::Image::~Image()
JNIEXPORT void JNICALL Java_org_diplib_Image_Destructor( JNIEnv *, jobject, jlong );

} // extern "C"

namespace dip {

namespace javaio {

void RegisterImageNatives( JNIEnv *env );

} // namespace javaio

} // namespace dip

#endif // DIP_JAVAIO_IMAGE_H
