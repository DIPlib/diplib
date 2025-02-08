\comment (c)2016-2020, Cris Luengo.
\comment Based on original DIPlib code: (c)1995-2014, Delft University of Technology.

\comment Licensed under the Apache License, Version 2.0 [the "License"];
\comment you may not use this file except in compliance with the License.
\comment You may obtain a copy of the License at
\comment
\comment    http://www.apache.org/licenses/LICENSE-2.0
\comment
\comment Unless required by applicable law or agreed to in writing, software
\comment distributed under the License is distributed on an "AS IS" BASIS,
\comment WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
\comment See the License for the specific language governing permissions and
\comment limitations under the License.


\page styleguide Style guide for contributors

When contributing to *DIPlib*, please follow the style and layout of other files.
Specifically:

\section style_general Programming style

- Everything should be declared within the `dip` namespace, or a sub-namespace. The
  exception is functionality that interfaces *DIPlib* with other libraries or software,
  which should be defined in their own namespaces (e.g. the `dml` namespace for the
  *DIPlib--MATLAB* interface).

- Don't put `dip::` in front of every identifier within the library code, but do always
  do so for `dip::sint` and `dip::uint`, as they might be confused with types commonly
  present in the base namespace or as preprocessor macros.

- Do explicitly state the namespace for identifiers from every other library, including
  `std::`. This makes it easier to find references to specific types or functions.

- All functions local to a translation unit must either be declared `static` or within
  an unnamed (anonymous) namespace, to prevent name space pollution and to prevent
  them from being exported out of the library.

- Prefer `using` over `typedef`.

- Use `struct` for classes without any private members.

- Do not declare more than one variable on the same line of code. That is, avoid things
  like `float *a, b = 2.5, *c = NULL`, which is confusing.

- Declare variables where they are first initialized, or as close as possible to that
  point. Uninitialized variables need some description.

\section style_names Naming conventions

- Use camel case for variable, function and class names. Variable names start with
  a lowercase letter, function and class names start with an uppercase letter. Don't
  use underscores except for in a few special cases. Private class member variables
  end in an underscore. Constants are in all uppercase letters, with underscores to
  separate words if necessary. Internal names (the ones that are not meant to be used
  directly by library users), if public, do not have documentation and clearly
  warn the user by their name (such as `dip::Image::ShiftOriginUnsafe`), or are declared
  in the `dip::detail` namespace; private internal names (declared static or inside
  an unnamed namespace) do not have any specific requirements.

- Setter member functions start with `Set`. But getter member functions do *not* start
  with `Get`. Query functions that return a boolean start with `Is`. Member functions
  that do something have a name that resembles a verb: `Forge`, `Convert`, `PermuteDimensions`.

- The exception is in classes such as `dip::DimensionArray`, which is meant to emulate
  the `std::vector` class, and therefore follows the naming convention of the C++ Standard
  Library. Also, most classes define a `swap` operator that needs to be named as in
  the C++ Standard Library to be useful.

- Use all uppercase letters for preprocessor macros. Separate words with an underscore.
  Macros always start with `DIP_`. Include guards also start with `DIP_`, and end with
  `_H`; the part in the middle is an all-uppercase version of the file name.

- File names are in all lowercase and use underscores between words. There's no need
  to shorten names to 8 characters, so don't make the names cryptic.

\section style_formatting Formatting

- All loops and conditional statements should be surrounded by braces, even if they
  are only one statement long.

- Indents are three spaces, don't use tab characters. Continuation indents are double
  regular indents, as are the indents for class definitions. Namespace scope is not
  indented.

- The keyword `const` comes after the type name it modifies: `dip::Image const& img`.

- Braces and brackets have spaces on the inside, not the outside.

- Keep each statement on its own line.

\section style_functions Function signatures

- Option parameters to high-level functions (those that should be available in interfaces
  to other languages such as *MATLAB*) should be strings or string arrays, which are easier
  to translate to scripted languages.

- Option parameters to low-level functions (those that are meant to be called only from
  C++ code) should be defined as `enum class`, these are simpler and more efficient than
  strings. Use the `DIP_DECLARE_OPTIONS` macro to turn the enumerator into a flag set.
  Declare these option types within the `dip::Option::` namespace or another sub-namespace
  if more appropriate.

- Don't use `bool` as a function parameter, prefer meaningful strings in high-level functions
  (e.g. "black"/"white"), and `enum class` with two options defined in `dip::Option::`
  namespace for low-level functions. Only private functions can deviate from this.

- Multiple return values are preferably combined in a `struct`, rather than a `std::tuple`
  or similar, as a `struct` has named members and is easier to use. Output should rarely
  be put into the function's argument list, with the exception of images
  (see \ref design_function_signatures).

- For every function that produces an output image, there should be two signatures,
  the main one with the `out` image as an input argument, and a second one, defined
  as `inline` in the header file, as follows:

        :::cpp
        inline Image Function( Image const& in, ... ) {
           Image out;
           Function( in, out, ... );
           return out;
        }

- Add default values to as many input parameters as possible in the high-level functions.
  Sort the parameters such that the more important ones (the ones that the user is most likely
  to want to set) appear first. Image input parameters always appear first, with input image
  as first parameter, and output image as last image parameter:

        :::cpp
        void Function(
           Image const& in,
           Image const& kernel,
           Image& out,
           dfloat size = 1,
           BooleanArray process = {}
        );

\section style_headers Header files

- In header files that define "modules" or parts of them (e.g. `diplib/linear.h`), always
  first include `diplib.h`. In header files that define helper classes and don't reference
  the `dip::Image` object, you can instead include only the minimal subset of header files
  (e.g. include only `diplib/library/types.h`).

- Try to pull in as few header files as possible in public header files. Include those
  header files that define any function input types (e.g. in `diplib/linear.h` we include
  `diplib/kernel.h`). Types that are output arguments can be forward-declared if that
  makes sense. For example, `dip::Kernel` has a method that creates a `dip::PixelTable`,
  which is used mostly internally. Most users of `dip::Kernel` will not use this method.
  Therefore, there is no point in pulling in `diplib/pixel_table.h` for all users of
  `diplib/kernel.h`, and we forward-declare `dip::PixelTable` instead.

- Always explicitly include all header files used in a translation unit, even hose
  that are already included by another header file. Clang-tidy is very helpful with
  pointing out missing header files.

\section documentation Documentation

- The first line of a documentation block is the brief string. You can add a `\brief`
  command to extend the brief string to a whole paragraph, but please keep it brief.
  Add additional paragraphs of documentation if needed.

- Add `\see` to link to functions in other modules, or in the same module but "far away".
  Linking to the next or previous function is not very important, but do so if it makes
  sense and there is a `\see` section anyway. Note that the documentation lists the functions
  in the same order as that they appear in the header files.

- Use `!!! literatue` to add a literature list. Put it at the end of a documentation block.

- Use `!!! note` or `!!! note "Title"` to add an info box that
  stands out slightly. It is rendered the same as the "see also" and "literature" blocks.

- Use `!!! par "Title"` for an info box that is less attention-grabbing.

- Use `!!! attention`, `!!! warning` and `!!! bug` for info boxes with three levels of attention-grabbiness.
  These are rendered with a colored background: light orange, light green, and dark green respectively.

- Don't use `!!! aside`, or any of the other boxes.
