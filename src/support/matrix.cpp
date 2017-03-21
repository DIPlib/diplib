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

#include "diplib/library/numeric.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

namespace dip {

void SymmetricEigenDecomposition(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > lambdas,
      SampleIterator< dfloat > vectors
) {
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   if( vectors ) {
      Eigen::SelfAdjointEigenSolver< Eigen::MatrixXd > eigensolver( matrix );
      //if( eigensolver.info() != Eigen::Success ) { abort(); }
      Eigen::VectorXd eigenvalues = eigensolver.eigenvalues();
      Eigen::MatrixXd eigenvectors = eigensolver.eigenvectors();
      std::vector< dip::uint > indices( n );
      std::iota( indices.begin(), indices.end(), 0 );
      std::sort( indices.begin(), indices.end(), [ & ]( int a, int b ) { return eigenvalues[ b ] < eigenvalues[ a ]; } );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         dip::uint kk = indices[ ii ];
         lambdas[ ii ] = eigenvalues[ kk ];
         dip::uint offset = ii * n;
         for( dip::uint jj = 0; jj < n; ++jj ) {
            vectors[ jj + offset ] = eigenvectors( jj, kk );
         }
      }
   } else {
      Eigen::Map< Eigen::VectorXd, 0, Eigen::InnerStride<> > eigenvalues( lambdas.Pointer(), n, Eigen::InnerStride<>( lambdas.Stride() ));
      eigenvalues = matrix.selfadjointView< Eigen::Lower >().eigenvalues();
      std::sort( lambdas, lambdas + n, std::greater< dfloat >() );
   }
}

void EigenDecomposition(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dcomplex > lambdas,
      SampleIterator< dcomplex > vectors
) {
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::Map< Eigen::VectorXcd, 0, Eigen::InnerStride<> > eigenvalues( lambdas.Pointer(), n, Eigen::InnerStride<>( lambdas.Stride() ));
   if( vectors ) {
      Eigen::EigenSolver< Eigen::MatrixXd > eigensolver( matrix );
      Eigen::Map< Eigen::MatrixXcd, 0, Eigen::InnerStride<> > eigenvectors( vectors.Pointer(), n, n, Eigen::InnerStride<>( vectors.Stride() ));
      eigenvalues = eigensolver.eigenvalues();
      eigenvectors = eigensolver.eigenvectors();
   } else {
      eigenvalues = matrix.eigenvalues();
   }
}

void EigenDecomposition(
      dip::uint n,
      ConstSampleIterator< dcomplex > input,
      SampleIterator< dcomplex > lambdas,
      SampleIterator< dcomplex > vectors
) {
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::Map< Eigen::VectorXcd, 0, Eigen::InnerStride<> > eigenvalues( lambdas.Pointer(), n, Eigen::InnerStride<>( lambdas.Stride() ));
   if( vectors ) {
      Eigen::ComplexEigenSolver< Eigen::MatrixXcd > eigensolver( matrix );
      Eigen::Map< Eigen::MatrixXcd, 0, Eigen::InnerStride<> > eigenvectors( vectors.Pointer(), n, n, Eigen::InnerStride<>( vectors.Stride() ));
      eigenvalues = eigensolver.eigenvalues();
      eigenvectors = eigensolver.eigenvectors();
   } else {
      eigenvalues = matrix.eigenvalues();
   }
}

dfloat Determinant( dip::uint n, ConstSampleIterator< dfloat > input ) {
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   return matrix.determinant();
}

dcomplex Determinant( dip::uint n, ConstSampleIterator< dcomplex > input ) {
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   return matrix.determinant();
}

void SingularValueDecomposition(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > output,
      SampleIterator< dfloat > U,
      SampleIterator< dfloat > V
) {
   if( U && V ) {
      // TODO
   } else {
      // TODO
   }
}

void SingularValueDecomposition(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dcomplex > input,
      SampleIterator< dcomplex > output,
      SampleIterator< dcomplex > U,
      SampleIterator< dcomplex > V
) {
   if( U && V ) {
      // TODO
   } else {
      // TODO
   }
}

void Inverse( dip::uint n, ConstSampleIterator< dfloat > input, SampleIterator< dfloat > output ) {
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::Map< Eigen::MatrixXd, 0, Eigen::InnerStride<> > result( output.Pointer(), n, n, Eigen::InnerStride<>( output.Stride() ));
   result = matrix.inverse();
}

void Inverse( dip::uint n, ConstSampleIterator< dcomplex > input, SampleIterator< dcomplex > output ) {
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), n, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::Map< Eigen::MatrixXcd, 0, Eigen::InnerStride<> > result( output.Pointer(), n, n, Eigen::InnerStride<>( output.Stride() ));
   result = matrix.inverse();
}

void PseudoInverse( dip::uint m, dip::uint n, ConstSampleIterator< dfloat > input, SampleIterator< dfloat > output ) {
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::CompleteOrthogonalDecomposition< Eigen::MatrixXd > decomposition( matrix );
   Eigen::Map< Eigen::MatrixXd, 0, Eigen::InnerStride<> > result( output.Pointer(), n, m, Eigen::InnerStride<>( output.Stride() ));
   result = decomposition.pseudoInverse();
}

void PseudoInverse( dip::uint m, dip::uint n, ConstSampleIterator< dcomplex > input, SampleIterator< dcomplex > output ) {
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::CompleteOrthogonalDecomposition< Eigen::MatrixXcd > decomposition( matrix );
   Eigen::Map< Eigen::MatrixXcd, 0, Eigen::InnerStride<> > result( output.Pointer(), n, m, Eigen::InnerStride<>( output.Stride() ));
   result = decomposition.pseudoInverse();
}

dip::uint Rank( dip::uint m, dip::uint n, ConstSampleIterator< dfloat > input ) {
   Eigen::Map< Eigen::MatrixXd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::CompleteOrthogonalDecomposition< Eigen::MatrixXd > decomposition( matrix );
   return static_cast< dip::uint >( decomposition.rank() );
}

dip::uint Rank( dip::uint m, dip::uint n, ConstSampleIterator< dcomplex > input ) {
   Eigen::Map< Eigen::MatrixXcd const, 0, Eigen::InnerStride<> > matrix( input.Pointer(), m, n, Eigen::InnerStride<>( input.Stride() ));
   Eigen::CompleteOrthogonalDecomposition< Eigen::MatrixXcd > decomposition( matrix );
   return static_cast< dip::uint >( decomposition.rank() );
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::SymmetricEigenXXX functions") {
   double matrix2[] = { 4, 8, 0 };
   double lambdas[ 3 ];
   double vectors[ 9 ];
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
   DOCTEST_CHECK( vectors[ 0 ] == doctest::Approx(  cos( M_PI/4 )) ); // signs might be different here...
   DOCTEST_CHECK( vectors[ 1 ] == doctest::Approx( -sin( M_PI/4 )) );
   DOCTEST_CHECK( vectors[ 2 ] == doctest::Approx(  sin( M_PI/4 )) );
   DOCTEST_CHECK( vectors[ 3 ] == doctest::Approx(  cos( M_PI/4 )) );

   double matrix3[] = { 4, 8, 6, 0, 0, 0 };
   dip::SymmetricEigenDecompositionPacked( 3, matrix3, lambdas );
   DOCTEST_CHECK( lambdas[ 0 ] == 8 );
   DOCTEST_CHECK( lambdas[ 1 ] == 6 );
   DOCTEST_CHECK( lambdas[ 2 ] == 4 );
   dip::SymmetricEigenDecompositionPacked( 3, matrix3, lambdas, vectors );
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
