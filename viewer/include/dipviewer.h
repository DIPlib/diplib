/*
 * DIPlib 3.0 viewer
 * This file contains definitions for all classes need to use the DIPviewer.
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

#ifndef DIPVIEWER_H
#define DIPVIEWER_H

#include "diplib.h"
#include "dipviewer_export.h"

// TODO: Document usage:
// ```cpp
//     #include "diplib.h"
//     #include "dipviewer.h"
//     int main() {
//        dip::Image grey( { 300, 200 }, 1, dip::DT_UINT16 );
//        dip::FillRadiusCoordinate( grey );
//        dip::Image result = dip::Dilation( grey );
//        dip::viewer::Show( grey );
//        dip::viewer::Show( result );
//        dip::Viewer::Spin( );
//        return 0;
//     }
// ```

namespace dip { namespace viewer {

/// Show an image in the slice viewer.
DIPVIEWER_EXPORT void Show( Image const& image, String const& title = "" );

/// Show a 2D RGB image.
DIPVIEWER_EXPORT void ShowSimple( Image const& image, String const& title = "" );

/// Wait until all windows are closed.
DIPVIEWER_EXPORT void Spin();

/// Process user event queue.
/// NOTE: Spin( ) must still be called before exiting.
DIPVIEWER_EXPORT void SpinOnce();

}} // namespace dip::viewer

#endif // DIPVIEWER_H
