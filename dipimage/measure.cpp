/*
 * (c)2017-2021, Cris Luengo.
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
            std::cout << features.size() << " measurement features available:\n";
            for( auto const& feature : features ) {
               std::cout << " - '" << feature.name << "': " << feature.description;
               if( feature.needsGreyValue ) {
                  std::cout << " *";
               }
               std::cout << '\n';
            }
            std::cout << "Features marked with a \"*\" require a grey-value input image.\n";
            return;
         }
         if( str == "features" ) {
            DML_MAX_ARGS( 1 );
            auto features = measurementTool.Features();
            char const* fields[ 2 ] = { "name", "description" };
            plhs[ 0 ] = mxCreateStructMatrix( features.size(), 1, 2, fields );
            for( dip::uint ii = 0; ii < features.size(); ++ii ) {
               mxSetFieldByNumber( plhs[ 0 ], ii, 0, dml::GetArray( features[ ii ].name ));
               mxSetFieldByNumber( plhs[ 0 ], ii, 1, dml::GetArray( features[ ii ].description ));
            }
            return;
         }
         DIP_THROW( "Unrecognized option: " + str );
      }

      DML_MAX_ARGS( 5 );

      dip::Image label = dml::GetImage( prhs[ 0 ] ); // Non-const, we might want to modify the image (not the data pointed at)
      dip::Image const grey = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image();
      dip::StringArray features;
      if( nrhs > 2 ) {
         features = dml::GetStringArray( prhs[ 2 ] );
         // Find known feature names
         auto infoArray = measurementTool.Features();
         // Put lower-case version of names in a map
         tsl::robin_map< dip::String, dip::uint > knownFeatures;
         for( dip::uint ii = 0; ii < infoArray.size(); ++ii ) {
            dip::String name = infoArray[ ii ].name;
            dip::ToLowerCase( name );
            knownFeatures.emplace( name, ii );
         }
         // Put in aliases for backwards compatibility
         auto it = knownFeatures.find( "standarddeviation" );
         if( it != knownFeatures.end() ) {
            knownFeatures.emplace( "stddev", it.value() );
         }
         it = knownFeatures.find( "statistics" );
         if( it != knownFeatures.end() ) {
            knownFeatures.emplace( "skewness", it.value() );
            knownFeatures.emplace( "excesskurtosis", it.value() );
         }
         it = knownFeatures.find( "mass" );
         if( it != knownFeatures.end() ) {
            knownFeatures.emplace( "sum", it.value() );
         }
         // Find requested features in map, using case-insensitive search, and copy name with correct case
         for( auto& f : features ) {
            dip::ToLowerCase( f );
            it = knownFeatures.find( f );
            DIP_THROW_IF( it == knownFeatures.end(), "Feature name not recognized" );
            f = infoArray[ it.value() ].name;
         }
      } else {
         features = { "Size" };
      }
      dip::UnsignedArray objectIDs;
      if( nrhs > 3 ) {
         objectIDs = dml::GetUnsignedArray( prhs[ 3 ] );
      }
      dip::uint connectivity = nrhs > 4 ? dml::GetUnsigned( prhs[ 4 ] ) : 0;

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
      char const* featuresFieldNames[ 3 ] = { "Name", "StartColumn", "NumberValues" };
      mxInputArgs[ 1 ] = mxCreateStructMatrix( 1, msr.NumberOfFeatures(), 3, featuresFieldNames );
      auto dipFeatures = msr.Features();
      for( dip::uint ii = 0; ii < dipFeatures.size(); ++ii ) {
         mxSetFieldByNumber( mxInputArgs[ 1 ], ii, 0, dml::GetArray( dipFeatures[ ii ].name ));
         mxSetFieldByNumber( mxInputArgs[ 1 ], ii, 1, dml::GetArray( dipFeatures[ ii ].startColumn + 1 ));
         mxSetFieldByNumber( mxInputArgs[ 1 ], ii, 2, dml::GetArray( dipFeatures[ ii ].numberValues ));
      }
      // - Values
      char const* valuesFieldNames[ 2 ] = { "Name", "Units" };
      mxInputArgs[ 2 ] = mxCreateStructMatrix( 1,  msr.NumberOfValues(), 2, valuesFieldNames );
      auto dipValues = msr.Values();
      for( dip::uint ii = 0; ii <  msr.NumberOfValues(); ++ii ) {
         mxSetFieldByNumber( mxInputArgs[ 2 ], ii, 0, dml::GetArray( dipValues[ ii ].name ));
         mxSetFieldByNumber( mxInputArgs[ 2 ], ii, 1, dml::GetArrayUnicode( dipValues[ ii ].units.StringUnicode() ));
      }

      // Step 2: create the object
      mexCallMATLAB( 1, plhs, 3, mxInputArgs, "dip_measurement" );

      // Step 3: copy the data over
      if( !msr.IsForged() ) {
         // There are no samples to copy over, we're done
      } else if( msr.DataSize() == 1 ) {
         // Create a scalar array and replace the data block
         mxArray* dataArray = mxCreateDoubleScalar( msr.FirstObject().FirstFeature()[ 0 ] );
         mxSetProperty( plhs[ 0 ], 0, "Data", dataArray );
      } else {
         // Get a pointer to the data block and write the data into it -- this doesn't work if Data is a scalar
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

   } DML_CATCH
}
