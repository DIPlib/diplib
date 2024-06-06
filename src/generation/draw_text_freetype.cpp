/*
 * (c)2021, Cris Luengo.
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

#include "diplib/generation.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "diplib.h"

#ifdef DIP_CONFIG_HAS_FREETYPE
#  include <ft2build.h> // IWYU pragma: keep
#  include FT_FREETYPE_H
#  include FT_GLYPH_H
#endif

namespace dip {

#ifdef DIP_CONFIG_HAS_FREETYPE

namespace {

inline bool IsContinuationChar( unsigned char ch ) {
   // When the first two bits are 10, it's a continuation char.
   return ( ch >= 0x80 ) && ( ch <= 0xBF );
}

dip::uint GetNextCodePoint( unsigned char const*& buffer, unsigned char const* end ) {
   // See https://www.unicode.org/versions/Unicode14.0.0/ch03.pdf at D92 (page 123, 124 and 125).
   constexpr char const* ILLEGAL_UNICODE = "Ill-formed UTF-8 encoded string";
   if( buffer[ 0 ] <= 0x7F ) {
      // First bit is 0, it's an ASCII character.
      dip::uint code = buffer[ 0 ];
      buffer += 1;
      return code;
   }
   DIP_THROW_IF( IsContinuationChar( buffer[ 0 ] ), ILLEGAL_UNICODE );
   DIP_THROW_IF(( buffer[ 0 ] == 0xC0 ) || ( buffer[ 0 ] == 0xC1 ), ILLEGAL_UNICODE );
   if( buffer[ 0 ] <= 0xDF ) { // we already established it's >= 0xC2
      // First three bits are 110
      DIP_THROW_IF( buffer + 1 >= end, ILLEGAL_UNICODE );
      DIP_THROW_IF( !IsContinuationChar( buffer[ 1 ] ), ILLEGAL_UNICODE );
      dip::uint code = static_cast< dip::uint >( buffer[ 0 ] & 0b0001'1111u ) << 6u |
                       static_cast< dip::uint >( buffer[ 1 ] & 0b0011'1111u );
      buffer += 2;
      return code;
   }
   if( buffer[ 0 ] <= 0xEF ) { // we already established it's >= 0xE0
      // First four bits are 1110
      DIP_THROW_IF( buffer + 2 >= end, ILLEGAL_UNICODE );
      DIP_THROW_IF( !IsContinuationChar( buffer[ 1 ] ), ILLEGAL_UNICODE ); // There are a few more illegal values for this byte, depending on what the first one was, we ignore that here
      DIP_THROW_IF( !IsContinuationChar( buffer[ 2 ] ), ILLEGAL_UNICODE );
      dip::uint code = static_cast< dip::uint >( buffer[ 0 ] & 0b0000'1111u ) << 12u |
                       static_cast< dip::uint >( buffer[ 1 ] & 0b0011'1111u ) << 6u |
                       static_cast< dip::uint >( buffer[ 2 ] & 0b0011'1111u );
      buffer += 3;
      return code;
   }
   if( buffer[ 0 ] <= 0xF4 ) { // we already established it's >= 0xF0
      // First five bits are 11110
      DIP_THROW_IF( buffer + 3 >= end, ILLEGAL_UNICODE );
      DIP_THROW_IF( !IsContinuationChar( buffer[ 1 ] ), ILLEGAL_UNICODE ); // There are a few more illegal values for this byte, depending on what the first one was, we ignore that here
      DIP_THROW_IF( !IsContinuationChar( buffer[ 2 ] ), ILLEGAL_UNICODE );
      DIP_THROW_IF( !IsContinuationChar( buffer[ 3 ] ), ILLEGAL_UNICODE );
      dip::uint code = static_cast< dip::uint >( buffer[ 0 ] & 0b0000'0111u ) << 18u |
                       static_cast< dip::uint >( buffer[ 1 ] & 0b0011'1111u ) << 12u |
                       static_cast< dip::uint >( buffer[ 2 ] & 0b0011'1111u ) << 6u |
                       static_cast< dip::uint >( buffer[ 3 ] & 0b0011'1111u );
      buffer += 4;
      return code;
   }
   DIP_THROW( ILLEGAL_UNICODE ); // 0xF5--0xFF are illegal too
}

struct GlyphData {
   FT_UInt index = 0;         // glyph index
   FT_Vector pos = { 0, 0 };  // glyph origin on the baseline
   FT_Glyph image = nullptr;  // glyph image
};

struct GlyphSequence {
   std::vector< GlyphData > glyphs;
   FT_BBox boundingBox = { 0, 0, 0, 0 }; // in 1/64th of a pixel
   FT_Vector endPos = { 0, 0 };
};

// The output data structure contains FT_Glyph pointers that it owns and must be destroyed.
// RenderGlyphSequence() must be called exactly once with this data structure to correctly
// destroy the owned pointers.
GlyphSequence GetGlyphSequence(
      FT_Face face,
      String const& text,
      dfloat orientation
) {
   FT_GlyphSlot slot = face->glyph;
   bool use_kerning = FT_HAS_KERNING( face );
   GlyphSequence glyphSequence;
   FT_Vector pen = { 0, 0 };   /* start at (0,0) */
   double advanceScaleX = std::cos( orientation );
   double advanceScaleY = std::sin( orientation );
   FT_Matrix matrix{
         static_cast< FT_Fixed >( advanceScaleX * 0x10000L ),
         static_cast< FT_Fixed >( advanceScaleY * 0x10000L ),
         static_cast< FT_Fixed >(-advanceScaleY * 0x10000L ),
         static_cast< FT_Fixed >( advanceScaleX * 0x10000L ),
   };
   FT_UInt previous = 0;
   auto* str = reinterpret_cast< unsigned char const* >( text.data() );
   auto* end = str + text.size();
   while( str < end ) {
      // Find glyph and kerning
      dip::uint cp = GetNextCodePoint( str, end );
      FT_UInt index = FT_Get_Char_Index( face, cp );
      //std::cout << "Found codepoint " << cp << " at index " << index << '\n';
      if( use_kerning && previous && index ) {
         FT_Vector delta;
         FT_Get_Kerning( face, previous, index, FT_KERNING_DEFAULT, &delta );
         pen.x += delta.x / 64;
         //std::cout << "    delta.x = " << delta.x << '\n';
      }
      //std::cout << "    pen.x = " << pen.x << ", pen.y = " << pen.y << '\n';
      GlyphData glyph{ index, pen, nullptr };
      if( FT_Load_Glyph( face, index, FT_LOAD_DEFAULT )) {
         continue;
      }
      if( FT_Get_Glyph( face->glyph, &( glyph.image ))) {
         continue;
      }
      // Transform glyph image
      FT_Glyph_Transform( glyph.image, &matrix, nullptr );
      // Extend bounding box
      FT_BBox bbox;
      FT_Glyph_Get_CBox( glyph.image, ft_glyph_bbox_pixels, &bbox );
      //std::cout << "    bbox = " << bbox.xMin << ',' << bbox.xMax << ',' << bbox.yMin << ',' << bbox.yMax << '\n';
      glyphSequence.boundingBox.xMin = std::min( glyphSequence.boundingBox.xMin, bbox.xMin + pen.x );
      glyphSequence.boundingBox.xMax = std::max( glyphSequence.boundingBox.xMax, bbox.xMax + pen.x );
      glyphSequence.boundingBox.yMin = std::min( glyphSequence.boundingBox.yMin, bbox.yMin - pen.y ); // minus because FreeType increases y axis upwards
      glyphSequence.boundingBox.yMax = std::max( glyphSequence.boundingBox.yMax, bbox.yMax - pen.y );
      // Save
      glyphSequence.glyphs.push_back( glyph );
      // Prepare for next glyph
      pen.x += round_cast( static_cast< double >( slot->advance.x ) / 64.0 * advanceScaleX );
      pen.y += round_cast( static_cast< double >( slot->advance.x ) / 64.0 * advanceScaleY );
      previous = index;
   }
   glyphSequence.endPos = pen;
   return glyphSequence;
}

