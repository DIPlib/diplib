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

#include "diplib/geometry.h"
#include "diplib/analysis.h"

namespace {

void affine_trans( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::FloatArray matrix;
   int index;
   if( nrhs >= 4 ) {
      // affine_trans( image_in, zoom, translation, angle [, method] )
      DIP_THROW_IF( in.Dimensionality() != 2, "When given ZOOM, TRANSLATION and ANGLE, the image must be 2D" );
      dip::FloatArray zoom = dml::GetFloatArray( prhs[ 1 ] );
      if( zoom.size() == 1 ) {
         zoom.push_back( zoom[ 0 ] );
      }
      DIP_THROW_IF( zoom.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
      dip::FloatArray translation = dml::GetFloatArray( prhs[ 2 ] );
      if( translation.size() == 1 ) {
         translation.push_back( translation[ 0 ] );
      }
      DIP_THROW_IF( translation.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
      dip::dfloat angle = dml::GetFloat( prhs[ 3 ] );
      matrix.resize( 6, 0 );
      matrix[ 0 ] =  zoom[ 0 ] * std::cos( angle );
      matrix[ 1 ] =  zoom[ 1 ] * std::sin( angle );
      matrix[ 2 ] = -zoom[ 0 ] * std::sin( angle );
      matrix[ 3 ] =  zoom[ 1 ] * std::cos( angle );
      matrix[ 4 ] = translation[ 0 ];
      matrix[ 5 ] = translation[ 1 ];
      index = 4;
      if( nlhs > 1 ) {
         plhs[ 1 ] = mxCreateDoubleMatrix( 3, 3, mxREAL );
         double* out = mxGetPr( plhs[ 1 ] );
         out[ 0 ] = matrix[ 0 ];
         out[ 1 ] = matrix[ 1 ];
         out[ 2 ] = 0.0;
         out[ 3 ] = matrix[ 2 ];
         out[ 4 ] = matrix[ 3 ];
         out[ 5 ] = 0.0;
         out[ 6 ] = matrix[ 4 ];
         out[ 7 ] = matrix[ 5 ];
         out[ 8 ] = 1.0;
      }
   } else {
      // affine_trans( image_in, R [, method] )
      dip::uint nDims = in.Dimensionality();
      mxArray const* R = prhs[ 1 ];
      DIP_THROW_IF( mxGetM( R ) != nDims, "Matrix R of wrong size" );
      dip::uint cols = mxGetN( R );
      DIP_THROW_IF(( cols != nDims ) && ( cols != nDims + 1 ), "Matrix R of wrong size" );
      double const* Rptr = mxGetPr( R );
      matrix.resize( nDims * cols );
      std::copy( Rptr, Rptr + nDims * cols, matrix.begin() );
      index = 2;
   }
   dip::String method = nrhs > index ? dml::GetString( prhs[ index ] ) : dip::S::LINEAR;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::AffineTransform( in, out, matrix, method );
   plhs[ 0 ] = dml::GetArray( out );
}

void resample( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::FloatArray zoom = nrhs > 1 ? dml::GetFloatArray( prhs[ 1 ] ) : dip::FloatArray{ 2.0 };
   dip::FloatArray shift = nrhs > 2 ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 0.0 };
   dip::String method = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::String{};
   dip::StringArray boundaryCondition = nrhs > 4 ? dml::GetStringArray( prhs[ 4 ] ) : dip::StringArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Resampling( in, out, zoom, shift, method, boundaryCondition );
   plhs[ 0 ] = dml::GetArray( out );
}

void rotation( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::dfloat angle = dml::GetFloat( prhs[ 1 ] );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 2, "Defined only for images with 2 or more dimensions" );
   dip::uint dimension1 = 0;
   dip::uint dimension2 = 1;
   dip::String method{};
   dip::String boundaryCondition = dip::S::ADD_ZEROS;
   if(( nrhs > 2 ) && ( mxIsChar( prhs[ 2 ] ))) {
      // rotation(image_in,angle,interpolation_method,boundary_condition)
      DIP_THROW_IF( nDims != 2, "Missing argument before INTERPOLATION_METHOD" );
      DML_MAX_ARGS( 4 );
      method = dml::GetString( prhs[ 2 ] );
      if( nrhs > 3 ) {
         boundaryCondition = dml::GetString( prhs[ 3 ] );
      }
   } else if(( nrhs == 3 ) || (( nrhs > 3 ) && mxIsChar( prhs[ 3 ]))) {
      // rotation(image_in,angle,axis,interpolation_method,boundary_condition)
      DIP_THROW_IF( nDims > 3, "For images with more than 3 dimensions, use the syntax with two DIMENSION parameters" );
      DML_MAX_ARGS( 5 );
      dip::uint axis = dml::GetUnsigned( prhs[ 2 ] );
      if( nDims == 3 ) { // Ignore value if nDims == 2.
         switch( axis ) {
            case 1:
               dimension1 = 1;
               dimension2 = 2;
               break;
            case 2:
               dimension1 = 2;
               dimension2 = 0;
               break;
            case 3:
               dimension1 = 0;
               dimension2 = 1;
               break;
            default:
               DIP_THROW( dip::E::INVALID_PARAMETER );
         }
      }
      if( nrhs > 3 ) {
         method = dml::GetString( prhs[ 3 ] );
      }
      if( nrhs > 4 ) {
         boundaryCondition = dml::GetString( prhs[ 4 ] );
      }
   } else {
      // rotation(image_in,angle,dimension1,dimension2,interpolation_method,boundary_condition)
      DML_MAX_ARGS( 6 );
      if( nrhs > 2 ) {
         dimension1 = dml::GetUnsigned( prhs[ 2 ] );
         DIP_THROW_IF( dimension1 == 0, dip::E::INVALID_PARAMETER );
         --dimension1;
      }
      if( nrhs > 3 ) {
         dimension2 = dml::GetUnsigned( prhs[ 3 ] );
         DIP_THROW_IF( dimension2 == 0, dip::E::INVALID_PARAMETER );
         --dimension2;
      }
      if( nrhs > 4 ) {
         method = dml::GetString( prhs[ 4 ] );
      }
      if( nrhs > 5 ) {
         boundaryCondition = dml::GetString( prhs[ 5 ] );
      }
   }
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Rotation( in, out, angle, dimension1, dimension2, method, boundaryCondition );
   plhs[ 0 ] = dml::GetArray( out );
}

void skew( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 3 );
   DML_MAX_ARGS( 6 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::dfloat shear = dml::GetFloat( prhs[ 1 ] );
   dip::uint skew = dml::GetUnsigned( prhs[ 2 ] );
   DIP_THROW_IF( skew == 0, dip::E::INVALID_PARAMETER );
   --skew;
   dip::uint axis;
   if( nrhs > 3 ) {
      axis = dml::GetUnsigned( prhs[ 3 ] );
      DIP_THROW_IF( axis == 0, dip::E::INVALID_PARAMETER );
      --axis;
   } else {
      axis = skew == 0 ? 1 : 0;
   }
   dip::String method = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::String{};
   dip::String boundaryCondition = nrhs > 5 ? dml::GetString( prhs[ 5 ] ) : dip::String{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Skew( in, out, shear, skew, axis, method, boundaryCondition );
   // TODO: Implement also the other form of skew, using `shearArray`.
   plhs[ 0 ] = dml::GetArray( out );
}

void wrap( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::IntegerArray shift = dml::GetIntegerArray( prhs[ 1 ] );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::Wrap( in, out, shift );
   plhs[ 0 ] = dml::GetArray( out );
}

void findshift( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 5 );
   dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
   dip::Image const in2 = dml::GetImage( prhs[ 1 ] );
   dip::String method = "integer only";
   if( nrhs > 2 ) {
      method = dml::GetString( prhs[ 2 ] );
      if(( method == "integer" ) || ( method == "integer only" )) {
         method = "integer only";
      } else if( method == "ffts" ) {
         method = "CPF";
      } else if( method == "grs" ) {
         method = "MTS";
      } else {
         dml::ToUpper( method );
      }
   }
   dip::dfloat parameter = nrhs > 3 ? dml::GetFloat( prhs[ 3 ] ) : 0;
   dip::UnsignedArray maxShift = nrhs > 4 ? dml::GetUnsignedArray( prhs[ 4 ] ) : dip::UnsignedArray{};
   auto out = dip::FindShift( in1, in2, method, parameter, maxShift );
   plhs[ 0 ] = dml::GetArray( out );
}

void fmmatch( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 3 );
   dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
   dip::Image const in2 = dml::GetImage( prhs[ 1 ] );
   dip::String interpolationMethod = nrhs > 2 ? dml::GetString( prhs[ 2 ] ) : dip::S::LINEAR;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   auto matrix = dip::FourierMellinMatch2D( in1, in2, out, interpolationMethod );
   plhs[ 0 ] = dml::GetArray( out );
   if( nlhs > 1 ) {
      DIP_ASSERT( matrix.size() == 6 );
      plhs[ 1 ] = mxCreateDoubleMatrix( 2, 3, mxREAL );
      double* ptr = mxGetPr( plhs[ 1 ] );
      for( dip::uint ii = 0; ii < 6; ++ii ) {
         ptr[ ii ] = matrix[ ii ];
      }
   }
}

