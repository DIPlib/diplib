/*
 * DIPlib 3.0
 * This file contains definitions for function that computes 3D surface area.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Original code written by Jim Mullikin, Pattern Recognition Group.
 */

#include "diplib/library/numeric.h"

#include <Eigen/Eigenvalues>


namespace dip {


void SymmetricEigenValues2D( double const* input, double* lambdas ) {
   Eigen::Map< Eigen::Matrix2d const > matrix( input );
   Eigen::Map< Eigen::Vector2d > eigenvalues( lambdas );
   eigenvalues = matrix.selfadjointView< Eigen::Lower >().eigenvalues();
   if( lambdas[ 0 ] < lambdas[ 1 ] ) {
      std::swap( lambdas[ 0 ], lambdas[ 1 ]);
   }
}

void SymmetricEigenSystem2D( double const* input, double* lambdas, double* v1, double* v2 ) {
   // TODO
}

void SymmetricEigenValues3D( double const* input, double* lambdas ) {
   Eigen::Map< Eigen::Matrix3d const > matrix( input );
   Eigen::Map< Eigen::Vector3d > eigenvalues( lambdas );
   eigenvalues = matrix.selfadjointView< Eigen::Lower >().eigenvalues();
   if( lambdas[ 0 ] < lambdas[ 1 ] ) {
      std::swap( lambdas[ 0 ], lambdas[ 1 ]);
   }
   if( lambdas[ 1 ] < lambdas[ 2 ] ) {
      std::swap( lambdas[ 1 ], lambdas[ 2 ]);
   }
   if( lambdas[ 0 ] < lambdas[ 1 ] ) {
      std::swap( lambdas[ 0 ], lambdas[ 1 ]);
   }
}

void SymmetricEigenSystem3D( double const* input, double* lambdas, double* v1, double* v2, double* v3 ) {
   // TODO
}

void EigenValues2D( double const* input, double* lambdas ) {
   // TODO
}

void EigenSystem2D( double const* input, double* lambdas, double* v1, double* v2 ) {
   // TODO
}

void EigenValues3D( double const* input, double* lambdas ) {
   // TODO
}

void EigenSystem3D( double const* input, double* lambdas, double* v1, double* v2, double* v3 ) {
   // TODO
}

void EigenValues( dip::uint n, double const* input, double* lambdas ) {
   // TODO
}

void EigenSystem( dip::uint n, double const* input, double* lambdas, double* vectors ) {
   // TODO
}


} // namespace dip