// Destroys the FT_Glyph pointers owned by the GlyphSequence.
// This function ust be called exactly once with the GlyphSequence data structure returned by
// GetGlyphSequence().
void RenderGlyphSequence(
      GlyphSequence& glyphSequence,
      Image& out,
      FloatArray const& origin,
      Image::Pixel const& value
) {
   FT_Vector offset{
      static_cast< FT_Pos >( std::round( origin[ 0 ] )),
      static_cast< FT_Pos >( std::round( origin[ 1 ] ))
   };
   FT_Long imageWidth = static_cast< FT_Long >( out.Size( 0 ));
   FT_Long imageLength = static_cast< FT_Long >( out.Size( 1 ));
   for ( auto& glyph : glyphSequence.glyphs ) {
      // Is the bounding box for this glyph within the image?
      FT_BBox bbox;
      FT_Glyph_Get_CBox( glyph.image, ft_glyph_bbox_pixels, &bbox );
      //std::cout << "Glyph " << glyph.index << '\n';
      glyph.pos.x += offset.x;
      glyph.pos.y += offset.y;
      bbox.xMax += glyph.pos.x;
      bbox.xMin += glyph.pos.x;
      bbox.yMax = glyph.pos.y - bbox.yMax;
      bbox.yMin = glyph.pos.y - bbox.yMin;
      std::swap( bbox.yMax, bbox.yMin );
      //std::cout << "    (after) pos.x = " << glyph.pos.x << ", pos.y = " << glyph.pos.y << '\n';
      //std::cout << "    (after) bbox = " << bbox.xMin << ',' << bbox.xMax << ',' << bbox.yMin << ',' << bbox.yMax << '\n';
      if( bbox.xMax >= 0 && bbox.xMin < imageWidth && bbox.yMax >= 0 && bbox.yMin < imageLength ) { // TODO: This is not right...
         // Create the bitmap (modifies the glyph object)
         if( !FT_Glyph_To_Bitmap( &glyph.image, FT_RENDER_MODE_NORMAL, nullptr, true )) {
            FT_BitmapGlyph bitmap = reinterpret_cast< FT_BitmapGlyph >( glyph.image );
            if( bitmap->bitmap.buffer ) {
               DIP_ASSERT( bitmap->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY );
               //std::cout << "    width = " << bitmap->bitmap.width << ", height = " << bitmap->bitmap.rows << ", pitch = " << bitmap->bitmap.pitch << '\n';
               Image mask( NonOwnedRefToDataSegment( bitmap->bitmap.buffer ), bitmap->bitmap.buffer,
                           DT_UINT8, { bitmap->bitmap.width, bitmap->bitmap.rows }, { 1, bitmap->bitmap.pitch }, Tensor{ 1 } );
               IntegerArray pos{ glyph.pos.x + bitmap->left, glyph.pos.y - bitmap->top };
               BlendBandlimitedMask( out, mask, dip::Image( value ), pos );
            }
         }
      }
      // Delete the bitmap, we don't need it any more
      FT_Done_Glyph( glyph.image );
   }
}

