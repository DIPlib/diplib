/*
 * DIPlib 3.0
 * This file contains macros to simplify overloading on image data type.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_OVERLOAD_H
#define DIP_OVERLOAD_H

/// \file
/// Help with instantiating templates for different pixel data types.
/// The preprocessor macros in this file insert a block of code that
/// calls or retrieves a function pointer to the right instance of a
/// template, according to a DataType argument. For example, the code
///
///     DIP_OVL_CALL_ALL( myFunc, ( param1, param2 ), datatype );
///
/// causes a call to `myFunc( param1, param2 )`, where `myFunc` is
/// a templated function. The template will be specialized to
/// whichever value the variable `datatype` has at run time. The
/// compiler generates instances of your template for all possible
/// values of `datatype`. If you want to restrict the allowed data
/// types, use a macro ending in other than `_ALL`. For example, the
/// macro `DIP_OVL_CALL_REAL` only allows integer and floating point
/// data types (not binary nor complex types).
///
/// A function parameter cannot determine the template type, as the
/// code calling your templated function must work with images of
/// many different data types. You can think of `param1` above being of
/// type `void*` or dip::Image, for example.
///
/// **Note** the parenthesis around the function parameters in the macro
/// call above!
///
/// There are three groups of macros defined in this file:
/// - DIP_OVL_CALL_xxx calls a function, discarding any return value.
/// - DIP_OVL_CALL_ASSIGN_xxx calls a function, assigning the return
///   value in a variable.
/// - DIP_OVL_ASSIGN_xxx assigns a function pointer to a variable, without
///   calling the function.

// Note: in some macros below, we start with assigning a random function
// pointer into `f`. This is necessary to determine the type of `f`.

#define DIP__OVL__HEAD( dtype ) \
   switch( dtype ) {

#define DIP__OVL__FOOT \
   default: dip_Throw( dip::E::DATA_TYPE_NOT_SUPPORTED ); \
}

#define DIP__OVL__BIN( fname ) \
   case dip::DT_BIN      : f = fname <dip::bin>     ; break;

#define DIP__OVL__UINT( fname ) \
   case dip::DT_UINT8    : f = fname <dip::uint8>   ; break; \
   case dip::DT_UINT16   : f = fname <dip::uint16>  ; break; \
   case dip::DT_UINT32   : f = fname <dip::uint32>  ; break;

#define DIP__OVL__SINT( fname ) \
   case dip::DT_SINT8    : f = fname <dip::sint8>   ; break; \
   case dip::DT_SINT16   : f = fname <dip::sint16>  ; break; \
   case dip::DT_SINT32   : f = fname <dip::sint32>  ; break;

#define DIP__OVL__FLOAT( fname ) \
   case dip::DT_SFLOAT   : f = fname <dip::sfloat>  ; break; \
   case dip::DT_DFLOAT   : f = fname <dip::dfloat>  ; break;

#define DIP__OVL__COMPLEX( fname ) \
   case dip::DT_SCOMPLEX : f = fname <dip::scomplex>; break; \
   case dip::DT_DCOMPLEX : f = fname <dip::dcomplex>; break;

#define DIP__OVL__INTEGER( fname ) \
   DIP__OVL__UINT( fname ) \
   DIP__OVL__SINT( fname )

#define DIP__OVL__UNSIGNED( fname ) \
   DIP__OVL__UINT( fname )

#define DIP__OVL__SIGNED( fname ) \
   DIP__OVL__SINT( fname )    \
   DIP__OVL__FLOAT( fname )   \
   DIP__OVL__COMPLEX( fname )

#define DIP__OVL__REAL( fname ) \
   DIP__OVL__UINT( fname )  \
   DIP__OVL__SINT( fname )  \
   DIP__OVL__FLOAT( fname )

#define DIP__OVL__NONCOMPLEX( fname ) \
   DIP__OVL__BIN( fname )   \
   DIP__OVL__UINT( fname )  \
   DIP__OVL__SINT( fname )  \
   DIP__OVL__FLOAT( fname )

#define DIP__OVL__NONBINARY( fname ) \
   DIP__OVL__UINT( fname )    \
   DIP__OVL__SINT( fname )    \
   DIP__OVL__FLOAT( fname )   \
   DIP__OVL__COMPLEX( fname )

#define DIP__OVL__ALL( fname ) \
   DIP__OVL__BIN( fname )     \
   DIP__OVL__UINT( fname )    \
   DIP__OVL__SINT( fname )    \
   DIP__OVL__FLOAT( fname )   \
   DIP__OVL__COMPLEX( fname )

//
// DIP_OVL_CALL_xxx
//

/// Calls the overloaded function for all unsigned integer types.
#define DIP_OVL_CALL_UINT( fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__UINT( fname )       \
   DIP__OVL__FOOT                \
   f paramlist; }

/// Calls the overloaded function for all signed integer types.
#define DIP_OVL_CALL_SINT( fname, paramlist, dtype ) \
{  auto f = fname< dip::sint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__SINT( fname )       \
   DIP__OVL__FOOT                \
   f paramlist; }

/// Calls the overloaded function for all float types.
#define DIP_OVL_CALL_FLOAT( fname, paramlist, dtype ) \
{  auto f = fname< dip::sfloat >; \
   DIP__OVL__HEAD( dtype )        \
   DIP__OVL__FLOAT( fname )       \
   DIP__OVL__FOOT                 \
   f paramlist; }

/// Calls the overloaded function for all complex types.
#define DIP_OVL_CALL_COMPLEX( fname, paramlist, dtype ) \
{  auto f = fname< dip::scomplex >; \
   DIP__OVL__HEAD( dtype )          \
   DIP__OVL__COMPLEX( fname )       \
   DIP__OVL__FOOT                   \
   f paramlist; }

/// Calls the overloaded function for all integer types.
#define DIP_OVL_CALL_INTEGER( fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__INTEGER( fname )    \
   DIP__OVL__FOOT                \
   f paramlist; }

/// Calls the overloaded function for all unsigned types.
#define DIP_OVL_CALL_UNSIGNED( fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__UNSIGNED( fname )   \
   DIP__OVL__FOOT                \
   f paramlist; }

/// Calls the overloaded function for all signed (integer + float + complex) types.
#define DIP_OVL_CALL_SIGNED( fname, paramlist, dtype ) \
{  auto f = fname< dip::sint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__SIGNED( fname )     \
   DIP__OVL__FOOT                \
   f paramlist; }

/// Calls the overloaded function for all real (integer + float) types.
#define DIP_OVL_CALL_REAL( fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__REAL( fname )       \
   DIP__OVL__FOOT                \
   f paramlist; }

/// Calls the overloaded function for all non-complex types.
#define DIP_OVL_CALL_NONCOMPLEX( fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__NONCOMPLEX( fname ) \
   DIP__OVL__FOOT                \
   f paramlist; }

/// Calls the overloaded function for all types but binary.
#define DIP_OVL_CALL_NONBINARY( fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__NONBINARY( fname )  \
   DIP__OVL__FOOT                \
   f paramlist; }

/// Calls the overloaded function for all types.
#define DIP_OVL_CALL_ALL( fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__ALL( fname )        \
   DIP__OVL__FOOT                \
   f paramlist; }

//
// DIP_OVL_CALL_ASSIGN_xxx
//

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_UINT( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__UINT( fname )       \
   DIP__OVL__FOOT                \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_SINT( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::sint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__SINT( fname )       \
   DIP__OVL__FOOT                \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_FLOAT( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::sfloat >; \
   DIP__OVL__HEAD( dtype )        \
   DIP__OVL__FLOAT( fname )       \
   DIP__OVL__FOOT                 \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_COMPLEX( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::scomplex >; \
   DIP__OVL__HEAD( dtype )          \
   DIP__OVL__COMPLEX( fname )       \
   DIP__OVL__FOOT                   \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_INTEGER( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__INTEGER( fname )    \
   DIP__OVL__FOOT                \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_UNSIGNED( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__UNSIGNED( fname )   \
   DIP__OVL__FOOT                \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_SIGNED( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::sint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__SIGNED( fname )     \
   DIP__OVL__FOOT                \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_REAL( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__REAL( fname )       \
   DIP__OVL__FOOT                \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_NONCOMPLEX( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__NONCOMPLEX( fname ) \
   DIP__OVL__FOOT                \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_NONBINARY( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__NONBINARY( fname )  \
   DIP__OVL__FOOT                \
   x = f paramlist; }

/// Calls the overloaded function and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_ALL( x, fname, paramlist, dtype ) \
{  auto f = fname< dip::uint8 >; \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__ALL( fname )        \
   DIP__OVL__FOOT                \
   x = f paramlist; }

//
// DIP_OVL_ASSIGN_xxx
//

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_UINT( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__UINT( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_SINT( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SINT( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_FLOAT( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype )  \
   DIP__OVL__FLOAT( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_COMPLEX( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype )    \
   DIP__OVL__COMPLEX( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_INTEGER( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype )    \
   DIP__OVL__INTEGER( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_UNSIGNED( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype )     \
   DIP__OVL__UNSIGNED( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_SIGNED( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype )   \
   DIP__OVL__SIGNED( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_REAL( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__REAL( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_NONCOMPLEX( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype )       \
   DIP__OVL__NONCOMPLEX( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_NONBINARY( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype )      \
   DIP__OVL__NONBINARY( fname ) \
   DIP__OVL__FOOT

/// Assigns a pointer to the overloaded function to the variable `f`.
#define DIP_OVL_ASSIGN_ALL( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__ALL( fname )  \
   DIP__OVL__FOOT


#endif
