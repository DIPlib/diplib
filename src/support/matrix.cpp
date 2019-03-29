/*
 * DIPlib 3.0
 * This file contains definitions for functions that do matrix computations using Eigen
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

#include <array>
#include "diplib/library/numeric.h"

#if defined(__GNUG__) || defined(__clang__)
// For this file, turn off -Wsign-conversion, Eigen is really bad at this!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#if __GNUC__ >= 7
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#endif
#endif

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

namespace dip {

namespace {

template< class T >
struct GreaterMagnitude {
   bool operator()( T const& a, T const& b ) const {
      return std::abs( a ) > std::abs( b );
   }
};

} // namespace

void SymmetricEigenDecomposition(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > lambdas,
      SampleIterator< dfloat > vectors
) {
   DIP_ASSERT( input.Stride() >= 0 ); // TODO: (here and other asserts in this file) Eigen doesn't support negative strides, but there's a ticket for that: http://eigen.tuxfamily.org/bz/show_bug.cgi?id=747
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   if( vectors ) {
      Eigen::SelfAdjointEigenSolver< Eigen::MatrixXd > eigensolver( matrix );
      //if( eigensolver.info() != Eigen::Success ) { abort(); }
      Eigen::VectorXd const& eigenvalues = eigensolver.eigenvalues();
      Eigen::MatrixXd const& eigenvectors = eigensolver.eigenvectors();
      std::vector< dip::uint > indices( n );
      std::iota( indices.begin(), indices.end(), 0 );
      std::sort( indices.begin(), indices.end(), [ & ]( dip::uint a, dip::uint b ) { return std::abs( eigenvalues[ b ] ) < std::abs( eigenvalues[ a ] ); } );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         dip::uint kk = indices[ ii ];
         lambdas[ ii ] = eigenvalues[ kk ];
         dip::uint offset = ii * n;
         for( dip::uint jj = 0; jj < n; ++jj ) {
            vectors[ jj + offset ] = eigenvectors( jj, kk );
         }
      }
   } else {
      DIP_ASSERT( lambdas.Stride() >= 0 );
      Eigen::Map< Eigen::VectorXd, 0, Eigen::InnerStride<> > eigenvalues( lambdas.Pointer(), n, Eigen::InnerStride<>( lambdas.Stride() ));
      eigenvalues = matrix.selfadjointView< Eigen::Lower >().eigenvalues();
      std::sort( lambdas, lambdas + n, GreaterMagnitude< dfloat >() );
   }
}

void SymmetricEigenDecomposition2(
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > lambdas,
      SampleIterator< dfloat > vectors
) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::Matrix2d const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), Eigen::InnerStride<>( input.Stride() ));
   if( vectors ) {
      Eigen::SelfAdjointEigenSolver< Eigen::Matrix2d > eigensolver( matrix );
      //if( eigensolver.info() != Eigen::Success ) { abort(); }
      Eigen::Vector2d const& eigenvalues = eigensolver.eigenvalues();
      Eigen::Matrix2d const& eigenvectors = eigensolver.eigenvectors();
      dip::uint indices0 = 0;
      dip::uint indices1 = 1;
      if( std::abs( eigenvalues[ 0 ] ) < std::abs( eigenvalues[ 1 ] )) {
         indices0 = 1;
         indices1 = 0;
      }
      lambdas[ 0 ] = eigenvalues[ indices0 ];
      lambdas[ 1 ] = eigenvalues[ indices1 ];
      vectors[ 0 ] = eigenvectors( 0, indices0 );
      vectors[ 1 ] = eigenvectors( 1, indices0 );
      vectors[ 2 ] = eigenvectors( 0, indices1 );
      vectors[ 3 ] = eigenvectors( 1, indices1 );
   } else {
      DIP_ASSERT( lambdas.Stride() >= 0 );
      Eigen::Map< Eigen::Vector2d, 0, Eigen::InnerStride<> > eigenvalues( lambdas.Pointer(), Eigen::InnerStride<>( lambdas.Stride() ));
      eigenvalues = matrix.selfadjointView< Eigen::Lower >().eigenvalues();
      if( std::abs( lambdas[ 0 ] ) < std::abs( lambdas[ 1 ] )) {
         std::swap( lambdas[ 0 ], lambdas[ 1 ] );
      }
   }
}

void SymmetricEigenDecomposition3(
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > lambdas,
      SampleIterator< dfloat > vectors
) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::Matrix3d const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), Eigen::InnerStride<>( input.Stride() ));
   if( vectors ) {
      Eigen::SelfAdjointEigenSolver< Eigen::Matrix3d > eigensolver( matrix );
      //if( eigensolver.info() != Eigen::Success ) { abort(); }
      Eigen::Vector3d const& eigenvalues = eigensolver.eigenvalues();
      Eigen::Matrix3d const& eigenvectors = eigensolver.eigenvectors();
      std::array< dip::uint, 3 > indices{{ 0, 1, 2 }};
      std::sort( indices.begin(), indices.end(), [ & ]( dip::uint a, dip::uint b ) { return std::abs( eigenvalues[ b ] ) < std::abs( eigenvalues[ a ] ); } );
      for( dip::uint ii = 0; ii < 3; ++ii ) {
         dip::uint kk = indices[ ii ];
         lambdas[ ii ] = eigenvalues[ kk ];
         dip::uint offset = ii * 3;
         for( dip::uint jj = 0; jj < 3; ++jj ) {
            vectors[ jj + offset ] = eigenvectors( jj, kk );
         }
      }
   } else {
      DIP_ASSERT( lambdas.Stride() >= 0 );
      Eigen::Map< Eigen::Vector3d, 0, Eigen::InnerStride<> > eigenvalues( lambdas.Pointer(), Eigen::InnerStride<>( lambdas.Stride() ));
      eigenvalues = matrix.selfadjointView< Eigen::Lower >().eigenvalues();
      std::sort( lambdas, lambdas + 3, GreaterMagnitude< dfloat >() );
   }
}

void LargestEigenvector(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > vector
) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::SelfAdjointEigenSolver< Eigen::MatrixXd > eigensolver( matrix );
   //if( eigensolver.info() != Eigen::Success ) { abort(); }
   Eigen::VectorXd const& eigenvalues = eigensolver.eigenvalues();
   Eigen::MatrixXd const& eigenvectors = eigensolver.eigenvectors();
   std::vector< dip::uint > indices( n );
   std::iota( indices.begin(), indices.end(), 0 );
   std::sort( indices.begin(), indices.end(), [ & ]( dip::uint a, dip::uint b ) { return std::abs( eigenvalues[ b ] ) < std::abs( eigenvalues[ a ] ); } );
   dip::uint kk = indices[ 0 ];
   for( dip::uint jj = 0; jj < n; ++jj ) {
      vector[ jj ] = eigenvectors( jj, kk );
   }
}

void SmallestEigenvector(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > vector
) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::SelfAdjointEigenSolver< Eigen::MatrixXd > eigensolver( matrix );
   //if( eigensolver.info() != Eigen::Success ) { abort(); }
   Eigen::VectorXd const& eigenvalues = eigensolver.eigenvalues();
   Eigen::MatrixXd const& eigenvectors = eigensolver.eigenvectors();
   std::vector< dip::uint > indices( n );
   std::iota( indices.begin(), indices.end(), 0 );
   std::sort( indices.begin(), indices.end(), [ & ]( dip::uint a, dip::uint b ) { return std::abs( eigenvalues[ b ] ) < std::abs( eigenvalues[ a ] ); } );
   dip::uint kk = indices.back();
   for( dip::uint jj = 0; jj < n; ++jj ) {
      vector[ jj ] = eigenvectors( jj, kk );
   }
}

void EigenDecomposition(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dcomplex > lambdas,
      SampleIterator< dcomplex > vectors
) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   if( vectors ) {
      Eigen::EigenSolver< Eigen::MatrixXd > eigensolver( matrix );
      Eigen::VectorXcd const& eigenvalues = eigensolver.eigenvalues();
      Eigen::MatrixXcd const& eigenvectors = eigensolver.eigenvectors();
      std::vector< dip::uint > indices( n );
      std::iota( indices.begin(), indices.end(), 0 );
      std::sort( indices.begin(), indices.end(), [ & ]( dip::uint a, dip::uint b ) { return std::abs( eigenvalues[ b ] ) < std::abs( eigenvalues[ a ] ); } );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         dip::uint kk = indices[ ii ];
         lambdas[ ii ] = eigenvalues[ kk ];
         dip::uint offset = ii * n;
         for( dip::uint jj = 0; jj < n; ++jj ) {
            vectors[ jj + offset ] = eigenvectors( jj, kk );
         }
      }
   } else {
      DIP_ASSERT( lambdas.Stride() >= 0 );
      Eigen::Map< Eigen::VectorXcd, 0, Eigen::InnerStride<> > eigenvalues( lambdas.Pointer(), n, Eigen::InnerStride<>( lambdas.Stride() ));
      eigenvalues = matrix.eigenvalues();
      std::sort( lambdas, lambdas + n, GreaterMagnitude< dcomplex >() );
   }
}

void EigenDecomposition(
      dip::uint n,
      ConstSampleIterator< dcomplex > input,
      SampleIterator< dcomplex > lambdas,
      SampleIterator< dcomplex > vectors
) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   if( vectors ) {
      Eigen::ComplexEigenSolver< Eigen::MatrixXcd > eigensolver( matrix );
      Eigen::VectorXcd const& eigenvalues = eigensolver.eigenvalues();
      Eigen::MatrixXcd const& eigenvectors = eigensolver.eigenvectors();
      std::vector< dip::uint > indices( n );
      std::iota( indices.begin(), indices.end(), 0 );
      std::sort( indices.begin(), indices.end(), [ & ]( dip::uint a, dip::uint b ) { return std::abs( eigenvalues[ b ] ) < std::abs( eigenvalues[ a ] ); } );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         dip::uint kk = indices[ ii ];
         lambdas[ ii ] = eigenvalues[ kk ];
         dip::uint offset = ii * n;
         for( dip::uint jj = 0; jj < n; ++jj ) {
            vectors[ jj + offset ] = eigenvectors( jj, kk );
         }
      }
   } else {
      DIP_ASSERT( lambdas.Stride() >= 0 );
      Eigen::Map< Eigen::VectorXcd, 0, Eigen::InnerStride<> > eigenvalues( lambdas.Pointer(), n, Eigen::InnerStride<>( lambdas.Stride() ));
      eigenvalues = matrix.eigenvalues();
      std::sort( lambdas, lambdas + n, GreaterMagnitude< dcomplex >() );
   }
}

dfloat Determinant( dip::uint n, ConstSampleIterator< dfloat > input ) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   return matrix.determinant();
}

dcomplex Determinant( dip::uint n, ConstSampleIterator< dcomplex > input ) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   return matrix.determinant();
}

void SingularValueDecomposition(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > Sout,
      SampleIterator< dfloat > Uout,
      SampleIterator< dfloat > Vout
) {
   dip::uint p = std::min( m, n );
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > M( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::JacobiSVD< Eigen::MatrixXd > svd( M, Eigen::ComputeThinU | Eigen::ComputeThinV );
   DIP_ASSERT( Sout.Stride() >= 0 );
   Eigen::Map< Eigen::VectorXd, 0, Eigen::InnerStride<> > S( Sout.Pointer(), p, Eigen::InnerStride<>( Sout.Stride() ));
   S = svd.singularValues();
   if( Uout && Vout ) {
      DIP_ASSERT( Uout.Stride() >= 0 );
      Eigen::Map< Eigen::MatrixXd, 0, Eigen::InnerStride<> > U( Uout.Pointer(), m, p, Eigen::InnerStride<>( Uout.Stride() ));
      DIP_ASSERT( Vout.Stride() >= 0 );
      Eigen::Map< Eigen::MatrixXd, 0, Eigen::InnerStride<> > V( Vout.Pointer(), n, p, Eigen::InnerStride<>( Vout.Stride() ));
      U = svd.matrixU();
      V = svd.matrixV();
   }
}

void SingularValueDecomposition(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dcomplex > input,
      SampleIterator< dcomplex > Sout,
      SampleIterator< dcomplex > Uout,
      SampleIterator< dcomplex > Vout
) {
   dip::uint p = std::min( m, n );
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > M( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::JacobiSVD< Eigen::MatrixXcd > svd( M, Eigen::ComputeThinU | Eigen::ComputeThinV );
   DIP_ASSERT( Sout.Stride() >= 0 );
   Eigen::Map< Eigen::VectorXcd, 0, Eigen::InnerStride<> > S( Sout.Pointer(), p, Eigen::InnerStride<>( Sout.Stride() ));
   S = svd.singularValues();
   if( Uout && Vout ) {
      DIP_ASSERT( Uout.Stride() >= 0 );
      Eigen::Map< Eigen::MatrixXcd, 0, Eigen::InnerStride<> > U( Uout.Pointer(), m, p, Eigen::InnerStride<>( Uout.Stride() ));
      DIP_ASSERT( Vout.Stride() >= 0 );
      Eigen::Map< Eigen::MatrixXcd, 0, Eigen::InnerStride<> > V( Vout.Pointer(), n, p, Eigen::InnerStride<>( Vout.Stride() ));
      U = svd.matrixU();
      V = svd.matrixV();
   }
}

void Inverse( dip::uint n, ConstSampleIterator< dfloat > input, SampleIterator< dfloat > output ) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   DIP_ASSERT( output.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd, 0, Eigen::InnerStride<> > result( output.Pointer(), n, n, Eigen::InnerStride<>( output.Stride() ));
   result = matrix.inverse();
}

void Inverse( dip::uint n, ConstSampleIterator< dcomplex > input, SampleIterator< dcomplex > output ) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   DIP_ASSERT( output.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXcd, 0, Eigen::InnerStride<> > result( output.Pointer(), n, n, Eigen::InnerStride<>( output.Stride() ));
   result = matrix.inverse();
}

void PseudoInverse(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > output,
      dfloat tolerance
) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::JacobiSVD< Eigen::MatrixXd > svd( matrix, Eigen::ComputeThinU | Eigen::ComputeThinV );
   tolerance = tolerance * static_cast< dfloat >( std::max( m, n )) * svd.singularValues().array().abs()( 0 );
   DIP_ASSERT( output.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd, 0, Eigen::InnerStride<> > result( output.Pointer(), n, m, Eigen::InnerStride<>( output.Stride() ));
   result = svd.matrixV() *
            ( svd.singularValues().array().abs() > tolerance ).select( svd.singularValues().array().inverse(), 0 )
                                                              .matrix().asDiagonal()
            * svd.matrixU().adjoint();
}

void PseudoInverse(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dcomplex > input,
      SampleIterator< dcomplex > output,
      dfloat tolerance
) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::JacobiSVD< Eigen::MatrixXcd > svd( matrix, Eigen::ComputeThinU | Eigen::ComputeThinV );
   tolerance = tolerance * static_cast< dfloat >( std::max( m, n )) * svd.singularValues().array().abs()( 0 );
   DIP_ASSERT( output.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXcd, 0, Eigen::InnerStride<> > result( output.Pointer(), n, m, Eigen::InnerStride<>( output.Stride() ));
   result = svd.matrixV() *
            ( svd.singularValues().array().abs() > tolerance ).select( svd.singularValues().array().inverse(), 0 )
                                                              .matrix().asDiagonal()
            * svd.matrixU().adjoint();
}

dip::uint Rank( dip::uint m, dip::uint n, ConstSampleIterator< dfloat > input ) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::CompleteOrthogonalDecomposition< Eigen::MatrixXd > decomposition( matrix );
   return static_cast< dip::uint >( decomposition.rank() );
}

dip::uint Rank( dip::uint m, dip::uint n, ConstSampleIterator< dcomplex > input ) {
   DIP_ASSERT( input.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::CompleteOrthogonalDecomposition< Eigen::MatrixXcd > decomposition( matrix );
   return static_cast< dip::uint >( decomposition.rank() );
}

void Solve(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dfloat > A,
      ConstSampleIterator< dfloat > b,
      SampleIterator< dfloat > output
) {
   DIP_ASSERT( A.Stride() >= 0 );
   DIP_ASSERT( b.Stride() >= 0 );
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( A.Pointer(), m, n, Eigen::InnerStride<>( A.Stride() ));
   Eigen::JacobiSVD< Eigen::MatrixXd > svd( matrix, Eigen::ComputeThinU | Eigen::ComputeThinV );
   Eigen::Map< Eigen::VectorXd const, 0, Eigen::InnerStride<> > vector( b.Pointer(), m, Eigen::InnerStride<>( b.Stride() ));
   Eigen::Map< Eigen::VectorXd, 0, Eigen::InnerStride<> > result( output.Pointer(), n, Eigen::InnerStride<>( output.Stride() ));
   result = svd.solve( vector );
}

} // namespace dip


#if defined(__GNUG__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing the EigenDecomposition functions") {
   // Test generic symmetric code with 2x2 matrix
   dip::dfloat matrix2[] = { 4, 8, 0 };
   dip::dfloat lambdas[ 3 ];
   dip::dfloat vectors[ 9 ];
   dip::SymmetricEigenDecompositionPacked( 2, matrix2, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   dip::SymmetricEigenDecompositionPacked( 2, matrix2, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   DOCTEST_CHECK( vectors[ 0 ] == 0 );
   DOCTEST_CHECK( vectors[ 1 ] == 1 );
   DOCTEST_CHECK( vectors[ 2 ] == 1 );
   DOCTEST_CHECK( vectors[ 3 ] == 0 );
   matrix2[ 0 ] = 8;
   matrix2[ 1 ] = 4;
   dip::SymmetricEigenDecompositionPacked( 2, matrix2, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   dip::SymmetricEigenDecompositionPacked( 2, matrix2, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   DOCTEST_CHECK( vectors[ 0 ] == 1 );
   DOCTEST_CHECK( vectors[ 1 ] == 0 );
   DOCTEST_CHECK( vectors[ 2 ] == 0 );
   DOCTEST_CHECK( vectors[ 3 ] == 1 );
   matrix2[ 0 ] = 3;
   matrix2[ 1 ] = 3;
   matrix2[ 2 ] = -1;
   dip::SymmetricEigenDecompositionPacked( 2, matrix2, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 4 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   dip::SymmetricEigenDecompositionPacked( 2, matrix2, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 4 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   DOCTEST_CHECK( vectors[ 0 ] == doctest::Approx(  std::cos( dip::pi/4 ))); // signs might be different here...
   DOCTEST_CHECK( vectors[ 1 ] == doctest::Approx( -std::sin( dip::pi/4 )));
   DOCTEST_CHECK( vectors[ 2 ] == doctest::Approx(  std::sin( dip::pi/4 )));
   DOCTEST_CHECK( vectors[ 3 ] == doctest::Approx(  std::cos( dip::pi/4 )));

   // Test 2x2-specific symmetric code
   dip::dfloat matrix2f[] = { 4, 0, 0, 8 };
   dip::SymmetricEigenDecomposition2( matrix2f, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   dip::SymmetricEigenDecomposition2( matrix2f, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   DOCTEST_CHECK( vectors[ 0 ] == 0 );
   DOCTEST_CHECK( vectors[ 1 ] == 1 );
   DOCTEST_CHECK( vectors[ 2 ] == 1 );
   DOCTEST_CHECK( vectors[ 3 ] == 0 );
   matrix2f[ 0 ] = 8;
   matrix2f[ 3 ] = 4;
   dip::SymmetricEigenDecomposition2( matrix2f, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   dip::SymmetricEigenDecomposition2( matrix2f, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 4 );
   DOCTEST_CHECK( vectors[ 0 ] == 1 );
   DOCTEST_CHECK( vectors[ 1 ] == 0 );
   DOCTEST_CHECK( vectors[ 2 ] == 0 );
   DOCTEST_CHECK( vectors[ 3 ] == 1 );
   matrix2f[ 0 ] = 3;
   matrix2f[ 1 ] = -1;
   matrix2f[ 2 ] = -1;
   matrix2f[ 3 ] = 3;
   dip::SymmetricEigenDecomposition2( matrix2f, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 4 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   dip::SymmetricEigenDecomposition2( matrix2f, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 4 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   DOCTEST_CHECK( vectors[ 0 ] == doctest::Approx(  std::cos( dip::pi/4 ))); // signs might be different here...
   DOCTEST_CHECK( vectors[ 1 ] == doctest::Approx( -std::sin( dip::pi/4 )));
   DOCTEST_CHECK( vectors[ 2 ] == doctest::Approx(  std::sin( dip::pi/4 )));
   DOCTEST_CHECK( vectors[ 3 ] == doctest::Approx(  std::cos( dip::pi/4 )));

   // Test generic symmetric code with 3x3 matrix
   dip::dfloat matrix3[] = { 3, 1.5, 1.5, 0.0, 0.0, -0.5 };
   dip::SymmetricEigenDecompositionPacked( 3, matrix3, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 3 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   DOCTEST_CHECK( lambdas[ 2 ] == 1 );
   dip::SymmetricEigenDecompositionPacked( 3, matrix3, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 3 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   DOCTEST_CHECK( lambdas[ 2 ] == 1 );
   DOCTEST_CHECK( vectors[ 0 ] == doctest::Approx(  1.0 ));
   DOCTEST_CHECK( vectors[ 1 ] == doctest::Approx(  0.0 ));
   DOCTEST_CHECK( vectors[ 2 ] == doctest::Approx(  0.0 ));
   DOCTEST_CHECK( vectors[ 3 ] == doctest::Approx(  0.0 ));
   DOCTEST_CHECK( vectors[ 4 ] == doctest::Approx(  1.0 / std::sqrt( 2.0 )));
   DOCTEST_CHECK( vectors[ 5 ] == doctest::Approx( -1.0 / std::sqrt( 2.0 )));
   DOCTEST_CHECK( vectors[ 6 ] == doctest::Approx(  0.0 ));
   DOCTEST_CHECK( vectors[ 7 ] == doctest::Approx(  1.0 / std::sqrt( 2.0 )));
   DOCTEST_CHECK( vectors[ 8 ] == doctest::Approx(  1.0 / std::sqrt( 2.0 )));

   // Test 3x3-specific symmetric code
   dip::dfloat matrix3f[] = { 3, 0.0, 0.0, 0.0, 1.5, -0.5, 0.0, -0.5, 1.5 };
   dip::SymmetricEigenDecomposition3( matrix3f, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 3 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   DOCTEST_CHECK( lambdas[ 2 ] == 1 );
   dip::SymmetricEigenDecomposition3( matrix3f, lambdas, vectors );
   DOCTEST_CHECK( lambdas[ 0 ] == 3 );
   DOCTEST_CHECK( lambdas[ 1 ] == 2 );
   DOCTEST_CHECK( lambdas[ 2 ] == 1 );
   DOCTEST_CHECK( vectors[ 0 ] == doctest::Approx(  1.0 ));
   DOCTEST_CHECK( vectors[ 1 ] == doctest::Approx(  0.0 ));
   DOCTEST_CHECK( vectors[ 2 ] == doctest::Approx(  0.0 ));
   DOCTEST_CHECK( vectors[ 3 ] == doctest::Approx(  0.0 ));
   DOCTEST_CHECK( vectors[ 4 ] == doctest::Approx(  1.0 / std::sqrt( 2.0 )));
   DOCTEST_CHECK( vectors[ 5 ] == doctest::Approx( -1.0 / std::sqrt( 2.0 )));
   DOCTEST_CHECK( vectors[ 6 ] == doctest::Approx(  0.0 ));
   DOCTEST_CHECK( vectors[ 7 ] == doctest::Approx(  1.0 / std::sqrt( 2.0 )));
   DOCTEST_CHECK( vectors[ 8 ] == doctest::Approx(  1.0 / std::sqrt( 2.0 )));

   // Test generic non-symmetric code with 2x2 matrix
   dip::dfloat matrix22[] = { 3, -1, -1, 3 };
   dip::dcomplex c_lambdas[ 2 ];
   dip::dcomplex c_vectors[ 4 ];
   dip::EigenDecomposition( 2, matrix22, c_lambdas, c_vectors );
   DOCTEST_CHECK( c_lambdas[ 0 ].real() == doctest::Approx( 4.0 ));
   DOCTEST_CHECK( c_lambdas[ 1 ].real() == doctest::Approx( 2.0 ));
   DOCTEST_CHECK( c_vectors[ 0 ].real() == doctest::Approx(  cos( dip::pi/4 ))); // signs might be different here...
   DOCTEST_CHECK( c_vectors[ 1 ].real() == doctest::Approx( -sin( dip::pi/4 )));
   DOCTEST_CHECK( c_vectors[ 2 ].real() == doctest::Approx(  sin( dip::pi/4 )));
   DOCTEST_CHECK( c_vectors[ 3 ].real() == doctest::Approx(  cos( dip::pi/4 )));
   DOCTEST_CHECK( c_lambdas[ 0 ].imag() == 0 );
   DOCTEST_CHECK( c_lambdas[ 1 ].imag() == 0 );
   DOCTEST_CHECK( c_vectors[ 0 ].imag() == 0 );
   DOCTEST_CHECK( c_vectors[ 1 ].imag() == 0 );
   DOCTEST_CHECK( c_vectors[ 2 ].imag() == 0 );
   DOCTEST_CHECK( c_vectors[ 3 ].imag() == 0 );

}

DOCTEST_TEST_CASE("[DIPlib] testing the SingularValueDecomposition and related functions") {
   dip::dfloat matrix22[] = { 4, 0, 0, 8 };
   dip::dfloat S[ 2 ];
   dip::dfloat U[ 4 ];
   dip::dfloat V[ 6 ];
   dip::SingularValueDecomposition( 2, 2, matrix22, S, U, V );
   DOCTEST_CHECK( S[ 0 ] == 8 );
   DOCTEST_CHECK( S[ 1 ] == 4 );
   DOCTEST_CHECK( U[ 0 ] == 0 );
   DOCTEST_CHECK( U[ 1 ] == 1 );
   DOCTEST_CHECK( U[ 2 ] == 1 );
   DOCTEST_CHECK( U[ 3 ] == 0 );
   DOCTEST_CHECK( V[ 0 ] == 0 );
   DOCTEST_CHECK( V[ 1 ] == 1 );
   DOCTEST_CHECK( V[ 2 ] == 1 );
   DOCTEST_CHECK( V[ 3 ] == 0 );

   dip::dfloat matrix23[] = { 3, 2, 2, 3, 2, -2 };
   dip::SingularValueDecomposition( 2, 3, matrix23, S, U, V );
   DOCTEST_CHECK( S[ 0 ] == doctest::Approx( 5.0 ));
   DOCTEST_CHECK( S[ 1 ] == doctest::Approx( 3.0 ));
   DOCTEST_CHECK( U[ 0 ] == doctest::Approx( -1.0 / std::sqrt( 2 ))); // signs might be different here...
   DOCTEST_CHECK( U[ 1 ] == doctest::Approx( -1.0 / std::sqrt( 2 )));
   DOCTEST_CHECK( U[ 2 ] == doctest::Approx( 1.0 / std::sqrt( 2 )));
   DOCTEST_CHECK( U[ 3 ] == doctest::Approx( -1.0 / std::sqrt( 2 )));
   DOCTEST_CHECK( V[ 0 ] == doctest::Approx( -1.0 / std::sqrt( 2 )));
   DOCTEST_CHECK( V[ 1 ] == doctest::Approx( -1.0 / std::sqrt( 2 )));
   DOCTEST_CHECK( V[ 2 ] == doctest::Approx( 0.0 ));
   DOCTEST_CHECK( V[ 3 ] == doctest::Approx( 1.0 / std::sqrt( 18 )));
   DOCTEST_CHECK( V[ 4 ] == doctest::Approx( -1.0 / std::sqrt( 18 )));
   DOCTEST_CHECK( V[ 5 ] == doctest::Approx( 4.0 / std::sqrt( 18 )));

   DOCTEST_CHECK( dip::Rank( 2, 3, matrix23 ) == 2 );

   dip::dfloat matrix32[ 6 ];
   dip::PseudoInverse( 2, 3, matrix23, matrix32 );
   DOCTEST_CHECK( matrix32[ 0 ] == doctest::Approx( 28.0 / 180.0 ));
   DOCTEST_CHECK( matrix32[ 1 ] == doctest::Approx(  8.0 / 180.0 ));
   DOCTEST_CHECK( matrix32[ 2 ] == doctest::Approx( 40.0 / 180.0 ));
   DOCTEST_CHECK( matrix32[ 3 ] == doctest::Approx(  8.0 / 180.0 ));
   DOCTEST_CHECK( matrix32[ 4 ] == doctest::Approx( 28.0 / 180.0 ));
   DOCTEST_CHECK( matrix32[ 5 ] == doctest::Approx(-40.0 / 180.0 ));

   dip::dfloat b[ 3 ] = { 44.0 / 180.0, 64.0 / 180.0, -40.0 / 180.0 };
   dip::dfloat x[ 2 ];
   dip::Solve( 3, 2, matrix32, b, x );
   DOCTEST_CHECK( x[ 0 ] == doctest::Approx( 1.0 ));
   DOCTEST_CHECK( x[ 1 ] == doctest::Approx( 2.0 ));
}

#endif // DIP__ENABLE_DOCTEST
