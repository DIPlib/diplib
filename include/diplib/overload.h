/*
 * DIPlib 3.0
 * This file contains macros to simplify overloading on image data type.
 *
 * (c)2014-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#ifndef DIP_OVERLOAD_H
#define DIP_OVERLOAD_H

/// \file
/// \brief Help with instantiating function templates for different pixel data types.
/// \see overload


/// \defgroup overload Overloading
/// \ingroup infrastructure
/// \brief Help with instantiating function templates and class templates for different pixel data types.
///
/// These preprocessor macros insert a block of code that
/// calls or retrieves a function pointer to the right instance of a
/// template, according to a `dip::DataType` argument (or create an object of
/// a template class). For example, the code
///
/// ```cpp
///     DIP_OVL_CALL_ALL( myFunc, ( param1, param2 ), datatype );
/// ```
///
/// causes a call to `myFunc( param1, param2 )`, where `myFunc` is
/// a function template. The template will be specialized to
/// whichever value the variable `datatype` has at run time. The
/// compiler generates instances of your template for all possible
/// values of `datatype`. If you want to restrict the allowed data
/// types, use a macro ending in other than `_ALL`. For example, the
/// macro `DIP_OVL_CALL_REAL` only allows integer and floating point
/// data types (not binary nor complex types).
///
/// Note that the function parameters are the same for all instances
/// of the template function; the template parameter is not used in the
/// function's parameter list. This is the only way that generic code
/// (i.e. code that works with pixels of any data type) can work.
/// You can think of `param1` above being of type `void*` or `dip::Image`,
/// for example.
///
/// \attention Note the parenthesis around the function parameters in the macro
/// call above!
///
/// `DIP_OVL_NEW_ALL` and friends work similarly, but create a new object
/// of a template class with operator `new`. For such an assignment to work,
/// the template class must have a base class that is not a template, and
/// the object must be referred to through a pointer to its base class. The
/// result of `new` is cast to the declaration type of the variable being
/// assigned to, so that one can assign e.g. into a `std::unique_ptr`.
///
/// There are four groups of macros defined in `diplib/overload.h`:
/// - `DIP_OVL_CALL_xxx` calls a function, discarding any return value.
/// - `DIP_OVL_CALL_ASSIGN_xxx` calls a function, assigning the return
///   value in a variable.
/// - `DIP_OVL_ASSIGN_xxx` assigns a function pointer to a variable, without
///   calling the function.
/// - `DIP_OVL_NEW_xxx` allocates an object of a templated class, assigns
///   to pointer to a variable.
///
/// Each of the four groups of macros exist in the following flavors:
///
/// Macro name `xxx` | Corresponding `%dip::DataType::Classes` value
/// ---------------- | ---------------------------------------------
/// `BIN`            | `dip::DataType::Class_Binary`
/// `UINT`           | `dip::DataType::Class_UInt`
/// `SINT`           | `dip::DataType::Class_SInt`
/// `INTEGER`        | `dip::DataType::Class_Integer`
/// `INT_OR_BIN`     | `dip::DataType::Class_IntOrBin`
/// `FLOAT`          | `dip::DataType::Class_Float`
/// `COMPLEX`        | `dip::DataType::Class_Complex`
/// `FLEX`           | `dip::DataType::Class_Flex`
/// `FLEXBIN`        | `dip::DataType::Class_FlexBin`
/// `UNSIGNED`       | `dip::DataType::Class_Unsigned`
/// `SIGNED`         | `dip::DataType::Class_Signed`
/// `REAL`           | `dip::DataType::Class_Real`
/// `SIGNEDREAL`     | `dip::DataType::Class_SignedReal`
/// `NONBINARY`      | `dip::DataType::Class_NonBinary`
/// `NONCOMPLEX`     | `dip::DataType::Class_NonComplex`
/// `ALL`            | `dip::DataType::Class_All`
/// \{

#define DIP__OVL__HEAD( dtype ) \
   do { switch( dtype ) {

#define DIP__OVL__FOOT \
   default: DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED ); \
}} while( false )

#define DIP__OVL__BIN( assign_name, paramlist ) \
   case dip::DT_BIN      : assign_name< dip::bin >paramlist     ; break;

#define DIP__OVL__UINT( assign_name, paramlist ) \
   case dip::DT_UINT8    : assign_name< dip::uint8 >paramlist   ; break; \
   case dip::DT_UINT16   : assign_name< dip::uint16 >paramlist  ; break; \
   case dip::DT_UINT32   : assign_name< dip::uint32 >paramlist  ; break; \
   case dip::DT_UINT64   : assign_name< dip::uint64 >paramlist  ; break;

#define DIP__OVL__SINT( assign_name, paramlist ) \
   case dip::DT_SINT8    : assign_name< dip::sint8 >paramlist   ; break; \
   case dip::DT_SINT16   : assign_name< dip::sint16 >paramlist  ; break; \
   case dip::DT_SINT32   : assign_name< dip::sint32 >paramlist  ; break; \
   case dip::DT_SINT64   : assign_name< dip::sint64 >paramlist  ; break;

#define DIP__OVL__FLOAT( assign_name, paramlist ) \
   case dip::DT_SFLOAT   : assign_name< dip::sfloat >paramlist  ; break; \
   case dip::DT_DFLOAT   : assign_name< dip::dfloat >paramlist  ; break;

#define DIP__OVL__COMPLEX( assign_name, paramlist ) \
   case dip::DT_SCOMPLEX : assign_name< dip::scomplex >paramlist; break; \
   case dip::DT_DCOMPLEX : assign_name< dip::dcomplex >paramlist; break;

#define DIP__OVL__INTEGER( assign_name, paramlist ) \
   DIP__OVL__UINT( assign_name, paramlist ) \
   DIP__OVL__SINT( assign_name, paramlist )

#define DIP__OVL__INT_OR_BIN( assign_name, paramlist ) \
   DIP__OVL__BIN( assign_name, paramlist ) \
   DIP__OVL__INTEGER( assign_name, paramlist )

#define DIP__OVL__UNSIGNED( assign_name, paramlist ) \
   DIP__OVL__BIN( assign_name, paramlist ) \
   DIP__OVL__UINT( assign_name, paramlist )

#define DIP__OVL__SIGNED( assign_name, paramlist ) \
   DIP__OVL__SINT( assign_name, paramlist ) \
   DIP__OVL__FLOAT( assign_name, paramlist ) \
   DIP__OVL__COMPLEX( assign_name, paramlist )

#define DIP__OVL__REAL( assign_name, paramlist ) \
   DIP__OVL__INTEGER( assign_name, paramlist ) \
   DIP__OVL__FLOAT( assign_name, paramlist )

#define DIP__OVL__SIGNEDREAL( assign_name, paramlist ) \
   DIP__OVL__SINT( assign_name, paramlist ) \
   DIP__OVL__FLOAT( assign_name, paramlist )

#define DIP__OVL__NONCOMPLEX( assign_name, paramlist ) \
   DIP__OVL__BIN( assign_name, paramlist ) \
   DIP__OVL__REAL( assign_name, paramlist )

#define DIP__OVL__FLEX( assign_name, paramlist ) \
   DIP__OVL__FLOAT( assign_name, paramlist ) \
   DIP__OVL__COMPLEX( assign_name, paramlist )

#define DIP__OVL__FLEXBIN( assign_name, paramlist ) \
   DIP__OVL__BIN( assign_name, paramlist ) \
   DIP__OVL__FLEX( assign_name, paramlist )

#define DIP__OVL__NONBINARY( assign_name, paramlist ) \
   DIP__OVL__REAL( assign_name, paramlist ) \
   DIP__OVL__COMPLEX( assign_name, paramlist )

#define DIP__OVL__ALL( assign_name, paramlist ) \
   DIP__OVL__BIN( assign_name, paramlist ) \
   DIP__OVL__NONBINARY( assign_name, paramlist )

//
// DIP_OVL_CALL_xxx
//

/// \brief Calls the overloaded function for the binary type.
#define DIP_OVL_CALL_BINARY( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__BIN( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all unsigned integer types.
#define DIP_OVL_CALL_UINT( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__UINT( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all signed integer types.
#define DIP_OVL_CALL_SINT( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SINT( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all float types.
#define DIP_OVL_CALL_FLOAT( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLOAT( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all complex types.
#define DIP_OVL_CALL_COMPLEX( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__COMPLEX( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all integer types.
#define DIP_OVL_CALL_INTEGER( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__INTEGER( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all integer and binary types.
#define DIP_OVL_CALL_INT_OR_BIN( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__INT_OR_BIN( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all unsigned types.
#define DIP_OVL_CALL_UNSIGNED( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__UNSIGNED( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all signed (integer + float + complex) types.
#define DIP_OVL_CALL_SIGNED( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SIGNED( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all real (integer + float) types.
#define DIP_OVL_CALL_REAL( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__REAL( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all signed real (integer + float) types.
#define DIP_OVL_CALL_SIGNEDREAL( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SIGNEDREAL( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all non-complex types.
#define DIP_OVL_CALL_NONCOMPLEX( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__NONCOMPLEX( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all floating-point and complex types.
#define DIP_OVL_CALL_FLEX( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLEX( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all floating-point, complex and binary types.
#define DIP_OVL_CALL_FLEXBIN( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLEXBIN( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all types but binary.
#define DIP_OVL_CALL_NONBINARY( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__NONBINARY( fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all types.
#define DIP_OVL_CALL_ALL( fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__ALL( fname, paramlist ) \
   DIP__OVL__FOOT

//
// DIP_OVL_CALL_ASSIGN_xxx
//

/// \brief Calls the overloaded function for the binary type, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_BINARY( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__BIN( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all unsigned integer types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_UINT( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__UINT( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all signed integer types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_SINT( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SINT( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all float types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_FLOAT( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLOAT( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all complex types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_COMPLEX( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__COMPLEX( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all integer types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_INTEGER( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__INTEGER( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all integer and binary types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_INT_OR_BIN( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__INT_OR_BIN( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function function for all unsigned types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_UNSIGNED( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__UNSIGNED( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all signed (integer + float + complex) types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_SIGNED( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SIGNED( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all real (integer + float) types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_REAL( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__REAL( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all signed real (integer + float) types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_SIGNEDREAL( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SIGNEDREAL( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all non-complex types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_NONCOMPLEX( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__NONCOMPLEX( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all floating-point and complex types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_FLEX( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLEX( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all floating-point, complex and binary types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_FLEXBIN( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLEXBIN( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all types but binary, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_NONBINARY( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__NONBINARY( x = fname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Calls the overloaded function for all types, and assigns the output value to variable `x`.
#define DIP_OVL_CALL_ASSIGN_ALL( x, fname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__ALL( x = fname, paramlist ) \
   DIP__OVL__FOOT

//
// DIP_OVL_ASSIGN_xxx
//

/// \brief Assigns a pointer to the overloaded function for the binary type to the variable `f`.
#define DIP_OVL_ASSIGN_BINARY( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__BIN( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all unsigned integer types to the variable `f`.
#define DIP_OVL_ASSIGN_UINT( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__UINT( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all signed integer types to the variable `f`.
#define DIP_OVL_ASSIGN_SINT( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SINT( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all float types to the variable `f`.
#define DIP_OVL_ASSIGN_FLOAT( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLOAT( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all complex types to the variable `f`.
#define DIP_OVL_ASSIGN_COMPLEX( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__COMPLEX( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all integer types to the variable `f`.
#define DIP_OVL_ASSIGN_INTEGER( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__INTEGER( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all integer and binary types to the variable `f`.
#define DIP_OVL_ASSIGN_INT_OR_BIN( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__INT_OR_BIN( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all unsigned types to the variable `f`.
#define DIP_OVL_ASSIGN_UNSIGNED( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__UNSIGNED( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all signed (integer + float + complex) types to the variable `f`.
#define DIP_OVL_ASSIGN_SIGNED( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SIGNED( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all real (integer + float) types to the variable `f`.
#define DIP_OVL_ASSIGN_REAL( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__REAL( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all signed real (integer + float) types to the variable `f`.
#define DIP_OVL_ASSIGN_SIGNEDREAL( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SIGNEDREAL( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all non-complex types to the variable `f`.
#define DIP_OVL_ASSIGN_NONCOMPLEX( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__NONCOMPLEX( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all floating-point and complex types to the variable `f`.
#define DIP_OVL_ASSIGN_FLEX( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLEX( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all floating-point, complex and binary types to the variable `f`.
#define DIP_OVL_ASSIGN_FLEXBIN( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLEXBIN( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all types but binary to the variable `f`.
#define DIP_OVL_ASSIGN_NONBINARY( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__NONBINARY( f = fname, ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded function for all types to the variable `f`.
#define DIP_OVL_ASSIGN_ALL( f, fname, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__ALL( f = fname, ) \
   DIP__OVL__FOOT

//
// DIP_OVL_NEW_xxx
//

/// \brief Assigns a pointer to the overloaded class for the binary type to the variable `x`.
#define DIP_OVL_NEW_BINARY( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__BIN( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all unsigned integer types to the variable `x`.
#define DIP_OVL_NEW_UINT( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__UINT( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all signed integer types to the variable `x`.
#define DIP_OVL_NEW_SINT( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SINT( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all float types to the variable `x`.
#define DIP_OVL_NEW_FLOAT( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLOAT( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all complex types to the variable `x`.
#define DIP_OVL_NEW_COMPLEX( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__COMPLEX( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all integer types to the variable `x`.
#define DIP_OVL_NEW_INTEGER( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__INTEGER( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all integer and binary types to the variable `x`.
#define DIP_OVL_NEW_INT_OR_BIN( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__INT_OR_BIN( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all unsigned types to the variable `x`.
#define DIP_OVL_NEW_UNSIGNED( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__UNSIGNED( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all signed (integer + float + complex) types to the variable `x`.
#define DIP_OVL_NEW_SIGNED( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SIGNED( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all real (integer + float) types to the variable `x`.
#define DIP_OVL_NEW_REAL( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__REAL( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all signed real (integer + float) types to the variable `x`.
#define DIP_OVL_NEW_SIGNEDREAL( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__SIGNEDREAL( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all non-complex types to the variable `x`.
#define DIP_OVL_NEW_NONCOMPLEX( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__NONCOMPLEX( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all floating-point and complex types to the variable `x`.
#define DIP_OVL_NEW_FLEX( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLEX( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all floating-point, complex and binary types to the variable `x`.
#define DIP_OVL_NEW_FLEXBIN( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__FLEXBIN( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all types but binary to the variable `x`.
#define DIP_OVL_NEW_NONBINARY( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__NONBINARY( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT

/// \brief Assigns a pointer to the overloaded class for all types to the variable `x`.
#define DIP_OVL_NEW_ALL( x, cname, paramlist, dtype ) \
   DIP__OVL__HEAD( dtype ) \
   DIP__OVL__ALL( x = ( decltype( x )) new cname, paramlist ) \
   DIP__OVL__FOOT


/// \}

#endif // DIP_OVERLOAD_H
