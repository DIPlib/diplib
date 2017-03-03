/*
 * DIPlib 3.0
 * This file contains definitions for function that compute eigenvalues and eigenvectors.
 *
 * (c)2016-2017, Cris Luengo.
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

#include "diplib/library/numeric.h"

#include <Eigen/Eigenvalues>

namespace dip {


void SymmetricEigenValues2D( dfloat const* input, dfloat* lambdas ) {
   Eigen::Map< Eigen::Matrix2d const > matrix( input );
   Eigen::Map< Eigen::Vector2d > eigenvalues( lambdas );
   eigenvalues = matrix.selfadjointView< Eigen::Lower >().eigenvalues();
   if( lambdas[ 0 ] < lambdas[ 1 ] ) {
      std::swap( lambdas[ 0 ], lambdas[ 1 ]);
   }
}

void SymmetricEigenSystem2D( dfloat const* input, dfloat* lambdas, dfloat* vectors ) {
   Eigen::Map< Eigen::Matrix2d const > matrix( input );
   Eigen::SelfAdjointEigenSolver< Eigen::Matrix2d > eigensolver( matrix );
   //if( eigensolver.info() != Eigen::Success ) { abort(); }
   Eigen::Map< Eigen::Vector2d > eigenvalues( lambdas );
   eigenvalues = eigensolver.eigenvalues();
   Eigen::Map< Eigen::Matrix2d > eigenvectors( vectors );
   eigenvectors = eigensolver.eigenvectors();
   if( lambdas[ 0 ] < lambdas[ 1 ] ) {
      std::swap( lambdas[ 0 ], lambdas[ 1 ]);
      std::swap( vectors[ 0 ], vectors[ 2 ]);
      std::swap( vectors[ 1 ], vectors[ 3 ]);
   }
}

void SymmetricEigenValues3D( dfloat const* input, dfloat* lambdas ) {
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

void SymmetricEigenSystem3D( dfloat const* input, dfloat* lambdas, dfloat* vectors) {
   Eigen::Map< Eigen::Matrix3d const > matrix( input );
   Eigen::SelfAdjointEigenSolver< Eigen::Matrix3d > eigensolver( matrix );
   //if( eigensolver.info() != Eigen::Success ) { abort(); }
   Eigen::Map< Eigen::Vector3d > eigenvalues( lambdas );
   eigenvalues = eigensolver.eigenvalues();
   Eigen::Map< Eigen::Matrix3d > eigenvectors( vectors );
   eigenvectors = eigensolver.eigenvectors();
   if( lambdas[ 0 ] < lambdas[ 1 ] ) {
      std::swap( lambdas[ 0 ], lambdas[ 1 ]);
      std::swap( vectors[ 0 ], vectors[ 3 ]);
      std::swap( vectors[ 1 ], vectors[ 4 ]);
      std::swap( vectors[ 2 ], vectors[ 5 ]);
   }
   if( lambdas[ 1 ] < lambdas[ 2 ] ) {
      std::swap( lambdas[ 1 ], lambdas[ 2 ]);
      std::swap( vectors[ 3 ], vectors[ 6 ]);
      std::swap( vectors[ 4 ], vectors[ 7 ]);
      std::swap( vectors[ 5 ], vectors[ 8 ]);
   }
   if( lambdas[ 0 ] < lambdas[ 1 ] ) {
      std::swap( lambdas[ 0 ], lambdas[ 1 ]);
      std::swap( vectors[ 0 ], vectors[ 3 ]);
      std::swap( vectors[ 1 ], vectors[ 4 ]);
      std::swap( vectors[ 2 ], vectors[ 5 ]);
   }
}

void EigenValues( dip::uint n, dfloat const* input, dcomplex* lambdas ) {
   // TODO
}

void EigenValues( dip::uint n, dcomplex const* input, dcomplex* lambdas ) {
   // TODO
}

void EigenSystem( dip::uint n, dfloat const* input, dcomplex* lambdas, dcomplex* vectors ) {
   // TODO
}

void EigenSystem( dip::uint n, dcomplex const* input, dcomplex* lambdas, dcomplex* vectors ) {
   // TODO
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::SymmetricEigenXXX functions") {
   double matrix2[] = { 4, 0, 8 };
   double lambdas[ 3 ];
   double vectors[ 9 ];
   dip::SymmetricEigenValues2DPacked( matrix2, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   dip::SymmetricEigenSystem2DPacked( matrix2, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   DOCTEST_CHECK( vectors[ 0 ] == 0 );
   DOCTEST_CHECK( vectors[ 1 ] == 1 );
   DOCTEST_CHECK( vectors[ 2 ] == 1 );
   DOCTEST_CHECK( vectors[ 3 ] == 0 );
   matrix2[ 0 ] = 8;
   matrix2[ 2 ] = 4;
   dip::SymmetricEigenValues2DPacked( matrix2, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   dip::SymmetricEigenSystem2DPacked( matrix2, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   DOCTEST_CHECK( vectors[ 0 ] == 1 );
   DOCTEST_CHECK( vectors[ 1 ] == 0 );
   DOCTEST_CHECK( vectors[ 2 ] == 0 );
   DOCTEST_CHECK( vectors[ 3 ] == 1 );
   matrix2[ 0 ] = 3;
   matrix2[ 1 ] = -1;
   matrix2[ 2 ] = 3;
   dip::SymmetricEigenValues2DPacked( matrix2, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 4 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   dip::SymmetricEigenSystem2DPacked( matrix2, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 4 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   DOCTEST_CHECK( vectors[ 0 ] == doctest::Approx(  cos( M_PI/4 )) ); // signs might be different here...
   DOCTEST_CHECK( vectors[ 1 ] == doctest::Approx( -sin( M_PI/4 )) );
   DOCTEST_CHECK( vectors[ 2 ] == doctest::Approx(  sin( M_PI/4 )) );
   DOCTEST_CHECK( vectors[ 3 ] == doctest::Approx(  cos( M_PI/4 )) );

   double matrix3[] = { 4, 0, 0, 8, 0, 6 };
   dip::SymmetricEigenValues3DPacked( matrix3, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 6 );
   DOCTEST_CHECK( lambdas[ 2 ] == 4 );
   dip::SymmetricEigenSystem3DPacked( matrix3, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 6 );
   DOCTEST_CHECK( lambdas[ 2 ] == 4 );
   DOCTEST_CHECK( vectors[ 0 ] == 0 );
   DOCTEST_CHECK( vectors[ 1 ] == 1 );
   DOCTEST_CHECK( vectors[ 2 ] == 0 );
   DOCTEST_CHECK( vectors[ 3 ] == 0 );
   DOCTEST_CHECK( vectors[ 4 ] == 0 );
   DOCTEST_CHECK( vectors[ 5 ] == 1 );
   DOCTEST_CHECK( vectors[ 6 ] == 1 );
   DOCTEST_CHECK( vectors[ 7 ] == 0 );
   DOCTEST_CHECK( vectors[ 8 ] == 0 );
}

#endif // DIP__ENABLE_DOCTEST
