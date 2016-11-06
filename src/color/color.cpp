/*
 * DIPlib 3.0
 * This file contains main functionality for color image support.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib/color.h"

namespace dip {

std::array< double, 9 > WhitePoint::InverseMatrix() {} // TODO

ColorSpaceManager::ColorSpaceManager() {} // TODO

void ColorSpaceManager::Set( Image& in, String const& name ) const {} // TODO

void ColorSpaceManager::Convert( Image const& in, Image const& out, String const& name, WhitePoint const& whitepoint ) const {} // TODO

std::vector <dip::uint> ColorSpaceManager::FindPath( dip::uint start, dip::uint stop ) const {} // TODO

} // namespace dip
