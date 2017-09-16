/*
 * DIPimage 3.0
 * This MEX-file implements the `measure` function
 *
 * (c)2017, Cris Luengo.
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

#undef DIP__ENABLE_DOCTEST
#include "dip_matlab_interface.h"
#include "diplib/measurement.h"
#include "diplib/regions.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   dml::streambuf streambuf;

   static dip::MeasurementTool measurementTool;

   try {

      DML_MIN_ARGS( 1 );

      if( mxIsChar( prhs[ 0 ] )) {

         dip::String str = dml::GetString( prhs[ 0 ]);
         if( str == "help" ) {
            DML_MAX_ARGS( 1 );
            auto features = measurementTool.Features();
            std::cout << features.size() << " measurement features available:" << std::endl;
            for( auto const& feature : features ) {
               std::cout << " - '" << feature.name << "': " << feature.description;
               if( feature.needsGreyValue ) {
                  std::cout << " *";
               }
               std::cout << std::endl;
            }
            std::cout << "Features marked with a \"*\" require a grey-value input image.\n";
         } else {
            DIP_THROW( "Unrecognized option: " + str );
         }

      } else {

         DML_MAX_ARGS( 5 );

         dml::MatlabInterface mi;
         dip::Image label = dml::GetImage( prhs[ 0 ] ); // Non-const, we might want to modify the image (not the data pointed at)
         dip::Image const grey = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image();
         dip::StringArray features;
         if( nrhs > 2 ) {
            features = dml::GetStringArray( prhs[ 2 ] );
            // Find known feature names
            auto infoArray = measurementTool.Features();
            // Put lower-case version of names in a map
            std::map< dip::String, dip::uint > knownFeatures;
            for( dip::uint ii = 0; ii < infoArray.size(); ++ii ) {
               dip::String name = infoArray[ ii ].name;
               dml::ToLower( name );
               knownFeatures.emplace( name, ii );
            }
            // Find requested features in map, using case-insensitive search, and copy name with correct case
            for( auto& f : features ) {
               dml::ToLower( f );
               auto it = knownFeatures.find( f );
               DIP_THROW_IF( it == knownFeatures.end(), "Feature name not recognized" );
               f = infoArray[ it->second ].name;
            }
         } else {
            features = { "Size" };
         }
         dip::UnsignedArray objectIDs;
         if( nrhs > 3 ) {
            objectIDs = dml::GetUnsignedArray( prhs[ 3 ] );
         }
         dip::uint connectivity = nrhs > 4 ? dml::GetUnsigned( prhs[ 4 ] ) : label.Dimensionality();

         if( !label.DataType().IsUInt() ) {
            // Not yet labeled
            DIP_THROW_IF( !label.DataType().IsBinary(), "Object input image must be either labeled or binary." );
            dip::Image tmp = dip::Label( label, connectivity ); // using this form so that we don't overwrite input data.
            label.Strip();
            label = tmp;
         }

         dip::Measurement msr = measurementTool.Measure( label, grey, features, objectIDs, connectivity );

         // Convert `msr` to a `dip_measurement` object

         // Step 1: create {mxObjects, mxFeatures, mxValues}, the input arguments to the constructor
         mxArray* mxInputArgs[ 3 ];
         // - Objects
         mxInputArgs[ 0 ] = dml::GetArray( msr.Objects() );
         // - Features
         const char* featuresFieldNames[ 3 ] = { "Name", "StartColumn", "NumberValues" };
         mxInputArgs[ 1 ] = mxCreateStructMatrix( 1, msr.NumberOfFeatures(), 3, featuresFieldNames );
         auto dipFeatures = msr.Features();
         for( dip::uint ii = 0; ii < dipFeatures.size(); ++ii ) {
            mxSetFieldByNumber( mxInputArgs[ 1 ], ii, 0, dml::GetArray( dipFeatures[ ii ].name ));
            mxSetFieldByNumber( mxInputArgs[ 1 ], ii, 1, dml::GetArray( dipFeatures[ ii ].startColumn + 1 ));
            mxSetFieldByNumber( mxInputArgs[ 1 ], ii, 2, dml::GetArray( dipFeatures[ ii ].numberValues ));
         }
         // - Values
         const char* valuesFieldNames[ 2 ] = { "Name", "Units" };
         mxInputArgs[ 2 ] = mxCreateStructMatrix( 1,  msr.NumberOfValues(), 2, valuesFieldNames );
         auto dipValues = msr.Values();
         for( dip::uint ii = 0; ii <  msr.NumberOfValues(); ++ii ) {
            mxSetFieldByNumber( mxInputArgs[ 2 ], ii, 0, dml::GetArray( dipValues[ ii ].name ));
            mxSetFieldByNumber( mxInputArgs[ 2 ], ii, 1, dml::GetArrayUnicode( dipValues[ ii ].units.String() ));
         }

         // Step 2: create the object
         mexCallMATLAB( 1, plhs, 3, mxInputArgs, "dip_measurement" );

         // Step 3: get a pointer to the data block, and copy the data over
         mxArray* dataArray = mxGetPropertyShared( plhs[ 0 ], 0, "Data" );
         double* data = mxGetPr( dataArray );
         dip::uint step = msr.NumberOfObjects();
         auto objIt = msr.FirstObject();
         do {
            double* d = data;
            auto ftrIt = objIt.FirstFeature();
            do {
               for( auto& value : ftrIt ) {
                  *d = value;
                  d += step;
               }
            } while( ++ftrIt );
            ++data;
         } while( ++objIt );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
