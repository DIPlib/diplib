classdef dip3_image
   % dip3_image   Represents an image
   %   Objects of this class contain all information relevant to define an
   %   image.
   %   TODO: Add more documentation for this class
   
   % ------- PROPERTIES -------
   
   properties
      % PixelSize - A cell array indicating the size of a pixel along each of the spatial
      % dimensions
      PixelSize = {}
      % ColorSpace - A string indicating the color space, if any. An empty string
      % indicates there is no color space associated to the image data.
      ColorSpace = ''
   end
   
   properties (Access=private)
      Data = [] % Pixel data, see Array property
      TrailingSingletons = 0 % Number of trailing singleton dimensions.
      TensorShapeInternal = dip_tensor_shape.COL_VECTOR % How the tensor is stored, see TensorShape property.
      TensorSizeInternal = [1,1] % Size of the tensor, see TensorSize property.
   end
   
   properties (Dependent)
      % Array - The pixel data, an array of size [C,T,Y,X,Z,...].
      %    The array is either logical or numeric, and never complex.
      %    C is either 1 for real data or 2 for complex data.
      %    T is either 1 for a scalar image or larger for a tensor image. 
      %    X, Y, Z, etc. are the spatial dimensions. There do not need to be
      %    any (for a 0D image, a single sample), and there can be as many as
      %    required.
      %
      %    Set this property to replace the image data. The array assigned to
      %    this property is interpreted as described above.
      Array = []
      NDims       % The number of spatial dimensions.
      Size        % The spatial size of the image.
      IsComplex   % True if the data type is complex.
      IsTensor    % True if the image is a tensor image.
      TensorElements % The number of tensor elements.
      TensorSize  % The size of the tensor: [ROWS,COLUMNS].
      % TensorShape - How the tensor is stored.
      %    A single value of class dip_tensor_shape.
      %
      %    Set this property to change how the tensor storage is interpreted.
      %    Assign either a value of class dip_tensor_shape, or a numeric
      %    array representing a size. The setting must be consistent with the
      %    TensorElements property. The tensor size is set to match.
      TensorShape 
      IsLogical   % True if the image is binary.
      DataType    % Gives the MATLAB class corresponding to the image data type.
   end
   
   % ------- METHODS -------
   
   methods
      
      % ------- CONSTRUCTOR -------
      
      function img = dip3_image(varargin)
         % dip3_image   Constructor
         %    Construct an object with one of the following syntaxes:
         %       IMG = DIP3_IMAGE(IMAGE)
         %       IMG = DIP3_IMAGE(IMAGE,TENSOR_SHAPE)
         %       IMG = DIP3_IMAGE('array',ARRAY)
         %       IMG = DIP3_IMAGE('array',ARRAY,TENSOR_SHAPE)
         %       IMG = DIP3_IMAGE('array',ARRAY,TENSOR_SHAPE,NDIMS)
         %
         %    IMAGE is a matrix representing an image. Its
         %    class must be numeric or logical. It can be complex, but data
         %    will be copied in that case. If TENSOR_SHAPE is given, the
         %    first dimension is taken to be the tensor dimension.
         %
         %    IMAGE can also be a cell array containing the tensor
         %    components of the image. Each element in the cell array must
         %    be the same class and size.
         %
         %    ARRAY is as the internal representation of the pixel data,
         %    see the dip3_image.Array property.
         %
         %    TENSOR_SHAPE is either a string indicating the shape, a
         %    vector indicating the matrix size, or a struct as the
         %    dip3_image.TensorShape property.
         %
         %    NDIMS is the number of dimensions in the data. It equals
         %    SIZE(ARRAY)-2, but can be set to be larger, to add singleton
         %    dimensions at the end.
         disp('creating dip3_image object')
         % Fix input data
         if nargin < 1
            return
         end
         if isequal(varargin{1},'array')
            % Quick method
            if nargin < 2
               return
            end
            data = varargin{2};
            img.Array = data; % calls set.Array, does checks
            if nargin > 2
               img.TensorShape = varargin{3}; % calls set.TensorShape
            end
            if nargin > 3
               img.NDims = varargin{4}; % calls set.NDims, does checks
            % else: already set to 0 by set.Array.   
            end
         else
            % User-friendly method
            data = varargin{1};
            tensor_shape = [];
            if nargin > 1
               tensor_shape = varargin{2};
            end
            if iscell(data)
               error('Cell data not yet supported');
               % TODO: implement this!
            else
               if isempty(tensor_shape)
                  % Add tensor dimension
                  data = shiftdim(data,-1);
               end
            end
            % Add the complex dimension
            data = shiftdim(data,-1);
            if ~isreal(data)
               data = cat(1,real(data),imag(data));
            end
            % Write properties
            img.Array = data; % calls set.Array
            if ~isempty(tensor_shape)
               img.TensorShape = tensor_shape; % calls set.TensorShape
            end
         end
      end
      
      % ------- SET PROPERTIES -------
      
      function img = set.Array(img,data)
         disp('replacing Array in dip3_image')
         if ( ~islogical(data) && ~isnumeric(data) ) || ~isreal(data)
            error('Pixel data must be real, numeric or logical');
         end
         img.Data = data;
         img.TensorShapeInternal = dip_tensor_shape.COL_VECTOR;
         img.TensorSizeInternal = [size(img.Data,2),1];
         img.TrailingSingletons = 0;
      end
      
      function img = set.NDims(img,nd)
         minnd = ndims(img.Data)-2;
         if ~isscalar(nd) || ~isnumeric(nd) || mod(nd,1) ~= 0 || nd < minnd
            error('NDims must be a scalar value and at least as large as the number of spatial dimensions')
         end
         img.TrailingSingletons = double(nd) - minnd; % convert to double just in case...
      end
      
      function img = set.TensorShape(img,tshape)
         disp('replacing TensorShape in dip3_image')
         nelem = size(img.Data,2);
         if isa(tshape, 'dip_tensor_shape')
            % Figure out what the size must be
            switch tshape
               case dip_tensor_shape.COL_VECTOR
                  tsize = [nelem,1];
               case dip_tensor_shape.ROW_VECTOR
                  tsize = [1,nelem];
               case {dip_tensor_shape.COL_MAJOR_MATRIX,...
                     dip_tensor_shape.ROW_MAJOR_MATRIX}
                  if prod(img.TensorShapeInternal) == nelem
                     tsize = img.TensorShapeInternal;
                  else
                     rows = ceil(sqrt(nelem));
                     tsize = [rows,nelem/rows];
                     if mod(tsize(2),1) ~= 0
                        error('TensorShape value not consistent with number of tensor elements')
                     end
                  end
               case dip_tensor_shape.DIAGONAL_MATRIX
                  tsize = [nelem,nelem];
               case {dip_tensor_shape.SYMMETRIC_MATRIX,...
                     dip_tensor_shape.UPPTRIANG_MATRIX,...
                     dip_tensor_shape.LOWTRIANG_MATRIX}
                  rows = (sqrt(1+8*nelem)-1)/2;
                  if mod(rows,1) ~= 0
                     error('TensorShape value not consistent with number of tensor elements')
                  end
                  tsize = [rows,rows];
               otherwise
                  error('Bad value for TensorShape property')                  
            end
         else
            if ~isnumeric(tshape) || ~isrow(tshape) || isempty(tshape) || ...
                  numel(tshape) > 2 || any(mod(tshape,1) ~= 0)
               error('Bad value for TensorShape property')
            end
            tshape = double(tshape); % convert to double just in case...
            if numel(tshape) == 1
               tsize = [tshape,nelem / tshape];
               if mod(tsize(2),1) ~= 0
                  error('TensorShape value not consistent with number of tensor elements')
               end
            else
               if prod(tshape) ~= nelem
                  error('TensorShape value not consistent with number of tensor elements')
               end
               tsize = tshape;
            end
            tshape = dip_tensor_shape.COL_MAJOR_MATRIX;
         end
         if tsize(2) == 1
            tshape = dip_tensor_shape.COL_VECTOR;
         elseif tsize(1) == 1
            tshape = dip_tensor_shape.ROW_VECTOR;
         end
         img.TensorShapeInternal = tshape;
         img.TensorSizeInternal = tsize;
      end
      
      function img = set.PixelSize(img,pxsz)
         % TODO (how?)
      end
      
      function img = set.ColorSpace(img,colsp)
         if isempty(colsp) || isstring(colsp)
            img.ColorSpace = colsp;
         else
            error('ColorSpace must be a string')
         end
      end
      
      % ------- GET PROPERTIES -------
      
      function data = get.Array(obj)
         data = obj.Data;
      end
      
      function nd = get.NDims(obj)
         nd = ndims(obj.Data) - 2 + obj.TrailingSingletons;
      end
      
      function sz = get.Size(obj)
         sz = [size(obj.Data),ones(1,obj.TrailingSingletons)];
         sz(1:2) = [];
      end
      
      function res = get.IsComplex(obj)
         res = size(obj.Data,1) > 1;
      end
      
      function res = get.IsTensor(obj)
         res = size(obj.Data,2) > 1;
      end
      
      function n = get.TensorElements(obj)
         n = size(obj.Data,2);
      end
      
      function sz = get.TensorSize(obj)
         sz = obj.TensorSizeInternal;
      end
      
      function sz = get.TensorShape(obj)
         sz = obj.TensorShapeInternal;
      end
      
      function res = get.IsLogical(obj)
         res = islogical(obj.Data);
      end
      
      function dt = get.DataType(obj)
         dt = class(obj.Data);
      end
      
   end
end

function res = isstring(in)
   res = ischar(in) && isrow(in);
end
