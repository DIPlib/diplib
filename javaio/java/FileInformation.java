/*
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

/// \brief A data structure with information about an image file.
public class FileInformation {
   String                name;              ///< File name
   String                fileType;          ///< File type (currently, "ICS" or "TIFF")
   String                dataType;          ///< Data type for all samples
   long                  significantBits;   ///< Number of bits used for each sample
   long[]                sizes;             ///< Size of image in pixels
   long                  tensorElements;    ///< Size of pixel in samples
   String                colorSpace;        ///< Color space
   PhysicalQuantity[]    pixelSize;         ///< Pixel size
   PhysicalQuantity[]    origin;            ///< Real-world location of origin pixel
   long                  numberOfImages;    ///< Number of images in the file. Only TIFF can have more than 1 here.
   String[]              history;           ///< Assorted metadata in the file, in the form of strings.
}