constexpr char const* NO_FONT_SET = "No font set";

} // namespace

FreeTypeTool::FreeTypeTool() {
   DIP_THROW_IF( FT_Init_FreeType( &library ), "Could not initialize the FreeType library" );
}

FreeTypeTool::~FreeTypeTool() {
   if( face ) {
      FT_Done_Face( face );
   }
   if( library ) {
      FT_Done_FreeType( library );
   }
}

void FreeTypeTool::SetFont( String const& font ) {
   if( face ) {
      FT_Done_Face( face );
   }
   auto error = FT_New_Face( library, font.c_str(), 0, &face );
   DIP_THROW_IF( error == FT_Err_Unknown_File_Format, "Font file format not recognized" );
   DIP_THROW_IF( error, "Font file not found or could not be read" );
   FT_Set_Char_Size( face, 0, 12l * 64, 72, 72 );
}

void FreeTypeTool::SetSize( dfloat size ) {
   DIP_THROW_IF( !face, NO_FONT_SET );
   DIP_THROW_IF( size <= 0, E::INVALID_PARAMETER );
   FT_Set_Char_Size( face, 0, round_cast( size * 64 ), 72, 72 );
}

void FreeTypeTool::DrawText(
      Image& out,
      String const& text,
      FloatArray origin,
      Image::Pixel const& value,
      dfloat orientation,
      String const& align
) {
   DIP_THROW_IF( !face, NO_FONT_SET );
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( out.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( origin.size() != 2, E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( !value.IsScalar() && ( value.TensorElements() != out.TensorElements() ), E::NTENSORELEM_DONT_MATCH );
   GlyphSequence glyphSequence = GetGlyphSequence( face, text, orientation );
   if( align == S::CENTER ) {
      origin[ 0 ] -= static_cast< double >( glyphSequence.endPos.x ) / 2.0;
      origin[ 1 ] -= static_cast< double >( glyphSequence.endPos.y ) / 2.0;
   } else if( align == S::RIGHT ) {
      origin[ 0 ] -= static_cast< double >( glyphSequence.endPos.x );
      origin[ 1 ] -= static_cast< double >( glyphSequence.endPos.y );
   } else if( align != S::LEFT ) {
      DIP_THROW_INVALID_FLAG( align );
   }
   RenderGlyphSequence( glyphSequence, out, origin, value );
}

FreeTypeTool::TextInfo FreeTypeTool::DrawText(
      String const& text,
      dfloat orientation
) {
   DIP_THROW_IF( !face, NO_FONT_SET );
   GlyphSequence glyphSequence = GetGlyphSequence( face, text, orientation );
   UnsignedArray sizes = {
         static_cast< dip::uint >( std::max( 1l, glyphSequence.boundingBox.xMax - glyphSequence.boundingBox.xMin )),
         static_cast< dip::uint >( std::max( 1l, glyphSequence.boundingBox.yMax - glyphSequence.boundingBox.yMin ))
   };
   dip::sint xpos = -glyphSequence.boundingBox.xMin;
   dip::sint ypos = glyphSequence.boundingBox.yMax;
   TextInfo out;
   out.image.ReForge( sizes, 1, DT_UINT8 );
   out.image.Fill( 0 );
   out.left = { xpos, ypos };
   out.right = { xpos + glyphSequence.endPos.x, ypos + glyphSequence.endPos.y };
   FloatArray origin{ static_cast< double >( xpos ), static_cast< double >( ypos ) };
   RenderGlyphSequence( glyphSequence, out.image, origin, { 255 } );
   return out;
}

#else // DIP_CONFIG_HAS_FREETYPE

FreeTypeTool::FreeTypeTool() {
   DIP_THROW( "DIPlib was compiled without FreeType support" );
}

// There's no way to call these functions below, because we cannot construct the object.
FreeTypeTool::~FreeTypeTool() {}
void FreeTypeTool::SetFont( String const& ) {}
void FreeTypeTool::SetSize( dfloat ) {}
void FreeTypeTool::DrawText( Image&, String const&, FloatArray, Image::Pixel const&, dfloat, String const& ) {}
FreeTypeTool::TextInfo FreeTypeTool::DrawText( String const&, dfloat ) { return {}; }

#endif // DIP_CONFIG_HAS_FREETYPE

} // namespace dip
