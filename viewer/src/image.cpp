/*
 * DIPlib 3.0 viewer
 * This file contains functionality for the simple 2D RGB image viewer.
 *
 * (c)2017, Wouter Caarls
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

#include "diplib/viewer/include_gl.h"
#include "diplib/viewer/image.h"

using namespace dip;

void ImageView::rebuild()
{
  glGenTextures(1, &texture_);
  
  // Set texture
  glBindTexture( GL_TEXTURE_2D, texture_ );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  DIP_THROW_IF( !image_.HasContiguousData(), E::NO_NORMAL_STRIDE );
  DIP_THROW_IF( image_.DataType() != DT_UINT8, E::WRONG_DATA_TYPE );
  DIP_THROW_IF( image_.TensorElements() != 3, E::NTENSORELEM_DONT_MATCH );

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)image_.Size(0), (GLsizei)image_.Size(1), 0, GL_RGB, GL_UNSIGNED_BYTE, image_.Origin());
}

void ImageView::render()
{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture_);
  
  glBegin(GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex2i(0, 0);
    glTexCoord2d(1.0,0.0); glVertex2i((GLint)image_.Size(0), 0);
    glTexCoord2d(1.0,1.0); glVertex2i((GLint)image_.Size(0), (GLint)image_.Size(1));
    glTexCoord2d(0.0,1.0); glVertex2i(0, (GLint)image_.Size(1));
  glEnd();

  glDisable(GL_TEXTURE_2D);
}

void ImageViewPort::render()
{
  double aspect_window  = width()/(double)height(),
         aspect_image = (double)view()->size(0)/(double)view()->size(1);
         
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(x_, viewer()->height()-y_-height(), width(), height());
  if (aspect_image > aspect_window)
    glOrtho(0, (GLdouble)view()->size(0), (GLdouble)view()->size(0)/aspect_window, 0, -1, 1);
  else
    glOrtho(0, (GLdouble)view()->size(1)*aspect_window, (GLdouble)view()->size(1), 0, -1, 1);
  
  glMatrixMode(GL_MODELVIEW);
  
  view()->render();
}

void ImageViewer::create()
{
  setWindowTitle("");
}
