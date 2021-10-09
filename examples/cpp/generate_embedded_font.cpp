/*
 * Writes font data to a file for creating src/generation/draw_text_builtin.cpp
 */

#include <fstream>
#include "diplib.h"
#include "diplib/generation.h"

int main() {
   dip::FreeTypeTool freeTypeTool( "/usr/share/fonts/truetype/opensans/static/OpenSans/OpenSans-Regular.ttf" );
   freeTypeTool.SetSize( 14 );

   constexpr dip::uint beginChar = 32;
   constexpr dip::uint endChar = 127;
   constexpr dip::uint nGlyphs = endChar - beginChar + 1;
   dip::uint glyphOrigin[ nGlyphs ];   // start value in array for image data
   dip::uint glyphWidth[ nGlyphs ];    // width of image
   dip::uint glyphHeight[ nGlyphs ];   // height of image
   dip::sint glyphShift[ nGlyphs ];    // horizontal shift for image
   dip::sint glyphBaseline[ nGlyphs ]; // vertical index of baseline
   dip::sint glyphAdvance[ nGlyphs ];  // cursor advance for glyph

   std::ofstream file("draw_text_builtin_data.txt");
   file << "constexpr dip::uint beginChar = 32;\n";
   file << "constexpr dip::uint endChar = 127;\n";
   file << "constexpr dip::uint nGlyphs = endChar - beginChar + 1;\n";
   file << "constexpr dip::uint8 glyphImage[] = {\n";

   char str[2] = { 0, 0 };
   dip::uint origin = 0;
   for( dip::uint ch = beginChar; ch < endChar; ++ch ) {
      // Render glyph
      str[ 0 ] = static_cast< char >( ch );
      auto glyph = freeTypeTool.DrawText( str );
      if( ch == beginChar ) {
         glyphOrigin[ ch - beginChar ] = origin;
         glyphWidth[ ch - beginChar ] = 0;
         glyphHeight[ ch - beginChar ] = 0;
         glyphShift[ ch - beginChar ] = 0;
         glyphBaseline[ ch - beginChar ] = 0;
         glyphAdvance[ ch - beginChar ] = glyph.right[ 0 ] - glyph.left[ 0 ];
         continue;
      }
      // Write image bytes
      DIP_ASSERT( glyph.image.DataType() == dip::DT_UINT8 );
      DIP_ASSERT( glyph.image.Dimensionality() == 2 );
      DIP_ASSERT( glyph.image.TensorElements() == 1 );
      DIP_ASSERT( glyph.image.HasNormalStrides() );
      auto ptr = static_cast< dip::uint8 const* >( glyph.image.Origin());
      file << "      ";
      for( dip::uint ii = 0; ii < glyph.image.NumberOfPixels(); ++ii, ++ptr ) {
         file << int( *ptr ) << ", ";
      }
      file << "\n";
      // Store other glyph data for later
      glyphOrigin[ ch - beginChar ] = origin;
      glyphWidth[ ch - beginChar ] = glyph.image.Size( 0 );
      glyphHeight[ ch - beginChar ] = glyph.image.Size( 1 );
      glyphShift[ ch - beginChar ] = glyph.left[ 0 ];
      glyphBaseline[ ch - beginChar ] = glyph.left[ 1 ] - 1;
      glyphAdvance[ ch - beginChar ] = glyph.right[ 0 ] - glyph.left[ 0 ];
      // Prepare for next one
      origin += glyph.image.NumberOfPixels();
   }
   file << "};\n";
   glyphOrigin[ endChar - beginChar ] = 0;
   glyphWidth[ endChar - beginChar ] = 0;
   glyphHeight[ endChar - beginChar ] = 0;
   glyphShift[ endChar - beginChar ] = 0;
   glyphBaseline[ endChar - beginChar ] = 0;
   glyphAdvance[ endChar - beginChar ] = 0;

   // Write other glyph data
   file << "constexpr dip::uint glyphOrigin[ nGlyphs ] = { ";
   for( auto v : glyphOrigin ) {
      file << v << ", ";
   }
   file << "};\n";
   file << "constexpr dip::uint glyphWidth[ nGlyphs ] = { ";
   for( auto v : glyphWidth ) {
      file << v << ", ";
   }
   file << "};\n";
   file << "constexpr dip::uint glyphHeight[ nGlyphs ] = { ";
   for( auto v : glyphHeight ) {
      file << v << ", ";
   }
   file << "};\n";
   file << "constexpr dip::sint glyphShift[ nGlyphs ] = { ";
   for( auto v : glyphShift ) {
      file << v << ", ";
   }
   file << "};\n";
   file << "constexpr dip::sint glyphBaseline[ nGlyphs ] = { ";
   for( auto v : glyphBaseline ) {
      file << v << ", ";
   }
   file << "};\n";
   file << "constexpr dip::sint glyphAdvance[ nGlyphs ] = { ";
   for( auto v : glyphAdvance ) {
      file << v << ", ";
   }
   file << "};\n";
}
