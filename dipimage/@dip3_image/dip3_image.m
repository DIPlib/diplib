classdef dip3_image
   %dip3_image   Represents an image
   %   Objects of this class contain all information relevant to define an
   %   image.
   %
   %   TODO: Add more documentation for this class

   % DIPimage 3.0
   %
   % (c)2017, Cris Luengo.
   % Based on original DIPimage code: (c)1999-2014, Delft University of Technology.

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
      %Array - The pixel data, an array of size [C,T,Y,X,Z,...].
      %   The array is either logical or numeric, and never complex.
      %   C is either 1 for real data or 2 for complex data.
      %   T is either 1 for a scalar image or larger for a tensor image.
      %   X, Y, Z, etc. are the spatial dimensions. There do not need to be
      %   any (for a 0D image, a single sample), and there can be as many as
      %   required.
      %
      %   Set this property to replace the image data. The array assigned to
      %   this property is interpreted as described above.
      Array = []
      NDims       % The number of spatial dimensions.
      Size        % The spatial size of the image.
      IsComplex   % True if the data type is complex.
      IsTensor    % True if the image is a tensor image, false if scalar.
      TensorElements % The number of tensor elements.
      TensorSize  % The size of the tensor: [ROWS,COLUMNS].
      %TensorShape - How the tensor is stored.
      %   A single value of class dip_tensor_shape.
      %
      %   Set this property to change how the tensor storage is interpreted.
      %   Assign either a value of class dip_tensor_shape, or a numeric
      %   array representing a size. The setting must be consistent with the
      %   TensorElements property. The tensor size is set to match.
      TensorShape
      DataType    % Gives the MATLAB class corresponding to the image data type.
   end

   % ------- METHODS -------

   methods

      % ------- CONSTRUCTOR -------

      function img = dip3_image(varargin)
         %dip3_image   Constructor
         %   Construct an object with one of the following syntaxes:
         %      IMG = DIP3_IMAGE(IMAGE)
         %      IMG = DIP3_IMAGE(IMAGE,TENSOR_SHAPE)
         %      IMG = DIP3_IMAGE('array',ARRAY)
         %      IMG = DIP3_IMAGE('array',ARRAY,TENSOR_SHAPE)
         %      IMG = DIP3_IMAGE('array',ARRAY,TENSOR_SHAPE,NDIMS)
         %
         %   IMAGE is a matrix representing an image. Its
         %   class must be numeric or logical. It can be complex, but data
         %   will be copied in that case. If TENSOR_SHAPE is given, the
         %   first dimension is taken to be the tensor dimension.
         %   Matrices with only one column or one row are converted to a 1D
         %   image. Matrices with one value are converted to 0D images.
         %   Otherwise, the dimensionality is not affected, and singleton
         %   dimensions are kept.
         %
         %   IMAGE can also be a cell array containing the tensor
         %   components of the image. Each element in the cell array must
         %   be the same class and size.
         %
         %   ARRAY is as the internal representation of the pixel data,
         %   see the dip3_image.Array property.
         %
         %   TENSOR_SHAPE is either a string indicating the shape, a
         %   vector indicating the matrix size, or a struct as the
         %   dip3_image.TensorShape property.
         %
         %   NDIMS is the number of dimensions in the data. It equals
         %   SIZE(ARRAY)-2, but can be set to be larger, to add singleton
         %   dimensions at the end.
         %
         %   TODO: Old behavior was DIP_IMAGE(IMAGE,DATATYPE).

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
            % Handle 1D images properly
            sz = size(data);
            if sum(sz(3:end)>1) == 1
               data = reshape(data,sz(1),sz(2),[]);
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
         if isempty(obj.Data)
            nd = 0;
         else
            nd = ndims(obj.Data) - 2 + obj.TrailingSingletons;
         end
      end

      function sz = get.Size(obj)
         if isempty(obj.Data)
            sz = 0;
         else
            sz = [size(obj.Data),ones(1,obj.TrailingSingletons)];
            if length(sz)==2
               sz = [];
            else
               sz(1:2) = [];
            end
            if length(sz) > 1
               sz(1:2) = sz([2,1]);
            end
         end
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

      function shape = get.TensorShape(obj)
         shape = obj.TensorShapeInternal;
      end

      function dt = get.DataType(obj)
         dt = class(obj.Data);
      end

      function varargout = imsize(obj,dim)
         %IMSIZE   Size of the image.
         %   IMSIZE(B) returns an N-D array containing the lengths of
         %   each dimension in the image in B.
         %
         %   IMSIZE(B,DIM) returns the length of the dimension specified
         %   by the scalar DIM.
         %
         %   [M,N] = IMSIZE(B) returns the number of rows and columns in
         %   separate output variables. [M1,M2,M3,...,MN] = IMSIZE(B)
         %   returns the length of the first N dimensions of B.
         %
         %   If B is an array of images, IMSIZE only works if ISTENSOR is
         %   true.
         sz = obj.Size;
         if nargout > 1
            if nargin ~= 1, error('Unknown command option.'); end
            if sz ~= 0
               for ii=1:min(length(sz),nargout)
                  varargout{ii} = sz(ii);
               end
               for ii=ii+1:nargout
                  varargout{ii} = 1;
               end
            else
               for ii=1:nargout
                  varargout{ii} = 0;
               end
            end
         else
            if nargin > 1
               varargout{1} = sz(dim);
            else
               varargout{1} = sz;
            end
         end
      end

      function [m,n] = imarsize(obj)
         %IMARSIZE   Size of the image array.
         %   IMARSIZE(B) returns a 2D array containing the length of the
         %   two tensor dimensions in the tensor image B.
         %
         %   IMARSIZE(B,DIM) returns the length of the dimension specified
         %   by the scalar DIM.
         %
         %   [M,N] = IMARSIZE(B) returns the number of rows and columns in
         %   separate output variables.
         %
         %   If B is a scalar image, IMARSIZE returns [1,1].
         sz = obj.TensorSize;
         if nargout > 1
            if nargin ~= 1, error('Unknown command option.'); end
            m = sz(1);
            n = sz(2);
         else
            if nargin == 1
               m = sz;
            else
               m = sz(dim);
            end
         end
      end

      function size(~)
         error('SIZE should not be used with DIP_IMAGE objects: use IMSIZE or IMARSIZE');
      end

      function n = numel(obj)
         %NUMEL   Returns the number of samples in the image
         n = numel(obj.Data);
         if obj.IsComplex
            n = n / 2;
         end
      end

      function n = ndims(obj)
         %NDIMS   Returns the number of spatial dimensions
         n = obj.NDims;
      end

      function n = numtensorel(obj)
         %NUMTENSOREL   Returns the number of tensor elements in the image
         n = obj.TensorElements;
      end

      function n = numpixels(obj)
         %NUMPIXELS   Returns the number of pixels in the image
         sz = size(obj.Data);
         n = prod(sz(3:end));
      end

      function res = isempty(obj)
         %ISEMPTY   Returns true if there are no pixels in the image
         res = isempty(obj.Data);
      end

      function dt = datatype(obj)
         %DATATYPE   Returns the data type of the samples (class of underlying data array).
         dt = obj.DataType;
      end

      function res = isfloat(obj)
         %ISFLOAT   Returns true if the image is of a floating-point type (complex or not)
         res = isfloat(obj.Data);
      end

      function res = isinteger(obj)
         %ISINTEGER   Returns true if the image is of an integer type
         res = isinteger(obj.Data);
      end

      function res = issigned(obj)
         %ISSIGNED   Returns true if the image is of a signed integer type
         res = isa(obj.Data,'int8') || isa(obj.Data,'int16') || isa(obj.Data,'int32');
      end

      function res = isunsigned(obj)
         %ISUNSIGNED   Returns true if the image is of an unsigned integer type
         res = isa(obj.Data,'uint8') || isa(obj.Data,'uint16') || isa(obj.Data,'uint32');
      end

      function res = islogical(obj)
         %ISLOGICAL   Returns true if the image is binary
         res = islogical(obj.Data);
      end

      function res = isreal(obj)
         %ISREAL   Returns true if the image is of a non-complex type
         res = ~obj.IsComplex;
      end

      function res = iscomplex(obj)
         %ISCOMPLEX   Returns true if the image is of a complex type
         res = obj.IsComplex;
      end

      function res = isscalar(obj)
         %isscalar   Returns true if the image is scalar
         res = ~obj.IsTensor;
      end

      function res = istensor(~)
         %istensor   Returns true (always) -- for backwards compatibility
         res = true;
      end

      function res = isvector(obj)
         %isvector   Returns true if it is a vector image
         res = obj.TensorShape == dip_tensor_shape.COL_VECTOR || ...
               obj.TensorShape == dip_tensor_shape.ROW_VECTOR;
      end

      function res = iscolumn(obj)
         %iscolumn   Returns true if it is a column vector image
         res = obj.TensorShape == dip_tensor_shape.COL_VECTOR;
      end

      function res = isrow(obj)
         %isrow   Returns true if it is a row vector image
         res = obj.TensorShape == dip_tensor_shape.ROW_VECTOR;
      end

      function res = ismatrix(obj)
         %ismatrix   Returns true (always) -- for backwards compatibility
         res = true;
      end

      function res = iscolor(obj)
         %ISCOLOR   Returns true if color image
         res = ~isempty(obj.ColorSpace);
      end

      function display(obj)
         %DISPLAY   Called when not terminating a statement with a semicolon

         % TODO: check 'DisplayToFigure' preference, yada yada.
         if isequal(get(0,'FormatSpacing'),'compact')
            disp([inputname(1),' ='])
            disp(obj)
         else
            disp(' ')
            disp([inputname(1),' ='])
            disp(' ')
            disp(obj)
            disp(' ')
         end
      end

      function disp(obj)
         % DISP   Display array
         if obj.IsTensor
            sz = obj.TensorSize;
            shape = tensorshapestring(obj.TensorShape);
            tensor = [num2str(sz(1)),'x',num2str(sz(2)),' ',shape];
         end
         if iscolor(obj)
            disp(['Color image (',tensor,', ',obj.ColorSpace,'):']);
         elseif obj.IsTensor
            disp(['Tensor image (',tensor,'):']);
         else
            disp('Scalar image:');
         end
         disp(['    data type ',datatypestring(obj)]);
         if isempty(obj)
            disp('    - empty -');
         else
            disp(['    dimensionality ',num2str(ndims(obj))]);
            if ndims(obj)~=0
               v = imsize(obj);
               sz = num2str(v(1));
               for jj=2:length(v)
                  sz = [sz,'x',num2str(v(jj))];
               end
               disp(['    size ',sz]);
               if ~isempty(obj.PixelSize)
                  % TODO
                  % disp(['        pixel size ',sz]);
               end
            else
               v = 1;
            end
            if prod(v) == 1
               data = double(obj);
               disp(['    value ',mat2str(data,5)]);
            end
         end
      end


      % ------- EXTRACT DATA -------

      function varargout = dip_array(obj,dt)
         %DIP_ARRAY   Extracts the data array from a dip_image.
         %   A = DIP_ARRAY(B) converts the dip_image B to a MATLAB
         %   array without changing the type. A can therefore become
         %   an array of type other than double, which cannot be used
         %   in most MATLAB functions.
         %
         %   [A1,A2,A3,...An] = DIP_ARRAY(B) returns the n images in the
         %   tensor image B in the arrays A1 through An.
         %
         %   DIP_ARRAY(B,DATATYPE) converts the dip_image B to a MATLAB
         %   array of class DATATYE.
         %
         %   If B is a tensor image and only one output argument is given,
         %   the tensor dimension is expandend into extra MATLAB array
         %   dimension, which will be the first dimension.
         %
         %   If B is a tensor image with only one pixel, and only one
         %   output argument is given, an array with the tensor dimensions
         %   is returned. That is, the image dimensions are discarded.
         if nargin == 1
            dt = '';
         elseif ~isstring(dt)
            error('DATATYPE must be a string.')
         end
         % TODO: allow for the old data type aliases
         out = obj.Data;
         sz = size(out);
         if sz(1) > 1
            out = complex(out(1,:),out(2,:));
         end
         if length(sz) == 3
            sz = [sz(1:2),1,sz(3)]; % a 1D image must become a row vector
         else
            sz = [sz,1,1]; % add singleton dimensions at the end, we need at least 2 dimensions at all times!
         end
         if sz(2) == 1
            out = reshape(out,sz(3:end));
         else
            out = reshape(out,sz(2:end));
         end
         if nargout<=1
            if ~isempty(dt)
               out = di_convert(out,dt);
            end
            varargout = {out};
         elseif nargout==sz(2)
            varargout = cell(1,nargout);
            for ii = 1:nargout
               varargout{ii} = reshape(out(ii,:),sz(3:end));
               if ~isempty(dt)
                  varargout{ii} = di_convert(varargout{ii},dt);
               end
            end
         else
            error('Output arguments must match number of tensor elements or be 1.')
         end
      end

      function out = double(in)
         %DOUBLE   Convert dip_image object to double matrix.
         %   A = DOUBLE(B) corresponds to A = DIP_ARRAY(IN,'double')
         %   See also dip_image.dip_array
         out = dip_array(in,'double');
      end
      function out = single(in)
         %SINGLE   Convert dip_image object to single matrix.
         %   A = SINGLE(B) corresponds to A = DIP_ARRAY(IN,'single')
         %   See also dip_image.dip_array
         out = dip_array(in,'single');
      end
      function out = uint32(in)
         %UINT32   Convert dip_image object to uint32 matrix.
         %   A = UINT32(B) corresponds to A = DIP_ARRAY(IN,'uint32')
         %   See also dip_image.dip_array
         out = dip_array(in,'uint32');
      end
      function out = uint16(in)
         %UINT16   Convert dip_image object to uint16 matrix.
         %   A = UINT16(B) corresponds to A = DIP_ARRAY(IN,'uint16')
         %   See also dip_image.dip_array
         out = dip_array(in,'uint16');
      end
      function out = uint8(in)
         %UINT8   Convert dip_image object to uint8 matrix.
         %   A = UINT8(B) corresponds to A = DIP_ARRAY(IN,'uint8')
         %   See also dip_image.dip_array
         out = dip_array(in,'uint8');
      end
      function out = int32(in)
         %INT32   Convert dip_image object to int32 matrix.
         %   A = INT32(B) corresponds to A = DIP_ARRAY(IN,'int32')
         %   See also dip_image.dip_array
         out = dip_array(in,'int32');
      end
      function out = int16(in)
         %INT16   Convert dip_image object to int16 matrix.
         %   A = INT16(B) corresponds to A = DIP_ARRAY(IN,'int16')
         %   See also dip_image.dip_array
         out = dip_array(in,'int16');
      end
      function out = int8(in)
         %INT8   Convert dip_image object to int8 matrix.
         %   A = INT8(B) corresponds to A = DIP_ARRAY(IN,'int8')
         %   See also dip_image.dip_array
         out = dip_array(in,'int8');
      end
      function out = logical(in)
         %LOGICAL   Convert dip_image object to logical matrix.
         %   A = LOGICAL(B) corresponds to A = DIP_ARRAY(IN,'logical')
         %   See also dip_image.dip_array
         out = dip_array(in,'logical');
      end

      % ------- OPERATORS -------

      function out = plus(lhs,rhs) % +
         out = dip_operators('+',lhs,rhs);
      end

      function out = minus(lhs,rhs) % -
         out = dip_operators('-',lhs,rhs);
      end

      function out = mtimes(lhs,rhs) % *
         out = dip_operators('*',lhs,rhs);
      end

      function out = times(lhs,rhs) % .*
         out = dip_operators('.',lhs,rhs);
      end

      function out = mrdivide(lhs,rhs) % /
         if rhs.IsTensor
            error('Not implented');
         end
         out = dip_operators('/',lhs,rhs);
      end

      function out = rdivide(lhs,rhs) % ./
         out = dip_operators('/',lhs,rhs);
      end

      function out = eq(lhs,rhs) % ==
         out = dip_operators('=',lhs,rhs);
      end

      function out = gt(lhs,rhs) % >
         out = dip_operators('>',lhs,rhs);
      end

      function out = lt(lhs,rhs) % <
         out = dip_operators('<',lhs,rhs);
      end

      function out = ge(lhs,rhs) % >=
         out = dip_operators('g',lhs,rhs);
      end

      function out = le(lhs,rhs) % <=
         out = dip_operators('l',lhs,rhs);
      end

      function out = ne(lhs,rhs) % ~=
         out = dip_operators('n',lhs,rhs);
      end

      function out = and(lhs,rhs) % &
         out = dip_operators('&',lhs,rhs);
      end

      function out = or(lhs,rhs) % |
         out = dip_operators('|',lhs,rhs);
      end

      function out = xor(lhs,rhs)
         out = dip_operators('^',lhs,rhs);
      end

      function in = not(in)
         in = dip_operators('~',in);
      end

      function in = uminus(in) % -
         in = dip_operators('u',in);
      end

      function in = uplus(in) % +
         if islogical(in.Data)
            in.Data = uint8(in.Data);
         end
      end

      function in = ctranspose(in) % '
         in = conj(dip_operators('''',in));
      end

      function in = transpose(in) % .'
         in = dip_operators('''',in);
      end

      function in = conj(in)
         if in.IsComplex
            in.Data(2,:) = -in.Data(2,:);
         end
      end

      function in = real(in)
         if in.IsComplex
            sz = size(in.Data);
            sz(1) = 1;
            in.Data = reshape(in.Data(1,:),sz);
         end
      end

      function in = imag(in)
         if in.IsComplex
            sz = size(in.Data);
            sz(1) = 1;
            in.Data = reshape(in.Data(2,:),sz);
         end
      end

   end
end

% ------- PRIVATE FUNCTIONS -------

function res = isstring(in)
   res = ischar(in) && isrow(in);
end

function in = di_convert(in,class)
   %#function int8, uint8, int16, uint16, int32, uint32, int64, uint64, single, double, logical
   in = feval(class,in);
   % TODO: make sure we don't convert complex values to an integer type
end

function str = tensorshapestring(tshape)
   switch(tshape)
      case dip_tensor_shape.COL_VECTOR
         str = 'column vector';
      case dip_tensor_shape.ROW_VECTOR
         str = 'row vector';
      case dip_tensor_shape.COL_MAJOR_MATRIX
         str = 'column-major matrix';
      case dip_tensor_shape.ROW_MAJOR_MATRIX
         str = 'row-major matrix';
      case dip_tensor_shape.DIAGONAL_MATRIX
         str = 'diagonal matrix';
      case dip_tensor_shape.SYMMETRIC_MATRIX
         str = 'symmetric matrix';
      case dip_tensor_shape.UPPTRIANG_MATRIX
         str = 'upper triangular matrix';
      case dip_tensor_shape.LOWTRIANG_MATRIX
         str = 'lower triangular matrix';
      otherwise
         str = '';
   end
end

function str = datatypestring(in)
   str = in.DataType;
   if strcmp(str,'logical')
      str = 'binary';
   end
   if in.IsComplex
    str = [str,' complex'];
   end
end
