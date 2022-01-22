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

#ifndef DIP_JAVAIO_PHYSICALQUANTITY_H
#define DIP_JAVAIO_PHYSICALQUANTITY_H

#include "diplib.h"

#include <jni.h>

namespace dip {

namespace javaio {

/// \brief Convert org.diplib.PhysicalQuantity to dip::PhysicalQuantity
PhysicalQuantity PhysicalQuantityFromJava( JNIEnv *env, jobject obj );

/// \brief Convert dip::PhysicalQuantity to org.diplib.PhysicalQuantity
jobject PhysicalQuantityToJava( JNIEnv *env, PhysicalQuantity const &quantity );

/// \brief Convert org.diplib.PhysicalQuantity[] to dip::PhysicalQuantityArray
PhysicalQuantityArray PhysicalQuantityArrayFromJava( JNIEnv *env, jobjectArray obj );

/// \brief Convert dip::PhysicalQuantityArray to org.diplib.PhysicalQuantity[]
jobjectArray PhysicalQuantityArrayToJava( JNIEnv *env, PhysicalQuantityArray const &arr );

} // namespace javaio

} // namespace dip

#endif // DIP_JAVAIO_PHYSICALQUANTITY_H
