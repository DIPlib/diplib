/*
 * libics: Image Cytometry Standard file reading and writing.
 *
 * C++ interface.
 *
 * Copyright 2018 Cris Luengo.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * FILE : libics.hpp
 *
 * This is the only file you need to include in your C++ project
 * if you don't want to use any of the low-level library functionality.
 *
 * The C++ interface throws exceptions where the C interface returns an
 * error code, and uses RAII to prevent files not being closed and
 * memory being leaked. Other than that, it provides identical
 * functionality to the high-level interface in libics.h.
 */


#ifndef LIBICS_CPP_H
#define LIBICS_CPP_H

#include <string>
#include <utility>
#include <vector>

#if defined(__WIN32__) && !defined(WIN32)
#  define WIN32
#endif

#ifdef WIN32
#  ifdef BUILD_ICSCPP
#    define ICSCPPEXPORT __declspec(dllexport)
#  else
#    ifdef USE_ICSCPP_DLL
#      define ICSCPPEXPORT __declspec(dllimport)
#    else
#      define ICSCPPEXPORT
#    endif
#  endif
#else
#  ifdef BUILD_ICSCPP
#    define ICSCPPEXPORT __attribute__((visibility("default")))
#  else
#    define ICSCPPEXPORT
#  endif
#endif

extern "C" {
   struct _ICS;
   struct _Ics_HistoryIterator;
}

namespace ics {

enum class DataType {
   Unknown = 0,
   UInt8,     // integer, unsigned,  8 bpp
   SInt8,     // integer, signed,    8 bpp
   UInt16,    // integer, unsigned, 16 bpp
   SInt16,    // integer, signed,   16 bpp
   UInt32,    // integer, unsigned, 32 bpp
   SInt32,    // integer, signed,   32 bpp
   UInt64,    // integer, unsigned, 64 bpp
   SInt64,    // integer, signed,   64 bpp
   Real32,    // real,    signed,   32 bpp
   Real64,    // real,    signed,   64 bpp
   Complex32, // complex, signed, 2*32 bpp
   Complex64  // complex, signed, 2*64 bpp
};

enum class Compression {
   Uncompressed, // No compression
   GZip          // Using zlib (ICS_ZLIB must be defined)
};

enum class ByteOrder {
   LittleEndian, // Little endian byte order
   BigEndian     // Big endian byte order
};

struct Units {
   double origin;       // The position of each imel is given by
   double scale;        // origin + coordinate * scale,
   std::string units;   // in units.
};                      // Imel intensities use the same concept.

class ICS;


// Use this to read history lines. This is a bit of an awkward iterator, we
// adapted libics' iterator without trying to match the C++ iterator concept.
// Repeated calls to `String` or `KeyValue` will produce all history lines.
// An empty output indicates the end of the history has been reached.
class HistoryIterator {
public:

   // Without a key, all history lines will be read. Otherwise only those with
   // key equal to the string key will be read.
   ICSCPPEXPORT explicit HistoryIterator(ICS &ics, std::string const &key = "");

   HistoryIterator(HistoryIterator const &other) = delete;

   HistoryIterator(HistoryIterator &&other) noexcept { swap(other); }

   HistoryIterator &operator=(HistoryIterator const &other) = delete;

   HistoryIterator &operator=(HistoryIterator &&other) noexcept {
      swap(other);
      return *this;
   }

   void swap(HistoryIterator &other) noexcept {
      std::swap(historyIterator, other.historyIterator);
   }

   ICSCPPEXPORT ~HistoryIterator();

   // Get the next history line as a single string.
   // If there are no more history lines, returns an empty string.
   ICSCPPEXPORT std::string String();

   // Get the next history line as a key/value pair.
   // If there are no more history lines, returns two empty strings.
   struct KeyValuePair {
      std::string key;
      std::string value;
   };
   ICSCPPEXPORT KeyValuePair KeyValue();

   // Delete last retrieved history line (iterator still points to the same
   // string).
   ICSCPPEXPORT void Delete();

   // Replace last retrieved history line (iterator still points to the same
   // string).
   ICSCPPEXPORT void Replace(std::string const &key, std::string const &value);

private:
    // std::unique_ptr doesn't work with an incomplete type. We do not want to
    // include "libics.h" here, so we need to work with incomplete types. A
    // naked pointer is the best we can do I think.
    struct _Ics_HistoryIterator *historyIterator = nullptr;
    struct _ICS *ics = nullptr;   // Non-owning pointer to ICS data structure.
};


// This class encapsulates an ICS file.
class ICS {
   friend class HistoryIterator;
public:

