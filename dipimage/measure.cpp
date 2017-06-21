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

dip::MeasurementTool* measurementTool = nullptr;

// Destroy MeasurementTool object when MEX-file is removed from memory
static void AtExit(void) {
   if( measurementTool ) {
      //mexPrintf( "Deleting MeasurementTool.\n" );
      delete measurementTool;
      measurementTool = nullptr;
   }
}

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   dml::streambuf streambuf;

   try {

      DML_MIN_ARGS( 1 );

      // Create MeasurementTool object
      if( !measurementTool ) {
         //mexPrintf( "Creating MeasurementTool.\n" );
         measurementTool = new dip::MeasurementTool;
         mexAtExit( AtExit );
      }

      if( mxIsChar( prhs[ 0 ] )) {

         dip::String str = dml::GetString( prhs[ 0 ]);
         if( str == "help" ) {
            DML_MAX_ARGS( 1 );
            std::cout << "\nAvailable measurement features:\n";
            auto features = measurementTool->Features();
            std::cout << features.size() << " features." << std::endl;
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

         dip::Measurement msr = measurementTool->Measure( label, grey, features, objectIDs, connectivity );

         plhs[ 0 ] = mxCreateDoubleMatrix( msr.NumberOfValues(), msr.NumberOfObjects(), mxREAL );
         double* data = mxGetPr( plhs[ 0 ] );
         auto objIt = msr.FirstObject();
         do {
            auto ftrIt = objIt.FirstFeature();
            do {
               for( auto& value : ftrIt ) {
                  * data = value;
                  ++data;
               }
            } while( ++ftrIt );
         } while( ++objIt );
         // TODO: convert to dip_measurement object.
         // TODO: create a dip_measurement object to convert to.
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
