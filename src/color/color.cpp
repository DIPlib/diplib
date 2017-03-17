/*
 * DIPlib 3.0
 * This file contains main functionality for color image support.
 *
 * (c)2016, Cris Luengo.
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

#include "diplib/color.h"

#include "rgb.h"
#include "cmyk.h"

namespace dip {

ColorSpaceManager::ColorSpaceManager() {
   // grey (or gray)
   Define( "grey", 1 );
   DefineAlias( "gray", "grey" );
   // RGB
   Define( "RGB", 3 );
   DefineAlias( "rgb", "RGB" );
   Register( ColorSpaceConverterPointer( new grey2rgb ));
   Register( ColorSpaceConverterPointer( new rgb2grey ));
   // nlRGB (or R'G'B')
   Define( "R'G'B'", 3 );
   DefineAlias( "nlRGB", "R'G'B'" );
   Register( ColorSpaceConverterPointer( new rgb2nlrgb ));
   Register( ColorSpaceConverterPointer( new nlrgb2rgb ));
   // CMY
   Define( "CMY", 3 );
   DefineAlias( "cmy", "CMY" );
   Register( ColorSpaceConverterPointer( new rgb2cmy ));
   Register( ColorSpaceConverterPointer( new cmy2rgb ));
   // CMYK
   Define( "CMYK", 4 );
   DefineAlias( "cmyk", "CMYK" );
   Register( ColorSpaceConverterPointer( new cmy2cmyk ));
   Register( ColorSpaceConverterPointer( new cmyk2cmy ));
   // HCV
   Define( "HCV", 3 );
   DefineAlias( "hcv", "HCV" );
   //Register( ColorSpaceConverterPointer( new rgb2hcv ));
   //Register( ColorSpaceConverterPointer( new hcv2rgb ));
   // HSV
   Define( "HSV", 3 );
   DefineAlias( "hsv", "HSV" );
   //Register( ColorSpaceConverterPointer( new hcv2hsv ));
   //Register( ColorSpaceConverterPointer( new hsv2hcv ));
   // XYZ
   Define( "XYZ", 3 );
   DefineAlias( "xyz", "XYZ" );
   //Register( ColorSpaceConverterPointer( new grey2xyz ));
   //Register( ColorSpaceConverterPointer( new rgb2xyz ));
   //Register( ColorSpaceConverterPointer( new xyz2grey ));
   //Register( ColorSpaceConverterPointer( new xyz2rgb ));
   // Yxy
   Define( "Yxy", 3 );
   DefineAlias( "yxy", "Yxy" );
   //Register( ColorSpaceConverterPointer( new xyz2yxy ));
   //Register( ColorSpaceConverterPointer( new yxy2grey ));
   //Register( ColorSpaceConverterPointer( new yxy2xyz ));
   // Lab (or L*a*b*, CIELAB)
   Define( "Lab", 3 );
   DefineAlias( "lab", "Lab" );
   DefineAlias( "L*a*b*", "Lab" );
   DefineAlias( "CIELAB", "Lab" );
   DefineAlias( "cielab", "Lab" );
   //Register( ColorSpaceConverterPointer( new grey2lab ));
   //Register( ColorSpaceConverterPointer( new xyz2lab ));
   //Register( ColorSpaceConverterPointer( new lab2grey ));
   //Register( ColorSpaceConverterPointer( new lab2xyz ));
   // Luv (or L*u*v*, CIELUV)
   Define( "Luv", 3 );
   DefineAlias( "luv", "Luv" );
   DefineAlias( "L*u*v*", "Luv" );
   DefineAlias( "CIELUV", "Luv" );
   DefineAlias( "cieluv", "Luv" );
   //Register( ColorSpaceConverterPointer( new grey2luv ));
   //Register( ColorSpaceConverterPointer( new xyz2luv ));
   //Register( ColorSpaceConverterPointer( new luv2xyz ));
   //Register( ColorSpaceConverterPointer( new luv2grey )); // identical code to lab2xyz!
}

void ColorSpaceManager::Convert(
      Image const& in,
      Image const& out,
      String const& name
) const {
   // TODO
}

std::vector< dip::uint > ColorSpaceManager::FindPath( dip::uint start, dip::uint stop ) const {
   // TODO
   return {};
}

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include <random>
#include "diplib/math.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the ColorSpaceManager class") {
   dip::ColorSpaceManager csm;
   DOCTEST_CHECK( csm.NumberOfChannels( "rgb" ) == 3 );
   DOCTEST_CHECK( csm.NumberOfChannels( "CMYK" ) == 4 );
   DOCTEST_CHECK( csm.NumberOfChannels( "grey" ) == 1 );
   DOCTEST_CHECK( csm.CanonicalName( "CIELUV" ) == "Luv" );
}

#endif // DIP__ENABLE_DOCTEST
