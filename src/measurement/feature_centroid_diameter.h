/*
 * (c)2026, Tolga Ciftcicelik.
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


namespace dip {
namespace Feature {


/// \brief Maximum and minimum object diameters, measured as chords through the object's centroid.
///
/// For every point on the object's boundary, the object's width is computed as the distance to
/// the boundary point diametrically opposite it (in the direction rotated by `pi`), as seen from
/// the object's centroid (see \ref "Center"). The minimum and maximum of this width, taken over
/// the whole boundary, are reported, together with the angle at which each was found.
///
/// !!! note
///     This is related to, but distinct from, the classical "Martin diameter" of the particle
///     characterization literature, which is the chord that divides the particle's area in half
///     at a given angle. `CentroidDiameter` instead always passes through the centroid. This is
///     considerably cheaper to compute (the classical Martin diameter requires a search for the
///     area-bisecting position at each angle, rather than a single lookup), and the two measures
///     coincide for centrally symmetric shapes, but they can diverge for strongly asymmetric ones.
///
/// Unlike \ref "Feret", which is based on the object's convex hull, `CentroidDiameter` uses the
/// object's full boundary. This makes it sensitive to concave defects (a chip, notch, or flat
/// spot) that `Feret` cannot detect by construction: the convex hull bridges over any concavity,
/// whereas a chord through the centroid can pass right through it.
class FeatureCentroidDiameter : public PolygonBased {
   public:
      FeatureCentroidDiameter() : PolygonBased( { "CentroidDiameter", "Maximum and minimum object diameters through the centroid (2D)", false } ) {};

      ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 4 );
         PhysicalQuantity pq = label.PixelSize().UnitLength();
         scale_ = pq.magnitude;
         out[ 0 ].units = pq.units;
         out[ 1 ].units = pq.units;
         out[ 2 ].units = Units::Radian();
         out[ 3 ].units = Units::Radian();
         out[ 0 ].name = "Max";
         out[ 1 ].name = "Min";
         out[ 2 ].name = "MaxAng";
         out[ 3 ].name = "MinAng";
         return out;
      }

      void Measure( Polygon const& polygon, Measurement::ValueIterator output ) override {
         struct Sample {
            dfloat angle;
            dfloat radius;
         };
         VertexFloat centroid = polygon.Centroid();
         std::vector< Sample > samples;
         samples.reserve( polygon.vertices.size() );
         for( auto const& v : polygon.vertices ) {
            dfloat dx = v.x - centroid.x;
            dfloat dy = v.y - centroid.y;
            samples.push_back( { std::atan2( dy, dx ), std::hypot( dx, dy ) } );
         }
         if( samples.size() < 2 ) {
            // Degenerate polygon, not enough vertices to compute a meaningful diameter.
            output[ 0 ] = 0;
            output[ 1 ] = 0;
            output[ 2 ] = 0;
            output[ 3 ] = 0;
            return;
         }
         std::sort( samples.begin(), samples.end(), []( Sample const& a, Sample const& b ) {
            return a.angle < b.angle;
         } );
         dip::uint n = samples.size();

         // Linearly interpolates the object's radius at an arbitrary angle, between the two
         // nearest samples. The polygon typically has one vertex per boundary pixel, so this
         // interpolation error is sub-pixel.
         auto radiusAt = [ & ]( dfloat angle ) {
            while( angle < -pi ) {
               angle += 2.0 * pi;
            }
            while( angle >= pi ) {
               angle -= 2.0 * pi;
            }
            dip::uint idx = static_cast< dip::uint >( std::lower_bound( samples.begin(), samples.end(), angle,
                  []( Sample const& s, dfloat a ) { return s.angle < a; } ) - samples.begin() );
            dip::uint i0 = ( idx == 0 ) ? ( n - 1 ) : ( idx - 1 );
            dip::uint i1 = idx % n;
            dfloat a0 = samples[ i0 ].angle;
            dfloat a1 = samples[ i1 ].angle;
            dfloat da = a1 - a0;
            if( da < 0 ) {
               da += 2.0 * pi;
            }
            dfloat t = angle - a0;
            if( t < 0 ) {
               t += 2.0 * pi;
            }
            t = ( da < 1e-9 ) ? 0.0 : ( t / da );
            return samples[ i0 ].radius + t * ( samples[ i1 ].radius - samples[ i0 ].radius );
         };

         dfloat diameterMin = std::numeric_limits< dfloat >::max();
         dfloat diameterMax = 0;
         dfloat minAngle = 0;
         dfloat maxAngle = 0;
         for( auto const& s : samples ) {
            dfloat diameter = s.radius + radiusAt( s.angle + pi );
            if( diameter < diameterMin ) {
               diameterMin = diameter;
               minAngle = s.angle;
            }
            if( diameter > diameterMax ) {
               diameterMax = diameter;
               maxAngle = s.angle;
            }
         }

         output[ 0 ] = diameterMax * scale_;
         output[ 1 ] = diameterMin * scale_;
         output[ 2 ] = maxAngle;
         output[ 3 ] = minAngle;
      }

   private:
      dfloat scale_;
};


} // namespace Feature
} // namespace dip
