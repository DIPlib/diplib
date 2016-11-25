/*
 * DIPlib 3.0
 * This file contains definitions for exceptions and support functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_ERROR_H
#define DIP_ERROR_H

#include <exception>
#include <vector>
#include <string>

/// \file
/// \brief Defines error macros and default error strings. This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


/// \brief Base exception class. All exceptions thrown in DIPlib are derived of this class.
///
/// You can catch this exception at the top level, where you can communicate the problem to the user,
/// and only if you want to prevent your program from exiting abnormally.
/// This class is derived from `std::exception`, so you can choose to catch that instead.
class Error : public std::exception {

   public:

      Error() = default;
      explicit Error( char const* message ) : message_( message ) {}
      explicit Error( const std::string& message ) : message_( message ) {};

      /// \brief Return a message indicating what caused the exception to be thrown, as well as the location
      /// where the error occurred.
      ///
      /// Sometimes multiple locations are given, this is an (incomplete) stack trace that might help figure
      /// out the error. Such a stack trace is generally created when it is a helper function that threw the
      /// exception. The calling function sometimes will catch such an exception, add its name to the stack
      /// trace, and re-throw the exception.
      /// \see dip_AddStackTrace
      virtual char const* what() const noexcept override { // std::exception::what() is declared noexcept, but this one is not.
         std::string msg = message_;
         for( auto const& callSig : stackTrace_ ) {
            msg += "\nin function: " + callSig.functionName +
                   " (" + callSig.fileName + " at line number " + std::to_string( callSig.lineNumber ) + ")";
         }
         return msg.c_str();
      }

      /// \brief Add an entry to the stack trace. Typically called through the `dip_AddStackTrace` macro.
      Error& AddStackTrace(
            std::string const& functionName,
            std::string const& fileName,
            unsigned int lineNumber
      ) {
         stackTrace_.emplace_back( functionName, fileName, lineNumber );
         return *this;
      }

   private:

      struct CallSig {
         std::string functionName;
         std::string fileName;
         unsigned int lineNumber;
         CallSig( std::string functionName, std::string fileName, unsigned int lineNumber ) :
               functionName( functionName ), fileName( fileName ), lineNumber( lineNumber ) {}
      };

      std::string message_;
      std::vector< CallSig > stackTrace_;
};

/// \brief Exception class indicating that an internal inconsistency was found (the library code is wrong).
///
/// You shouldn't need to catch exceptions of this type.
class AssertionError : public Error {
      using Error::Error;
};

/// \brief Exception class indicating that a function received an inconsitent or out of range parameter
/// (the calling code is wrong).
///
/// Catch exceptions of this type only if you don't control the input arguments (i.e. in a use interface).
class ParameterError : public Error {
      using Error::Error;
};

/// \brief Exception class indicating that something happened that we couldn't predict (e.g. file error).
///
/// Catch exceptions of this type if you want to account for run time errors. Note that memory allocation
/// errors are typically `std::bad_alloc`, unless a different allocator is chosen. None of the
/// library functions catch and translate this exception.
class RunTimeError : public Error {
      using Error::Error;
};


namespace E {
// These are some of the standard what() strings thrown.
// These should all happen in multiple places. You don't need to add
// a string here that is used in only one function or one file.

// image creation errors
constexpr char const* IMAGE_NOT_RAW = "Image is not raw";
constexpr char const* IMAGE_NOT_FORGED = "Image is not forged";

// image data type errors
constexpr char const* DATA_TYPE_NOT_SUPPORTED = "Data type not supported";
constexpr char const* WRONG_DATA_TYPE = "Data type does not match";

// image dimensionality and dimensions error
constexpr char const* DIMENSIONALITY_EXCEEDS_LIMIT = "Dimensionality exceeds address limit";
constexpr char const* ILLEGAL_DIMENSIONALITY = "Illegal dimensionality";
constexpr char const* DIMENSIONALITY_NOT_SUPPORTED = "Dimensionality not supported";
constexpr char const* DIMENSIONALITIES_DONT_MATCH = "Dimensionalities don't match";
constexpr char const* ILLEGAL_DIMENSION = "Illegal dimension";
constexpr char const* SIZES_DONT_MATCH = "Sizes don't match";
constexpr char const* NOT_SCALAR = "Image is not scalar";
constexpr char const* NTENSORELEM_DONT_MATCH = "Number of tensor elements doesn't match";

// image properties errors
constexpr char const* NO_NORMAL_STRIDE = "Image has a non-normal stride";

// mask image properties errors
constexpr char const* MASK_NOT_BINARY = "Mask image not binary";
constexpr char const* MASK_TOO_MANY_DIMENSIONS = "Mask image has too many dimensions";

// indexing errors
constexpr char const* INDEX_OUT_OF_RANGE = "Index out of range";
constexpr char const* COORDINATES_OUT_OF_RANGE = "Coordinates out of range";
constexpr char const* ITERATOR_NOT_VALID = "Iterator is not valid";

// miscellaneous errors
constexpr char const* NOT_IMPLEMENTED = "Functionality has not (yet) been implemented";

// array errors
constexpr char const* ARRAY_ILLEGAL_SIZE = "Array has an illegal size";
constexpr char const* ARRAY_SIZES_DONT_MATCH = "Array sizes don't match";
constexpr char const* ARRAY_OVERFLOW = "Array overflow";
constexpr char const* INITIALIZERLIST_ILLEGAL_SIZE = "Initializer list has an illegal size";

// function parameter errors
constexpr char const* INVALID_PARAMETER = "Parameter has invalid value";
constexpr char const* INVALID_FLAG = "Invalid flag";
constexpr char const* PARAMETER_OUT_OF_RANGE = "Parameter value out of range";
constexpr char const* ARRAY_PARAMETER_WRONG_LENGTH = "Array parameter has the wrong number of elements";
constexpr char const* FILTER_SHAPE_NOT_SUPPORTED = "Filter shape is not supported";

}

//
// Test and throw exception
//

#if HAS_PRETTY_FUNCTION
// This is a better thing to use than __func__, if available
#define DIP__FUNC__ __PRETTY_FUNCTION__
#else
// This is in the C++11 standard, so should always be available
#define DIP__FUNC__ __func__
#endif

/// \brief Adds information from current function (including source file and location within file) to the `dip::Error`.
///
/// This macro is useful for building a stack trace. If you want a stack trace, each function must catch `dip::Error`,
/// add its name to the stack trace, and re-throw the exception.
///
/// You can use this macro as follows:
///
///     try {
///        [some DIPlib functions that might throw here...]
///     }
///     catch( dip::Error& e ) {
///        dip_AddStackTrace( e );
///        throw;
///     }
#define dip_AddStackTrace( error ) error.AddStackTrace( DIP__FUNC__, __FILE__, __LINE__ )

/// \brief Throw a `dip::ParameterError`.
#define dip_Throw( str ) do { auto e = dip::ParameterError( str ); dip_AddStackTrace( e ); throw e; } while( false )

/// \brief Test a condition, throw a `dip::ParameterError` if the condition is met.
#define dip_ThrowIf( test, str ) do { if( test ) dip_Throw( str ); } while( false )

/// \brief Throw a `dip::RunTimeError`.
#define dip_ThrowRunTime( str ) do { auto e = dip::RunTimeError( str ); dip_AddStackTrace( e ); throw e; } while( false )

/// \brief Throw a `dip::AssertionError`.
#define dip_ThrowAssertion( str ) do { auto e = dip::AssertionError( str ); dip_AddStackTrace( e ); throw e; } while( false )

/// \brief Test a condition, throw a `dip::AssertionError` if the condition is not met.
#define dip_Assert( test ) do { if( !( test ) ) dip_ThrowAssertion( "Failed assertion: " #test ); } while( false )

// These are the old DIPlib names, let's not use them any more:
//#define DIPASSERT( test, str ) dip_ThrowIf( !(test), str )
//#define DIPTS( test, str )     dip_ThrowIf( test, str )
//#define DIPSJ( str )           dip_Throw( str )

/// \}

} // namespace dip

#endif // DIP_ERROR_H
