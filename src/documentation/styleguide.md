# Style guide for contributors {#styleguide}

When contributing to *DIPlib*, please follow the style and layout of other files.
Specifically:

- Use camel case for variable, function and class names. Variable names start with
  a lowercase letter, function and class names start with an uppercase letter. Don't
  use underscores except for in a few special cases. Private class member variables
  end in an underscore. Enumerator constants are in all uppercase letters.

- Setter member functions start with `Set`. But getter member functions do not start
  with `Get`. Query functions that return a boolean start with `Is`. Member functions
  that do something have a name that resembles a verb: `Forge`, `Convert`, `PermuteDimensions`.

- The exception is in classes such as `dip::DimensionArray`, which is meant to emulate
  the `std::vector` class, and therefore follows the naming convention of the STL. Also,
  most classes define a `swap` operator that needs to be named as in the STL to be
  useful.

- Use all uppercase letters for preprocessor macros. Separate words with an underscore.
  Macros always start with `DIP_`, or `DIP__` if it is an internal macro not meant to
  be used outside the scope of the file in which it is defined. Include guards also
  start with `DIP_`, and end with `_H`; the part in the middle is an all-uppercase
  version of the file name.

- File names are in all lowercase and use underscores between words. There's no need
  to shorten names to 8 characters, so don't make the names cryptic.

- Everything should be declared within the `dip` namespace, or a sub-namespace. The
  exception is functionality that interfaces *DIPlib* with other libraries or software,
  which should be defined in their own namespaces (e.g. the `dml` namespace for the
  *DIPlib*--*MATLAB* interface).

- Prefer using `using` over `typedef`.

- Use `struct` for classes without any private members.

- All loops and conditional statements should be surrounded by braces, even if they
  are only one statement long.

- Indents are three spaces, don't use tab characters. Continuation indents are double
  regular indents, as are the indents for class definitions. Namespace scope is not
  indented.

- The keyword `const` comes after the type name it modifies: `dip::Image const& img`.

- Braces and brackets have spaces on the inside, not the outside.

- All functions local to a translation unit must be declared `static` to prevent
  name space pollution. This also prevents them for being exported out of the library.
