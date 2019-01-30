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

import loci.formats.ImageReader;
import loci.formats.FormatTools;
import ome.units.quantity.Length;

import loci.common.services.ServiceFactory;
import loci.formats.meta.IMetadata;
import loci.formats.services.OMEXMLService;

/// Main class called from embedded JVM to load file
public class BioFormatsInterface {
   public static FileInformation Read( String file, long pointer ) throws Exception {
      final Image image = new Image( pointer );
      final ImageReader reader = new ImageReader();
      
      ServiceFactory factory = new ServiceFactory();
      OMEXMLService service = factory.getInstance( OMEXMLService.class );
      IMetadata meta = service.createOMEXMLMetadata();

      reader.setMetadataStore( meta );
      reader.setId( file );
      
      if ( reader.getImageCount() == 1 ) {
         // 2D
         long[] sizes = { reader.getSizeX(), reader.getSizeY() };
         image.SetSizes( sizes );
         image.SetTensorSizes( reader.getRGBChannelCount() );
         image.SetPixelSize( 0, GetPhysicalQuantity( meta.getPixelsPhysicalSizeX( 0 ) ) );
         image.SetPixelSize( 1, GetPhysicalQuantity( meta.getPixelsPhysicalSizeY( 0 ) ) );
         
         if ( reader.isInterleaved() ) {
            long[] strides = { reader.getRGBChannelCount(), reader.getSizeX()*reader.getRGBChannelCount() };
            image.SetStrides( strides );
            image.SetTensorStride( 1 );
         } else {
            long[] strides = { 1, reader.getSizeX() };
            image.SetStrides( strides );
            image.SetTensorStride( reader.getSizeX()*reader.getSizeY() );
         }
      } else {
         // More than 2D. Read only first series, and interpret all images as third dimension.
         long[] sizes = { reader.getSizeX(), reader.getSizeY(), reader.getImageCount() };
         image.SetSizes( sizes );
         image.SetTensorSizes( reader.getRGBChannelCount() );
         image.SetPixelSize( 0, GetPhysicalQuantity( meta.getPixelsPhysicalSizeX( 0 ) ) );
         image.SetPixelSize( 1, GetPhysicalQuantity( meta.getPixelsPhysicalSizeY( 0 ) ) );
         image.SetPixelSize( 2, GetPhysicalQuantity( meta.getPixelsPhysicalSizeZ( 0 ) ) );
         
         if ( reader.isInterleaved() ) {
            long[] strides = { reader.getRGBChannelCount(), reader.getSizeX()*reader.getRGBChannelCount(), reader.getSizeX()*reader.getSizeY()*reader.getRGBChannelCount() };
            image.SetStrides( strides );
            image.SetTensorStride( 1 );
         } else {
            long[] strides = { 1, reader.getSizeX(), reader.getSizeX()*reader.getSizeY() };
            image.SetStrides( strides );
            image.SetTensorStride( reader.getSizeX()*reader.getSizeY()*reader.getImageCount() );
         }
      }
      
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
      
      image.Forge();
      ByteBuffer buf = image.Origin();
      
      for (int ii=0; ii < reader.getImageCount(); ii++)
        buf.put( reader.openBytes( ii ) );
        
      FileInformation info = new FileInformation();
      info.name = file;
      info.fileType = reader.getFormat();
      
      return info;
   }

   protected static PhysicalQuantity GetPhysicalQuantity( Length length ) {
      if ( length == null ) {
         return new PhysicalQuantity( 1, "px" );
      }
      
      return new PhysicalQuantity( length.value().doubleValue(), length.unit().getSymbol() );
   }
}
