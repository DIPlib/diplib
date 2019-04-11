Customizing the *DIPimage* Environment
======================================

Figure Windows
--------------

The single most important thing that can be customized in the *DIPimage*
environment is the way that images are displayed to figure windows. It
is possible to link a variable name with a figure handle, such that that
variable is always displayed in that same window. If a variable is not
linked to any window, a new one will be opened to display it. The
command `dipfig` is used to create these links (see
[@sec:function_dipfig]).

Graphical user Interface {#sec:customizing_gui}
------------------------

The *DIPimage* toolbox contains a GUI with a menu system for easy calling
of toolbox functions. It is not necessary to use this GUI, but it is the
easy way of finding the functions defined in the toolbox (see
[@sec:function_dipimage]). The menu structure can be modified at will,
see [@sec:function_adding] for instructions.

The *DIPimage* GUI will call the `dipinit` command when starting. It
initializes the working environment. See
[@sec:customizing_dipinit].

Another thing that can be customized in the GUI is whether the command
it executes should be printed to *MATLAB*'s command window. This is useful
for copying and pasting the command being executed to some script or
function. It is on by default, and can be switched off by typing
```matlab
    dipsetpref('PutInCommandWindow','off')
```

Adding Functions to the GUI {#sec:function_adding}
---------------------------

To add a function to the GUI, it must be on the MATLAB path, and you must
make information about it available to the GUI.
The second requirement is accomplished by writing a function `localdipmenus`.
The function should be defined by iteself in a file called `localdipmenus.m` and
be somewhere on the MATLAB path. See `help addpath` to learn about the MATLAB
path. `localdipmenus` is defined as follows:

```matlab
    function [menulist,functionlist] = localdipmenus(menulist)

    menulist = [menulist;{'My Menu',{'myfunction'}}];
    I = strcmp('Restoration',menulist(:,1));
    menulist(I,:) = []; % remove the 'Restoration' menu
    I = strcmp('readtimeseries',menulist{1,2});
    menulist{1,2}(I) = []; % remove the 'readtimeseries' function from the first menu

    functionlist = containers.Map('KeyType','char','ValueType','any');
    functionlist('myfunction') = struct(...
       'display','My Function',...
       'inparams',struct('description',{'Parameter 1','Parameter 2'},...
                         'type',       {'image',      'array'},...
                         'constraint', {{'real'},     []},...
                         'default',    {'a',          3.7}),...
       'outparams',{{'Output image'}});
```

There are two things happening in this function:

1- The variable `menulist` is being edited. It defines what the menu structure
   in the GUI looks like. You can add menus, add items to menus, as well as
   remove items and menus. You can even ignore the input `menulist` and
   create one from scratch. See the function `dipmenus` for the definition of
   the default `menulist` structure. In the code above, we added a menu
   called `'My Menu'`, which contains a single command, `myfunction`. `myfunction`
   must be a function on the MATLAB path.

2- A map object is created that contains a single entry for `myfunction`. This
   map will be added to the default map defined in `dipmenus`, which provides
   information about all the DIPimage functions in the GUI menu. The structure
   assigned to the `myfunction` entry in the map is rather complex. We describe
   it in detail below.

The cell array `menulist` has two columns. The left column gives the names of
the menus, the right column contains cell arrays with the function names and menu
names that are to be put under each menu. Names in the right column that start
with `#` are menu names, and put the corresponding menu as a sub-menu at that
point. A string `'-'` inserts a menu separator at that point.
See the code for `dipmenus` to see how it is defined.

The structure that describes the input and output parameters of a function
contains three values:

  ------------- -----------------------------------------------------
  `display`     Name for the function in the menu (string).
  `inparams`    Structure array with input parameters.
  `outparams`   Structure array with output parameters.
  ------------- -----------------------------------------------------

Both `inparams` and `outparams` are optional, if not provided the function does
not accept input or output parameters.

`paramlist.outparams` is a cell array with strings that describe the output
parameters

`paramlist.inparams` is a struct array that defines the input parameters,
and contains the following fields for each parameter:

  --------------- ---------------------------------------------------------
  `description`   Description to show the user (string).
  `type`          Expected data type (string).
  `constraint`    Meaning depends on the parameter type.
  `default`       Default value to use if the parameter is not given.
  --------------- ---------------------------------------------------------

Each parameter type produces different controls in the GUI. Recognized
types are listed below. Please examine the `dipmenus` function to learn
more about this structure.

### 'image'

An object of type `dip_image`. The GUI presents an edit box where you can
type any expression. Furthermore, a right-click in this edit box brings
up a list with variables of class `dip_image` defined in the base
workspace.

`constraint` is used to specify the type of image
expected. It determine which images are shown in the right-click
menu for the control. It is a cell array containing the following optional
components:
 - A two-element vector `[m,n]` defining the allowed image dimensionalities.
   `m` is the lowest dimensionality and `n` is the highest dimensionality
   allowed. The expressions `0` and `[]` map to `[0,Inf]`, meaning any
   dimensionality is OK. Any scalar `m` maps to `[m,m]`, meaning only images
   with `m` dimensions are allowed. If not given, `[0,Inf]` is presumed.
   For example, to limit your function to 2D and 3D images, use `[2,3]`.
 - A set of strings defining the allowed image types:
   `'scalar'` (requires `isscalar` to be `true`), `'vector'` (`isvector` is
   `true`), `'color'` (`iscolor` is `true`), or `'tensor'` or `'array'`
   (any `dip_image` object is OK). `'tensor'` is the default.
 - A set of strings defining the allowed data types.
   Allowed are any combination of `dip_image` data types (see
   [@tbl:datatypes]) as well as the data type aliases defined in
   [@tbl:datatypealias]. `'all'` is the default.

Note that the numeric vector defining the dimensionality must come first
if present. The other elements are all strings, and can be presented in
any order.

`default` is a string to be evaluated in the base workspace (therefore,
you can use any expression with names of variables in the base
workspace). The toolbox typically uses letters such as `'a'` or `'b'` as
a default value for an image, under the assumption that these letters
are used to store images. But it is also possible to specify something like
`'[1,1,1;1,1,1;1,1,1]'` as a default image (as does the function `convolve`).

  Name                       maps to
  -------------------------- ---------------------------------
  `'any'`                    `'complex'` + `bin`
  `'complex'`                `'real'` + `scomplex, dcomplex`
  `'noncomplex'`             `'real'` + `bin`
  `'real'`                   `'float'` + `'integer'`
  `'int'` or `'integer'`     `'signed'` + `'unsigned'`
  `'float'`                  `sfloat`, `dfloat`
  `'sint'` or `'signed'`     `sint8`, `sint16`, `sint32`, `sint64`
  `'uint'` or `'unsigned'`   `uint8`, `uint16`, `uint32`, `uint64`

  : Data type aliases used in the `constraint` parameter for
  images. {#tbl:datatypealias}

### 'measurement'

An object of type `dip_measurement`. This input is treated the same as
one of type `'image'`, except that `constraint` is not
used; set it to `[]` to avoid problems if this value becomes
significant in the future.

### 'dataset'

An object of type `dataset` (from PRTOOLS). This input is treated the
same as one of type `'image'`, except that `constraint` is not
used; set it to `[]` to avoid problems if this value becomes
significant in the future.

### 'array'

An array of doubles. `constraint` is not used; set it to `[]` to avoid
problems if this value becomes significant in the future.

The type `anytypearray` is identical, but does not convert numeric data
to doubles, allowing numeric arrays of other types (integer, float, etc).

### 'measureid'

A measurement ID in a `dip_measurement` object.

`constraint` is a positive integer that points to a parameter of type
`'measurement'` (note that counting starts at 1). The GUI shows, in a
drop-down list, all measurement IDs present in the referenced object.

`default` is ignored. The default is always the first
measurement in the `dip_measurement` object.

### 'option'

A value (numerical or string) selected from a list. The GUI presents a
drop-down list with options to choose from.

`constraint` is a `cell` array with possible options, for example:

-   `{1,2,3,4}`

-   `{'rectangular','elliptic','parabolic'}`

`default` is any one value from the list.

### 'optionarray'

A `cell` array (with numbers or strings) selected from a list. The GUI
presents an edit box with a button. Pressing the button brings up a
dialog box that allows selecting one or more items from a list.

`constraint` is as in `'option'`. `default` is a `cell` array with values
from the list, or a single value.

### 'cellarray'

A `cell` array (with arbitrary cell content). `constraint` is
ignored. `default` must be a cellarray.

### 'infile'

The name of an existing file (for input). The GUI presents an edit box
and a button that, when pressed, presents an "Open..." dialog box.

`constraint` is a string containing the mask for the file name,
and `default` is a string with the default file name.

### 'outfile'

The name of a file (for output). The GUI presents an edit box and a
button that, when pressed, presents an "Save as..." dialog box. See the
comments for `'infile'`.

### 'indir'

The GUI presents an edit box and a button that, when pressed, presents
an "Select a directory ..." dialog box. `constraint` is ignored,
`default` gives the default directory.

### 'handle'

The handle of a figure window created by `dipshow`. The GUI shows a
drop-down list with the titles of all figure windows that fit the
description. Right-clicking the control shows a context menu with the
option to reload the control, which can be used when new windows were
created after the dialog box was displayed.

`constraint` is a `cell` array with strings that specify the type of
figure window required. All figure windows that satisfy any of the
strings are valid. Examples are:

-   `{'1D','2D','3D'}` : either one-, two- or three-dimensional displays.

-   `{'Color','Grey','Binary'}` : either color, grey-value or binary
    displays.

-   `{'1D_Color','2D_Grey'}` : either 1D color or 2D grey-value
    displays.

An empty array means that any window created by `dipshow` is acceptable.
Note that these strings are not case-sensitive. It is, however,
important that the order shown here is maintained. No window will
satisfy the string `'Binary_2D'`, for example, but `'2D_Binary'` is
valid.

`default` is ignored. The default value is always `gcf` (the current figure).

### 'string'

Any string. `constraint` is ignored. `default` must be a string.

### 'boolean'

The value `true` or `false`. The GUI presents a drop-down box with the words
"yes" and "no". `constraint` is ignored. `default` should
be a scalar value (`true`, `false`, `0` or `1`), or `yes` or `no`.

Initialization File {#sec:customizing_dipinit}
-------------------

The *DIPimage* GUI will call the `dipinit` command when starting. It
initializes the working environment, setting up figure windows and the
like. You can also call it yourself, to return the windows to their
starting positions. You can edit this file to suit your need (or you can
create a local copy, making sure that it sits on the *MATLAB* path before
the original one; this is recommended in multi-user systems). Since it
is a script, not a function, it can initialize some variables if you
like. It can also be used to position the *DIPimage* GUI to the place of
your liking:
```matlab
    set(0,'ShowHiddenHandles','on')
    h = findobj('tag','DIPimage_Main_Window');
    set(h,'Position',[500,600,500,100])
    set(0,'ShowHiddenHandles','off')
```

Other Settings {#sec:customizing_dippref}
--------------

Other settings are available through the `dipsetpref` command (see
[@sec:function_dippref]). They are listed below:

### BinaryDisplayColor

*Value*: 3x1 array of floats between 0 and 1

*Default*: `[1 0 0]`

This specifies the color used to display the object pixels in a binary
image. Be default they are red, out of historical reasons. Some people
prefer a different color, such as white (`[1 1 1]`) or green (`[0 1 0]`).

### BringToFrontOnDisplay

*Value*: `'on'` or `'off'`

*Default*: `'on'`

This setting controls whether `dipshow` brings a window to the front
when displaying a new image, or updating an old one.

### ComplexMappingDisplay

*Value*: string

*Default*: `'x+iy'`

This only affects display of complex images in `dipshow`. When using the [*Pixel
testing*]{} mode in the image display window, the pixel value can be
displayed as real and imaginary components (`'x+iy'`), or as magnitude
and phase components(`'r/phi'`).

### CurrentImageFileDir

*Value*: string

*Default*: `"`

This setting stores the directory last visited by the file selection
dialog boxes of `readim`, `readtimeseries` and `writeim`. It
is used by these functions to open the file selection dialog box in the
directory you last used.

### CurrentImageSaveDir

*Value*: string

*Default*: `"`

This setting stores the directory last visited by the file selection
dialog box of the "Save display..." option of the "File" menu of the
figure windows. It is used to open the file selection dialog box in the
directory you last used. An empty string means that the current
directory is to be used.

### DefaultActionState

*Value*: string

*Default*: `'diptest'`

This is the action mode that will be enabled by `dipshow` when
displaying an image to a new window, or to a window with a mode not
compatible with the image being displayed. Possible values are `'none'`,
`'diptest'`, `'dipzoom'` and `'dipstep'`. See
[@sec:figure_mouse].

### DefaultColorMap

*Value*: string

*Default*: `'grey'`

This is the colormap that will be used by `dipshow` when displaying an
image to a new window. Possible values are `'grey'`, `'periodic'`,
`'saturation'`, `'zerobased'` and `'labels'`. See
[@sec:function_dipmapping;@sec:figure_menus].

### DefaultComplexMapping

*Value*: string

*Default*: `'abs'`

This is the complex mapping mode that will be enabled by `dipshow` when
displaying an image to a new window, or to a window with a mode not
compatible with the image being displayed. Possible values are `'abs'`,
`'phase'`, `'real'` and `'imag'`. See [@sec:function_dipmapping;@sec:figure_menus].

### DefaultFigureHeight

*Value*: integer

*Default*: `256`

This value determines the height of a window created by `dipshow` or
`dipfig`, unless a size is explicitly given.

### DefaultFigureWidth

*Value*: integer

*Default*: `256`

This value determines the width of a window created by `dipshow` or
`dipfig`, unless a size is explicitly given.

### DefaultGlobalStretch

*Value*: `'on'` or `'off'`

*Default*: `'off'`

Set this option if you want global stretching for 3D/4D images on by
default in `dipshow`. See [@sec:function_dipmapping;@sec:figure_menus].

### DefaultMappingMode

*Value*: string

*Default*: `'normal'`

This is the mapping mode that will be enabled by `dipshow` when
displaying an image to a new window, or to a window with a mode not
compatible with the image being displayed. Possible values are `'lin'`,
`'percentile'`, `'log'`, `'base'`, `'angle'` and `'orientation'`. See
[@sec:function_dipmapping;@sec:figure_menus].

### DefaultSlicing

*Value*: string

*Default*: `'xy'`

Sets the direction in which 3D/4D volumes are sliced by default in `dipshow`.
Possible values are `'xy'`, `'xz'`, `'yz'`, `'xt'`, `'yt'` and `'zt'`.
But if you select one of the options with `t`, 3D images cannot be displayed.
See [@sec:function_dipmapping;@sec:figure_menus].

### DisplayFunction

*Value*: `'dipshow'` or `'viewslice'`

*Default*: `'dipshow'`

This option selects how images are shown to a figure window when the
command does not end with a semicolon. `dipshow` is the default
method, yielding well-integrated display windows. Many of the settings
described in this section apply only to the figure windows created by `dipshow`,
as do the interactive tools described in [@sec:function_dipcrop].
See [@sec:dum_figurewindows] for more information on these figure
windows.

`viewslice` is the alternate method. It uses the *DIPviewer* tool.
This tool has no limitations to what type of images can be shown
(i.e. it can show images with more than 4 dimensions, as well as tensor
images). However, all the `dipshow`-related options mentioned in this
section are ignored, and no further programmatic interaction with
the figure windows is supported. A new window will always be opened
for each display command, `dipfig` ([@sec:function_dipfig]) does not
apply either. See the [*DIPviewer* documention](https://diplib.github.io/diplib-docs/group__viewer.html#details)
for more information on these fi

### DisplayToFigure

*Value*: `'on'` or `'off'`

*Default*: `'on'`

When this setting is `'on'`, the `display` method of the `dip_image`
object sends the image data to a figure window. When it is `'off'`,
`disp` is called instead. The display method is called when a *MATLAB*
command does not end with a semicolon.
See [@sec:dip_image_display] for more information on this behavior.

### EnableKeyboard

*Value*: `'on'` or `'off'`

*Default*: `'on'`

If you set this value to `'off'`, the keyboard will be disabled when
displaying an image with `dipshow`. This is useful for Windows machines, on which the
figure window will get keyboard focus when displaying an image. This can
be annoying when you want to continue typing. Enable the keyboard
callback for a figure window using the appropriate menu item under
"Actions".

### FileWriteWarning

*Value*: `'on'` or `'off'`

*Default*: `'off'`

If you set this to `'on'` everything you write a non-standard TIFF image
in terms of byte depth or compression a warning will be displayed on the
screen. This is useful as many image viewers cannot read anything but
`uint8` uncompressed images (e.g. the standard Windows image TIFF
viewer).

### Gamma

**NOTE: This setting is not yet used**

*Value*: 3x1 array of floats

*Default*: `[1 1 1]`

These parameters control the display of all colour images shown by
`dipshow`. If the values are different from unity a gamma correction is
applied before displaying any image. The different values control the
behaviour for the Red, Green and Blue channel respectively.

### GammaGrey

**NOTE: This setting is not yet used**

*Value*: float

*Default*: `1`

Similar to `'Gamma'`, but only for grey-value images. This parameter
controls the display of all grey-value images shown by `dipshow`. If the
value is different from unity a gamma correction is applied before
displaying any image.

### ImageFilePath

*Value*: string

*Default*: `"`

This setting stores the path used to find image files. The functions
`readim`, `readtiff`, `readics` and `readtimeseries` look for a file first in the
current directory, and then in each of the directories given by this
option, unless the filename already contains a path. On UNIX and Linux
systems, directories are separated by a colon (`:`), on Windows systems
by a semicolon (`;`).

### ImageSizeLimit

*Value*: integer

*Default*: `4096`

This is the maximum size of an image automatically displayed through
`display`. If any of the sizes of an image is larger, you will need to
display it manually using `dipshow` or `viewslice`. The reason behind this behavior is
that such an image is most likely to be created accidentally, and not
meant for display anyway. For example, `a(a>10)` returns a 1D image with
all pixel values of `a` larger than 10; this is very useful, but not
interesting to look at. For a large `a` (such as a 3D image), the
display of the resulting 1D image might require a lot of memory.

### KeepDataType

*Value*: `'on'` or `'off'`

*Default*: `'off'`

By default, *DIPimage* performs arithmetic computations in a floating-point type
(either single or double precision depending on the types of the input). The
output image is always of the type used in the computations.

Setting this option to `'on'` causes these arithmetic operations to be more
conservative with memory usage. If one of the operands is a single-pixel image
(as is the case in `img + 1`), the type of the other image is used, except if
the single-pixel image is complex, in which case a complex output image must be produced.
Otherwise, a type is chosen that can hold values from both input types. Typically this means
the larger of the two types is chosen, but when signed and unsigned integers are
mixed, a larger type can result.

### NumberOfThreads

*Value*: integer

*Default*: usually the number of cores in your system, or the value
given by the `OMP_NUM_THREAD` environment variable.

The number of threads used for computation by *DIPlib*. This does not
affect the computations performed by *MATLAB* itself.

### PutInCommandWindow

*Value*: `'on'` or `'off'`

*Default*: `'on'`

This option causes commands that are executed from the *DIPimage* GUI to
be printed to the command window. This makes it possible to copy and
paste commands being executed to a *MATLAB* script.

### RespectVisibility

*Value*: `'on'` or `'off'`

*Default*: `'off'`

By default, `dipshow` hides a window while it prepares for displaying a
new image, then makes it visible again. This speeds up the process, and
removes flickering. Setting `'RespectVisibility'` to `'on'` the window
remains visible if it was visible (some flickering might occur), and
hidden if it was hidden.

### TrueSize

*Value*: `'on'` or `'off'`

*Default*: `'on'`

This setting controls whether `diptruesize` is called after an image is
displayed to a figure window with `dipshow` (see [@sec:function_diptruesize]).

### UserManualLocation

*Value*: string

*Default*: The location where this manual is installed.

This setting stores the location of the *DIPimage* User Manual (a PDF
file). You can change it to point to wherever you keep a local copy of the
PDF file. A link on the Help menu of the *DIPimage* GUI is affected by this
setting.
