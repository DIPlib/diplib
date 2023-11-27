/*
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

#include <memory>

#include "diplib.h"
#include "diplib/viewer/export.h"

/// \file
/// \brief Declares the high-level interface to \ref dipviewer.

namespace dip {

/// \addtogroup dipviewer

/// \brief Contains all functionality for \ref dipviewer.
namespace viewer {

class SliceViewer;
class ImageViewer;

/// \brief Show an image in the slice viewer.
///
/// A new interactive \ref dip::viewer::SliceViewer window is created for the image. `title` sets the window title.
/// If both `width` and `height` are larger than zero, they specify the size of the window created.
///
/// Call \ref dip::viewer::Draw or \ref dip::viewer::Spin to enable user interaction.
///
/// Calling `dip::viewer::Show` or \ref dip::viewer::ShowSimple creates an internal \ref dip::viewer::Manager object,
/// which needs to be freed before exiting the application to prevent memory leaks. \ref dip::viewer::Spin and
/// \ref dip::viewer::CloseAll will free the internal manager object. You need to call one of these two functions
/// at an appropriate time after calling `dip::viewer::Show` or \ref dip::viewer::ShowSimple.
DIPVIEWER_EXPORT std::shared_ptr<SliceViewer> Show( Image const& image, String const& title = "", dip::uint width = 0, dip::uint height = 0 );

/// \brief Show a 2D grey-value or RGB image, of type \ref dip::DT_UINT8.
///
/// A new non-interactive \ref dip::viewer::ImageViewer window is created for the image. `title` sets the window title.
/// `width` and `height` specify the size of the window created.
///
/// If either `width` or `height` is 0, it is computed from the other value so as to preserve the image's aspect ratio.
/// If both are 0 (the default), the image is displayed in its natural size (one image pixel to one screen pixel) but
/// scaled down if otherwise the window would exceed 512 pixels along either dimension.
///
/// A scalar (grey-value) image will be replicated across three channels to form an RGB image (meaning that data
/// will be copied).
///
/// Calling \ref dip::viewer::Show or `dip::viewer::ShowSimple` creates an internal \ref dip::viewer::Manager object,
/// which needs to be freed before exiting the application to prevent memory leaks. \ref dip::viewer::Spin and
/// \ref dip::viewer::CloseAll will free the internal manager object. You need to call one of these two functions
/// at an appropriate time after calling \ref dip::viewer::Show or `dip::viewer::ShowSimple`.
DIPVIEWER_EXPORT std::shared_ptr<ImageViewer> ShowSimple( Image const& image, String const& title = "", dip::uint width = 0, dip::uint height = 0 );

/// \brief Wait until all windows are closed.
///
/// This function allows user interaction in all slice viewer windows created. Returns when all windows are closed.
/// It also frees the internal window manager object.
///
/// Calling \ref dip::viewer::Show or \ref dip::viewer::ShowSimple creates an internal \ref dip::viewer::Manager object,
/// which needs to be freed before exiting the application to prevent memory leaks. `dip::viewer::Spin` and
/// \ref dip::viewer::CloseAll will free the internal manager object. You need to call one of these two functions
/// at an appropriate time after calling \ref dip::viewer::Show or \ref dip::viewer::ShowSimple.
DIPVIEWER_EXPORT void Spin();

/// \brief Process user event queue.
///
/// This function allows user interaction in all slice viewer windows created. Returns immediately, and needs to be
/// called repeatedly for continuous interaction.
///
/// !!! warning
///     \ref dip::viewer::Spin or \ref dip::viewer::CloseAll must still be called before exiting, to prevent memory leaks.
DIPVIEWER_EXPORT void Draw();

/// \brief Close all open windows.
///
/// Closes all open windows and frees the internal window manager object.
///
/// Calling \ref dip::viewer::Show or \ref dip::viewer::ShowSimple creates an internal \ref dip::viewer::Manager object,
/// which needs to be freed before exiting the application to prevent memory leaks. \ref dip::viewer::Spin and
/// `dip::viewer::CloseAll` will free the internal manager object. You need to call one of these two functions
/// at an appropriate time after calling \ref dip::viewer::Show or \ref dip::viewer::ShowSimple.
DIPVIEWER_EXPORT void CloseAll();

/// \endgroup

}} // namespace dip::viewer

#endif // DIPVIEWER_H
