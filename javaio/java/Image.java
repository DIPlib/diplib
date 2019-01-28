/*
 * DIPlib 3.0
 * This file contains a rudimentary Java wrapper for dip::Image.
 *
 * (c)2019, Wouter Caarls
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

package org.diplib;

import java.io.File;
import java.nio.ByteBuffer;

import java.lang.NullPointerException;

/// JNI wrapper to dip::Image
public class Image {
   long ptr_ = 0;
   
   // Constructors and destructor
   
   public Image( long ptr ) {
      ptr_ = ptr;
   }
   public Image( ) {
      long[] sizes = { };
      ptr_ = Constructor( sizes, 1, "SFLOAT" );
   }
   public Image( long[] sizes ) {
      ptr_ = Constructor( sizes, 1, "SFLOAT" );
   }
   public Image( long[] sizes, long nelems ) {
      ptr_ = Constructor( sizes, nelems, "SFLOAT" );
   }
   public Image( long[] sizes, long nelems, String dt ) {
      ptr_ = Constructor( sizes, nelems, dt );
   }
   public void release() {
      if (ptr_ != 0) {
         Destructor( ptr_ );
         ptr_ = 0;
      }
   }
   public void verify() {
      if (ptr_ == 0) {
         throw new NullPointerException("org.diplib.Image not backed by valid dip::Image");
      }
   }

   // These are to avoid introspection of 'this' on the C++ side
      
   public long[] Sizes() {
      verify();
      return Sizes( ptr_ );
   }
   public void SetSizes( long[] sizes ) {
      verify();
      SetSizes( ptr_, sizes );
   }
   public long[] Strides() {
      verify();
      return Strides( ptr_ );
   }
   public void SetStrides( long[] strides ) {
      verify();
      SetStrides( ptr_, strides );
   }
   public long TensorStride() {
      verify();
      return TensorStride( ptr_ );
   }
   public void SetTensorStride( long stride ) {
      verify();
      SetTensorStride( ptr_, stride );
   }
   public long[] TensorSizes() {
      verify();
      return TensorSizes( ptr_ );
   }
   public void SetTensorSizes( long nelems ) {
      verify();
      long[] sizes = { nelems };
      SetTensorSizes( ptr_, sizes );
   }
   public void SetTensorSizes( long[] sizes ) {
      verify();
      SetTensorSizes( ptr_, sizes );
   }
   public String DataType() { 
      verify();
      return DataType( ptr_ );
   }
   public void SetDataType( String dt ) { 
      verify();
      SetDataType( ptr_, dt );
   }
   public void Forge( ) {
      verify();
      Forge( ptr_ );
   }
   public void Strip( ) {
      verify();
      Strip( ptr_ );
   }
   public ByteBuffer Origin( ) {
      verify();
      return Origin( ptr_ );
   }
   
   // Native methods
   
   protected native long[] Sizes( long ptr );
   protected native void SetSizes( long ptr, long[] sizes );
   protected native long[] Strides( long ptr );
   protected native void SetStrides( long ptr, long[] strides );
   protected native long TensorStride( long ptr );
   protected native void SetTensorStride( long ptr, long stride );
   protected native long[] TensorSizes( long ptr );
   protected native void SetTensorSizes( long ptr, long[] sizes);
   protected native String DataType( long ptr );
   protected native void SetDataType( long ptr, String dt );
   protected native void Forge( long ptr );
   protected native void Strip( long ptr );
   protected native ByteBuffer Origin( long ptr );

   protected native long Constructor( long[] sizes, long nelems, String dt );
   protected native void Destructor( long ptr );
}
