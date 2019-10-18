# Figure Windows {#sec_dum_figurewindows}

[//]: # (DIPlib 3.0)

[//]: # ([c]2017-2019, Cris Luengo.)
[//]: # (Based on original DIPimage usre manual: [c]1999-2014, Delft University of Technology.)

[//]: # (Licensed under the Apache License, Version 2.0 [the "License"];)
[//]: # (you may not use this file except in compliance with the License.)
[//]: # (You may obtain a copy of the License at)
[//]: # ()
[//]: # (   http://www.apache.org/licenses/LICENSE-2.0)
[//]: # ()
[//]: # (Unless required by applicable law or agreed to in writing, software)
[//]: # (distributed under the License is distributed on an "AS IS" BASIS,)
[//]: # (WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.)
[//]: # (See the License for the specific language governing permissions and)
[//]: # (limitations under the License.)

\m_footernavigation

The display is a very important part of any image-processing package.
`dip_image` objects containing scalar or color images with 1 to 4
dimensions are displayed to *MATLAB*'s figure windows. These windows are
completely cleared beforehand, meaning that images never share a window
with each other or with other graphical elements. This chapter describes
the possible interactions with figure windows, how to link variables
with them, and their placing on the desktop.

The figure windows described in this chapter are those created by the `dipshow`
function. This function is also used by default to automatically display
images when a when a *MATLAB* command does not end with a semicolon.
See the <tt>'DisplayToFigure'</tt> option in \ref sec_dum_customizing_dippref.

The alternative way to display images is with the `viewslice` function.
The figure windows created use the *DIPviewer* tool, which has better
image display capabilities (for example it can show tensor images),
but is not integrated equally well into *MATLAB*.
See \ref viewer_ui "the *DIPviewer* documentation"
for more information on these figure windows.

The <tt>'DisplayFunction'</tt> option (see \ref sec_dum_customizing_dippref)
controls which of these two commands is used to automatically display
images.

\tableofcontents

\section sec_dum_figurewindows_menus The figure window menus

The display for an image contains four menus: "File", "Sizes",
"Mappings" and "Actions".

The first menu contains a "Save display..." option that saves the
display to a file in PNG, TIFF, JPEG or EPS format.
This allows you, for example, to save an image
with labels, or to zoom into a portion of an image and only save that.
It also contains a "Close" and a "Clear" item. On Windows machines,
there is a "Copy display" option. It does the same as "Save", but writes
the image as a bitmap to the clipboard, so that it can be pasted into
other applications.

"Sizes" contains options that call `diptruesize`, which causes the image
to be displayed with an aspect ratio of 1, and various different zoom
factors (see \ref sec_dum_functions_diptruesize). It also contains an
option that causes a the image to be stretched to fill the figure
window. The last option on this menu, "Default window size" resizes the
window to some pre-defined size (which is 256 by 256 pixels, but you can
change it using `dipsetpref`, see \ref sec_dum_functions_dippref and \ref sec_dum_customizing_dippref).

"Mappings" contains different ways of mapping the data for display.
These options correspond to calls to `dipmapping` (see \ref sec_dum_functions_dipmapping).
The first section here contains stretching modes, which correspond to the
range parameter in `dipshow` (see \ref sec_dum_functions_dipshow). One of these options
is "Manual...", which, through a dialog box, allows the user to select a
custom range. The second section, only available for grey-value images,
selects a colormap. The options in the first section will sometimes
change the selection of the colormap. If the image being displayed is
complex, this menu allows choosing the complex to real mapping performed
(magnitude, phase, real or imaginary part). For 3D and 4D images you can
select the orientation of the slices shown (X-Y, X-Z, Y-Z, X-T, Y-T,
Z-T), as well as decide whether the stretching mode selected is to be
computed on the whole volume ("Global stretch") or only on the current
slice.

The "Actions" menu selects the actions that can be performed through the
mouse. The options "none", "Pixel testing", "Zoom", "Looking glass" and
"Pan" (which correspond to the `diptest`, `dipzoom`, `diplooking` and
`dippan` commands) are available to all image types. The 3D/4D image
display also contains an option to "Step through slices" (`dipstep`).
See \ref sec_dum_figurewindows_mouse for more information on these modes, and
\ref sec_dum_functions_diptest for the associated commands. This menu also contains
a command to enable or disable the keyboard functionality in the window.
See \ref sec_dum_figurewindows_keyboard for more information on this.

Finally, the "Actions" menu contains some more options:

- "Link displays..." (`diplink`)
  allows the user to link a display with other displays. When zooming,
  panning, stepping through the slices, or changing the orientation of
  the slicing, the images in the other displays will be kept in sync.
  This can be used to easily compare various images.

- "Animate" (`dipanimate`) will step through all slices of a 3D/4D image
  in sequence. Calling this function from the command line allows the
  user to choose the speed of this animation.

- "Isosurface plot..." (`dipisosurface`) opens a new window, showing
  an isosurface plot of the image. This window contains some controls
  to modify the surface. You should be aware that it takes a while to
  generate an isosurface. It is recommended to smooth and down-sample
  an image before generating an isosurface plot. The isosurface plot
  is only available for 3D displays.

- "View5d" (not yet ported over from *DIPimage 2*)

\section sec_dum_figurewindows_programmatic_updating Programmatic updating of figure windows

The function `dipshow` (\ref sec_dum_functions_dipshow) displays an image to
a figure window. It accepts multiple optional arguments to specify
how the image is to be shown. Once displayed, it is possible to
further change these properties using the menus, as described above,
but also using additional commands:

- `dipmapping` (\ref sec_dum_functions_dipmapping) changes how pixel values
  are mapped to screen colors (stretching modes, color maps, slicing
  directions, etc.)

- `diptruesize` (\ref sec_dum_functions_diptruesize) changes the spatial
  scaling of the image (zoom factor).

\section sec_dum_figurewindows_mouse Using the mouse in figure windows

As discussed above, the "Actions" menu allows selecting a mode for the
mouse to work in. Depending on the dimensionality and type of the image,
the modes are (the commands between brackets can also be used to turn
these modes on and off, see \ref sec_dum_functions_diptest):

- "None": The mouse does nothing.

- "Pixel testing" (`diptest`): The mouse is used to examine pixel
  values and location. Depressing the left mouse button will cause
  the current cursor position to be displayed in the title bar, together
  with the grey-value (or color values) of the pixel at that location. It
  is possible to move the mouse while holding down the button. Depressing
  the right mouse button does the same thing, but the cursor position
  becomes the origin of the coordinate system. This allows for length
  measurements in images.

- "Zoom" (`dipzoom`): The mouse is used to zoom the image in and out.
  Clicking with the left mouse button zooms the image in with a
  factor 2, and clicking with the right one will zoom the image out
  with a factor 2. Double-clicking any mouse button will cause the image
  to be stretched to fill the figure window. Dragging a rectangle around
  an area of interest will cause it to be zoomed-in on.

  The aspect ratio is set to 1:1 when zooming in or out, except after
  double-clicking. See \ref sec_dum_figurewindows_keyboard to learn how to zoom
  using the keyboard.

- "Looking glass" (`diplooking`): The mouse is used to enlarge a part
  of the image.

- "Pan" (`dippan`): The mouse is used to pan the image if it doesn't
  fit in the window. Just press the left mouse button and move the
  mouse with the button down. It is also possible to pan using the
  keyboard (see \ref sec_dum_figurewindows_keyboard).

- "Step through slices" (`dipstep`): The mouse is used to step through
  the slices of a 3D or 4D volume. It allows the user to click or drag the
  cursor over the image to go back and fourth through the slices that make
  up the volume. Moving the mouse down or to the right, while holding down
  the left button, displays higher slice numbers along the first hidden
  dimension. Moving the mouse up or to the left displays lower slice
  numbers. Alternatively, click with the left mouse button to go up, and
  with the right one to go down. If the displayed image is 4D, dragging
  the mouse with the right button down moves the display along the second
  hidden dimension. \ref sec_dum_figurewindows_keyboard explains how to do step
  through slices with the keyboard.

\section sec_dum_figurewindows_keyboard Using the keyboard in figure windows

When the keyboard is enabled for a display window, it can be used to
step through the slices of a 3D/4D image, zoom in and out, and pan the
image. These functions are independent of the chosen mode for the mouse
under the "Actions" menu.

The keys **N** and **P** step to the next and previous slice,
respectively, of a 3D image. Additionally, you can type the number of a
slice and press **Enter** to go to it. Note that slice numbers start
with 0. In case of a 4D image, **N** and **P** step through the first
hidden dimension (Z), whereas **F** and **B** step through the second
hidden dimension (T).

The keys **I** and **O** are used to zoom in and out, respectively. The
zoom factor is 2. When zoomed in, use the arrow keys to pan the
image and get to the area of interest. Alternatively you can use
the following keys instead: **W** for up, **S** for down,
**A** for left, and **D** for right.

The **Esc** key disables the keyboard. This is useful under Windows,
where displaying an image causes its window to gain keyboard focus. You
would have to click on the command window to continue typing a new
command. Instead, press **Esc**, which disables the keyboard for the
window and causes your keystrokes to be send to the command window. To
enable the keyboard again, use the menu item "Enable keyboard" under the
"Actions" menu. With the command

```m
    dipsetpref('EnableKeyboard','off')
```

you disable the keyboard by default, and will have to use the above
mentioned menu item to enable it. See \ref sec_dum_functions_dippref and \ref sec_dum_customizing_dippref.

\section sec_dum_figurewindows_linking Linking variables with figure windows

A variable name can be linked with the handle of a figure window, such
that any image stored in that variable will always be displayed in the
same window. This is done through the `dipfig` function (see
\ref sec_dum_functions_dipfig). It is not possible to link a single
variable with more than one figure window, but it is possible to link
many variables to the same figure window. This system allows the user to
create a series of figure windows that will be reused, instead of having
new windows created all the time. These links do not, however, promise
that an image displayed is actually up-to-date. Changing the contents of
a variable does not change the contents of a figure window. By not
adding the semicolon at the end of commands, it is possible to
automatically update the figure windows (see \ref sec_dum_dip_image_displaying).

A special name <tt>'other'</tt> is defined in `dipfig`, that is a substitute
for all variables not explicitly linked to a figure window. It allows
the user to have a window for all possible images he can create.
<tt>'other'</tt> can be linked to a series of windows, which then will be used
sequentially.

Closing a window does not destroy the links that were made for it. Since
variable names are linked to window numbers (handle), a window can be reopened to
display the image with which it is linked.

Note that many toolbox functions that require a figure window handle as
input also accept a variable name. Variable names linked with a figure
window are considered aliases for a figure window handle.

\section sec_dum_figurewindows_position Setting the position of figure windows

The position of a figure window can be changed by manipulating its
<tt>'Position'</tt> property, which is defined by an array with four values:
`left`, `bottom`, `width` and `height`.

```m
    set(handle,'Position',[left,bottom,width,heigth]);
```

The coordinates for figure windows start at the bottom-left corner of
the screen, and are in screen pixels by default. This can be changed to
`centimeters`, `inches` and other units:

```m
    set(handle,'Units','points');
```

See [*MATLAB* Function Reference](https://www.mathworks.com/help/releases/R2017a/matlab/ref/figure-properties.html)
for more information on figure window properties. You can see a local
copy of this page by typing from within MATLAB

```m
    web(fullfile(docroot,'matlab/ref/figure-properties.html'))
```

The `dipfig` function has an additional optional parameter, which can be
used to set the position of a figure window at the same time that it is
created. This parameter comes at the end of the parameter list, and is
the same array used for the <tt>'Position'</tt> property:

```m
    dipfig('a',[400,600,256,256]);
```

The `width` and `height` values are those of the image that will fit in
the window, and the window itself is drawn around this area. These
values are always in screen pixels.

If an image is larger or smaller than the size of the window, the window
will be resized so that the image fits exactly. That is, unless the
<tt>'TrueSize'</tt> option is turned off (see \ref sec_dum_customizing_dippref),
in which case the window will not be
resized, and the image will be stretched to fit. To have your windows
fixed on the desktop, disable the <tt>'TrueSize'</tt> option.

As with all other settings, the position of the figure windows cannot be
saved from one session to the next. Add the appropriate commands to your
`startup.m` or `dipinit.m` files to have the same settings across
sessions (see \ref sec_dum_customizing_dipinit).