void get_subpixel( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::FloatCoordinateArray coords = dml::GetFloatCoordinateArray( prhs[ 1 ] );
   dip::String mode = "linear";
   if( nrhs > 2 ) {
      mode = dml::GetString( prhs[ 2 ] );
      if( mode == "spline" ) {
         mode = "cubic";
      }
   }
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   out.SetDataType( in.DataType().IsComplex() ? dip::DT_DCOMPLEX : dip::DT_DFLOAT );
   out.SetSizes( { in.TensorElements(), coords.size() } ); // Creates 1x1xNxT matrix
   out.Forge();
   out.SpatialToTensor( 0 ); // Out is 1D image with right number of tensor elements
   out.Protect();
   dip::ResampleAt( in, out, coords, mode );
   out.TensorToSpatial( 0 ); // Return to original shape
   plhs[ 0 ] = dml::GetArrayAsArray( out );
   // the image has 2, 3 or 4 dimensions, we want to get rid of the first two singleton dimension
   mwSize nDims = mxGetNumberOfDimensions( plhs[ 0 ] );
   const mwSize* dims = mxGetDimensions( plhs[ 0 ] );
   DIP_ASSERT( dims[ 0 ] == 1 );
   DIP_ASSERT( dims[ 1 ] == 1 );
   mwSize newDims[ 2 ] = { nDims > 2 ? dims[ 2 ] : 1, nDims > 3 ? dims[ 3 ] : 1 };
   mxSetDimensions( plhs[ 0 ], newDims, 2 );
}

