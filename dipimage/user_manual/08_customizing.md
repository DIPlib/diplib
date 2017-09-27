Customizing the *DIPimage* Environment
======================================

**NOTE: This section needs updating, things have changed since this was written**

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

**NOTE: The GUI is not yet ported, but will work quite differently than described here**

The *DIPimage* toolbox contains a GUI with a menu system for easy calling
of toolbox functions. It is not necessary to use this GUI, but it is the
easy way of finding the functions defined in the toolbox (see
[@sec:function_dipimage]).

All functions that appear on the menus are in the toolbox directory or
on the *DIPimage* path. If you want to add any functions to this menu
system, read [@sec:function_adding]. If you want your function to
appear in a specific place in the menu system, you will have to create a
function called `localdipmenus`. It gives you the opportunity to edit
the cell array `menulist` created by `dipmenus`, which specifies in
which menu each function should be placed. It also allows you to provide
a list of functions not to be put on the menus at all.

The cell array `menulist` has two columns. The left column gives the
names of the menus, the right column contains cell arrays with the
function names and menu names that are to be put under each menu. Any
function not mentioned in this array will be put at the bottom of the
menu specified by the function itself, in alphabetical order. See the
code for `dipmenus` to see how it is defined.

The list of functions to be excluded overrides the `menulist`. Any
function in this list will not be queried when generating the menu
system.

This is an example for a `localdipmenus` function.
It adds a menu to the `menulist`, and puts all AVI-related functions on
the `exclude` list. Note the string `'-'` that inserts a separator in
the menu.

```
    function [menulist,excludelist] = localdipmenus(menulist)
        I = size(menulist,1)+1;
        menulist{I,1} = 'My Functions';
        menulist{I,2} = {'gaussf','unif','kuwahara','-','closing','opening'};
        excludelist = {'readavi','writeavi','writedisplayavi'};
```

An alternative is to edit the `dipmenus` function. We do not recommend
this because you will be required to make the same changes each time you
install a new version of *DIPimage*.

The *DIPimage* GUI will call the `dipinit` command when starting. It
initializes the working environment. See
[@sec:customizing_dipinit].

Another thing that can be customized in the GUI is whether the command
it executes should be printed to *MATLAB*'s command window. This is useful
for copying and pasting the command being executed to some script or
function. It is on by default, and can be switched off by typing
```
    dipsetpref('PutInCommandWindow','off')
```

Initialization File {#sec:customizing_dipinit}
-------------------

**NOTE: The GUI is not yet available in 3.0 (we still have work to do)**

The *DIPimage* GUI will call the `dipinit` command when starting. It
initializes the working environment, setting up figure windows and the
like. You can also call it yourself, to return the windows to their
starting positions. You can edit this file to suit your need (or you can
create a local copy, making sure that it sits on the *MATLAB* path before
the original one; this is recommended in multi-user systems). Since it
is a script, not a function, it can initialize some variables if you
like. It can also be used to position the *DIPimage* GUI to the place of
your liking:
```
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

*Default*: \[`1 0 0`\]

This specifies the color used to display the object pixels in a binary
image. Be default they are red, out of historical reasons. Some people
prefer a different color, such as \[`1 1 1`\] or \[`0 1 0`\].

### BringToFrontOnDisplay

*Value*: `'on'` or `'off'`

*Default*: `'on'`

This setting controls whether `dipshow` brings a window to the front
when displaying a new image, or updating an old one.

### ComplexMappingDisplay

*Value*: string

*Default*: `'x+iy'`

This only affects display of complex images. When using the [*Pixel
testing*]{} mode in the image display window, the pixel value can be
displayed as real and imaginary components (`'x+iy'`), or as magnitude
and phase components(`'r/phi'`).

### CurrentImageFileDir

**NOTE: This setting is not yet used**

*Value*: string

*Default*: `"`

This setting stores the directory last visited by the file selection
dialog boxes of `readim`, `readcolorim`, `readroiim` and `writeim`. It
is used by these functions to open the file selection dialog box in the
directory you last used.

### CurrentImageSaveDir

**NOTE: This setting is not yet used**

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
default. See [@sec:function_dipmapping;@sec:figure_menus].

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

Sets the direction in which 3D/4D volumes are sliced by default.
Possible values are `'xy'`, `'xz'` and `'yz'`. See
[@sec:function_dipmapping;@sec:figure_menus].

### DisplayToFigure

*Value*: `'on'` or `'off'`

*Default*: `'on'`

When this setting is `'on'`, the `display` method of the `dip_image`
object sends the image data to a figure window. When it is `'off'`,
`disp` is called instead. The display method is called when a *MATLAB*
command does not end with a semicolon. See
[@sec:dip_image_display] for more information on this behavior.

### EnableKeyboard

*Value*: `'on'` or `'off'`

*Default*: `'on'`

If you set this value to `'off'`, the keyboard will be disabled when
displaying an image. This is useful for Windows machines, on which the
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
`readim`, `readcolorim` and `readroiim` look for a file first in the
current directory, and then in each of the directories given by this
option, unless the filename already contains a path. On UNIX and Linux
systems, directories are separated by a colon (`:`), on Windows systems
by a semicolon (`;`).

### ImageSizeLimit

*Value*: integer

*Default*: `4096`

This is the maximum size of an image automatically displayed through
`display`. If any of the sizes of an image is larger, you will need to
display it manually using `dipshow`. The reason behind this behavior is
that such an image is most likely to be created accidentally, and not
meant for display anyway. For example, `a(a>10)` returns a 1D image with
all pixel values of `a` larger than 10; this is very useful, but not
interesting to look at. For a large `a` (such as a 3D image), the
display of the resulting 1D image might require a lot of memory.

### NumberOfThreads

*Value*: integer

*Default*: usually the number of cores in your system, or the value
given by the `OMP_NUM_THREAD` environment variable.

The number of threads used for computation by *DIPlib*. This does not
affect the computations performed by *MATLAB* itself.

### PutInCommandWindow

**NOTE: This setting is not yet used**

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
displayed to a figure window (see [@sec:function_diptruesize]).

### UserManualLocation

**NOTE: This setting is not yet used**

*Value*: string

*Default*: The URL needed to fetch the user manual online.

This setting stores the location of the *DIPimage* User Manual (a PDF
file). By default it points to an address online, but you can change it
to point to a local copy of the PDF file. A link on the Help menu of the
*DIPimage* GUI and on the *MATLAB* Start Button are affected by this
setting.