   // The default constructor creates an object not associated to any files.
   // Use Open to associate it with a file.
   ICS() = default;

   // This constructor creates the object and associates it to a file. See
   // Open for the description of the parameters.
   ICS(std::string const& filename, std::string const& mode) {
      Open(filename, mode);
   }

   ICS(ICS const& other) = delete;
   ICS(ICS&& other) noexcept { swap(other); }
   ICS& operator=(ICS const& other) = delete;
   ICS& operator=(ICS&& other) noexcept {
      swap(other);
      return *this;
   }
   void swap(ICS& other) noexcept {
      std::swap(ics, other.ics);
   }

   // The destructor closes the ICS file. When writing, closing the file causes
   // the data to be written. The destructor cannot inform of errors that occur
   // during writing. The Close method throws an exception when an error occurs,
   // use that function to catch errors.
   ICSCPPEXPORT ~ICS();

   // Open an ICS file for reading (mode = "r") or writing (mode = "w"). When
   // writing, append a "2" to the mode string to create an ICS version 2.0
   // file. Append an "f" to mode if, when reading, you want to force the file name
   // to not change (no ".ics" is appended). Append a "l" to mode if, when reading,
   // you don't want the locale forced to "C" (to read ICS files written with some
   // other locale, set the locale properly then open the file with "rl").
   ICSCPPEXPORT void Open(std::string const& filename, std::string const& mode);

   // Close the ICS file. The ICS object is no longer associated to a file after
   // calling this function.  No files are actually written until this function is
   // called. Note that the destructor calls this function before exiting.
   ICSCPPEXPORT void Close();

   // Retrieve the layout of an ICS image. Only valid if reading.
   struct Layout {
      DataType dataType;
      std::vector<std::size_t> dimensions;
   };
   ICSCPPEXPORT Layout GetLayout() const;

   // Set the layout for an ICS image. Only valid if writing.
   ICSCPPEXPORT void SetLayout(DataType dt, std::vector<std::size_t> const& dims);

   // These three functions retrieve info from the ICS file.
   // GetDataSize() == GetImelSize() * GetImageSize()
   ICSCPPEXPORT std::size_t GetDataSize() const;
   ICSCPPEXPORT std::size_t GetImelSize() const;
   ICSCPPEXPORT std::size_t GetImageSize() const;

   // Read the image data from an ICS file. Only valid if reading.
   ICSCPPEXPORT void GetData(void* dest, std::size_t n);

   // Read a square region of the image from an ICS file. To use the defaults in
   // one of the parameters, pass an empty vector. Only valid if reading.
   ICSCPPEXPORT void GetROIData(std::vector<std::size_t> const& offset,
                                std::vector<std::size_t> const& size,
                                std::vector<std::size_t> const& sampling,
                                void* dest,
                                std::size_t n);

   // Read the image from an ICS file into a sub-block of a memory block. To use
   // the defaults strides, pass an empty vector. Only valid if reading.
   ICSCPPEXPORT void GetDataWithStrides(void* dest,
                                        std::vector<std::ptrdiff_t> const& stride);

   // Read a portion of the image data from an ICS file. Only valid if reading.
   ICSCPPEXPORT void GetDataBlock(void *dest, std::size_t n);

   // Skip a portion of the image from an ICS file. Only valid if reading.
   ICSCPPEXPORT void SkipDataBlock(std::size_t n);

   // Read a plane of the image data from an ICS file, and convert it to
   // uint8. Only valid if reading.
   ICSCPPEXPORT void GetPreviewData(void *dest,
                                    std::size_t n,
                                    std::size_t planeNumber);

   // Set the image data for an ICS image. The pointer to this data must be
   // accessible until Close has been called. Only valid if writing.
   ICSCPPEXPORT void SetData(void const* src, std::size_t n);

   // Set the image data for an ICS image. The pointer to this data must be
   // accessible until Close has been called. Only valid if writing.
   ICSCPPEXPORT void SetDataWithStrides(void const* src,
                                        std::size_t n,
                                        std::vector<ptrdiff_t> const& strides);

   // Set the image source parameter for an ICS version 2.0 file. Only valid if
   // writing.
   ICSCPPEXPORT void SetSource(std::string const& fname, std::size_t offset);

   // Set the image source byte order for an ICS version 2.0 file. Only valid if
   // writing, and only valid after calling SetSource. If the data type is
   // changed after this call, the byte order information written to file might
   // not be correct.
   ICSCPPEXPORT void SetByteOrder(ByteOrder order);

