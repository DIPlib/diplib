/*
 * DIPimage 3.0
 * This MEX-file implements the `testobject` function
 *
 * (c)2018, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

#include "dip_matlab_interface.h"
#include "diplib/generation.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      static dip::Random random;

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();
      int index = 0;

      dip::TestObjectParams params;

      if(( nrhs > 0 ) && !mxIsChar( prhs[ 0 ] )) {
         // testobject(image, [name-value pairs])
         dip::Image tmp = dml::GetImage( prhs[ 0 ] );
         dip::Convert( tmp, out, dip::DataType::SuggestFloat( tmp.DataType() ));
         params.objectShape = "custom";
         index = 1;
      } else {
         // testobject(object, imgSizes, objSizes, [name-value pairs])
         params.objectShape = ( nrhs > 0 ) ? dml::GetString( prhs[ 0 ] ) : dip::String( dip::S::ELLIPSOID );
         dip::UnsignedArray imgSizes = ( nrhs > 1 ) ? dml::GetUnsignedArray( prhs[ 1 ] ) : dip::UnsignedArray{ 256, 256 };
         params.objectSizes = ( nrhs > 2 ) ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 128, 128 };
         out.ReForge( imgSizes, 1, dip::DT_SFLOAT );
         index = 3;
      }

      // Name-value pairs
      int n = nrhs - index;
      DIP_THROW_IF( n & 1, "Wrong number of input arguments, an even number of arguments needed for the name-value pairs" );
      while( index < nrhs ) {
         dip::String name = dml::GetString( prhs[ index ] );
         ++index;
         if( name == "objectAmplitude" ) {
            params.objectAmplitude = dml::GetFloat( prhs[ index ] );
         } else if( name == "randomShift" ) {
            params.randomShift = dml::GetBoolean( prhs[ index ] );
         } else if( name == "generationMethod" ) {
            params.generationMethod = dml::GetString( prhs[ index ] );
         } else if( name == "modulationDepth" ) {
            params.modulationDepth = dml::GetFloat( prhs[ index ] );
         } else if( name == "modulationFrequency" ) {
            params.modulationFrequency = dml::GetFloatArray( prhs[ index ] );
         } else if( name == "pointSpreadFunction" ) {
            params.pointSpreadFunction = dml::GetString( prhs[ index ] );
         } else if( name == "oversampling" ) {
            params.oversampling = dml::GetFloat( prhs[ index ] );
         } else if( name == "backgroundValue" ) {
            params.backgroundValue = dml::GetFloat( prhs[ index ] );
         } else if( name == "signalNoiseRatio" ) {
            params.signalNoiseRatio = dml::GetFloat( prhs[ index ] );
         } else if( name == "gaussianNoise" ) {
            params.gaussianNoise = dml::GetFloat( prhs[ index ] );
         } else if( name == "poissonNoise" ) {
            params.poissonNoise = dml::GetFloat( prhs[ index ] );
         } else {
            DIP_THROW( "Invalid parameter name: " + name );
         }
         ++index;
      }

      dip::TestObject( out, params, random );
      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
