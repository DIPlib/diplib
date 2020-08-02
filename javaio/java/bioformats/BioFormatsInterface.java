/*
 * DIPlib 3.0
 * This file contains the Bio-Formats JavaIO interface.
 *
 * (c)2019, Wouter Caarls
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

package org.diplib;

import java.nio.ByteBuffer;
import java.util.Arrays;

import loci.formats.ImageReader;
import loci.formats.FormatTools;
import ome.units.quantity.Length;
import ome.units.quantity.Time;

import loci.common.DebugTools;
import loci.common.services.ServiceFactory;
import loci.formats.meta.IMetadata;
import loci.formats.services.OMEXMLService;

/// Main class called from embedded JVM to load file
public class BioFormatsInterface {
   public static FileInformation Read( String file, long pointer ) throws Exception {
      DebugTools.setRootLevel("warn") ;
   
      final Image image = new Image( pointer );
      final ImageReader reader = new ImageReader();
      
      ServiceFactory factory = new ServiceFactory();
      OMEXMLService service = factory.getInstance( OMEXMLService.class );
      IMetadata meta = service.createOMEXMLMetadata();

      reader.setMetadataStore( meta );
      reader.setId( file );
      
      // We assume full 5D data and squeeze afterwards
      PhysicalQuantity[] pixelSize = { GetPhysicalQuantity( meta.getPixelsPhysicalSizeX( 0 ) ), 
                                       GetPhysicalQuantity( meta.getPixelsPhysicalSizeY( 0 ) ),
                                       GetPhysicalQuantity( meta.getPixelsPhysicalSizeZ( 0 ) ),
                                       GetPhysicalQuantity( meta.getPixelsTimeIncrement( 0 ) ) };
      
      long[] sizes = { reader.getSizeX(), reader.getSizeY(), reader.getSizeZ(), reader.getSizeT() };
      image.SetSizes( sizes );
      image.SetTensorSizes( reader.getSizeC() );

      // Undo BioFormats standard of always putting XY first, even if data is interleaved
      String order = reader.getDimensionOrder();
      if ( reader.isInterleaved() )
         order = "CXY" + order.charAt( 3 ) + order.charAt( 4 );

      // Calculate strides
      long[] strides = { 0, 0, 0, 0 };
      long tstride = 0;
      for ( int oo=0, ss=0, stride=1; oo < 5; oo++ ) {
         long sz = 0;
         switch ( order.charAt( oo ) )
         {
            case 'X': sz = reader.getSizeX(); break;
            case 'Y': sz = reader.getSizeY(); break;
            case 'C': sz = reader.getSizeC(); break;
            case 'Z': sz = reader.getSizeZ(); break;
            case 'T': sz = reader.getSizeT(); break;
         }
 		
         if ( order.charAt( oo ) == 'C' )
            tstride = stride;
         else
            strides[ ss++ ] = stride;

         stride *= sz;
      }

      image.SetStrides( strides );
      image.SetTensorStride( tstride );

      switch ( reader.getPixelType() ) {
         case FormatTools.INT8:   image.SetDataType( "INT8" );
                                  break;
         case FormatTools.UINT8:  image.SetDataType( "UINT8" );
                                  break;
         case FormatTools.INT16:  image.SetDataType( "INT16" );
                                  break;
         case FormatTools.UINT16: image.SetDataType( "UINT16" );
                                  break;
         case FormatTools.INT32:  image.SetDataType( "INT32" );
                                  break;
         case FormatTools.UINT32: image.SetDataType( "UINT32" );
                                  break;
         case FormatTools.FLOAT:  image.SetDataType( "SFLOAT" );
                                  break;
         case FormatTools.DOUBLE: image.SetDataType( "DFLOAT" );
                                  break;
      }
      
      image.SetPixelSize( pixelSize );
      
      // Assume that 3-channel images are sRGB
      if ( reader.getRGBChannelCount() == 3 ) {
         image.SetColorSpace( "sRGB" );
      }
      
      image.Forge();
      image.Squeeze();
      ByteBuffer buf = image.Origin();
      
      for (int ii=0; ii < reader.getImageCount(); ii++)
        buf.put( reader.openBytes( ii ) );
        
      FileInformation info = new FileInformation();
      info.name = file;
      info.fileType = reader.getFormat();
      info.dataType = image.DataType();
      info.significantBits = meta.getPixelsSignificantBits( 0 ).getValue();
      info.sizes = image.Sizes();
      if ( image.TensorSizes().length > 0 ) {
         info.tensorElements = image.TensorSizes()[ 0 ];
      } else {
         info.tensorElements = 1;
      }
      info.colorSpace = image.ColorSpace();
      info.pixelSize = image.PixelSize();
      info.numberOfImages = 1;
      //info.history = {};
      
      return info;
   }

   protected static PhysicalQuantity GetPhysicalQuantity( Length length ) {
      if ( length == null ) {
         return new PhysicalQuantity( 1, "px" );
      }
      
      return new PhysicalQuantity( length.value().doubleValue(), length.unit().getSymbol() );
   }

   protected static PhysicalQuantity GetPhysicalQuantity( Time time ) {
      if ( time == null ) {
         return new PhysicalQuantity( 1, "px" );
      }
      
      return new PhysicalQuantity( time.value().doubleValue(), time.unit().getSymbol() );
   }
}