void subpixlocation( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::CoordinateArray coords = dml::GetCoordinateArray( prhs[ 1 ] );
   dip::String method = dip::S::PARABOLIC_SEPARABLE;
   if( nrhs > 2 ) {
      method = dml::GetString( prhs[ 2 ] );
      // Method names are different in the DIPimage interface...
      if(( method == "parabolic nonseparable" ) || ( method == "parabolic_nonseparable" )) {
         method = dip::S::PARABOLIC;
      } else if(( method == "gaussian nonseparable" ) || ( method == "gaussian_nonseparable" )) {
         method = dip::S::GAUSSIAN;
      } else if( method == "parabolic" ) {
         method = dip::S::PARABOLIC_SEPARABLE;
      } else if( method == "gaussian" ) {
         method = dip::S::GAUSSIAN_SEPARABLE;
      }
   }
   dip::String polarity = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::S::MAXIMUM;
   dip::uint N = coords.size();
   dip::uint nDims = in.Dimensionality();
   plhs[ 0 ] = mxCreateDoubleMatrix( N, nDims, mxREAL );
   double* coords_data = mxGetPr( plhs[ 0 ] );
   double* vals_data = nullptr;
   if( nlhs > 1 ) {
      plhs[ 1 ] = mxCreateDoubleMatrix( N, 1, mxREAL );
      vals_data = mxGetPr( plhs[ 1 ] );
   }
   for( dip::uint ii = 0; ii < N; ++ii ) {
      dip::SubpixelLocationResult loc;
      bool use = true;
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         if(( coords[ ii ][ jj ] == 0 ) || ( coords[ ii ][ jj ] >= in.Size( jj ) - 1 )) {
            use = false;
         }
      }
      if( use ) {
         loc = dip::SubpixelLocation( in, coords[ ii ], polarity, method );
      } else {
         loc.coordinates = dip::FloatArray( coords[ ii ] );
         loc.value = 0.0;
      }
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         coords_data[ jj * N ] = loc.coordinates[ jj ];
      }
      ++coords_data;
      if( vals_data ) {
         *vals_data = loc.value;
         ++vals_data;
      }
   }
}

} // namespace

// Gateway function

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 2 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      prhs += 1;
      nrhs -= 1;

      if( function == "affine_trans" ) {
         affine_trans( nlhs, plhs, nrhs, prhs );
      } else if( function == "resample" ) {
         resample( plhs, nrhs, prhs );
      } else if( function == "rotation" ) {
         rotation( plhs, nrhs, prhs );
      } else if( function == "skew" ) {
         skew( plhs, nrhs, prhs );
      } else if( function == "wrap" ) {
         wrap( plhs, nrhs, prhs );

      } else if( function == "findshift" ) {
         findshift( plhs, nrhs, prhs );
      } else if( function == "fmmatch" ) {
         fmmatch( nlhs, plhs, nrhs, prhs );

      } else if( function == "get_subpixel" ) {
         get_subpixel( plhs, nrhs, prhs );
      } else if( function == "subpixlocation" ) {
         subpixlocation( nlhs, plhs, nrhs, prhs );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

   } DML_CATCH
}
