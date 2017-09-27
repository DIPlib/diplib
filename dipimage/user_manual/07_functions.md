Toolbox Functions
=================

The GUI: `dipimage` {#sec:function_dipimage}
-------------------

**NOTE: The GUI is not yet available in 3.0 (we still have work to do)**

The GUI is started with the `dipimage` command. It contains menus with
all available image-processing functions in the toolbox. After choosing
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
this file will be created (or appended to if it already exists[^71]), and
loaded in the editor. Any commands executed through the *DIPimage* GUI
will be written to this file, in the same manner as that they are echoed
to the command line. When finished, select the same menu item again (its
text will have changed to "Stop recording macro"). Typing the macro name
on the command line:
```
    macro
```
will execute all recorded commands again. It is possible to append
commands to a recorded macro by starting the recording again with the
same macro name. It is also possible to edit the macro in the editor.
However, if you edit the macro file while recording, do remember to save
your changes before executing another command through the GUI.

The `dipshow` Function {#sec:function_dipshow}
----------------------

`dipshow` shows a `dip_image` object in a figure window. It is the function
that is called when leaving the semicolon off at the end of a MATLAB
statement that returns a `dip_image` object (see [@sec:dip_image_display]).
An optional second argument indicates the
display range required, and allows more flexibility than the options in
the "Display" menu. The general form for `dipshow` is:
```
    dipshow(a,range,colmap)
```
where `range` is either a grey-value range that should be displayed, or
one of `'log'` or `'base'`. A range is a numeric array with two values:
a lower and an upper limit. The pixels with the same or a lower value
than the lower limit will be mapped to black. The pixels that are equal
or larger than the upper limit will be mapped to white. All other values
are linearly spaced in between. The strings `'lin'` and `'all'` and the
empty array are a shortcut for `[min(image),max(image)]`, and cause the
image to be stretched linearly. The string `'percentile'` is a shortcut
for `[percentile(image,5) percentile(image,95)]`, and `'angle'` and
`'orientation'` are equivalent to `[-pi,pi]` and `[-pi,pi]/2`
respectively. The default range is `[0,255]`, which is used unless a
range is given explicitly. `colmap` is a colormap. It can either be
`'grey'`, `'periodic'`, `'labels'` or an array with 3 columns such as
those returned by the *MATLAB* functions `hsv`, `cool`, `summer`, etc.
(see the help on `colormap` for more information on this).

The strings `'angle'` and `'orientation'` imply `'periodic'` if no
explicit colormap is given. This colormap maps both the maximum and
minimum value to the same color, so as to hide a jump in angle or
orientation fields. The string `'labels'` implies a range of `[0,255]`,
and produces a colormap that gives each integer value a distinct color.

The string `'log'` causes the image to be stretched logarithmically.
`'base'` is a linear stretch that fixes the value 0 to a 50% grey value.

Examples:
```
    dipshow(a,'lin',summer(256))
    dipshow(a,[0,180],'periodic')
```
If the input argument is a color image, it will be converted to RGB for
display.

The image is displayed in a figure window according to the name of the
variable that contains the image. Links can be made using the `dipfig`
function (see [@sec:function_dipfig]). If the variable name is not
registered, a new figure window is opened for the image. To overrule
this behavior, it is possible to specify a figure handle in the
parameter list of `dipshow`:
```
    dipshow(handle,image,'lin')
```

Finally, an optional argument allows you to overrule the default setting
for the `'TrueSize'` option. By adding the string `'truesize'` at the
end of the parameter list for `dipshow`, you can make sure that
`diptruesize` is actually called. The string `'notruesize'` does the
reverse.

See [@sec:dum_figurewindows] for more information on the figure
windows used by `dipshow`.

Figure Window Support: `dipmapping` {#sec:function_dipmapping}
-----------------------------------

The function `dipmapping` can be used to change the image-to-display
mapping. All menu items under the "Mappings" menu are equivalent to a
call to `dipmapping`. In a single command, you can combine one setting
for each of the four categories: range, colormap, complex-to-real
mapping, the slicing direction and the global stretching for 3D images.
```
    dipmapping(h,range,colmap,torealstr,slicingstr,globalstr)
```
changes the mapping settings for the image in the figure window with
handle `h`. It is not necessary to provide all four values, and their
order is irrelevant. `range` can be any value as described for `dipshow`
in [@sec:function_dipshow]: a two-value numeric array or a string.
`colmap` can contain any of the strings described for `dipshow`, but not
a colormap. To specify a custom colormap, use
```
    dipmapping(h,'colormap',summer(256))
```

`torealstr` can be one of `'abs'`, `'real'`, `'imag'` or `'phase'`.

`slicingstr` can be one of `'xy'`, `'xz'`, `'yz'`, `'xt'`, `'yt'` or `'zt'`.

`globalstr` can be one of `'global'` or `'nonglobal'`.

If you don't specify a figure handle, the current figure will be used.

Additionally, you can specify a slice number. This is accomplished by
adding two parameters: the string `'slice'`, and the slice number. These
must be together and in that order, but otherwise can be combined in any
way with any of the other parameters. The same is true for the
`'colormap'` parameter.

Figure Window Support: `diptruesize` {#sec:function_diptruesize}
------------------------------------

The "Sizes" menu contains some options to call `diptruesize` (see
[@sec:figure_menus]). This function causes an image to be
displayed with an aspect ratio of 1:1, each pixel occupying one screen
pixel. An argument gives the zoom factor. For example, `200` would make
the image twice as large on the screen, but with the 1-to-1 aspect
ratio:
```
    diptruesize(200)
```

`diptruesize('off')` causes the image to fill the figure window,
possibly loosing the aspect ratio. `diptruesize` accepts a figure handle
as an optional first argument. If you provide a handle, you must also
provide a zoom factor. See `help diptruesize` for other options not
available through the figure window menu.

Figure Window Support: `diptest`, `dipzoom`, *et al.* {#sec:function_diptest}
-----------------------------------------------------

As explained in [@sec:figure_menus], the the "Actions" menu contains
items corresponding to the commands `diptest`, `dipzoom`, `diplooking`,
`dippan`, `diplink`, `dipstep`, `dipanimate`, and `dipisosurface`.
See the help for each of these functions (using the `help` command)
to learn more about them.

Creating, Linking and Clearing Figure Windows: `dipfig` and `dipclf` {#sec:function_dipfig}
--------------------------------------------------------------------

The single most important thing that can be customized in the *DIPimage*
environment is the way that images are displayed to figure windows. It
is possible to link a variable name with a figure handle, such that that
variable is always displayed in that same window. If a variable is not
linked to any window, a new one will be opened to display it. The
command
```
    dipfig a
```
opens a new figure window and links it to the variable named `a`.
Whenever that variable (if it contains an image) is displayed, it will
be send to that window. If the window is closed, it will be opened again
to display the variable. It is possible to link more than one variable
to the same window, like in the next example (which uses the functional
form):
```
    h = dipfig('a')
    dipfig(h,'b')
```

Finally, there is a special variable name, `'other'`, that creates a
link for all variables not explicitly linked to a window. It is possible
to have many windows linked to this special name, and they will be used
alternately. Creating a window for `'other'` avoids the opening of new
windows for ‘unregistered' variables.

To remove the links, type
```
    dipfig -unlink
```
Unlinking only a specific variable is not implemented.

To clear all figure windows (for example at the beginning of a demo),
use the function `dipclf`. It doesn't change the position or size of any
window, but removes the images in them. `dipclf` can also be used to
clear selected windows by giving it an array with handles or a `cell`
array with names as an argument (in a `cell` array you can actually
combine numeric handles and variable names).

Toolbox Preferences: `dipsetpref` and `dipgetpref` {#sec:function_dippref}
--------------------------------------------------

All toolbox preferences are stored in memory, and are only accessible
through the `dipsetpref` and `dipgetpref` functions. They are listed in
[@sec:customizing_dippref].
```
    v = dipgetpref('name');
```
retrieves the value of the named preference. Two special forms print all
current preferences and all factory settings to the command window:
```
    dipgetpref
    dipgetpref factory
```
Setting a preference is similar:
```
    dipsetpref('name',value)
```
Furthermore, it is possible to set many preferences at once:
```
    dipsetpref('name1',value1,'name2',value2,'name3',value3,...)
```

Interactive Tools: `dipgetcoords`, `diproi`, *et al.* {#sec:function_dipcrop}
-----------------------------------------------------

These are some tools that, using an image display, allow the user to
select points or regions in an image. `dipgetcoords` returns the
coordinates of one or more points selected by clicking on an image.

`diproi` returns a mask image (ROI stands for region of interest)
created by selecting the vertices of a polygon; it can only be used with 2D images.

**NOTE: The next functions described in this sub-section are not (yet?) ported.**

`dipcrop` returns a rectangular portion of an image selected by dragging
a rectangle. `dipprofile` returns a 1D image interpolated along a path
selected by the user on the display.

`dipgetimage` retrieves the image from a display. Use it if you lost an
image but can still see it in its display.

`dipstackinspect` lets the user click on a 3D display, and shows a 1D
plot of the hidden dimension at that point. The tool will stay active
until the right mouse button is clicked over the image.

Image Processing Functions
--------------------------

The largest part of the toolbox is made out of the image processing
functions. Most of them are listed in the menu system of the GUI, and
all are listed by typing
```
    help dipimage
```
The usage of each function can be retrieved through the `help` command
or through the GUI.

**NOTE: `help dipimage` currently lists all toolbox functions, TODO: make Contents file.**

Adding Functions to the GUI {#sec:function_adding}
---------------------------

**NOTE: The GUI is not yet ported, but will work quite differently than described here**

To add a function to the GUI, it must:

-   respond in certain ways to certain inputs, so that the GUI can query
    it for parameters, and

-   be on both the *MATLAB* path and the *DIPimage* path.

The second requirement is the easiest. If you have your functions in a
directory called `/myhome/mytools/`, then this command accomplishes it:
```
    dipaddpath('/myhome/mytools')
```

The first requirement is a bit more complicated. To add this
functionality to your own function, copy the code below.
It shows a complete skeleton for a function. The `paramlist`
structure is the most complicated part of the function,
but allows the drawing of the dialog box in the
GUI.

```
    function out = func_name(varargin)
        paramlist = struct(...
            'menu','Filters',...
            'display','Percentile Filter',...
            'inparams',struct(...
                'name',{'image_in','percentile','filterSize','filterShape'},...
                'description',{'Input image','Percentile','Size of filter','Shape of filter'},...
                'type',{'image','array','array','option'},...
                'dim_check',{[],0,1,0},...
                'range_check',{'scalar',[0,100],'N+',{'rectangular', 'elliptic','diamond','parabolic'}},...
                'required',{1,0,0,0},...
                'default',{'ans',50,7,'elliptic'}...
            ),...
            'outparams',struct(...
                'name',{'image_out'},...
                'description',{'Output image'},...
                'type',{'image'},...
                'suppress',{0}...
        ));
        if nargin== 1
            s = varargin{1};
            if ischar(s) & strcmp(s,'DIP_GetParamList')
                out = paramlist;
                return
            end
        end
        out = process_image(varargin);
    end
```

The parameter structure `paramlist` contains four values:

  ------------- -----------------------------------------------------
  `menu`        Name of the menu to place the function in (string).
  `display`     Name for the function in the menu (string).
  `inparams`    Structure array with input parameters.
  `outparams`   Structure array with output parameters.
  ------------- -----------------------------------------------------

The function will be added to the end of the menu specified (in
alphabetical order). If you want to change the order of the menu items,
you will need to create a `localdipmenus` function (see
[@sec:customizing_gui]).

`paramlist.inparams` defines the input parameters, and contains the
following fields for each parameter:

  --------------- ---------------------------------------------------------
  `name`          Variable's name (string). Not used (for now).
  `description`   Description to show the user (string).
  `type`          Expected data type (string).
  `dim_check`     Expected dimensionality or size.
  `range_check`   Expected range.
  `required`      1 or 0, to specify whether the default value is useful.
  `default`       Default value to use if the parameter is not given.
  --------------- ---------------------------------------------------------

`paramlist.outparams` defines the output parameters, and contains the
following fields for each parameter:

  --------------- -------------------------------------------------------------------
  `name`          Variable's name (string), the default output variable in the GUI.
  `description`   Description to show the user (string).
  `type`          Data type (string).
  `suppress`      Suppress output? (0 or 1, optional, defaults to 0)
  --------------- -------------------------------------------------------------------

The parameter description depends on the parameter type. What each of
`dim_check`, `range_check` and `default` mean depends on the type. Also,
each parameter type produces different controls in the GUI. Recognized
types are listed below. Please examine any of the functions in the
toolbox that put themselves on the menu to learn more about this
structure.

### 'image'

An object of type `dip_image` (or `dip_image_array`). Numeric arrays are
converted to a `dip_image`. The GUI presents an edit box where you can
type any expression. Furthermore, a right-click in this edit box brings
up a list with variables of class `dip_image` defined in the base
workspace.

`dim_check` and `range_check` are used to specify the type of image
expected. `dim_check` defines the allowed image dimensionalities through
a two-element vector `[m,n]`, where `m` is the lowest dimensionality and
`n` is the highest dimensionality allowed. The expressions `0` and `[]`
map to `[0,Inf]`, meaning any dimensionality is OK. Any scalar `m` maps
to `[m,m]`, meaning only images with `m` dimensions are allowed. For
example, to limit your function to 2D and 3D images, use `[2,3]`.

`range_check` is a string or a cell array with strings that defines both
the allowed data types and the image type (scalar, color, tensor, etc.)
Allowed are any combination of `dip_image` data types (see
[@tbl:datatypes]) as well as the data type aliases defined in
[@tbl:datatypealias], and one of the following strings:
`'scalar'` (requires `isscalar` to be `true`), `'tensor'` (`istensor` is
`true`, which also allows a scalar image), `'vector'` (`isvector` is
`true`, which does not allow a scalar image), `'color'` (`iscolor` is
`true`) or `'array'` (any `dip_image` or `dip_image_array` object is
OK). If none of this set is specified, `'tensor'` is assumed. If
`range_check` is `[]`, `{'all','tensor'}` is used. There is no way to
control the length of the vector or the dimensionality of the tensor,
you will need to write code to check those sizes yourself.

`default` is a string to be evaluated in the base workspace (therefore,
you can use any expression with names of variables in the base
workspace). Typically you would use `'a'` or `'b'` as a default value,
and set `required` to 1. This way, the GUI shows the name of a variable
possibly containing an image, but at the command-line (assuming you use
automatic parsing) this default value is never used. It is also possible
to specify something like `'[1,1,1;1,1,1;1,1,1]'` as a default image (as
does the function `convolve`).

  Name                       maps to
  -------------------------- ---------------------------------
  `'any'`                    `'complex'` + `bin`
  `'complex'`                `'real'` + `scomplex, dcomplex`
  `'noncomplex'`             `'real'` + `bin`
  `'real'`                   `'float'` + `'integer'`
  `'int'` or `'integer'`     `'signed'` + `'unsigned'`
  `'float'`                  `sfloat`, `dfloat`
  `'sint'` or `'signed'`     `sint8`, `sint16`, `sint32`
  `'uint'` or `'unsigned'`   `uint8`, `uint16`, `uint32`

  : Data type aliases used in the `range_check` parameter for
  images. {#tbl:datatypealias}

### 'measurement'

An object of type `dip_measurement`. This input is treated the same as
one of type `'image'`, except that `dim_check` and `range_check` are not
used; set them to `[]` to avoid problems if these values become
significant in the future.

### 'dataset'

An object of type `dataset` (from PRTOOLS). This input is treated the
same as one of type `'image'`, except that `dim_check` and `range_check`
are not used; set them to `[]` to avoid problems if these values become
significant in the future.

### 'array'

Any *MATLAB* array. This is a complicated type because of the flexibility
when specifying array size and data type.

`dim_check` defines the allowed array sizes in one of two ways:

-   by referring to an image parameter using a positive integer scalar,
    the dimensionality of the image pointed to gives the length of the
    vector required as input here; or

-   by directly giving an array size.

The first mode is useful when the array indicates e.g. a filter size
(see `gaussf`) or a coordinate in the image (see `findlocalmax`). In
both these cases one value per image dimension is required.

The second mode allows any array size, either fixed (`[4,4]` for a 3D
transformation matrix) or flexible (`[-1,3]` for an RGB color map on any
length). The `-1` indicates that the length along that dimension is not
tested for. The empty array `[]` indicates that an empty array is
required. An empty array is not very useful, of course, except that we
allow the combination of various size specifications using a cell array:
`{[],[1,3],[4,4]}` indicates either an empty array, a 3-element vector
or a 4-by-4 matrix are allowed. It is possible to combine references to
image parameters and direct array sizes: `{[],1}` indicates either an
empty array or a vector with as many elements as dimensions are in the
first input image.

`0` is a shortcut for `[1,1]`, a scalar value. `-1` is a shortcut for
`{[],[1,-1]}`, a row vector of any length or an empty array.

When using automatic parameter parsing, if a scalar input is given it is
extended to satisfy the required array size. Also, a vector is
transposed to match the template, but two- or higher-dimensional arrays
are not. If multiple array size options are given, the first one that
matches is the one used.

`range_check` determines the valid range for the values in the array. It
must be either an array with two values (minimum and maximum valid
values), an empty array (meaning `[-Inf Inf]`), or one of a few strings
that are defined for common ranges:

-   Integer types: `'N+'` = `[1 Inf]`. `'N-'` = `[-Inf -1]`. `'N'` =
    `[0 Inf]`, `'Z'` = `[]`.

-   Real types: `'R'` = `[]`. `'R+'` = `[0 Inf`]. `'R-'` = `[-Inf 0]`.

Note that if you specify a range by two values, it is considered real.
If you require some (finite) integer range, use the type `'option'`.

If `required` is false, `default` is any array that satisfies the
requirements of `dim_check` and `range_check`. For positive `dim_check`,
provide a scalar as default value, since it is always valid.

### 'measureid'

A measurement ID in a `dip_measurement` object.

`dim_check` is a positive integer that points to a parameter of type
`'measurement'`. The GUI shows, in a drop-down list, all measurement IDs
present in the referenced object. The automatic parameter parsing makes
sure the measurement ID given by the user exists in the referenced
object.

`required` should be 0, and `dim_check` and `default` are ignored. The
default is always the first measurement in the `dip_measurement` object
(passing the empty string yields the default as well).

### 'option'

A value (numerical or string) selected from a list. The GUI presents a
drop-down list with options to choose from.

`range_check` is a `cell` array with possible options, for example:

-   `{1,2,3,4}`

-   `{'rectangular','elliptic','parabolic'}`

`required` should be 0. `default` is any one value from the list.
`dim_check` is ignored.

### 'optionarray'

A `cell` array (with numbers or strings) selected from a list. The GUI
presents an edit box with a button. Pressing the button brings up a
dialog box that allows selecting one or more items from a list.

`range_check` is as in `'option'`. `required` should be 0. `default` is
a `cell` array with values from the list, or a single value. `dim_check`
is 0 if an empty cell array is allowed as input, 1 if at least one value
is required.

### 'cellarray'

A `cell` array (with arbitrary cell content). `dim_check` and
`range_check` are ignored. `default` must be a cellarray.

### 'infile'

The name of an existing file (for input). The GUI presents an edit box
and a button that, when pressed, presents an "Open..." dialog box.

`range_check` is a string containing the mask for the file name,
`dim_check` is ignored, and `default` is a string with the default file
name.

### 'outfile'

The name of a file (for output). The GUI presents an edit box and a
button that, when pressed, presents an "Save as..." dialog box. See the
comments for `'infile'`.

### 'indir'

The GUI presents an edit box and a button that, when pressed, presents
an "Select a directory ..." dialog box. `range_check` and `dim_check`
are ignored, `default` gives the default directory.

### 'handle'

The handle of a figure window created by `dipshow`. It is possible to
enter a handle or the name of a variable (the figure to which it is
linked is used). The GUI shows a drop-down list with the titles of all
figure windows that fit the description.

`range_check` is a `cell` array with strings that specify the type of
figure window required. All figure windows that satisfy any of the
strings are valid. Examples are:

-   `{'1D','2D','3D'}` : either two- or three-dimensional displays.

-   `{'Color','Grey','Binary'}` : either color, grey-value or binary
    displays.

-   `{'1D_Color','2D_Grey'}` : either 1D color or 2D grey-value
    displays.

An empty array means that any window created by `dipshow` is acceptable.
Note that these strings are not case-sensitive. It is, however,
important that the order shown here is maintained. No window will
satisfy the string `'Binary_2D'`, for example, but `'2D_Binary'` is
valid.

`dim_check` and `default` are ignored. The default value is always `gcf`
(the current figure).

### 'string'

Any string. `dim_check` and `range_check` are ignored. `default` must be
a string.

### 'boolean'

The value 1 or 0. Also accepted are the strings `'yes'`, `'no'`,
`'true'` and `'false'`, as well as only the first character of each. The
GUI presents a drop-down box with the words "yes" and "no". The
automatic parameter parsing, however, always returns either 1 or 0.
`dim_check` and `range_check` are ignored. `default` should be any of
the accepted values.


[^71]: *MATLAB* will ask if it is OK to overwrite the file, don't worry,
    the file will not be overwritten but appended to.
