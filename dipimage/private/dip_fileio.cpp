/*
 * DIPimage 3.0
 *
 * (c)2017-2018, Cris Luengo.
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

#include "diplib/file_io.h"

namespace {

void readics( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 4 );
   dip::String const& filename = dml::GetString( prhs[ 0 ] );
   dip::UnsignedArray origin = nrhs > 1 ? dml::GetUnsignedArray( prhs[ 1 ] ) : dip::UnsignedArray{};
   dip::UnsignedArray sizes = nrhs > 2 ? dml::GetUnsignedArray( prhs[ 2 ] ) : dip::UnsignedArray{};
   dip::UnsignedArray spacing = nrhs > 3 ? dml::GetUnsignedArray( prhs[ 3 ] ) : dip::UnsignedArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::FileInformation fileInformation = dip::ImageReadICS( out, filename, origin, sizes, spacing );
   // NOTE: "fast" option is useless here, as we cannot change the strides of out.
   plhs[ 0 ] = dml::GetArray( out );
   if( nlhs > 1 ) {
      plhs[ 1 ] = dml::GetArray( fileInformation );
   }
}

void readtiff( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 6 );
   dip::String const& filename = dml::GetString( prhs[ 0 ] );
   dip::Range imageNumbers = nrhs > 1 ? dml::GetRange( prhs[ 1 ] ) : dip::Range{ 0 };

   dip::UnsignedArray origin = nrhs > 2 ? dml::GetUnsignedArray( prhs[ 2 ] ) : dip::UnsignedArray{};
   dip::UnsignedArray sizes = nrhs > 3 ? dml::GetUnsignedArray( prhs[ 3 ] ) : dip::UnsignedArray{};
   dip::UnsignedArray spacing = nrhs > 4 ? dml::GetUnsignedArray( prhs[ 4 ] ) : dip::UnsignedArray{};
   dip::Range channels = nrhs > 5 ? dml::GetRange( prhs[ 5 ] ) : dip::Range {};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::FileInformation fileInformation = dip::ImageReadTIFF( out, filename, imageNumbers, origin, sizes, spacing, channels );
   plhs[ 0 ] = dml::GetArray( out );
   if( nlhs > 1 ) {
      plhs[ 1 ] = dml::GetArray( fileInformation );
   }
}

void readtiffseries( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 1 );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::StringArray const& filenames = dml::GetStringArray( prhs[ 0 ] );
   dip::ImageReadTIFFSeries( out, filenames );
   plhs[ 0 ] = dml::GetArray( out );
}

void writeics( int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 5 );
   dip::Image image = dml::GetImage( prhs[ 0 ] );
   dip::String const& filename = dml::GetString( prhs[ 1 ] );
   dip::StringArray history = nrhs > 2 ? dml::GetStringArray( prhs[ 2 ] ) : dip::StringArray{};
   dip::uint significantBits = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 0;
   dip::StringSet options = nrhs > 4 ? dml::GetStringSet( prhs[ 4 ] ) : dip::StringSet{ dip::S::FAST };
   dip::ImageWriteICS( image, filename, history, significantBits, options );
}

void writetiff( int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 4 );
   dip::Image image = dml::GetImage( prhs[ 0 ] );
   dip::String const& filename = dml::GetString( prhs[ 1 ] );
   dip::String compression = nrhs > 2 ? dml::GetString( prhs[ 2 ] ) : dip::String{};
   dip::uint jpegLevel = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 80;
   dip::ImageWriteTIFF( image, filename, compression, jpegLevel );
}

} // namespace

// Gateway function

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 2 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      prhs += 1;
      nrhs -= 1;

      if( function == "readics" ) {
         readics( nlhs, plhs, nrhs, prhs );
      } else if( function == "readtiff" ) {
         readtiff( nlhs, plhs, nrhs, prhs );
      } else if( function == "readtiffseries" ) {
         readtiffseries( plhs, nrhs, prhs );
      } else if( function == "writeics" ) {
         writeics( nrhs, prhs );
      } else if( function == "writetiff" ) {
         writetiff( nrhs, prhs );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

   } DML_CATCH
}
