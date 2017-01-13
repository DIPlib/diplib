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

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::SymmetricEigenXXX functions") {
   double matrix2[] = { 4, 0, 8 };
   double out[ 3 ];
   //double v1[ 3 ];
   //double v2[ 3 ];
   //double v3[ 3 ];
   SymmetricEigenValues2DPacked( matrix2, out );
   DOCTEST_CHECK( out[ 0 ] == 8 );
   DOCTEST_CHECK( out[ 1 ] == 4 );
   //SymmetricEigenSystem2DPacked( matrix2, out, v1, v2 );
   //DOCTEST_CHECK( out[ 1 ] == 8 );
   //DOCTEST_CHECK( out[ 2 ] == 4 );
   //DOCTEST_CHECK( v1[ 1 ] == 1 );
   //DOCTEST_CHECK( v1[ 2 ] == 0 );
   //DOCTEST_CHECK( v2[ 1 ] == 0 );
   //DOCTEST_CHECK( v2[ 2 ] == 1 );
   matrix2[ 0 ] = 8;
   matrix2[ 2 ] = 4;
   SymmetricEigenValues2DPacked( matrix2, out );
   DOCTEST_CHECK( out[ 0 ] == 8 );
   DOCTEST_CHECK( out[ 1 ] == 4 );
   //SymmetricEigenSystem2DPacked( matrix2, out, v1, v2 );
   //DOCTEST_CHECK( out[ 1 ] == 8 );
   //DOCTEST_CHECK( out[ 2 ] == 4 );
   //DOCTEST_CHECK( v1[ 1 ] == 1 );
   //DOCTEST_CHECK( v1[ 2 ] == 0 );
   //DOCTEST_CHECK( v2[ 1 ] == 0 );
   //DOCTEST_CHECK( v2[ 2 ] == 1 );
   matrix2[ 0 ] = 3;
   matrix2[ 1 ] = -1;
   matrix2[ 2 ] = 3;
   SymmetricEigenValues2DPacked( matrix2, out );
   DOCTEST_CHECK( out[ 0 ] == 4 );
   DOCTEST_CHECK( out[ 1 ] == 2 );
   //SymmetricEigenSystem2DPacked( matrix2, out, v1, v2 );
   //DOCTEST_CHECK( out[ 1 ] == 4 );
   //DOCTEST_CHECK( out[ 2 ] == 2 );
   //DOCTEST_CHECK( v1[ 1 ] == doctest::Approx( -cos( M_PI/4 )) );
   //DOCTEST_CHECK( v1[ 2 ] == doctest::Approx( sin( M_PI/4 )) );      // signs might be different here...
   //DOCTEST_CHECK( v2[ 1 ] == doctest::Approx( sin( M_PI/4 )) );
   //DOCTEST_CHECK( v2[ 2 ] == doctest::Approx( cos( M_PI/4 )) );

   double matrix3[] = { 4, 0, 0, 8, 0, 6 };
   SymmetricEigenValues3DPacked( matrix3, out );
   DOCTEST_CHECK( out[ 0 ] == 8 );
   DOCTEST_CHECK( out[ 1 ] == 6 );
   DOCTEST_CHECK( out[ 2 ] == 4 );
   //SymmetricEigenSystem3DPacked( matrix3, out, v1, v2, v3 );
}

#endif


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