   // Set the compression method and compression parameter. Only valid if
   // writing.
   ICSCPPEXPORT void SetCompression(Compression compression, int level = 9);

   // Get the position of the image in the real world: the origin of the first
   // pixel, the distances between pixels and the units in which to measure.
   // Dimensions start at 0. Only valid if reading.
   ICSCPPEXPORT Units GetPosition(int dimension) const;

   // Set the position of the image in the real world: the origin of the first
   // pixel, the distances between pixels and the units in which to measure.  If
   // units.units is empty, it is set to the default value of "undefined".
   // Dimensions start at 0. Only valid if writing.
   ICSCPPEXPORT void SetPosition(int dimension,
                                 Units const& units);

   // Get the ordering of the dimensions in the image. The ordering is defined by
   // names and labels for each dimension. The defaults are x, y, z, t (time) and p
   // (probe). Dimensions start at 0. Only valid if reading.
   ICSCPPEXPORT void GetOrder(int dimension,
                              std::string& order,
                              std::string& label) const;

   // Set the ordering of the dimensions in the image. The ordering is defined by
   // providing names and labels for each dimension. The defaults are x, y, z, t
   // (time) and p (probe). Dimensions start at 0. Only valid if writing.
   ICSCPPEXPORT void SetOrder(int dimension,
                              std::string const& order,
                              std::string const& label);

   // Get the coordinate system used in the positioning of the pixels.  Related to
   // GetPosition(). The default is "video". Only valid if reading.
   ICSCPPEXPORT std::string GetCoordinateSystem() const;

   // Set the coordinate system used in the positioning of the pixels.  Related to
   // SetPosition(). The default is "video". Only valid if writing.
   ICSCPPEXPORT void SetCoordinateSystem(std::string const& coord);

   // Get the number of significant bits. Only valid if reading.
   ICSCPPEXPORT std::size_t GetSignificantBits() const;

   // Set the number of significant bits. Only valid if writing.
   ICSCPPEXPORT void SetSignificantBits(std::size_t nBits);

   // Set the position of the pixel values: the offset and scaling, and the units
   // in which to measure. If you are not interested in one of the parameters, set
   // the pointer to NULL. Only valid if reading.
   ICSCPPEXPORT Units GetImelUnits() const;

   // Set the position of the pixel values: the offset and scaling, and the units
   // in which to measure. If units.units is empty, it is set to the default
   // value of "relative". Only valid if writing.
   ICSCPPEXPORT void SetImelUnits(Units const& units);

   // Add history lines to the ICS file. key can be NULL
   ICSCPPEXPORT void AddHistoryString(std::string const& key,
                                      std::string const& value);

   // Delete all history lines with key from ICS file. key can be empty, deletes
   // all.
   ICSCPPEXPORT void DeleteHistory(std::string const& key = "");

   // Get the number of HISTORY lines from the ICS file.
   ICSCPPEXPORT int GetNumHistoryStrings() const;

   HistoryIterator NewHistoryIterator(std::string const &key = "") {
      return HistoryIterator(*this, key);
   }

private:
   // std::unique_ptr doesn't work with an incomplete type. We do not want to
   // include "libics.h" here, so we need to work with incomplete types. A
   // naked pointer is the best we can do I think.
   struct _ICS* ics = nullptr;
};

// Returns a string that can be used to compare with ICSLIB_VERSION to check if
// the version of the library is the same as that of the headers.
inline std::string GetLibVersion();

// Returns 0 if it is not an ICS file, or the version number if it is.
// If forceName is true, no extension is appended.
inline int Version(std::string const& filename, bool forceName);

// Read a preview (2D) image out of an ICS file. dest is resized, and xsize
// and ysize are set to the image size.
inline void LoadPreview(std::string const& filename,
                        std::size_t planeNumber,
                        std::vector<std::uint8_t>& dest,
                        std::size_t& xsize,
                        std::size_t& ysize) {
   ICS ics(filename, "r");
   auto layout = ics.GetLayout();
   if (layout.dimensions.size() < 2 ) {
      throw std::runtime_error("Image has fewer than two dimensions");
   }
   std::size_t n = layout.dimensions[ 0 ] * layout.dimensions[ 1 ];
   dest.resize(n);
   ics.GetPreviewData(dest.data(), n, planeNumber);
   ics.Close();
   xsize = layout.dimensions[ 0 ];
   ysize = layout.dimensions[ 1 ];
}

} // namespace ics

#endif // LIBICS_CPP_H
