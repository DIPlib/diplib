/*
 * DIPlib 3.0
 * This file contains definitions for cross correlation
 *
 * (c)2017, Cris Luengo.
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

#include "diplib.h"
#include "diplib/analysis.h"
#include "diplib/math.h"
#include "diplib/linear.h"
#include "diplib/generic_iterators.h"

namespace dip {

void StructureTensor(
      Image const& in,
      Image const& mask,
      Image& out,
      FloatArray const& gradientSigmas,
      FloatArray const& tensorSigmas,
      String const& method,
      StringArray const& boundaryCondition,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   Image tmp;
   if( mask.IsForged() ) {
      tmp.ReForge( in.Sizes(), in.Dimensionality(), DataType::SuggestFlex( in.DataType() ), Option::AcceptDataTypeChange::DO_ALLOW );
      auto it = ImageTensorIterator( tmp );
      for( dip::uint ii = 0; ii < in.Dimensionality(); ++ii ) {
         DIP_STACK_TRACE_THIS( NormalizedDifferentialConvolution( in, mask, *it, ii, gradientSigmas, method, boundaryCondition, truncation ));
         ++it;
      }
   } else {
      DIP_STACK_TRACE_THIS( Gradient( in, tmp, gradientSigmas, method, boundaryCondition, {}, truncation ));
   }
   Multiply( tmp, Transpose( tmp ), out );
   DIP_STACK_TRACE_THIS( Gauss( out, out, tensorSigmas, {}, method, boundaryCondition, truncation ));
}

void StructureTensorAnalysis2D(
      Image const& in,
      Image* l1,
      Image* l2,
      Image* orientation,
      Image* energy,
      Image* anisotropy1,
      Image* anisotropy2,
      Image* curvature
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( in.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !in.Tensor().IsSymmetric() || ( in.TensorElements() != 3 ), "Input must be a 2x2 symmetric tensor image" );
   Image ll;
   Image tempOrientation;
   if( orientation || curvature) {
      // Curvature needs orientation
      if( !orientation )
         orientation = &tempOrientation;
      // Compute eigenvectors also
      Image vv;
      EigenDecomposition( in, ll, vv );
      Orientation( vv.TensorColumn( 0 ), *orientation );
   } else {
      // We only need eigenvalues
      Eigenvalues( in, ll );
   }
   if( l1 ) {
      *l1 = ll[ 0 ];
   }
   if( l2 ) {
      *l2 = ll[ 1 ];
   }
   if( energy ) {
      Add( ll[ 0 ], ll[ 1 ], *energy );
   }
   if( anisotropy1 ) {
      Image tmpval;
      if( energy ) {
         tmpval = energy->QuickCopy();
      } else {
         tmpval = ll[ 0 ] + ll[ 1 ];
      }
      Subtract( ll[ 0 ], ll[ 1 ], *anisotropy1 );
      SafeDivide( *anisotropy1, tmpval, *anisotropy1 );
   }
   if( anisotropy2 ) {
      Divide( ll[ 1 ], ll[ 0 ], *anisotropy2 );
      Subtract( 1, *anisotropy2, *anisotropy2, anisotropy2->DataType() );
      // *anisotropy2 = ( ll[0] == 0 ) ? 0 : *anisotropy2;
      Select( ll[ 0 ], Image{ 0.0 }, Image( 0.0, anisotropy2->DataType() ), *anisotropy2, *anisotropy2, "==" );
   }
   if( curvature ) {
      // phidx = (-sin( 2 * phi )*dx( cos( 2 * phi ), 1 ) + cos( 2 * phi )*dx( sin( 2 * phi ), 1 ))
      // phidy = (-sin( 2 * phi )*dy( cos( 2 * phi ), 1 ) + cos( 2 * phi )*dy( sin( 2 * phi ), 1 ))
      // out = 0.5*(-sin( phi )*phidx + cos( phi )*phidy);
      // TODO: restructure the computation to lower memory consumption
      Image cos2phi, sin2phi;
      {
         Image two_phi = dip::Multiply( *orientation, 2.0 );
         cos2phi = dip::Cos( two_phi );
         sin2phi = dip::Sin( two_phi );
      }
      // TODO: handle derivative sigmas and other parameters
      Image phidx, phidy;
      {
         Image dx_cos2phi = dip::Derivative( cos2phi, { 1, 0 } );
         Image dx_sin2phi = dip::Derivative( sin2phi, { 1, 0 } );
         phidx = -sin2phi*dx_cos2phi + cos2phi*dx_sin2phi;
      }
      {
         Image dy_cos2phi = dip::Derivative( cos2phi, { 0, 1 } );
         Image dy_sin2phi = dip::Derivative( sin2phi, { 0, 1 } );
         phidy = -sin2phi*dy_cos2phi + cos2phi*dy_sin2phi;
      }
      Image cosphi = dip::Cos( *orientation );
      Image sinphi = dip::Sin( *orientation );
      dip::Multiply( -sinphi*phidx + cosphi*phidy, 0.5, *curvature );
   }
}

void StructureTensorAnalysis3D(
      Image const& in,
      Image* l1,
      Image* phi1,
      Image* theta1,
      Image* l2,
      Image* phi2,
      Image* theta2,
      Image* l3,
      Image* phi3,
      Image* theta3,
      Image* energy,
      Image* cylindrical,
      Image* planar
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( in.Dimensionality() != 3, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !in.Tensor().IsSymmetric() || ( in.TensorElements() != 6 ), "Input must be a 3x3 symmetric tensor image" );
   Image ll;
   if( phi1 || theta1 || phi2 || theta2 || phi3 || theta3 ) {
      // Compute eigenvectors also
      Image vv;
      EigenDecomposition( in, ll, vv );
      Image tmp;
      if( phi1 || theta1 ) {
         Orientation( vv.TensorColumn( 0 ), tmp );
         if( phi1   ) { *phi1   = tmp[ 0 ]; }
         if( theta1 ) { *theta1 = tmp[ 1 ]; }
      }
      if( phi2 || theta2 ) {
         Orientation( vv.TensorColumn( 1 ), tmp );
         if( phi2   ) { *phi2   = tmp[ 0 ]; }
         if( theta2 ) { *theta2 = tmp[ 1 ]; }
      }
      if( phi3 || theta3 ) {
         Orientation( vv.TensorColumn( 2 ), tmp );
         if( phi3   ) { *phi3   = tmp[ 0 ]; }
         if( theta3 ) { *theta3 = tmp[ 1 ]; }
      }
   } else {
      // We only need eigenvalues
      Eigenvalues( in, ll );
   }
   if( l1 ) {
      *l1 = ll[ 0 ];
   }
   if( l2 ) {
      *l2 = ll[ 1 ];
   }
   if( l3 ) {
      *l3 = ll[ 3 ];
   }
   if( energy ) {
      Add( ll[ 0 ], ll[ 1 ], *energy );
      Add( *energy, ll[ 2 ], *energy );
   }
   if( cylindrical ) {
      Image tmpval;
      if( energy ) {
         tmpval = energy->QuickCopy();
      } else {
         tmpval = ll[ 1 ] + ll[ 2 ];
      }
      Subtract( ll[ 1 ], ll[ 2 ], *cylindrical );
      SafeDivide( *cylindrical, tmpval, *cylindrical );
   }
   if( planar ) {
      Image tmpval;
      if( energy ) {
         tmpval = energy->QuickCopy();
      } else {
         tmpval = ll[ 0 ] + ll[ 1 ];
      }
      Subtract( ll[ 0 ], ll[ 1 ], *planar );
      SafeDivide( *planar, tmpval, *planar );
   }
}

void StructureTensorAnalysis(
      Image const& in,
      ImageRefArray& out,
      StringArray const& outputs
) {
   dip::uint nOut = out.size();
   DIP_THROW_IF( outputs.size() != nOut, E::ARRAY_SIZES_DONT_MATCH );
   if( in.Dimensionality() == 2 ) {
      Image* l1 = nullptr;
      Image* l2 = nullptr;
      Image* orientation = nullptr;
      Image* energy = nullptr;
      Image* anisotropy1 = nullptr;
      Image* anisotropy2 = nullptr;
      Image* curvature = nullptr;
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         if( outputs[ ii ] == "l1" ) {
            l1 = &out[ ii ].get();
         } else if( outputs[ ii ] == "l2" ) {
            l2 = &out[ ii ].get();
         } else if( outputs[ ii ] == "orientation" ) {
            orientation = &out[ ii ].get();
         } else if( outputs[ ii ] == "energy" ) {
            energy = &out[ ii ].get();
         } else if(( outputs[ ii ] == "anisotropy1" ) || ( outputs[ ii ] == "anisotropy" )) {
            anisotropy1 = &out[ ii ].get();
         } else if( outputs[ ii ] == "anisotropy2" ) {
            anisotropy2 = &out[ ii ].get();
         } else if( outputs[ ii ] == "curvature" ) {
            curvature = &out[ ii ].get();
         } else {
            DIP_THROW_INVALID_FLAG( outputs[ ii ] );
         }
      }
      DIP_STACK_TRACE_THIS( StructureTensorAnalysis2D( in, l1, l2, orientation, energy, anisotropy1, anisotropy2, curvature ));
   } else {
      Image* l1 = nullptr;
      Image* phi1 = nullptr;
      Image* theta1 = nullptr;
      Image* l2 = nullptr;
      Image* phi2 = nullptr;
      Image* theta2 = nullptr;
      Image* l3 = nullptr;
      Image* phi3 = nullptr;
      Image* theta3 = nullptr;
      Image* energy = nullptr;
      Image* cylindrical = nullptr;
      Image* planar = nullptr;
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         if( outputs[ ii ] == "l1" ) {
            l1 = &out[ ii ].get();
         } else if( outputs[ ii ] == "phi1" ) {
            phi1 = &out[ ii ].get();
         } else if( outputs[ ii ] == "theta1" ) {
            theta1 = &out[ ii ].get();
         } else if( outputs[ ii ] == "l2" ) {
            l2 = &out[ ii ].get();
         } else if( outputs[ ii ] == "phi2" ) {
            phi2 = &out[ ii ].get();
         } else if( outputs[ ii ] == "theta2" ) {
            theta2 = &out[ ii ].get();
         } else if( outputs[ ii ] == "l3" ) {
            l3 = &out[ ii ].get();
         } else if( outputs[ ii ] == "phi3" ) {
            phi3 = &out[ ii ].get();
         } else if( outputs[ ii ] == "theta3" ) {
            theta3 = &out[ ii ].get();
         } else if( outputs[ ii ] == "energy" ) {
            energy = &out[ ii ].get();
         } else if( outputs[ ii ] == "cylindrical" ) {
            cylindrical = &out[ ii ].get();
         } else if( outputs[ ii ] == "planar" ) {
            planar = &out[ ii ].get();
         } else {
            DIP_THROW_INVALID_FLAG( outputs[ ii ] );
         }
      }
      DIP_STACK_TRACE_THIS( StructureTensorAnalysis3D( in, l1, phi1, theta1, l2, phi2, theta2, l3, phi3, theta3, energy, cylindrical, planar ));
   }
}

} // namespace dip
