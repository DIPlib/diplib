%dip_image   Represents an image
%   The DIP_IMAGE object represents an image. All DIPimage functions that
%   output image data do so in the form of a DIP_IMAGE object. However,
%   input image data can be either a DIP_IMAGE or a numeric array.
%
%   An image can have any number of dimensions, including 0 and 1 (as
%   opposed to the standard MATLAB arrays, which always have at least 2
%   dimensions. Image pixels are not necessarily scalar values. The pixels
%   can be tensors of any size, and rank up to 2 (i.e. a matrix; we use the
%   term tensor because it's more generic and also because it matches the
%   terminology often used in the field, e.g. structure tensor, tensor
%   flow, etc.). A color image is an image where the pixels are vectors.
%
%   To index into an image, use the '()' indexing as usual. The '{}'
%   indexing is used to index into the tensor components. For example,
%      A{1}(0,0)
%   returns the first tensor component of the pixel at coordinates (0,0).
%   Note that indexing in the spatial dimensions is 0-based, and the first
%   index is horizontal. Note also that the notation A(0,0){1} is not legal
%   in MATLAB, though we would have liked to support it. The keyword END is
%   only allowed for spatial indices (i.e. within the '()' indexing), not
%   for tensor indexing.
%
%   To create an image, you can call the constructor DIP_IMAGE/DIP_IMAGE,
%   or you can use the functions NEWIM or NEWCOLORIM.
%
%   To determine the size of the image, use IMSIZE, and to determine the
%   size of the tensor, use TENSORSIZE. Many methods that work on numeric
%   arrays also work on DIP_IMAGE objects, though not always identically.
%   For example, NDIMS will return 0 or 1 for some images. ISVECTOR tests 
%   the tensor shape, not the shape of the image. Some other methods also
%   work on the tensor, such as the transpose operator A' and the matrix
%   multiplication A*B.
%
%   A statement that returns a DIP_IMAGE object, when not terminated with a
%   semicolon, will not dump the pixel values to the command line, but
%   instead show the image in an interactive display window. See DIPSHOW to
%   learn more about that window.
%
%   TODO: Add more documentation for this class
%
%   NOTE:
%   The dimensions of an image start at 1. A 2D image has dimensions 1 and
%   2. This is different from how they are counted in DIPlib, which always
%   starts at 0.
%   Tensor indexing is also 1-based, as if they were standard MATLAB
%   matrices. Again, different from DIPlib.
%
%   See also: newim, newcolorim, dip_image.dip_image, dip_image.imsize,
%   dip_image.tensorsize, dip_image.ndims, dipshow

% DIPimage 3.0
%
% (c)2017, Cris Luengo.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
%
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
%
%    http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.

classdef dip_image

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
      Data = []                             % Pixel data, see Array property
      TrailingSingletons = 0                % Number of trailing singleton dimensions.
      TensorShapeInternal = 'column vector' % How the tensor is stored, see TensorShape property.
      TensorSizeInternal = [1,1]            % Size of the tensor: [ROWS,COLUMNS].
   end

   % These dependent properties are mostly meant for intenal use, and use of the MATLAB-DIPlib interface.
   % The user has regular methods to access these properties.

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
      Array
      % The number of spatial dimensions.
      NDims
      % The size of the tensor: [ROWS,COLUMNS].
      TensorSize
      %TensorShape - How the tensor is stored.
      %   A string, one of:
      %      'column vector'
      %      'row vector'
      %      'column-major matrix'
      %      'row-major matrix'
      %      'diagonal matrix'
      %      'symmetric matrix'
      %      'upper triangular matrix'
      %      'lower triangular matrix'
      %
      %   Set this property to change how the tensor storage is interpreted.
      %   Assign either one of the strings above, or a numeric array
      %   representing a size. The setting must be consistent with the
      %   number of tensor elements. The tensor size is set to match.
      TensorShape
   end

   % ------- METHODS -------

   methods

      % ------- CONSTRUCTOR -------

      function img = dip_image(varargin)
         %dip_image   Constructor
         %   Construct an object with one of the following syntaxes:
         %      OUT = DIP_IMAGE(IMAGE)
         %      OUT = DIP_IMAGE(IMAGE,TENSOR_SHAPE,DATATYPE)
         %      OUT = DIP_IMAGE('array',ARRAY)
         %      OUT = DIP_IMAGE('array',ARRAY,TENSOR_SHAPE)
         %      OUT = DIP_IMAGE('array',ARRAY,TENSOR_SHAPE,NDIMS)
         %
         %   IMAGE is a matrix representing an image. Its
         %   class must be numeric or logical. It can be complex, but data
         %   will be copied in that case. If TENSOR_SHAPE is given, the
         %   first dimension is taken to be the tensor dimension.
         %   Matrices with only one column or one row are converted to a 1D
         %   image. Matrices with one value are converted to 0D images.
         %   Otherwise, the dimensionality is not affected, and singleton
         %   dimensions are kept. If IMAGE is an object of class dip_image,
         %   it is kept as is, with possibly a different tensor shape and/or
         %   data type.
         %
         %   IMAGE can also be a cell array containing the tensor
         %   components of the image. Each element in the cell array must
         %   be the same class and size.
         %
         %   ARRAY is as the internal representation of the pixel data,
         %   see the dip_image.Array property.
         %
         %   TENSOR_SHAPE is either a string indicating the shape, a
         %   vector indicating the matrix size, or a struct as the
         %   dip_image.TensorShape property.
         %
         %   DATATYPE is a string representing the required data type of
         %   the created dip_image object. Possible string and aliases are:
         %    - 'binary', aliases 'logical','bin'
         %    - 'uint8'
         %    - 'uint16'
         %    - 'uint32'
         %    - 'int8', alias 'sint8'
         %    - 'int16', alias 'sint16'
         %    - 'int32', alias 'sint32'
         %    - 'single', alias 'sfloat'
         %    - 'double', alias 'dfloat'
         %    - 'scomplex'
         %    - 'dcomplex'
         %
         %   NDIMS is the number of dimensions in the data. It equals
         %   SIZE(ARRAY)-2, but can be set to be larger, to add singleton
         %   dimensions at the end.

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
            datatype = [];
            complex = false;
            if nargin > 1
               if validate_tensor_shape(varargin{2})
                  tensor_shape = varargin{2};
               else
                  [datatype,complex] = matlabtype(varargin{2});
               end
               if nargin > 2
                  if isempty(tensor_shape) && validate_tensor_shape(varargin{3})
                     tensor_shape = varargin{3};
                  elseif isempty(datatype)
                     [datatype,complex] = matlabtype(varargin{3});
                  else
                     error('Unrecognized string');
                  end
                  if nargin > 3
                     error('Too many arguments in call to dip_image constructor')
                  end
               end
            end
            if isa(data,'dip_image')
               % Convert dip_image to new dip_image
               img = data;
               % Data type conversion
               if ~isempty(datatype)
                  if ~complex && iscomplex(img)
                     warning('Ignoring data type conversion: complex data cannot be converted to requested type')
                  else
                     if ~isa(img.Data,datatype)
                        img.Data = array_convert_datatype(img.Data,datatype);
                     end
                     if complex && ~iscomplex(img)
                        img.Data = cat(1,img.Data,zeros(size(img.Data),class(img.Data)));
                     end
                  end
               end
               % Tensor shape
               if ~isempty(tensor_shape)
                  img.TensorShape = tensor_shape; % calls set.TensorShape
               end
            else
               % Create a new dip_image from given data
               if iscell(data)
                  % Convert cell array of images to new dip_image
                  error('Cell data not yet supported');
                  % TODO: implement this!
               elseif isnumeric(data) || islogical(data)
                  % Convert MATLAB array to new dip_image
                  % Data type conversion
                  if ~isempty(datatype)
                     if ~isreal(data) && (datatype ~= 'single') && (datatype ~= 'double')
                        warning('Ignoring data type conversion: complex data cannot be converted to requested type')
                     elseif ~isa(data,datatype)
                        data = array_convert_datatype(data,datatype);
                     end
                  end
                  % Add tensor dimension
                  if isempty(tensor_shape)
                     data = shiftdim(data,-1);
                  end
               else
                  error('Input image is not an image');
               end
               % Add the complex dimension
               data = shiftdim(data,-1);
               if ~isreal(data)
                  data = cat(1,real(data),imag(data));
               elseif complex
                  data = cat(1,data,zeros(size(data),class(data)));
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
      end

      function out = complex(out,im)
         %COMPLEX   Construct complex image from real and imaginary parts.
         %   COMPLEX(A,B) returns the complex result A + Bi, where A and B
         %   are identically sized images of the same data type.
         %
         %   COMPLEX(A) returns the complex result A + 0i, where A must
         %   be real.
         if nargin == 1
            if ~iscomplex(out)
               if isa(out.Data,'double') || isa(out.Data,'uint32') || isa(out.Data,'int32')
                  out.Data = cat(1,double(out.Data),zeros(size(out.Data)));
               else
                  out.Data = cat(1,single(out.Data),zeros(size(out.Data),'single'));
               end
            end
         else % nargin == 2
            if isa(out.Data,'double') || isa(out.Data,'uint32') || isa(out.Data,'int32') || ...
                  isa(im.Data,'double') || isa(im.Data,'uint32') || isa(im.Data,'int32')
               out.Data = cat(1,double(out.Data),double(im.Data));
            else
               out.Data = cat(1,single(out.Data),single(im.Data));
            end
         end
      end

      % ------- SET PROPERTIES -------

      function img = set.Array(img,data)
         if ( ~islogical(data) && ~isnumeric(data) ) || ~isreal(data)
            error('Pixel data must be real, numeric or logical');
         end
         img.Data = data;
         img.TensorShapeInternal = 'column vector';
         img.TensorSizeInternal = [size(img.Data,2),1];
         img.TrailingSingletons = 0;
         %disp('set.Array')
         %disp(img)
      end

      function img = set.NDims(img,nd)
         minnd = ndims(img.Data)-2;
         if ~isscalar(nd) || ~isnumeric(nd) || mod(nd,1) ~= 0 || nd < minnd
            error('NDims must be a scalar value and at least as large as the number of spatial dimensions')
         end
         img.TrailingSingletons = double(nd) - minnd; % convert to double just in case...
         %disp('set.NDims')
         %disp(img)
      end

      function img = set.TensorShape(img,tshape)
         nelem = size(img.Data,2);
         if isstring(tshape)
            % Figure out what the size must be
            switch tshape
               case 'column vector'
                  tsize = [nelem,1];
               case 'row vector'
                  tsize = [1,nelem];
               case {'column-major matrix','row-major matrix'}
                  if prod(img.TensorSizeInternal) == nelem
                     tsize = img.TensorSizeInternal;
                  else
                     rows = ceil(sqrt(nelem));
                     tsize = [rows,nelem/rows];
                     if mod(tsize(2),1) ~= 0
                        error('TensorShape value not consistent with number of tensor elements')
                     end
                  end
               case 'diagonal matrix'
                  tsize = [nelem,nelem];
               case {'symmetric matrix','upper triangular matrix','lower triangular matrix'}
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
            tshape = 'column-major matrix';
         end
         if tsize(2) == 1
            tshape = 'column vector';
         elseif tsize(1) == 1
            tshape = 'row vector';
         end
         img.TensorShapeInternal = tshape;
         img.TensorSizeInternal = tsize;
         %disp('set.TensorShape')
         %disp(img)
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
         %disp('set.ColorSpace')
         %disp(img)
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

      function sz = get.TensorSize(obj)
         sz = obj.TensorSizeInternal;
      end

      function shape = get.TensorShape(obj)
         shape = obj.TensorShapeInternal;
      end

      function varargout = size(obj,dim)
         %SIZE   Size of the image.
         %   SIZE(B) returns an array containing the lengths of each
         %   dimension in the image in B. The array always has at least
         %   two elements, filling in 1 for non-existent dimensions.
         %   Returns [0,0] for an empty image.
         %
         %   SIZE(B,DIM) returns the length of the dimension specified
         %   by the scalar DIM. If DIM > NDIMS(B), returns 1.
         %
         %   [M,N] = SIZE(B) returns the number of rows and columns in
         %   separate output variables. [M1,M2,M3,...,MN] = SIZE(B)
         %   returns the length of the first N dimensions of B.
         %
         %   See also dip_image.imsize, dip_image.tensorsize
         sz = imsize(obj);
         if nargout > 1
            if nargin ~= 1, error('Unknown command option'); end
            varargout = cell(1,nargout);
            if ~isempty(obj)
               n = min(length(sz),nargout);
               for ii=1:n
                  varargout{ii} = sz(ii);
               end
               for ii=n+1:nargout
                  varargout{ii} = 1;
               end
            else
               for ii=1:nargout
                  varargout{ii} = 0;
               end
            end
         else
            if ~isempty(obj)
               if nargin > 1
                  if dim <= length(sz)
                     varargout{1} = sz(dim);
                  else
                     varargout{1} = 1;
                  end
               else
                  if length(sz) == 0
                     varargout{1} = [1,1];
                  elseif length(sz) == 1
                     varargout{1} = [sz,1];
                  else
                     varargout{1} = sz;
                  end
               end
            else
               varargout{1} = [0,0];
            end
         end
      end

      function varargout = imsize(obj,dim)
         %IMSIZE   Size of the image.
         %   IMSIZE(B) returns an array containing the lengths of each
         %   dimension in the image in B. Returns 0 for an empty image.
         %
         %   IMSIZE(B,DIM) returns the length of the dimension specified
         %   by the scalar DIM.
         %
         %   [M,N] = IMSIZE(B) returns the number of rows and columns in
         %   separate output variables. [M1,M2,M3,...,MN] = IMSIZE(B)
         %   returns the length of the first N dimensions of B.
         %
         %   IMSIZE is identical to SIZE, except throws an error when a
         %   non-existing dimension is requested, and the first form always
         %   returns exactly as many values as there are image dimensions.
         %
         %   See also dip_image.tensorsize, dip_image.size
         if isempty(obj)
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
         if nargout > 1
            if nargin ~= 1, error('Unknown command option'); end
            if nargout > length(sz), error('Too many dimensions requested'); end
            varargout = cell(1,nargout);
            if ~isempty(obj)
               for ii=1:nargout
                  varargout{ii} = sz(ii);
               end
            else
               for ii=1:nargout
                  varargout{ii} = 0;
               end
            end
         else
            if ~isempty(obj)
               if nargin > 1
                  if dim <= length(sz)
                     varargout{1} = sz(dim);
                  else
                     error(['Dimension ',num2str(dim),' does not exist.']);
                  end
               else
                  varargout{1} = sz;
               end
            else
               varargout{1} = 0;
            end
         end
      end

      function [m,n] = tensorsize(obj)
         %TENSORSIZE   Size of the image's tensor.
         %   TENSORSIZE(B) returns a 2D array containing the length of the
         %   two tensor dimensions in the tensor image B.
         %
         %   TENSORSIZE(B,DIM) returns the length of the dimension specified
         %   by the scalar DIM.
         %
         %   [M,N] = TENSORSIZE(B) returns the number of rows and columns in
         %   separate output variables.
         %
         %   If B is a scalar image, TENSORSIZE returns [1,1].
         %
         %   See also dip_image.imsize, dip_image.size
         sz = obj.TensorSizeInternal;
         if nargout > 1
            if nargin ~= 1, error('Unknown command option'); end
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

      function varargout = imarsize(obj)
         %IMARSIZE   Size of the image's tensor.
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
         %
         %   IMARSIZE is identical to TENSORSIZE.
         varargout = cell(1,nargout);
         [varargout{:}] = tensorsize(varargin);
      end

      function n = numel(obj)
         %NUMEL   Returns the number of samples in the image
         %
         %   NUMEL(IMG) == NUMPIXELS(IMG) * NUMTENSOREL(IMG)
         %
         %   See also dip_image.numpixels, dip_image.numtensorel, dip_image.ndims
         n = numel(obj.Data);
         if iscomplex(obj)
            n = n / 2;
         end
      end

      function n = ndims(obj)
         %NDIMS   Returns the number of spatial dimensions
         n = obj.NDims;
      end

      function n = numtensorel(obj)
         %NUMTENSOREL   Returns the number of tensor elements in the image
         %
         %   NUMEL(IMG) == NUMPIXELS(IMG) * NUMTENSOREL(IMG)
         %
         %   See also dip_image.numel, dip_image.numpixels
         n = size(obj.Data,2);
      end

      function n = numpixels(obj)
         %NUMPIXELS   Returns the number of pixels in the image
         %
         %   NUMEL(IMG) == NUMPIXELS(IMG) * NUMTENSOREL(IMG)
         %
         %   See also dip_image.numel, dip_image.numtensorel, dip_image.ndims
         if isempty(obj)
            n = 0;
         else
            sz = size(obj.Data);
            n = prod(sz(3:end));
         end
      end

      function res = isempty(obj)
         %ISEMPTY   Returns true if there are no pixels in the image
         res = isempty(obj.Data);
      end

      function dt = datatype(obj)
         %DATATYPE   Returns a string representing  the data type of the samples
         %
         %   These are the strings it returns, and the corresponding MATLAB
         %   classes:
         %    - 'binary':   logical
         %    - 'uint8':    uint8
         %    - 'uint16':   uint16
         %    - 'uint32':   uint32
         %    - 'sint8':    int8
         %    - 'sint16':   int16
         %    - 'sint32':   int32
         %    - 'sfloat',   single
         %    - 'dfloat':   double
         %    - 'scomplex': single, complex
         %    - 'dcomplex': double, complex
         dt = datatypestring(obj);
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
         res = ~iscomplex(obj);
      end

      function res = iscomplex(obj)
         %ISCOMPLEX   Returns true if the image is of a complex type
         res = size(obj.Data,1) > 1;
      end

      function res = isscalar(obj)
         %isscalar   Returns true if the image is scalar
         res = size(obj.Data,2) == 1;
      end

      function res = istensor(~)
         %istensor   Returns true (always) -- for backwards compatibility
         res = true;
      end

      function res = isvector(obj)
         %isvector   Returns true if it is a vector image
         res = strcmp(obj.TensorShapeInternal,'column vector') || ...
               strcmp(obj.TensorShapeInternal,'row vector');
      end

      function res = iscolumn(obj)
         %iscolumn   Returns true if it is a column vector image
         res = strcmp(obj.TensorShapeInternal,'column vector');
      end

      function res = isrow(obj)
         %isrow   Returns true if it is a row vector image
         res = strcmp(obj.TensorShapeInternal,'row vector');
      end

      function res = ismatrix(~)
         %ismatrix   Returns true (always) -- for backwards compatibility
         res = true;
      end

      function res = iscolor(obj)
         %ISCOLOR   Returns true if color image
         res = ~isempty(obj.ColorSpace);
      end

      function res = colorspace(obj)
         %colorspace   Returns the name of the color space, or an empty string if ~ISCOLOR(OBJ).
         res = obj.ColorSpace;
      end

      function display(obj)
         %DISPLAY   Called when not terminating a statement with a semicolon
         if ~isempty(obj) && dipgetpref('DisplayToFigure') && (isscalar(obj) || iscolor(obj))
            sz = imsize(squeeze(obj));
            dims = length(sz);
            if dims >= 1 && dims <= 4
               if all(sz<=dipgetpref('ImageSizeLimit'))
                  h = dipshow(obj,'name',inputname(1));
                  if ~isnumeric(h)
                     if ishandle(h)
                        h = h.Number;
                     end
                  end
                  disp(['Displayed in figure ',num2str(h)])
                  return
               end
            end
         end
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
         if ~isscalar(obj)
            sz = obj.TensorSizeInternal;
            shape = obj.TensorShapeInternal;
            tensor = [num2str(sz(1)),'x',num2str(sz(2)),' ',shape,', ',...
                      num2str(numtensorel(obj)),' elements'];
         end
         if iscolor(obj)
            disp(['Color image (',tensor,', ',obj.ColorSpace,'):']);
         elseif ~isscalar(obj)
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
         %   array of class DATATYE. Various aliases are available, see
         %   dip_image.dip_image for a list of possible data type strings.
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
            error('DATATYPE must be a string')
         else
            [dt,~] = matlabtype(dt);
         end
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
               out = array_convert_datatype(out,dt);
            end
            varargout = {out};
         elseif nargout==sz(2)
            varargout = cell(1,nargout);
            for ii = 1:nargout
               varargout{ii} = reshape(out(ii,:),sz(3:end));
               if ~isempty(dt)
                  varargout{ii} = array_convert_datatype(varargout{ii},dt);
               end
            end
         else
            error('Output arguments must match number of tensor elements or be 1')
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

      % ------- INDEXING -------

      function n = numArgumentsFromSubscript(~,~,~)
         n = 1; % Indexing into a dip_image object always returns a single object.
      end

      function a = subsref(a,s)
         if isempty(a)
            error('Cannot index into empty image')
         end
         if length(s)==1 && strcmp(s(1).type,'.')
            name = s(1).subs;
            if strcmp(name,'Data') || ...
                  strcmp(name,'TrailingSingletons') || ...
                  strcmp(name,'TensorShapeInternal') || ...
                  strcmp(name,'TensorSizeInternal')
               error('Cannot access private properties')
            end
            a = builtin('subsref',a,s); % Call built-in method to access properties
            return
         end
         % Find the indices to use
         sz = imsize(a);
         [s,tsz,tsh,ndims] = construct_subs_struct(s,sz,a);
         if ndims == 1 && length(sz) > 1
            a.Data = reshape(a.Data,size(a.Data,1),size(a.Data,2),[]);
         end
         % Do the indexing!
         a.Data = subsref(a.Data,s);
         a.NDims = ndims;
         a.TensorShapeInternal = tsh;
         a.TensorSizeInternal = tsz;
      end

      function a = subsasgn(a,s,b)
         if isempty(a)
            error('Cannot index into empty image')
         end
         if length(s)==1 && strcmp(s(1).type,'.')
            name = s(1).subs;
            if strcmp(name,'Data') || ...
                  strcmp(name,'TrailingSingletons') || ...
                  strcmp(name,'TensorShapeInternal') || ...
                  strcmp(name,'TensorSizeInternal')
               error('Cannot access private properties')
            end
            a = builtin('subsasgn',a,s,b); % Call built-in method to access properties
            return
         end
         % Find the indices to use
         sz = imsize(a);
         [s,~,~,ndims] = construct_subs_struct(s,sz,a);
         if ndims == 1 && length(sz) > 1
            a.Data = reshape(a.Data,size(a.Data,1),size(a.Data,2),[]);
         end
         % Do the indexing!
         if isa(b,'dip_image')
            b = b.Data;
         end
         a.Data = subsasgn(a.Data,s,b);
         if ndims == 1 && length(sz) > 1
            a.Data = reshape(a.Data,[size(a.Data,1),size(a.Data,2),sz]);
         end
      end

      function ii = end(a,k,n)
         if n == 1
            ii = numpixels(a)-1;
         else
            if n ~= ndims(a)
               error('Number of indices does not match dimensionality')
            end
            ii = imsize(a,k)-1;
         end
      end

      function a = subsindex(a)
         if ~isscalar(a) || ~islogical(a)
            error('Can only index using scalar, binary images')
         end
         a = find(a.Data)-1;
      end

      % ------- RESHAPING -------

      function in = tensortospatial(in,dim)
         if ~isintscalar(dim) || dim < 1, error('Dimension argument must be a positive scalar integer'), end
         nd = in.NDims;
         dim = dim+2;
         n = 1:max(ndims(in.Data)+1,dim);
         if length(n) > 4
            n = n([1:2,4,3,5:end]);
         end
         n = [n(1:dim-1),2,n(dim:end)];
         n(2) = n(end);
         n(end) = [];
         if length(n) > 2
            n = n([1:2,4,3,5:end]);
         end
         in.Data = permute(in.Data,n);
         in.NDims = nd + 1;
         % TODO: pixels sizes!
      end

      function in = spatialtotensor(in,dim)
         if ~isscalar(in), error('Cannot create a tensor dimension, image is not scalar'), end
         if ~isintscalar(dim) || dim < 1 || dim > in.NDims, error('Dimension argument must be a positive scalar integer in the indexing range'), end
         nd = in.NDims;
         dim = dim+2;
         n = 1:ndims(in.Data);
         if length(n) > 4
            n = n([1:2,4,3,5:end]);
         end
         n = n([1,dim,3:dim-1,dim+1:end,2]);
         if length(n) > 2
            n = n([1:2,4,3,5:end]);
         end
         in.Data = permute(in.Data,n);
         in.NDims = nd - 1;
         % TODO: pixels sizes!
      end

      function in = expanddim(in,dims)
         %EXPANDDIM   Appends dimensions of size 1
         %   B = EXPANDDIM(A,N) increases the dimensionality of the image A
         %   to N, if its dimensionality is smaller. The dimensions added will
         %   have a size of 1.
         if ~isintscalar(dims), error('Number of dimensions must be scalar integer'), end
         if ndims(in) < dims
            in.NDims = dims;
         end
      end

      function in = permute(in,k)
         %PERMUTE   Permute image dimensions.
         %   B = PERMUTE(A,ORDER) rearranges the dimensions of A so that they
         %   are in the order specified by the vector ORDER. The array
         %   produced has the same values of A but the order of the subscripts
         %   needed to access any particular element are rearranged as
         %   specified by ORDER.
         %
         %   The elements of ORDER must be a rearrangement of the numbers from
         %   1 to N. However, singleton dimensions of A can be skipped.
         %
         %   ORDER can also include values of 0, which indicate singleton
         %   dimensions to add.
         if ~isnumeric(k) || any(fix(k)~=k)
            error('ORDER must be an integer vector')
         elseif any(k<0) | any(k>in.NDims)
            error('ORDER contains an index out of range')
         end
         k = k(:)'; % Make sure K is a 1xN vector.
         nd = length(k);
         tmp = k;
         tmp(tmp==0) = [];
         if length(unique(tmp)) ~= length(tmp)
            error('ORDER contains a repeated index')
         end
         notused = setdiff(1:in.NDims,tmp);
         sz = imsize(in);
         if any(sz(notused)>1)
            error('ORDER misses some non-singleton dimensions')
         end
         k = [k,notused];
         k(k==0) = max(k) + 1:length(k==0);
         if length(k) > 1
            % Where we say 1, we mean 2.
            I = k==1;
            J = k==2;
            k(I) = 2;
            k(J) = 1;
            % The X index must be the second one for MATLAB.
            k = k([2,1,3:end]);
         end
         in.Data = permute(in.Data,[1,2,k+2]);
         in.NDims = nd;
         % TODO: pixels sizes!
      end

      function [in,nshifts] = shiftdim(in,n)
         %SHIFTDIM   Shift dimensions (reorients/flips an image).
         %   B = SHIFTDIM(X,N) shifts the dimensions of X by N. When N is
         %   positive, SHIFTDIM shifts the dimensions to the left and wraps
         %   the N leading dimensions to the end.  When N is negative,
         %   SHIFTDIM shifts the dimensions to the right and pads with
         %   singletons.
         %
         %   [B,NSHIFTS] = SHIFTDIM(X) returns the array B with the same
         %   number of elements as X but with any leading singleton
         %   dimensions removed. NSHIFTS returns the number of dimensions
         %   that are removed.
         if nargin==1
            % Remove leading singleton dimensions
            sz = imsize(in);
            n = min(find(sz>1));                  % First non-singleton dimension.
            if isempty(n)
               n = length(sz);
            end
            nshifts = n-1;
            if n>1
               in = permute(in,n:length(sz));
            % else don't do anything
            end
         else
            if ~isintscalar(n)
                error('N should be a scalar integer')
            end
            if ~isequal(n,0) && ~isempty(in)
               if n>0
                  % Wrapped shift to the left
                  n = mod(n,in.NDims);
                  order = [n+1:in.NDims,1:n];
                  in = permute(in,order);
               else
                  % Shift to the right (padding with singletons).
                  order = [zeros(1,-n),1:in.NDims];
                  in = permute(in,order);
               end
            end
         end
      end

      function in = reshape(in,varargin)
         %RESHAPE   Change size of an image.
         %   B = RESHAPE(A,M,N,...) returns an image with the same
         %   pixels as A but reshaped to have the size M-by-N-by-...
         %   M*N*... must be the same as PROD(SIZE(A)).
         %
         %   B = RESHAPE(A,[M N ...]) is the same thing.
         %
         %   In general, RESHAPE(A,SIZ) returns an image with the same
         %   elements as A but reshaped to the size SIZ. PROD(SIZ) must be
         %   the same as PROD(SIZE(A)).
         %
         %   Note that RESHAPE takes pixels column-wise from A.
         if nargin > 2
            for ii=1:nargin-1
               if ~isintscalar(varargin{ii}), error('Size arguments must be positive scalar integers'), end
            end
            n = cat(2,varargin{:});
         else
            n = varargin{1}(:)';
            if ~isnumeric(n) || any(mod(n,1)) || any(n<1)
               error('Size vector must be a vector with positive integer elements')
            end
         end
         if length(n)>1
            n = n([2,1,3:end]);
         end
         % TODO: pixel size!
         in.Data = reshape(in.Data,[size(in.Data,1),size(in.Data,2),n]);
         in.NDims = length(n);
      end

      function in = squeeze(in)
         %SQUEEZE   Remove singleton dimensions.
         %   B = SQUEEZE(A) returns an image B with the same elements as
         %   A but with all the singleton dimensions removed. A dimension
         %   is singleton if size(A,dim)==1.
         sz = imsize(in);
         if length(sz)>1
            sz = sz([2,1,3:end]);
         end
         sz(sz==1) = [];
         if length(sz)>1
            sz = sz([2,1,3:end]);
         end
         in = reshape(in,sz);
      end

      function out = cat(dim,varargin)
         %CAT   Concatenate (append, join) images.
         %   CAT(DIM,A,B) concatenates the images A and B along
         %   the dimension DIM.
         %   CAT(1,A,B) is the same as [A,B].
         %   CAT(2,A,B) is the same as [A;B].
         %
         %   CAT(DIM,A1,A2,A3,A4,...) concatenates the input images
         %   A1, A2, etc. along the dimension DIM.
         if nargin < 2, error('Erroneus input'); end
         if ~isintscalar(dim), error('Erroneus input'), end
         out = varargin{1};
         % Collect all images in cell array
         n = nargin-1;
         if n == 1
            return
         end
         in = cell(1,n);
         %in_phys = cell(1,n);
         for ii=1:n
            img = varargin{ii};
            if ~isa(img,'dip_image')
               img = dip_image(img);
            end
            in{ii} = img.Data;
            %in_phys{ii} = img.physDims;
         end
         % Find the correct output datatype
         %out_phys = in_phys{1};
         out_type = di_findtypex(class(in{ii}),class(in{ii}),size(in{ii},2)==2);
         for ii=1:length(in)
            out_type = di_findtypex(out_type,class(in{ii}),size(in{ii},2)==2);
            %out_phys = di_findphysdims(out_phys,in_phys{ii});
         end
         % Convert images to the output type
         for ii=1:length(in)
            if ~strcmp(class(in{ii}),out_type)
               in{ii} = array_convert_datatype(in{ii},out_type);
            end
         end
         % Call CAT
         if dim == 1
            dim = 2;
         elseif dim == 2
            dim = 1;
         end
         out.Data = cat(dim+2,in{:});
      end

      function a = horzcat(varargin)
         a = cat(1,varargin{:});
      end

      function a = vertcat(varargin)
         a = cat(2,varargin{:});
      end

      function in = repmat(in,varargin)
         %REPMAT   Replicate and tile an image.
         %   B = REPMAT(A,M,N,...) replicates and tiles the image A to produce an
         %   image of size SIZE(A).*[M,N]. Any number of dimensions are allowed.
         %
         %   B = REPMAT(A,[M N ...]) produces the same thing.
         if nargin > 2
            for ii=1:nargin-1
               if ~isintscalar(varargin{ii}), error('Size arguments must be positive scalar integers'), end
            end
            n = cat(2,varargin{:});
         else
            n = varargin{1}(:)';
            if ~isnumeric(n) || any(mod(n,1)) || any(n<1)
               error('Size vector must be a vector with positive integer elements')
            end
         end
         switch length(n)
            case 0
               n = [1,1];
            case 1
               n = [n,1];
            otherwise
               n = n([2,1,3:end]);
         end
         n = [1,1,n];
         in.Data = repmat(in.Data,n);
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
         if ~isscalar(rhs)
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
         if iscomplex(in)
            in.Data(2,:) = -in.Data(2,:);
         end
      end

      function in = real(in)
         if iscomplex(in)
            sz = size(in.Data);
            sz(1) = 1;
            in.Data = reshape(in.Data(1,:),sz);
         end
      end

      function in = imag(in)
         if iscomplex(in)
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

function res = isintscalar(in)
   res = isnumeric(in) && numel(in)==1 && ~mod(in,1);
end

function in = array_convert_datatype(in,class)
   %#function int8, uint8, int16, uint16, int32, uint32, int64, uint64, single, double, logical
   in = feval(class,in);
end

% Gives a DIPlib data type string for the data in the image.
function str = datatypestring(in)
   str = class(in.Data);
   switch str
      case 'logical'
         str = 'binary';
      case {'uint8','uint16','uint32'}
         % nothing to do, it's OK.
      case 'int8'
         str = 'sint8';
      case 'int16'
         str = 'sint16';
      case 'int32'
         str = 'sint32';
      case 'single'
         if iscomplex(in)
            str = 'scomplex';
         else
            str = 'sfloat';
         end
      case 'double'
         if iscomplex(in)
            str = 'dcomplex';
         else
            str = 'dfloat';
         end
      otherwise
         error('Internal data of unrecognized type')
   end
end

% Converts a DIPlib data type string or one of the many aliases into a
% MATLAB class string and a complex flag.
function [str,complex] = matlabtype(str)
   if ~isstring(str), error('String expected'); end
   complex = false;
   switch str
      case {'logical','binary','bin'}
         str = 'logical';
      case {'uint8','uint16','uint32'}
         % nothing to do, it's OK.
      case {'int8','sint8'}
         str = 'int8';
      case {'int16','sint16'}
         str = 'int16';
      case {'int32','sint32'}
         str = 'int32';
      case {'single','sfloat'}
         str = 'single';
      case {'double','dfloat'}
         str = 'double';
      case 'scomplex'
         str = 'single';
         complex = true;
      case 'dcomplex'
         str = 'double';
         complex = true;
      otherwise
         error('Unknown data type string')
   end
end

% Determines the output type that should be used for
% concatenation between data of types DT1 and DT2.
% If COMP is true, returns either single or double
function dt_out = di_findtypex(dt1,dt2,comp)
   if strcmp(dt1,dt2)
      % Same type
      dt_out = dt1;
   else
      % Different types
      if strcmp(dt1,'double') || strcmp(dt2,'double')
         dt_out = 'double';
      elseif strcmp(dt1,'single') || strcmp(dt2,'single')
         dt_out = 'single';
      elseif strcmp(dt1,'logical')
         dt_out = dt2;
      elseif strcmp(dt2,'logical')
         dt_out = dt1;
      else
         % All that is left now is INTxx or UINTxx
         if dt1(1:3)=='int'
            dt1_signed = true;
         else
            dt1_signed = false;
         end
         if dt2(1:3)=='int'
            dt2_signed = true;
         else
            dt2_signed = false;
         end
         if dt1_signed && dt2_signed
            % Both signed
            if strcmp(dt1,'int32') || strcmp(dt2,'int32')
               dt_out = 'int32';
            elseif strcmp(dt1,'int16') || strcmp(dt2,'int16')
               dt_out = 'int16';
            else
               dt_out = 'int8';
            end
         else
            if ~dt1_signed && dt2_signed
               tmp = dt1; dt1 = dt2; dt2 = tmp;
            end
            if ~dt1_signed && ~dt2_signed
               % Both unsigned
               if strcmp(dt1,'uint32') || strcmp(dt2,'uint32')
                  dt_out = 'uint32';
               elseif strcmp(dt1,'uint16') || strcmp(dt2,'uint16')
                  dt_out = 'uint16';
               else
                  dt_out = 'uint8';
               end
            else
               % dt1 unsigned, dt2 signed
               if strcmp(dt1,'uint32')
                  dt_out = 'dfloat';
               elseif strcmp(dt1,'uint16') || strcmp(dt2,'sint32')
                  dt_out = 'sint32';
               elseif strcmp(dt1,'uint8') || strcmp(dt2,'sint16')
                  dt_out = 'sint16';
               else
                  dt_out = 'sint8';
               end
            end
         end
      end
   end
   if comp
      if strcmp(dt_out,'double') || strcmp(dt_out,'int32') || strcmp(dt_out,'sint32')
         dt_out = 'double';
      else
         dt_out = 'single';
      end
   end
end

function res = validate_tensor_shape(str)
   if isstring(str)
      res = ismember(str,{
         'column vector'
         'row vector'
         'column-major matrix'
         'row-major matrix'
         'diagonal matrix'
         'symmetric matrix'
         'upper triangular matrix'
         'lower triangular matrix'});
   else
      res = false;
   end
end

% Figures out how to index into an image, used by subsref and subsasgn
function [s,tsz,tsh,ndims] = construct_subs_struct(s,sz,a)
   tensorindex = 0;
   imageindex = 0;
   for ii=1:length(s)
      switch s(ii).type
         case '{}'
            if tensorindex
               error('Illegal indexing')
            end
            tensorindex = ii;
         case '()'
            if imageindex
               error('Illegal indexing')
            end
            imageindex = ii;
         otherwise
            error('Illegal indexing')
      end
   end
   % Find tensor dimension indices
   tsz = a.TensorSizeInternal;
   tsh = a.TensorShapeInternal;
   if tensorindex
      t = s(tensorindex).subs;
      N = numel(t);
      if N==1
         % One element: linear indexing
         telems = t{1};
         if isequal(telems,':')
            telems = 1:numtensorel(a);
         elseif any(telems<1) || any(telems>numtensorel(a))
            error('Tensor index out of range');
         end
         tsz = [length(telems),1];
         tsh = 'column vector';
      elseif N==2
         % Two elements: use lookup table
         ii = t{1};
         if isequal(ii,':')
            ii = 1:tsz(1);
         elseif any(ii<1) || any(ii>tsz(1))
            error('Tensor index out of range');
         end
         jj = t{2};
         if isequal(jj,':')
            jj = 1:tsz(2);
         elseif any(jj<1) || any(jj>tsz(2))
            error('Tensor index out of range');
         end
         stride = tsz(1);
         tsh = 'column-major matrix';
         tsz = [length(ii),length(jj)];
         lut = dip_tensor_indices(a);
         [ii,jj] = ndgrid(ii,jj);
         telems = lut(ii + (jj-1)*stride) + 1;
         telems = telems(:); % make into column vector
         if any(telems == 0)
            error('Indexing into non-stored tensor elements')
            % TODO: return zeros?
         end
      else
         error('Illegal tensor indexing');
      end
   else
      telems = 1:numtensorel(a);
   end
   % Find spatial indices
   ndims = length(sz);
   if imageindex
      s = s(imageindex);
      if length(s.subs) == 1 % (this should produce a 1D image)
         ind = s.subs{1};
         ndims = 1;
         if isa(ind,'dip_image')
            if numel(ind)==1 % You can index with a 0D scalar image, convenience for Piet Verbeek.
               ind = double(ind)+1;
            elseif ~islogical(ind) || ~isscalar(ind)
               error('Only binary scalar images can be used to index')
            elseif ~isequal(imsize(ind),sz) % TODO: allow singleton expansion?
               error('Mask image must have same sizes as image it''s indexing into')
            else
               ind = logical(ind.Data);
               ind = ind(:);
               ind = find(ind);
            end
         elseif isnumeric(ind)
            ind = ind+1;
            if any(ind > prod(sz)) || any(ind < 1)
               error('Index exceeds image dimensions')
            end
         elseif islogical(ind)
            msz = size(ind);
            msz = msz([2,1,3:end]);
            % TODO: remove trailing singleton dimensions from `sz` for comparison
            if ~isequal(msz,sz)
               error('Mask image must match image size when indexing')
            end
            ind = ind(:);
            ind = find(ind);
            % else it's ':'
         end
         s.subs{1} = ind;
      elseif length(s.subs) == length(sz)
         tmp = s.subs(2);
         s.subs(2) = s.subs(1);
         s.subs(1) = tmp;
         for ii=1:length(sz)
            ind = s.subs{ii};
            if islogical(ind)
               error('Illegal indexing')
            elseif isa(ind,'dip_image')
               if ndims(squeeze(ind))==0 %added for Piet (BR)
                  ind=double(ind);
               else
                  error('Illegal indexing');
               end
            end
            if isnumeric(ind)
               ind = ind+1;
               if any(ind > sz(ii)) || any(ind < 1)
                  error('Index exceeds image dimensions')
               end
            end
            s.subs{ii} = ind;
         end
      else
         error('Number of indices not the same as image dimensionality')
      end
   else
      s = substruct('()',repmat({':'},1,length(sz)));
   end
   % Combine spatial and tensor indexing, add indexing into complex dimension
   s.subs = [{':'},{telems},s.subs];
end
