/*
 * (c)2016-2025, Cris Luengo.
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

#ifndef DIP_CHAIN_CODE_H
#define DIP_CHAIN_CODE_H

#include <algorithm>
#include <array>
#include <vector>

#include "diplib.h"
#include "diplib/accumulators.h"
#include "diplib/polygon.h"


/// \file
/// \brief Support for chain-code object representation and quantification. Everything declared in
/// this file is explicitly 2D.
/// See \ref measurement.


namespace dip {

/// \addtogroup measurement


/// \brief The contour of an object as a chain code sequence.
///
/// This class supports 4-connected and 8-connected chain codes, see the `Code` definition for a description of the
/// chain codes.
///
/// A default-initialized `ChainCode` represents no object (`Empty` returns true). Set the `start` value to
/// represent a 1-pixel object. Larger objects have at least two values in the chain code. A chain code with a
/// single value is illegal.
struct DIP_NO_EXPORT ChainCode {

   DIP_EXPORT static constexpr VertexInteger deltas4[4] = {{  1,  0 },
                                                           {  0, -1 },
                                                           { -1,  0 },
                                                           {  0,  1 }};
   DIP_EXPORT static constexpr VertexInteger deltas8[8] = {{  1,  0 },
                                                           {  1, -1 },
                                                           {  0, -1 },
                                                           { -1, -1 },
                                                           { -1,  0 },
                                                           { -1,  1 },
                                                           {  0,  1 },
                                                           {  1,  1 }};

   /// \brief Provides data that are helpful when processing chain codes.
   ///
   /// The table is prepared using the \ref dip::ChainCode::PrepareCodeTable method. The method takes a stride array,
   /// which is expected to have exactly two elements (as chain codes only work with 2D images). The returned
   /// table contains a value `pos[code]` that says how the coordinates change when moving in the direction of the
   /// `code`, and a value `offset[code]` that says how to modify the image data pointer to reach the new pixel.
   ///
   /// `pos[code]` is identical to `code.Delta8()` or `code.Delta4()` (depending on connectivity).
   ///
   /// No checking is done when indexing. If the `CodeTable` is derived from a 4-connected chain code, only the
   /// first four table elements can be used. Otherwise, eight table elements exist and are valid.
   struct DIP_NO_EXPORT CodeTable {
         VertexInteger const* pos; ///< Array with position offsets for each chain code.
         std::array< dip::sint, 8 > offset{ 0 }; ///< Array with pointer offsets for each chain code.
      private:
         friend struct ChainCode; // make it so that we can only create one of these tables through the dip::ChainCode::PrepareCodeTable method.
         CodeTable( bool is8connected, IntegerArray strides ) : pos( is8connected ? deltas8 : deltas4 ) {
            dip::sint xS = strides[ 0 ];
            dip::sint yS = strides[ 1 ];
            for( dip::uint ii = 0; ii < ( is8connected ? 8u : 4u ); ++ii ) {
               offset[ ii ] = pos[ ii ].x * xS + pos[ ii ].y * yS;
            }
         }
   };

   /// \brief Encodes a single chain code, as used by \ref dip::ChainCode.
   ///
   /// Chain codes are between 0 and 3 for connectivity = 1, and between 0 and 7 for connectivity = 2.
   /// 0 means to the right in both cases. The border flag marks pixels at the border of the image.
   class DIP_NO_EXPORT Code {
      public:
         /// Default constructor
         Code() = default;
         /// Constructor
         Code( unsigned code, bool border = false ) : value( static_cast< dip::uint8 >(( code & 7u ) | ( static_cast< unsigned >( border ) << 3u )) ) {}
         /// Returns whether the border flag is set
         bool IsBorder() const { return static_cast< bool >( isBorder() ); }
         /// Returns the chain code
         operator unsigned() const { return code8(); }
         /// Is it an even code?
         bool IsEven() const { return !( value & 1u ); }
         /// Is it an off code?
         bool IsOdd() const { return !IsEven(); }
         /// The change in coordinates for an 8-connected chain code
         VertexInteger const& Delta8() const { return deltas8[ code8() ]; }
         /// The change in coordinates for a 4-connected chain code
         VertexInteger const& Delta4() const { return deltas4[ code4() ]; }
         /// Compare codes
         bool operator==( Code const& c2 ) const {
            return code8() == c2.code8();
         }
         /// Compare codes
         bool operator!=( Code const& c2 ) const {
            return !( *this == c2 );
         }
      private:
         dip::uint8 value = 0;
         unsigned code8() const { return value & 7u; }
         unsigned code4() const { return value & 3u; }
         bool isBorder() const { return static_cast< bool >( value & 8u ); }
   };

   std::vector< Code > codes;       ///< The chain codes
   VertexInteger start = { -1, -1 };///< The coordinates of the start pixel, the default value is outside the image to indicate there's no chain code here
   LabelType objectID = 0;          ///< The label of the object from which this chain code is taken
   bool is8connected = true;        ///< Is false when connectivity = 1, true when connectivity = 2

   /// Adds a code to the end of the chain.
   void Push( Code const& code ) { codes.push_back( code ); }

   /// \brief Returns a table that is useful when processing the chain code
   CodeTable PrepareCodeTable( IntegerArray const& strides ) const {
      DIP_THROW_IF( strides.size() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
      return { is8connected, strides };
   }

   /// \brief Returns a table that is useful when processing the chain code
   static CodeTable PrepareCodeTable( dip::uint connectivity, IntegerArray const& strides ) {
      DIP_THROW_IF( strides.size() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
      DIP_THROW_IF( connectivity > 2, E::CONNECTIVITY_NOT_SUPPORTED );
      return { connectivity != 1, strides }; // 0 means 8-connected also
   }

   /// \brief Creates a new chain code object that is 8-connected and represents the same shape.
   DIP_EXPORT ChainCode ConvertTo8Connected() const;

   /// \brief A chain code whose `start` value hasn't been set is considered empty.
   bool Empty() const {
      return start == VertexInteger{ -1, -1 };
   }

   /// \brief Returns the length of the chain code using the method by Vossepoel and Smeulders.
   ///
   /// If the chain code represents the closed contour of an object, add &pi; to the result to determine
   /// the object's perimeter.
   ///
   /// Any portions of the chain code that run along the image edge are not measured by default. That is, for
   /// an object that is only partially inside the image, the portion of the object's perimeter that
   /// is inside of the image is measured, the edge created by cutting the object is not. To include those
   /// portions of the perimeter, set `boundaryPixels` to `"include"`.
   ///
   /// !!! literature
   ///     - A.M. Vossepoel and A.W.M. Smeulders, "Vector code probability and metrication error in the representation
   ///       of straight lines of finite length", Computer Graphics and Image Processing 20(4):347-364, 1982.
   DIP_EXPORT dfloat Length( String const& boundaryPixels = S::EXCLUDE ) const;

   /// \brief Returns the Feret diameters, using an angular step size in radian of `angleStep`.
   /// It is better to use \ref dip::ConvexHull::Feret.
   DIP_EXPORT FeretValues Feret( dfloat angleStep = 5.0 / 180.0 * pi ) const;

   /// Computes the bending energy.
   ///
   /// Computes the bending energy directly from the chain code. The algorithm is rather imprecise. It is better
   /// to use \ref dip::Polygon::BendingEnergy.
   ///
   /// !!! literature
   ///     - I.T. Young, J.E. Walker and J.E. Bowie, "An Analysis Technique for Biological Shape I",
   ///       Information and Control 25(4):357-370, 1974.
   ///     - J.E. Bowie and I.T. Young, "An Analysis Technique for Biological Shape - II",
   ///       Acta Cytologica 21(5):455-464, 1977.
   DIP_EXPORT dfloat BendingEnergy() const;

   /// \brief Computes the area of the solid object described by the chain code. Uses the result of
   /// \ref dip::ChainCode::Polygon, so if you plan to do multiple similar measures, extract the polygon and
   /// compute the measures on that.
   dfloat Area() const {
      // There's another algorithm to compute this, that doesn't depend on the polygon. Should we implement that?
      if( Empty() ) {
         return 0;
      }
      return Polygon().Area() + 0.5;
   }

   /// \brief Computes the centroid of the solid object described by the chain code. Uses the result of
   /// \ref dip::ChainCode::Polygon, so if you plan to do multiple similar measures, extract the polygon and
   /// compute the measures on that.
   VertexFloat Centroid() const {
      // There's another algorithm to compute this, that doesn't depend on the polygon. Should we implement that?
      return Polygon().Centroid();
   }

   /// \brief Finds the bounding box for the object described by the chain code.
   DIP_EXPORT BoundingBoxInteger BoundingBox() const;

   /// Returns the length of the longest run of identical chain codes.
   DIP_EXPORT dip::uint LongestRun() const;

   /// \brief Returns a polygon representation of the object.
   ///
   /// Creates a polygon by joining the mid-points between an object pixel and a background pixel that are
   /// edge-connected neighbors. The polygon follows the "crack" between pixels, but without the biases
   /// one gets when joining pixel vertices into a polygon. The polygon always has an area exactly half a
   /// pixel smaller than the solid binary object it represents.
   ///
   /// If `borderCodes` is `"keep"` (the default), then the output polygon will have vertices for the
   /// full chain code. If it is `"lose"`, then the chain codes that go along the image border will
   /// be ignored; the polygon will still follow that edge of the object, but there will be no vertices
   /// along that edge.
   ///
   /// !!! literature
   ///     - K. Dunkelberger, and O. Mitchell, "Contour tracing for precision measurement",
   ///       Proceedings of the IEEE International Conference on Robotics and Automation, vol 2, 1985,
   ///       doi:10.1109/ROBOT.1985.1087356.
   ///     - S. Eddins, "Binary image convex hull – algorithm notes", MathWorks Blog, 2006,
   ///       <http://blogs.mathworks.com/steve/2011/10/04/binary-image-convex-hull-algorithm-notes/>.
   DIP_EXPORT dip::Polygon Polygon( String const& borderCodes = S::KEEP ) const;

   /// Returns the convex hull of the object, see \ref dip::ChainCode::Polygon.
   dip::ConvexHull ConvexHull() const {
      return Polygon().ConvexHull();
   }

   /// \brief Paints the pixels traced by the chain code in a binary image. The image has the size of the
   /// \ref dip::ChainCode::BoundingBox.
   DIP_EXPORT void Image( dip::Image& out ) const;
   dip::Image Image() const {
      dip::Image out;
      Image( out );
      return out;
   }

   /// \brief Returns the pixel coordinates for each of the pixels represented in the chain code.
   ///
   /// Very large coordinate values will be returned if the chain code runs outside the image
   /// on the left or top (i.e. if the pixels encoded by the chain code have negative coordinates)
   /// because the output object uses unsigned integers.
   DIP_EXPORT CoordinateArray Coordinates() const;

   /// \brief Create a new chain code that goes around the object in the same direction, but traces the background
   /// pixels that are 4-connected to the object. That is, it grows the object by one pixel. Only defined for
   /// 8-connected chain codes.
   DIP_EXPORT ChainCode Offset() const;
};

/// \brief A collection of object contours.
/// \relates dip::ChainCode
using ChainCodeArray = std::vector< ChainCode >;

/// \brief Returns the set of chain codes sequences that encode the contours of the given objects in a labeled image.
///
/// Note that only the first closed contour for each label is found; if an object has multiple connected components,
/// only part of it is found. The chain code traces the outer perimeter of the object, holes are ignored.
///
/// `objectIDs` is a list with object IDs present in the labeled image. If an empty array is given, all objects in
/// the image are used. For the meaning of `connectivity`, see \ref connectivity.
///
/// `labels` is a labeled image, and must be scalar and of an unsigned integer type.
ChainCodeArray DIP_EXPORT GetImageChainCodes(
      Image const& labels,
      std::vector< LabelType > const& objectIDs = {},
      dip::uint connectivity = 2
);
[[ deprecated( "objectIDs should be a std::vector< LabelType >." ) ]]
inline ChainCodeArray GetImageChainCodes(
      Image const& labels,
      UnsignedArray const& objectIDs,
      dip::uint connectivity = 2
) {
   std::vector< LabelType > ids( objectIDs.size() );
   std::transform( objectIDs.begin(), objectIDs.end(), ids.begin(), []( dip::uint v ){ return CastLabelType( v ); } );
   return GetImageChainCodes( labels, ids, connectivity );
}
// An another version to disambiguate when we call `GetImageChainCodes(labels, {1})`.
inline ChainCodeArray GetImageChainCodes(
      Image const& labels,
      std::initializer_list< dip::LabelType > const& initializerList,
      dip::uint connectivity = 2
) {
   std::vector< LabelType > ids( initializerList.size() );
   std::copy( initializerList.begin(), initializerList.end(), ids.begin() );
   return GetImageChainCodes( labels, ids, connectivity );
}


/// \brief Returns the chain codes sequence that encodes the contour of one object in a binary or labeled image.
///
/// Note that only one closed contour is found; if the object has multiple connected components,
/// only part of it is found. The chain code traces the outer perimeter of the object, holes are ignored.
///
/// `startCoord` is the 2D coordinates of a boundary pixel. If it points to a zero-valued pixel or a pixel not on
/// the boundary of an object, an exception will be thrown.
///
/// For the meaning of `connectivity`, see \ref connectivity.
///
/// `labels` is a labeled or binary image, and must be scalar and of a binary or unsigned integer type.
ChainCode DIP_EXPORT GetSingleChainCode(
      Image const& labels,
      UnsignedArray const& startCoord,
      dip::uint connectivity = 2
);

/// \endgroup

} // namespace dip

#endif // DIP_CHAIN_CODE_H
