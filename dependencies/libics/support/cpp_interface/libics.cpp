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
 * FILE : libics.cpp
 *
 * This file defines functions for the C++ interface.
 */


#include <stdexcept>
#include <cstring>

#include "libics.hpp"
#include "libics.h"

#define ICS_FIELD_SEP '\t' // Copied from libics_ll.h

namespace ics {

void ICS::Open(std::string const& filename, std::string const& mode) {
   if (ics) {
      Close(); // Let's close the file first!
   }
   Ics_Error err = IcsOpen(&ics, filename.c_str(), mode.c_str());
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::Close() {
   Ics_Error err = IcsClose(ics);
   ics = nullptr;
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

ICS::~ICS() {
   if (ics) {
      IcsClose(ics); // Ignore errors -- we cannot throw!
   }
}

//void ICS::GetLayout() {
ICS::Layout ICS::GetLayout() const {
   Ics_DataType type;
   int sz;
   std::vector<std::size_t> dims(ICS_MAXDIM);
   IcsGetLayout(ics, &type, &sz, dims.data());
   dims.resize(static_cast<std::size_t>(sz));
   DataType dt;
   switch( type ) {
      default:
      case Ics_unknown:
         dt = DataType::Unknown;
         break;
      case Ics_uint8:
         dt = DataType::UInt8;
         break;
      case Ics_sint8:
         dt = DataType::SInt8;
         break;
      case Ics_uint16:
         dt = DataType::UInt16;
         break;
      case Ics_sint16:
         dt = DataType::SInt16;
         break;
      case Ics_uint32:
         dt = DataType::UInt32;
         break;
      case Ics_sint32:
         dt = DataType::SInt32;
         break;
      case Ics_uint64:
         dt = DataType::UInt64;
         break;
      case Ics_sint64:
         dt = DataType::SInt64;
         break;
      case Ics_real32:
         dt = DataType::Real32;
         break;
      case Ics_real64:
         dt = DataType::Real64;
         break;
      case Ics_complex32:
         dt = DataType::Complex32;
         break;
      case Ics_complex64:
         dt = DataType::Complex64;
         break;
   }
   return {dt, std::move(dims)};
}

void ICS::SetLayout(DataType dt, std::vector<std::size_t> const& dims) {
   Ics_DataType type;
   switch( dt ) {
      default:
      //case DataType::Unknown:
         type = Ics_unknown;
         break;
      case DataType::UInt8:
         type = Ics_uint8;
         break;
      case DataType::SInt8:
         type = Ics_sint8;
         break;
      case DataType::UInt16:
         type = Ics_uint16;
         break;
      case DataType::SInt16:
         type = Ics_sint16;
         break;
      case DataType::UInt32:
         type = Ics_uint32;
         break;
      case DataType::SInt32:
         type = Ics_sint32;
         break;
      case DataType::UInt64:
         type = Ics_uint64;
         break;
      case DataType::SInt64:
         type = Ics_sint64;
         break;
      case DataType::Real32:
         type = Ics_real32;
         break;
      case DataType::Real64:
         type = Ics_real64;
         break;
      case DataType::Complex32:
         type = Ics_complex32;
         break;
      case DataType::Complex64:
         type = Ics_complex64;
         break;
   }
   IcsSetLayout(ics, type, static_cast<int>(dims.size()), dims.data());
}

std::size_t ICS::GetDataSize() const {
   return IcsGetDataSize(ics);
}
std::size_t ICS::GetImelSize() const {
   return IcsGetImelSize(ics);
}
std::size_t ICS::GetImageSize() const {
   return IcsGetImageSize(ics);
}

void ICS::GetData(void* dest, size_t n) {
   Ics_Error err = IcsGetData(ics, dest, n);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::GetROIData(std::vector<std::size_t> const& offset,
                     std::vector<std::size_t> const& size,
                     std::vector<std::size_t> const& sampling,
                     void* dest,
                     std::size_t n) {
   Ics_Error err = IcsGetROIData(
         ics,
         offset.empty()   ? nullptr : offset.data(),
         size.empty()     ? nullptr : size.data(),
         sampling.empty() ? nullptr : sampling.data(),
         dest, n);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

// Read the image from an ICS file into a sub-block of a memory block. To use
// the defaults strides, pass an empty vector. Only valid if reading.
void ICS::GetDataWithStrides(void* dest, std::vector<std::ptrdiff_t> const& stride) {
   Ics_Error err = IcsGetDataWithStrides(
         ics, dest, 0,
         stride.empty() ? nullptr : stride.data(),
         stride.empty() ? (ics ? ics->dimensions : 0) : static_cast<int>(stride.size()));
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}


// Read a portion of the image data from an ICS file. Only valid if reading.
void ICS::GetDataBlock(void *dest, std::size_t n) {
   Ics_Error err = IcsGetDataBlock(ics, dest, n);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}


/* Skip a portion of the image from an ICS file. Only valid if reading. */
void ICS::SkipDataBlock(std::size_t n) {
   Ics_Error err = IcsSkipDataBlock(ics, n);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::GetPreviewData(void *dest, std::size_t n, std::size_t planeNumber) {
   Ics_Error err = IcsGetPreviewData(ics, dest, n, planeNumber);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::SetData(void const* src, std::size_t n) {
   Ics_Error err = IcsSetData(ics, src, n);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::SetDataWithStrides(void const* src,
                             std::size_t n,
                             std::vector<ptrdiff_t> const& strides) {
   Ics_Error err = IcsSetDataWithStrides(ics, src, n, strides.data(), static_cast<int>(strides.size()));
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::SetSource(std::string const& fname, std::size_t offset) {
   Ics_Error err = IcsSetSource(ics, fname.c_str(), offset);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::SetByteOrder(ByteOrder order) {
   Ics_Error err = IcsSetByteOrder(
         ics,
         order == ByteOrder::LittleEndian ? IcsByteOrder_littleEndian
                                          : IcsByteOrder_bigEndian);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::SetCompression(Compression compression, int level) {
   Ics_Error err = IcsSetCompression(
         ics,
         compression == Compression::GZip ? IcsCompr_gzip
                                          : IcsCompr_uncompressed,
         level );
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

Units ICS::GetPosition(int dimension) const {
   char const* str;
   Units units;
   Ics_Error err = IcsGetPositionF(ics, dimension, &units.origin, &units.scale, &str);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
   units.units = str;
   return units;
}

void ICS::SetPosition(int dimension, Units const& units) {
   Ics_Error err = IcsSetPosition(ics, dimension, units.origin, units.scale, units.units.c_str());
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::GetOrder(int dimension,
                   std::string& order,
                   std::string& label) const {
   char const* str_order;
   char const* str_label;
   Ics_Error err = IcsGetOrderF(ics, dimension, &str_order, &str_label);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
   order = str_order;
   label = str_label;
}

void ICS::SetOrder(int dimension,
                   std::string const& order,
                   std::string const& label) {
   Ics_Error err = IcsSetOrder(ics, dimension, order.c_str(), label.c_str());
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

std::string ICS::GetCoordinateSystem() const {
   std::string coord;
   coord.resize(ICS_STRLEN_TOKEN); // max length
   Ics_Error err = IcsGetCoordinateSystem(ics, &(coord[0])); // TODO: this would be prettier as IcsGetCoordinateSystemF
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
   coord.resize(std::strlen(&(coord[0])));
   return coord;
}

void ICS::SetCoordinateSystem(std::string const& coord) {
   Ics_Error err = IcsSetCoordinateSystem(ics, coord.c_str());
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

std::size_t ICS::GetSignificantBits() const {
   std::size_t nBits;
   Ics_Error err = IcsGetSignificantBits(ics, &nBits);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
   return nBits;
}

void ICS::SetSignificantBits(std::size_t nBits) {
   Ics_Error err = IcsSetSignificantBits(ics, nBits);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

Units ICS::GetImelUnits() const {
   char const* str;
   Units units;
   Ics_Error err = IcsGetImelUnitsF(ics, &units.origin, &units.scale, &str);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
   units.units = str;
   return units;
}

void ICS::SetImelUnits(Units const& units) {
   Ics_Error err = IcsSetImelUnits(ics, units.origin, units.scale, units.units.c_str());
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::AddHistoryString(std::string const& key, std::string const& value) {
   Ics_Error err = IcsAddHistoryString(ics, key.c_str(), value.c_str());
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void ICS::DeleteHistory(std::string const& key) {
   Ics_Error err = IcsDeleteHistory(ics, key.c_str());
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

int ICS::GetNumHistoryStrings() const {
   int n;
   Ics_Error err = IcsGetNumHistoryStrings(ics, &n);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
   return n;
}

HistoryIterator::HistoryIterator(ICS& icsObj, std::string const& key) {
   ics = icsObj.ics;
   historyIterator = new Ics_HistoryIterator;
   Ics_Error err = IcsNewHistoryIterator(ics, historyIterator, key.c_str());
   if (err == IcsErr_EndOfHistory) {
      return;
   } else if (err != IcsErr_Ok) {
      delete historyIterator;
      historyIterator = nullptr;
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

HistoryIterator::~HistoryIterator() {
   delete historyIterator;
}

std::string HistoryIterator::String() {
   char const* str;
   Ics_Error err = IcsGetHistoryStringIF(ics, historyIterator, &str);
   if (err == IcsErr_EndOfHistory) {
      return {};
   } else if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
   std::string out(str);
   if (out.empty()) {
      // This should not happen, but we want to avoid an empty string if we haven't reached
      // the end of the history yet. Simply recurse until a valid entry was found.
      return String();
   }
   return {str};
}

HistoryIterator::KeyValuePair HistoryIterator::KeyValue() {
   char const* str;
   Ics_Error err = IcsGetHistoryStringIF(ics, historyIterator, &str);
   if (err == IcsErr_EndOfHistory) {
      return {};
   } else if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
   if (std::strlen(str) == 0) {
      // This should not happen, but we want to avoid an empty string if we haven't reached
      // the end of the history yet. Simply recurse until a valid entry was found.
      return KeyValue();
   }
   char const *ptr = std::strchr(str, ICS_FIELD_SEP);
   if (!ptr) {
      // Let's assume we only have a key
      return {std::string{str}, std::string{}};
   }
   return {std::string(str, static_cast<std::size_t>(ptr-str)), std::string(ptr + 1)};
}

void HistoryIterator::Delete() {
   Ics_Error err = IcsDeleteHistoryStringI(ics, historyIterator);
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

void HistoryIterator::Replace(std::string const& key, std::string const& value) {
   Ics_Error err = IcsReplaceHistoryStringI(ics, historyIterator, key.c_str(), value.c_str());
   if (err != IcsErr_Ok) {
      throw std::runtime_error(IcsGetErrorText(err));
   }
}

std::string GetLibVersion() {
   return IcsGetLibVersion();
}

int Version(std::string const& filename, bool forceName) {
   return IcsVersion(filename.c_str(), forceName);
}

} // namespace ics
