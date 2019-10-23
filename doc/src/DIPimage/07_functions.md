# Toolbox Functions {#sec_dum_functions}

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

\tableofcontents

\section sec_dum_functions_dipimage The GUI: dipimage

The GUI is started with the `dipimage` command. It contains menus with
the basic image-processing functions in the toolbox. After choosing
any of these menu items, the GUI window transforms itself into a dialog
box so that you can enter the appropriate parameters. The controls that
allow entering images have a context-menu (obtained by right-clicking in
them) with the names of the images currently defined. It is possible to
enter the name of a variable containing an image or any valid *MATLAB*
statement that evaluates to image data. (The same is true for other
objects, like measurements or data-sets. Also, the window selection
control, which is a drop-down list, can be updated through its
context-menu.) Pressing the "Execute" button causes the function to be
called. There is also a button to get help on the particular function.

In the "File I/O" menu is an option "Record macro". When selected, the
user is asked for the name under which the macro will be recorded. The
extension will be ".m", indicating it is an M-file. *MATLAB* scripts are
M-files, and can be executed by typing their name on the command line.
After entering the name (let's assume we use the default "macro.m"),
this file will be created (or appended to if it already exists), and
loaded in the editor.

\par ""
*MATLAB* will ask if it is OK to overwrite the file, don't worry,
the file will not be overwritten but appended to.

Any commands executed through the *DIPimage* GUI
will be written to this file, in the same manner as that they are echoed
to the command line. When finished, select the same menu item again (its
text will have changed to "Stop recording macro"). Typing the macro name
on the command line:

```m
    macro
```

will execute all recorded commands again. It is possible to append
commands to a recorded macro by starting the recording again with the
same macro name. It is also possible to edit the macro in the editor.
However, if you edit the macro file while recording, do remember to save
your changes before executing another command through the GUI.

\section sec_dum_functions_dipshow The dipshow function

`dipshow` shows a `dip_image` object in a figure window. It is the function
that is called when leaving the semicolon off at the end of a MATLAB
statement that returns a `dip_image` object (see \ref sec_dum_dip_image_displaying).
An optional second argument indicates the
display range required, and allows more flexibility than the options in
the "Display" menu. The general form for `dipshow` is:

```m
    dipshow(a,range,colmap)
```

where `range` is either a grey-value range that should be displayed, or
one of <tt>'log'</tt> or <tt>'base'</tt>. A range is a numeric array with two values:
a lower and an upper limit. The pixels with the same or a lower value
than the lower limit will be mapped to black. The pixels that are equal
or larger than the upper limit will be mapped to white. All other values
are linearly spaced in between. The strings <tt>'lin'</tt> and <tt>'all'</tt> and the
empty array are a shortcut for `[min(image),max(image)]`, and cause the
image to be stretched linearly. The string <tt>'percentile'</tt> is a shortcut
for `[percentile(image,5) percentile(image,95)]`, and <tt>'angle'</tt> and
<tt>'orientation'</tt> are equivalent to `[-pi,pi]` and `[-pi,pi]/2`
respectively. The default range is `[0,255]`, which is used unless a
range is given explicitly. `colmap` is a colormap. It can either be
<tt>'grey'</tt>, <tt>'periodic'</tt>, <tt>'labels'</tt> or an array with 3 columns such as
those returned by the *MATLAB* functions `hsv`, `cool`, `summer`, etc.
(see the help on `colormap` for more information on this).

The strings <tt>'angle'</tt> and <tt>'orientation'</tt> imply <tt>'periodic'</tt> if no
explicit colormap is given. This colormap maps both the maximum and
minimum value to the same color, so as to hide a jump in angle or
orientation fields. The string <tt>'labels'</tt> implies a range of `[0,255]`,
and produces a colormap that gives each integer value a distinct color.

The string <tt>'log'</tt> causes the image to be stretched logarithmically.
<tt>'base'</tt> is a linear stretch that fixes the value 0 to a 50% grey value.

Examples:

```m
    dipshow(a,'lin',summer(256))
    dipshow(a,[0,180],'periodic')
```

If the input argument is a color image, it will be converted to RGB for
display.

The image is displayed in a figure window according to the name of the
variable that contains the image. Links can be made using the `dipfig`
function (see \ref sec_dum_functions_dipfig). If the variable name is not
registered, a new figure window is opened for the image. To overrule
this behavior, it is possible to specify a figure handle in the
parameter list of `dipshow`:

```m
    dipshow(handle,image,'lin')
```

Finally, an optional argument allows you to overrule the default setting
for the <tt>'TrueSize'</tt> option. By adding the string <tt>'truesize'</tt> at the
end of the parameter list for `dipshow`, you can make sure that
`diptruesize` is actually called. The string <tt>'notruesize'</tt> does the
reverse.

See \ref sec_dum_figurewindows for more information on the figure
windows used by `dipshow`.

\section sec_dum_functions_dipmapping Figure window support: dipmapping

The function `dipmapping` can be used to change the image-to-display
mapping. All menu items under the "Mappings" menu are equivalent to a
call to `dipmapping`. In a single command, you can combine one setting
for each of the four categories: range, colormap, complex-to-real
mapping, the slicing direction and the global stretching for 3D images.

```m
    dipmapping(h,range,colmap,torealstr,slicingstr,globalstr)
```

changes the mapping settings for the image in the figure window with
handle `h`. It is not necessary to provide all four values, and their
order is irrelevant. `range` can be any value as described for `dipshow`
in \ref sec_dum_functions_dipshow "": a two-value numeric array or a string.
`colmap` can contain any of the strings described for `dipshow`, but not
a colormap. To specify a custom colormap, use

```m
    dipmapping(h,'colormap',summer(256))
```

`torealstr` can be one of <tt>'abs'</tt>, <tt>'real'</tt>, <tt>'imag'</tt> or <tt>'phase'</tt>.

`slicingstr` can be one of <tt>'xy'</tt>, <tt>'xz'</tt>, <tt>'yz'</tt>, <tt>'xt'</tt>, <tt>'yt'</tt> or <tt>'zt'</tt>.

`globalstr` can be one of <tt>'global'</tt> or <tt>'nonglobal'</tt>.

If you don't specify a figure handle, the current figure will be used.

Additionally, you can specify a slice number. This is accomplished by
adding two parameters: the string <tt>'slice'</tt>, and the slice number. These
must be together and in that order, but otherwise can be combined in any
way with any of the other parameters. The same is true for the
<tt>'colormap'</tt> parameter.

Note that this function applies only to figure windows created by `dipshow`.

\section sec_dum_functions_diptruesize Figure window support: diptruesize

The "Sizes" menu contains some options to call `diptruesize` (see
\ref sec_dum_figurewindows_menus). This function causes an image to be
displayed with an aspect ratio of 1:1, each pixel occupying one screen
pixel. An argument gives the zoom factor. For example, `200` would make
the image twice as large on the screen, but with the 1-to-1 aspect
ratio:

```m
    diptruesize(200)
```

<tt>diptruesize('off')</tt> causes the image to fill the figure window,
possibly loosing the aspect ratio. `diptruesize` accepts a figure handle
as an optional first argument. If you provide a handle, you must also
provide a zoom factor. See `help diptruesize` for other options not
available through the figure window menu.

Note that this function applies only to figure windows created by `dipshow`.

\section sec_dum_functions_diptest Figure window support: diptest, dipzoom, et al.

As explained in \ref sec_dum_figurewindows_menus, the the "Actions" menu contains
items corresponding to the commands `diptest`, `dipzoom`, `diplooking`,
`dippan`, `diplink`, `dipstep`, `dipanimate`, and `dipisosurface`.
See the help for each of these functions (using the `help` command)
to learn more about them.

Note that these functions apply only to figure windows created by `dipshow`.

\section sec_dum_functions_dipfig Creating, linking and clearing figure windows: dipfig and dipclf

The single most important thing that can be customized in the *DIPimage*
environment is the way that images are displayed to figure windows. It
is possible to link a variable name with a figure handle, such that that
variable is always displayed in that same window. If a variable is not
linked to any window, a new one will be opened to display it. The
command

```m
    dipfig a
```

opens a new figure window and links it to the variable named `a`.
Whenever that variable (if it contains an image) is displayed, it will
be send to that window. If the window is closed, it will be opened again
to display the variable. It is possible to link more than one variable
to the same window, like in the next example (which uses the functional
form):

```m
    h = dipfig('a')
    dipfig(h,'b')
```

Finally, there is a special variable name, <tt>'other'</tt>, that creates a
link for all variables not explicitly linked to a window. It is possible
to have many windows linked to this special name, and they will be used
alternately. Creating a window for <tt>'other'</tt> avoids the opening of new
windows for 'unregistered' variables.

To remove the links, type

```m
    dipfig -unlink
```

Unlinking only a specific variable is not implemented.

To clear all figure windows (for example at the beginning of a demo),
use the function `dipclf`. It doesn't change the position or size of any
window, but removes the images in them. `dipclf` can also be used to
clear selected windows by giving it an array with handles or a `cell`
array with names as an argument (in a `cell` array you can actually
combine numeric handles and variable names).

Note that these functions apply only to figure windows created by `dipshow`.

\section sec_dum_functions_dippref Toolbox preferences: dipsetpref and dipgetpref

All toolbox preferences are stored in memory, and are only accessible
through the `dipsetpref` and `dipgetpref` functions. They are listed in
\ref sec_dum_customizing_dippref.

```m
    v = dipgetpref('name');
```

retrieves the value of the named preference. Two special forms print all
current preferences and all factory settings to the command window:

```m
    dipgetpref
    dipgetpref factory
```

Setting a preference is similar:

```m
    dipsetpref('name',value)
```

Furthermore, it is possible to set many preferences at once:

```m
    dipsetpref('name1',value1,'name2',value2,'name3',value3,...)
```

\section sec_dum_functions_dipcrop Interactive tools: dipgetcoords, diproi, et al.

These are some tools that, using an image display, allow the user to
select points or regions in an image. `dipgetcoords` returns the
coordinates of one or more points selected by clicking on an image.

`diproi` returns a mask image (ROI stands for region of interest)
created by selecting the vertices of a polygon; it can only be used with 2D images.

`dipcrop` returns a rectangular portion of an image selected by dragging
a rectangle. `dipprofile` returns a 1D image interpolated along a path
selected by the user on the display.

`dipgetimage` retrieves the image from a display. Use it if you lost an
image but can still see it in its display.

`dipstackinspect` lets the user click on a 3D display, and shows a 1D
plot of the hidden dimension at that point. The tool will stay active
until the right mouse button is clicked over the image.

Note that these functions apply only to figure windows created by `dipshow`.

\section sec_dum_functions_improc Image processing functions

The largest part of the toolbox is made out of the image processing
functions. Most of them are listed in the menu system of the GUI, and
all are listed by typing

```m
    help DIPimage
```

The usage of each function can be retrieved through the `help` command
or through the GUI.
