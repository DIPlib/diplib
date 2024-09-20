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
%   The assignment
%      A(0,0) = [255,0,0]
%   assigns the three values into the three components of the tensor image
%   A. This type of assignment is triggered if A is a tensor image and one
%   of the dimensions of the assigned matrix matches the number of tensor
%   elements in A.
%
%   To create an image, you can call the constructor DIP_IMAGE/DIP_IMAGE,
%   or you can use the functions NEWIM, NEWTENSORIM or NEWCOLORIM.
%
%   To determine the size of the image, use IMSIZE, and to determine the
%   size of the tensor, use TENSORSIZE. Many methods that work on numeric
%   arrays also work on DIP_IMAGE objects, though not always identically.
%   For example, NDIMS will return 0 or 1 for some images. ISVECTOR tests
%   the tensor shape, not the shape of the image.
%
%   Matrix operators (i.e. A*B, DIAG(A), EIG(A), etc.) apply to the tensor
%   at each pixel. That is, the image itself is not seen as a matrix, but
%   the value at each pixel is. To apply matrix operations to an image as
%   if the image itself were a matrix, extract the image data using the
%   DIP_ARRAY method. This method only copies the data if the image is
%   complex. The method DOUBLE converts the pixel data to class double.
%
%   An image A has NUMPIXELS(A) pixels, NUMTENSOREL(A) tensor elements,
%   and NUMEL(A) samples. Note that:
%      NUMEL(A) == NUMPIXELS(A)*NUMTENSOREL(A)
%      NUMPIXELS(A) == PROD(IMSIZE(A)) == PROD(SIZE(A))
%      NUMTENSOREL(A) ~= PROD(TENSORSIZE(A))
%   The tensor size inequality is due to the compact representation of
%   symmetric, diagonal and triangular tensors. For example, a 3x3 diagonal
%   tensor image has only 3 tensor elements (we don't store the 0-valued
%   off-diagonal elements). For information on how specific tensor shapes
%   are stored, see the DIPlib documentation: dip::Tensor::Shape.
%
%   A statement that returns a DIP_IMAGE object, when not terminated with a
%   semicolon, will not dump the pixel values to the command line, but
%   instead show the image in an interactive display window. See DIPSHOW to
%   learn more about that window.
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

% (c)2017-2021, Cris Luengo.
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
      %PixelSize - The physical size of a pixel.
      %   A struct array indicating the size of a pixel along each of the spatial
      %   dimensions. The struct contains fields 'magnitude' and 'units'.
      %   'magnitude' is a double scalar value, and 'units' is a string formatted
      %   according to the dip::Units class in DIPlib.
      %
      %   It is possible to directly modify this struct:
      %      img.PixelSize = [1,1,3];  % assumes 'm' for units
      %      img.PixelSize = struct('magnitude',{1,1,3},'units','um'); same, but micrometer
      %      img.PixelSize(3).magnitude = 2.5; % only valid if struct already had 3 values
      PixelSize = struct('magnitude',{},'units',{})
      %ColorSpace - A string indicating the color space, if any.
      %   An empty string indicates there is no color space associated to the
      %   image data. Changing this value does not modify the pixel values, use the
      %   COLORSPACE method to change the color space representation of the image.
      %   See also DIP_IMAGE/COLORSPACE.
      ColorSpace = ''
   end

   properties (Access=private)
      Data = []                             % Pixel data, see Array property
      TrailingSingletons = 0                % Number of trailing singleton dimensions.
      TensorShapeInternal = 'column vector' % How the tensor is stored, see TensorShape property.
      TensorSizeInternal = [1,1]            % Size of the tensor: [ROWS,COLUMNS].
   end

   % These dependent properties are mostly meant for intenal use, and use of the
   % MATLAB-DIPlib interface. The user has regular methods to access these properties.

   properties (Dependent)
      %Array - The pixel data, an array of size [C,T,Y,X,Z,...].
      %   The array is either logical or numeric, and never complex.
      %   C is either 1 for real data or 2 for complex data.
      %   T is either 1 for a scalar image or larger for a tensor image.
      %   X, Y, Z, etc. are the spatial dimensions. There do not need to be
      %   any (for a 0D image, a single sample), and there can be as many as
      %   required. A 1D image has sizes [C,T,X], only images with two or
      %   more dimensions store the X dimension in the 4th index.
      %
      %   Set this property to replace the image data. The array assigned to
      %   this property is interpreted as described above.
      Array
      %NDims - The number of spatial dimensions.
      NDims
      %TensorSize - The size of the tensor: [ROWS,COLUMNS].
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
         %dip_image   Constructor.
         %   Construct an object with one of the following syntaxes:
         %      OUT = DIP_IMAGE(IMAGE)
         %      OUT = DIP_IMAGE(IMAGE,TENSOR_SHAPE,DATATYPE)
         %      OUT = DIP_IMAGE(IMAGE,DATATYPE,TENSOR_SHAPE)
         %      OUT = DIP_IMAGE('array',ARRAY)
         %      OUT = DIP_IMAGE('array',ARRAY,TENSOR_SHAPE)
         %      OUT = DIP_IMAGE('array',ARRAY,TENSOR_SHAPE,NDIMS)
         %
         %   IMAGE is a matrix representing an image. Its
         %   class must be numeric or logical. It can be complex, but data
         %   will be copied in that case. If TENSOR_SHAPE is given, the first
         %   dimension is taken to be the tensor dimension. Matrices with only
         %   one column or one row are converted to a 1D image. Matrices with
         %   one value are converted to 0D images. Otherwise, the dimensionality
         %   is not affected, and singleton dimensions are kept. If IMAGE is an
         %   object of class dip_image, it is kept as is, with possibly a
         %   different tensor shape and/or data type.
         %
         %   IMAGE can also be a cell array containing the tensor
         %   components of the image. Each element in the cell array must
         %   be scalar image and have the same size.
         %
         %   ARRAY is as the internal representation of the pixel data,
         %   see the dip_image.Array property.
         %
         %   TENSOR_SHAPE is either a string indicating the shape, or a
         %   vector indicating the matrix size. A 1 indicates a scalar image.
         %
         %   DATATYPE is a string representing the required data type of
         %   the created dip_image object. Possible string and aliases are:
         %    - 'binary' == 'logical' == 'bin' == 'bin8'
         %    - 'uint8'
         %    - 'uint16'
         %    - 'uint32' == 'uint'
         %    - 'uint64'
         %    - 'int8' == 'sint8'
         %    - 'int16' == 'sint16'
         %    - 'int32' == 'sint32' == 'int'
         %    - 'int64' == 'sint64'
         %    - 'single' == 'sfloat' == 'float'
         %    - 'double' == 'dfloat'
         %    - 'scomplex'
         %    - 'dcomplex' == 'complex'
         %
         %   NDIMS is the number of dimensions in the data. It equals
         %   SIZE(ARRAY)-2, but can be set to be larger, to add singleton
         %   dimensions at the end.

         % Fix input data
         if nargin < 1
            img.Data = zeros(1,1,0,'single');
            return
         end
         if isequal(varargin{1},'array')
            % Quick method
            if nargin < 2
               img.Data = zeros(1,1,0,'single');
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
            return
         end
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
               % Convert cell array of scalar images to tensor dip_image
               for ii=1:numel(data)
                  tmp = data{ii};
                  if ~isa(tmp,'dip_image')
                     tmp = dip_image(tmp);
                  end
                  if ~isscalar(tmp)
                     error('Images in cell array must be scalar')
                  end
                  sz = size(tmp.Data);
                  sz = [sz(1:2),1,sz(3:end)]; % add a 2nd dimension of size 1, we'll concatenate along this dimension
                  tmp.Data = reshape(tmp.Data,sz);
                  data{ii} = tmp;
               end
               img = cat(2,data{:}); % concatenate along 2nd dimension. Note that the 2nd dimension is the fastest changing one, the first spatial dimension in storage order
               sz = size(img.Data);
               sz = sz([1,3:end]); % make 2nd dimension the tensor dimension
               img.Data = reshape(img.Data,sz);
               tshape = size(data);
               if numel(tshape) > 2
                  tshape = [prod(tshape),1];
               end
               if prod(tshape) ~= size(img.Data,2)
                  tshape = [size(img.Data,2),1]; % this shouldn't happen!
               end
               img.TensorSizeInternal = tshape;
               if isempty(tensor_shape)
                  if tshape(2) == 1
                     tensor_shape = 'column vector';
                  elseif tshape(1) == 1
                     tensor_shape = 'row vector';
                  else
                     tensor_shape = 'column-major matrix';
                  end
               end
               img.TensorShape = tensor_shape; % calls set.TensorShape
               return
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

      % ------- SET PROPERTIES -------

      function img = set.Array(img,data)
         if ( ~islogical(data) && ~isnumeric(data) ) || ~isreal(data)
            error('Pixel data must be real, and numeric or logical');
         end
         if isempty(data)
            data = reshape(data,1,1,0);
         elseif size(data,1) < 1 || size(data,1) > 2
            error('Pixel data must have a 1st dimension with 1 (real) or 2 (complex) elements');
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
         if ~isintscalar(nd) || nd < minnd
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
                     if ~isint(tsize(2))
                        error('TensorShape value not consistent with number of tensor elements')
                     end
                  end
               case 'diagonal matrix'
                  tsize = [nelem,nelem];
               case {'symmetric matrix','upper triangular matrix','lower triangular matrix'}
                  rows = (sqrt(1+8*nelem)-1)/2;
                  if ~isint(rows)
                     error('TensorShape value not consistent with number of tensor elements')
                  end
                  tsize = [rows,rows];
               otherwise
                  error('Bad value for TensorShape property')
            end
         else
            if ~isint(tshape) || ~isrow(tshape) || isempty(tshape) || numel(tshape) > 2
               error('Bad value for TensorShape property')
            end
            tshape = double(tshape); % convert to double just in case...
            if numel(tshape) == 1
               tsize = [tshape,nelem / tshape];
               if ~isint(tsize(2))
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
         % Make sure we don't use more elements than image dimensions
         pxsz = validatePixelSize(pxsz);
         % Make the array as short as possible, given that we automatically replicate the last array element to cover all image dimensions
         nd = numel(pxsz);
         while nd > 1 && isequal(pxsz(nd),pxsz(nd-1))
            nd = nd-1;
         end
         % If isotropic, make sure we don't have pixels as units
         if nd == 1 && ~isempty(strfind(pxsz(1).units,'px'))
            nd = 0;
         end
         img.PixelSize = pxsz(1:nd);
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
               n = min(numel(sz),nargout);
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
                  if dim <= numel(sz)
                     varargout{1} = sz(dim);
                  else
                     varargout{1} = 1;
                  end
               else
                  if isempty(sz)
                     varargout{1} = [1,1];
                  elseif numel(sz) == 1
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
         varargout = cell(1,nargout);
         if isempty(obj)
            [varargout{:}] = deal(0);
            return
         end
         sz = [size(obj.Data),ones(1,obj.TrailingSingletons)];
         if numel(sz)==2
            sz = [];
         else
            sz(1:2) = [];
         end
         if numel(sz) > 1
            sz(1:2) = sz([2,1]);
         end
         if nargout > 1
            if nargin ~= 1, error('Unknown command option'); end
            if nargout > numel(sz), error('Too many dimensions requested'); end
            varargout = cell(1,nargout);
            for ii=1:nargout
               varargout{ii} = sz(ii);
            end
         else
            if nargin > 1
               if dim <= numel(sz) && dim >= 1
                  varargout{1} = sz(dim);
               else
                  error(['Dimension ',num2str(dim),' does not exist.']);
               end
            else
               varargout{1} = sz;
            end
         end
      end

      function [m,n] = tensorsize(obj,dim)
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
         %   NUMTENSOREL(IMG) is not the same as PROD(TENSORSIZE(IMG)), since it
         %   depends on IMG.TensorShape how the tensor is stored. For example, a
         %   diagonal tensor of size 3x3 has TENSORSIZE == [3,3], and
         %   NUMTENSOREL == 3.
         %
         %   When using linear tensor indexing IMG{I}, I must be between 1 and
         %   NUMTENSOREL. When using matrix tensor indexing IMG{I,J}, I and J are
         %   bounded by TENSORSIZE.
         %
         %   See also dip_image.imsize, dip_image.size, dip_image.numtensorel
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
         %IMARSIZE   Alias of TENSORSIZE for backwards compatibility.
         varargout = cell(1,nargout);
         [varargout{:}] = tensorsize(obj);
      end

      function n = numel(obj)
         %NUMEL   Returns the number of samples in the image.
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
         %NDIMS   Returns the number of spatial dimensions.
         %
         %   As opposed to standard MATLAB arrays, NDIMS(IMG) for DIP_IMAGE objects
         %   can return 0 or 1. NDIMS(IMG) is always equal to LENGTH(IMSIZE(IMG))
         n = obj.NDims;
      end

      function n = numtensorel(obj)
         %NUMTENSOREL   Returns the number of tensor elements in the image.
         %
         %   NUMEL(IMG) == NUMPIXELS(IMG) * NUMTENSOREL(IMG)
         %
         %   NUMTENSOREL(IMG) is not the same as PROD(TENSORSIZE(IMG)), since it
         %   depends on IMG.TensorShape how the tensor is stored. For example, a
         %   diagonal tensor of size 3x3 has TENSORSIZE == [3,3], and
         %   NUMTENSOREL == 3.
         %
         %   When using linear tensor indexing IMG{I}, I must be between 1 and
         %   NUMTENSOREL. When using matrix tensor indexing IMG{I,J}, I and J are
         %   bounded by TENSORSIZE.
         %
         %   See also dip_image.numel, dip_image.numpixels, dip_image.TensorShape
         n = size(obj.Data,2);
      end

      function n = numpixels(obj)
         %NUMPIXELS   Returns the number of pixels in the image.
         %
         %   NUMEL(IMG) == NUMPIXELS(IMG) * NUMTENSOREL(IMG)
         %
         %   NUMPIXELS(IMG) == PROD(IMSIZE(IMG))
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
         %ISEMPTY   Returns true if there are no pixels in the image.
         res = isempty(obj.Data);
      end

      function dt = datatype(obj)
         %DATATYPE   Returns a string representing the data type of the samples.
         %
         %   These are the strings it returns, and the corresponding MATLAB
         %   classes:
         %    - 'binary':   logical
         %    - 'uint8':    uint8
         %    - 'uint16':   uint16
         %    - 'uint32':   uint32
         %    - 'uint64':   uint64
         %    - 'sint8':    int8
         %    - 'sint16':   int16
         %    - 'sint32':   int32
         %    - 'sint64':   int64
         %    - 'sfloat',   single
         %    - 'dfloat':   double
         %    - 'scomplex': single, complex
         %    - 'dcomplex': double, complex
         dt = datatypestring(obj);
      end

      function res = isfloat(obj)
         %ISFLOAT   Returns true if the image is of a floating-point type (complex or not).
         res = isfloat(obj.Data);
      end

      function res = isinteger(obj)
         %ISINTEGER   Returns true if the image is of an integer type.
         res = isinteger(obj.Data);
      end

      function res = issigned(obj)
         %ISSIGNED   Returns true if the image is of a signed integer type.
         res = isa(obj.Data,'int8') || isa(obj.Data,'int16') || isa(obj.Data,'int32') || isa(obj.Data,'int64');
      end

      function res = isunsigned(obj)
         %ISUNSIGNED   Returns true if the image is of an unsigned integer type.
         res = isa(obj.Data,'uint8') || isa(obj.Data,'uint16') || isa(obj.Data,'uint32') || isa(obj.Data,'uint64');
      end

      function res = islogical(obj)
         %ISLOGICAL   Returns true if the image is binary.
         res = islogical(obj.Data);
      end

      function res = isreal(obj)
         %ISREAL   Returns true if the image is of a non-complex type.
         res = ~iscomplex(obj);
      end

      function res = iscomplex(obj)
         %ISCOMPLEX   Returns true if the image is of a complex type.
         res = size(obj.Data,1) > 1;
      end

      function res = isscalar(obj)
         %ISSCALAR   Returns true if the image is scalar (has a single tensor component).
         res = size(obj.Data,2) == 1;
      end

      function res = istensor(~)
         %ISTENSOR   Returns true (always) -- for backwards compatibility.
         res = true;
      end

      function res = isvector(obj)
         %ISVECTOR   Returns true if it is a vector image.
         %
         %   Note that scalar images are vector images.
         res = strcmp(obj.TensorShapeInternal,'column vector') || ...
               strcmp(obj.TensorShapeInternal,'row vector');
      end

      function res = iscolumn(obj)
         %ISCOLUMN   Returns true if it is a column vector image.
         %
         %   Note that scalar images are column vector images.
         res = strcmp(obj.TensorShapeInternal,'column vector');
      end

      function res = isrow(obj)
         %ISROW   Returns true if it is a row vector image.
         %
         %   Note that scalar images are usually not row vector images.
         res = strcmp(obj.TensorShapeInternal,'row vector');
      end

      function res = ismatrix(~)
         %ISMATRIX   Returns true (always) -- for backwards compatibility.
         res = true;
      end

      function res = iscolor(obj)
         %ISCOLOR   Returns true if color image.
         res = ~isempty(obj.ColorSpace);
      end

      function display(obj)
         %DISPLAY   Called when not terminating a statement with a semicolon.
         if ~isempty(obj) && numpixels(obj)>1 && dipgetpref('DisplayToFigure')
            sz = imsize(obj);
            if all(sz<=dipgetpref('ImageSizeLimit'))
               switch dipgetpref('DisplayFunction')
                  case 'viewslice'
                     viewslice(obj,inputname(1));
                     disp('Displayed to new DIPviewer figure')
                     return
                  case 'view5d'
                     dims = sum(sz>1);
                     if numtensorel(obj) > 1 && dims >= 2
                        dims = dims + 1;
                     end
                     if dims >= 2 && dims <= 5
                        h = view5d(obj);
                        h.NameWindow(inputname(1));
                        disp('Displayed to new View5D figure')
                        return
                     end
                  case 'dipshow'
                     dims = sum(sz>1);
                     if (isscalar(obj) || iscolor(obj)) && (dims >= 1 && dims <= 4)
                        h = dipshow(obj,'name',inputname(1));
                        if ~isnumeric(h) && ishandle(h)
                           h = h.Number;
                        end
                        disp(['Displayed in figure ',num2str(h)])
                        return
                     end
                  otherwise
                     error('Invalid value for option ''DisplayFunction''')
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
         % DISP   Display information on the DIP_IMAGE object.
         if isempty(obj)
            disp('Empty image:');
         else
            sz = obj.TensorSizeInternal;
            shape = obj.TensorShapeInternal;
            tensor = [num2str(sz(1)),'x',num2str(sz(2)),' ',shape,', ',...
                      num2str(numtensorel(obj)),' elements'];
            if iscolor(obj)
               disp(['Color image (',tensor,', ',obj.ColorSpace,'):']);
            elseif isscalar(obj)
               disp('Scalar image:');
            else
               disp(['Tensor image (',tensor,'):']);
            end
         end
         disp(['    data type ',datatypestring(obj)]);
         if ~isempty(obj)
            disp(['    dimensionality ',num2str(ndims(obj))]);
            if ndims(obj)~=0
               v = imsize(obj);
               sz = num2str(v(1));
               for jj=2:numel(v)
                  sz = [sz,'x',num2str(v(jj))];
               end
               disp(['    size ',sz]);
               if ~isempty(obj.PixelSize)
                  pxsz = obj.PixelSize;
                  if numel(pxsz)>numel(v)
                     pxsz = pxsz(1:numel(v)); % don't use all elements if there are too many
                  else
                     pxsz(end+1:numel(v)) = pxsz(end); % replicate the last element across all dimensions
                  end
                  sz = [num2str(pxsz(1).magnitude),' ',pxsz(1).units];
                  for jj=2:numel(pxsz)
                     sz = [sz,' x ',num2str(pxsz(jj).magnitude),' ',pxsz(jj).units];
                  end
                  disp(['    pixel size ',sz]);
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
         if numel(sz) == 3
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

      function im = double(im)
         %DOUBLE   Convert dip_image object to double matrix.
         %   A = DOUBLE(B) corresponds to A = DIP_ARRAY(IN,'double')
         %   See also dip_image.dip_array
         im = dip_array(im,'double');
      end
      function im = single(im)
         %SINGLE   Convert dip_image object to single matrix.
         %   A = SINGLE(B) corresponds to A = DIP_ARRAY(IN,'single')
         %   See also dip_image.dip_array
         im = dip_array(im,'single');
      end
      function im = uint64(im)
         %UINT64   Convert dip_image object to uint64 matrix.
         %   A = UINT64(B) corresponds to A = DIP_ARRAY(IN,'uint64')
         %   See also dip_image.dip_array
         im = dip_array(im,'uint64');
      end
      function im = uint32(im)
         %UINT32   Convert dip_image object to uint32 matrix.
         %   A = UINT32(B) corresponds to A = DIP_ARRAY(IN,'uint32')
         %   See also dip_image.dip_array
         im = dip_array(im,'uint32');
      end
      function im = uint16(im)
         %UINT16   Convert dip_image object to uint16 matrix.
         %   A = UINT16(B) corresponds to A = DIP_ARRAY(IN,'uint16')
         %   See also dip_image.dip_array
         im = dip_array(im,'uint16');
      end
      function im = uint8(im)
         %UINT8   Convert dip_image object to uint8 matrix.
         %   A = UINT8(B) corresponds to A = DIP_ARRAY(IN,'uint8')
         %   See also dip_image.dip_array
         im = dip_array(im,'uint8');
      end
      function im = int64(im)
         %INT64   Convert dip_image object to int64 matrix.
         %   A = INT64(B) corresponds to A = DIP_ARRAY(IN,'int64')
         %   See also dip_image.dip_array
         im = dip_array(im,'int64');
      end
      function im = int32(im)
         %INT32   Convert dip_image object to int32 matrix.
         %   A = INT32(B) corresponds to A = DIP_ARRAY(IN,'int32')
         %   See also dip_image.dip_array
         im = dip_array(im,'int32');
      end
      function im = int16(im)
         %INT16   Convert dip_image object to int16 matrix.
         %   A = INT16(B) corresponds to A = DIP_ARRAY(IN,'int16')
         %   See also dip_image.dip_array
         im = dip_array(im,'int16');
      end
      function im = int8(im)
         %INT8   Convert dip_image object to int8 matrix.
         %   A = INT8(B) corresponds to A = DIP_ARRAY(IN,'int8')
         %   See also dip_image.dip_array
         im = dip_array(im,'int8');
      end
      function im = logical(im)
         %LOGICAL   Convert dip_image object to logical matrix.
         %   A = LOGICAL(B) corresponds to A = DIP_ARRAY(IN,'logical')
         %   See also dip_image.dip_array
         im = dip_array(im,'logical');
      end

      % ------- INDEXING -------

      function n = numArgumentsFromSubscript(~,~,~)
         %numArgumentsFromSubscript   Overload for internal use by MATLAB.
         n = 1; % Indexing into a dip_image object always returns a single object.
      end

      function a = subsref(a,s)
         %SUBSREF   Overload for indexing syntax A{I,J}(X,Y,Z).
         if strcmp(s(1).type,'.')
            name = s(1).subs;
            if strcmp(name,'Data') || ...
                  strcmp(name,'TrailingSingletons') || ...
                  strcmp(name,'TensorShapeInternal') || ...
                  strcmp(name,'TensorSizeInternal')
               error('Cannot access private properties')
            end
            if strcmp(name,'pixelsize')
               ndims = a.NDims;
               a = [a.PixelSize.magnitude];
               if isempty(a)
                  a = ones(1,ndims);
               elseif numel(a) < ndims
                  a = [a,repmat(a(end),1,ndims-numel(a))];
               elseif numel(a) > ndims
                  a = a(1:ndims);
               end
               if numel(s) > 1
                  a = subsref(a,s(2:end));
               end
            elseif strcmp(name,'pixelunits')
               ndims = a.NDims;
               a = {a.PixelSize.units};
               if isempty(a)
                  a = repmat({'px'},1,ndims);
               elseif numel(a) < ndims
                  a = [a,repmat(a(end),1,ndims-numel(a))];
               elseif numel(a) > ndims
                  a = a(1:ndims);
               end
               if numel(s) > 1
                  a = subsref(a,s(2:end));
               end
               if numel(a) == 1
                  a = a{1};
               end
            else
               a = builtin('subsref',a,s); % Call built-in method to access properties
            end
            return
         end
         if isempty(a)
            error('Cannot index into empty image')
         end
         telems = numtensorel(a);
         % Find the indices to use
         sz = imsize(a);
         [s,tsz,tsh,ndims] = construct_subs_struct(s,sz,a);
         if ndims == 1 && numel(sz) > 1
            a.Data = reshape(a.Data,size(a.Data,1),size(a.Data,2),[]);
         end
         % Do the indexing!
         a.Data = subsref(a.Data,s);
         a.NDims = ndims;
         a.TensorShapeInternal = tsh;
         a.TensorSizeInternal = tsz;
         if numtensorel(a) ~= telems
            a.ColorSpace = '';
         end
      end

      function a = subsasgn(a,s,b)
         %SUBSASGN   Overload for indexing syntax A{I,J}(X,Y,Z) = B.
         if strcmp(s(1).type,'.')
            name = s(1).subs;
            if strcmp(name,'Data') || ...
                  strcmp(name,'TrailingSingletons') || ...
                  strcmp(name,'TensorShapeInternal') || ...
                  strcmp(name,'TensorSizeInternal')
               error('Cannot access private properties')
            end
            if strcmp(name,'pixelsize')
               pxsz = a.PixelSize;
               if numel(s) > 1
                  q = [pxsz.magnitude];
                  q = subsasgn(q,s(2:end),b);
               else
                  q = b;
               end
               q = q(:);
               if numel(q) > numel(pxsz)
                  pxsz = ensurePixelSizeDimensionality(pxsz,numel(q));
               elseif numel(pxsz) > numel(q)
                  q(end+1:numel(pxsz)) = q(end);
               end
               q = num2cell(q);
               [pxsz.magnitude] = deal(q{:});
               a.PixelSize = pxsz;
            elseif strcmp(name,'pixelunits')
               pxsz = a.PixelSize;
               if ~iscell(b)
                  b = {b};
               end
               if numel(s) > 1
                  q = {pxsz.units};
                  q = subsasgn(q,s(2:end),b);
               else
                  q = b;
               end
               q = q(:);
               if numel(q) > numel(pxsz)
                  pxsz = ensurePixelSizeDimensionality(pxsz,numel(q));
               elseif numel(pxsz) > numel(q)
                  q(end+1:numel(pxsz)) = q(end);
               end
               [pxsz.units] = deal(q{:});
               a.PixelSize = pxsz;
            else
               a = builtin('subsasgn',a,s,b); % Call built-in method to access properties
            end
            return
         end
         if isempty(a)
            error('Cannot index into empty image')
         end
         % Find the indices to use
         sz = imsize(a);
         [s,~,~,nd] = construct_subs_struct(s,sz,a);
         orig_sz = size(a.Data);
         if nd == 1 && numel(sz) > 1
            a.Data = reshape(a.Data,size(a.Data,1),size(a.Data,2),[]);
         end
         % Assigning a dip_image into a?
         if isa(b,'dip_image')
            b = b.Data;
            if nd == 1
               b = reshape(b,size(b,1),size(b,2),[]);
            end
            a.Data = subsasgn_dip(a.Data,s,b);
         elseif isnumeric(b)
            reshaped = false;
            if numel(s.subs{2}) > 1
               I = find(size(b) == numel(s.subs{2}));
               if ~isempty(I)
                  I = I(1);
                  if I ~= 1
                     order = 1:ndims(b);
                     order(I) = [];
                     order = [I,order];
                     b = permute(b,order);
                  end
                  if nd == 1
                     b = reshape(b,1,size(b,1),[]);
                  else
                     b = reshape(b,[1,size(b)]);
                  end
                  if ndims(b)==2 && numel(s.subs)>2
                     % It's become a 0D tensor image: we'll need to replicate it to allow
                     % assignment of 0D image to multiple locations in 'a'
                     sz = cellfun('prodofsize',s.subs);
                     for ii=3:numel(sz)
                        if isequal(s.subs{ii},':')
                           sz(ii) = size(a,ii);
                        end
                     end
                     sz(1:2) = 1;
                     b = repmat(b,sz);
                  end
                  reshaped = true;
               end
            end
            if ~reshaped
               if nd == 1
                  b = reshape(b,1,1,[]);
               else
                  b = reshape(b,[1,1,size(b)]);
               end
            end
            a.Data = subsasgn_mat(a.Data,s,b);
         else
            error('Cannot assign non-numeric data into image')
         end
         if nd == 1 && numel(sz) > 1
            a.Data = reshape(a.Data,orig_sz);
         end
      end

      function ii = end(a,k,n)
         %END   Overload for using END in indexing.
         %
         %   Use END in spatial indexing of a DIP_IMAGE object. For example, in
         %   A(5:END,3:4), END is equal to IMSIZE(A,1)-1. A(0:END) is the same as
         %   A(:).
         %
         %   Do not use END in tensor indexing. A{END} will most likely give an
         %   out-of-bounds indexing error, but might also simply give the wrong
         %   anser. There is no way for this overloaded function to know the
         %   shape of the braces in which it is used.
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
         %SUBSINDEX   Overload for indexing syntax M(A), where A is a DIP_IMAGE.
         if ~isscalar(a) || ~islogical(a)
            error('Can only index using scalar, binary images')
         end
         a = find(a.Data)-1;
      end

      % ------- RESHAPING -------

      function im = tensortospatial(im,dim)
         %TENSORTOSPATIAL   Converts the tensor dimension to a spatial dimension.
         %
         %   B = TENSORTOSPATIAL(A) converts the tensor image A into a scalar image B
         %   without copying pixel data. The tensor dimension becomes the second
         %   spatial dimension (due to the order in which MATLAB stores pixel data).
         %
         %   B = TENSORTOSPATIAL(A,DIM) converts the tensor image A into a scalar
         %   image B copying the pixel data. The tensor dimension becomes the
         %   spatial dimension DIM.
         if nargin < 2 || isequal(dim,2)
            nd = im.NDims;
            sz = size(im.Data);
            sz = [sz(1),1,sz(2:end)];
            im.Data = reshape(im.Data,sz);
            im.NDims = nd + 1;
            if ~isempty(im.PixelSize)
               im = insertPixelSizeElement(im,2,defaultPixelSize);
            end
         else
            if ~isintscalar(dim) || dim < 1, error('Dimension argument must be a positive scalar integer'), end
            nd = im.NDims;
            dim = dim+2;
            n = 1:max(ndims(im.Data)+1,dim);
            if numel(n) > 3
               n = n([1:2,4,3,5:end]);
            end
            n = [n(1:dim-1),2,n(dim:end)];
            n(2) = n(end);
            n(end) = [];
            if numel(n) > 3
               n = n([1:2,4,3,5:end]);
            end
            im.Data = permute(im.Data,n);
            im.NDims = max(nd+1, dim-2);
            if ~isempty(im.PixelSize)
               im = insertPixelSizeElement(im,dim-2,defaultPixelSize);
            end
         end
         im.TensorShapeInternal = 'column vector';
         im.TensorSizeInternal = [1,1];
         im.ColorSpace = '';
      end

      function im = spatialtotensor(im,dim)
         %SPATIALTOTENSOR   Converts a spatial dimension to the tensor dimension.
         %
         %   B = SPATIALTOTENSOR(A) converts the scalar image A into a tensor image B
         %   without copying pixel data. The second spatial dimension becomes the tensor
         %   dimension (due to the order in which MATLAB stores pixel data).
         %
         %   B = SPATIALTOTENSOR(A,DIM) converts the scalar image A into a tensor
         %   image B copying the pixel data. The spatial dimension DIM becomes the
         %   tensor dimension.
         if ~isscalar(im), error('Cannot create a tensor dimension, image is not scalar'), end
         if nargin < 2 || isequal(dim,2)
            nd = im.NDims;
            sz = size(im.Data);
            sz = sz([1,3:end]);
            if numel(sz)<2
               sz = [sz,1];
            end
            im.Data = reshape(im.Data,sz);
            im.NDims = nd - 1;
            im = removePixelSizeElement(im,2);
         else
            nd = im.NDims;
            if ~isintscalar(dim) || dim < 1 || dim > nd, error('Dimension argument must be a positive scalar integer in the indexing range'), end
            dim = dim+2;
            n = 1:ndims(im.Data);
            if numel(n) > 3
               n = n([1:2,4,3,5:end]);
            end
            n = n([1,dim,3:dim-1,dim+1:end,2]);
            if numel(n) > 4 % one more than necessary, since the trailing singleton dimension we want to keep trailing
               n = n([1:2,4,3,5:end]);
            end
            im.Data = permute(im.Data,n);
            im.NDims = nd - 1;
            im = removePixelSizeElement(im,dim-2);
         end
         im.TensorShapeInternal = 'column vector';
         im.TensorSizeInternal = [size(im.Data,2),1];
         im.ColorSpace = '';
      end

      function im = expanddim(im,dims)
         %EXPANDDIM   Appends dimensions of size 1.
         %   B = EXPANDDIM(A,N) increases the dimensionality of the image A
         %   to N, if its dimensionality is smaller. The dimensions added will
         %   have a size of 1.
         %
         %   EXPANDDIM is a trivial operation which never copies pixel data.
         if ~isintscalar(dims), error('Number of dimensions must be scalar integer'), end
         if ndims(im) < dims
            if ndims(im) == 1
                sz = size(im.Data);
                sz(3:4) = [1,sz(3)];
                im.Data = reshape(im.Data,sz); % make the existing single dimension be the x dimension of the expanded image
            end
            im.NDims = dims;
         end
         % PixelSize is automatically expanded to cover all dimensions, so we don't need to do anything here
      end

      function im = permute(im,k)
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
         %
         %   PERMUTE always copies the pixel data.
         %
         %   See also dip_image.shiftdim, dip_image.reshape
         if ~isint(k)
            error('ORDER must be an integer vector')
         elseif any(k<0) || any(k>im.NDims)
            error('ORDER contains an index out of range')
         end
         k = k(:).'; % Make sure K is a 1xN vector.
         nd = numel(k);
         tmp = k;
         tmp(tmp==0) = [];
         if numel(unique(tmp)) ~= numel(tmp)
            error('ORDER contains a repeated index')
         end
         notused = setdiff(1:im.NDims,tmp);
         sz = imsize(im);
         if any(sz(notused)>1)
            error('ORDER misses some non-singleton dimensions')
         end
         k_orig = k; % Save to change pixel sizes later
         k = [k,notused];
         k(k==0) = max(k) + (1:sum(k==0));
         if numel(k) > 1
            % Where we say 1, we mean 2.
            I = k==1;
            J = k==2;
            k(I) = 2;
            k(J) = 1;
            % The X index must be the second one for MATLAB.
            k = k([2,1,3:end]);
         end
         im.Data = permute(im.Data,[1,2,k+2]);
         im.NDims = nd;
         pxsz = im.PixelSize;
         if ~isempty(pxsz)
            pxsz = ensurePixelSizeDimensionality(pxsz,numel(sz));
            pxsz = [defaultPixelSize;pxsz];
            im.PixelSize = pxsz(k_orig+1); % where k_orig==0, read the default pixel size value
         end
      end

      function im = swapdim(im,dim1,dim2)
         %SWAPDIM   Swap two image dimensions.
         %   B = SWAPDIM(A,DIM1,DIM2) swaps DIM1 and DIM2.
         %   See PERMUTE.
         if nargin~=3
            error('Requires three arguments.')
         end
         if ~isnumeric(dim1) || numel(dim1)~=1 || fix(dim1)~=dim1 || dim1<1 || dim1>im.NDims
            error('DIM1 must be a positive integer.')
         end
         if ~isnumeric(dim2) || numel(dim2)~=1 || fix(dim2)~=dim2 || dim2<1 || dim2>im.NDims
            error('DIM2 must be a positive integer.')
         end
         order = 1:im.NDims;
         order(dim1) = dim2;
         order(dim2) = dim1;
         im = permute(im,order);
      end

      function [im,nshifts] = shiftdim(im,n)
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
         %
         %   SHIFTDIM is implemented through a call to PERMUTE, and always
         %   copies the pixel data.
         %
         %   See also dip_image.permute, dip_image.reshape
         if nargin==1
            % Remove leading singleton dimensions
            sz = imsize(im);
            n = min(find(sz>1,'first')); % First non-singleton dimension.
            if isempty(n)
               n = numel(sz);
            end
            nshifts = n-1;
            if n>1
               im = permute(im,n:numel(sz));
            % else don't do anything
            end
         else
            if ~isintscalar(n)
                error('N should be a scalar integer')
            end
            if ~isequal(n,0) && ~isempty(im)
               if n>0
                  % Wrapped shift to the left
                  n = mod(n,im.NDims);
                  order = [n+1:im.NDims,1:n];
                  im = permute(im,order);
               else
                  % Shift to the right (padding with singletons).
                  order = [zeros(1,-n),1:im.NDims];
                  im = permute(im,order);
               end
            end
         end
      end

      function im = reshape(im,varargin)
         %RESHAPE   Change size of an image.
         %   B = RESHAPE(A,M,N,...) returns an image with the same
         %   pixels as A but reshaped to have the size M-by-N-by-...
         %   M*N*... must be the same as PROD(SIZE(A)) or NUMPIXELS(A).
         %
         %   B = RESHAPE(A,[M N ...]) is the same thing.
         %   In general, RESHAPE(A,SIZ) returns an image with the same
         %   elements as A but reshaped to the size SIZ. PROD(SIZ) must be
         %   the same as PROD(SIZE(A)).
         %
         %   RESHAPE(A,...,[],...) calculates the length of the dimension
         %   represented by [], such that the product of the dimensions
         %   equals PROD(SIZE(A)). The value of PROD(SIZE(A)) must be evenly
         %   divisible by the product of the specified dimensions. You can use
         %   only one occurrence of [].
         %
         %   Note that RESHAPE takes pixels column-wise from A. RESHAPE
         %   never copies the pixel data. Pixel sizes are reset unless
         %   they are isotropic.
         %
         %   See also dip_image.squeeze, dip_image.permute
         if nargin > 2
            emptydim = 0;
            for ii=1:nargin-1
               if isempty(varargin{ii})
                  if emptydim == 0
                     emptydim = ii;
                  else
                     error('Only one occurrence of [] can be used'),
                  end
               else
                  if ~isintscalar(varargin{ii}), error('Size arguments must be positive scalar integers'), end
               end
            end
            n = cat(2,varargin{:});
            if emptydim ~= 0
               p = numpixels(im) / prod(n);
               if fix(p)~=p, error('Number of pixels not evenly divisible by given dimensions'), end
               n = [n(1:emptydim-1),p,n(emptydim:end)];
            end
         else
            n = varargin{1}(:).';
            if ~isint(n) || any(n<1)
               error('Size vector must be a vector with positive integer elements')
            end
         end
         if numpixels(im) ~= prod(n), error('Number of pixels must not change'), end
         if numel(n)>1
            n = n([2,1,3:end]);
         end
         im.Data = reshape(im.Data,[size(im.Data,1),size(im.Data,2),n]);
         im.NDims = numel(n);
         pxsz = im.PixelSize;
         if numel(pxsz) > 1
            pxsz = defaultPixelSize;
         end
         im.PixelSize = pxsz; % We set it even if we didn't change the value, so that 0D images can erase the array, etc.
      end

      function im = squeeze(im,use_cheap)
         %SQUEEZE   Remove singleton dimensions.
         %   B = SQUEEZE(A) returns an image B with the same elements as
         %   A but with all the singleton dimensions removed. A dimension
         %   is singleton if size(A,dim)==1.
         %
         %   SQUEEZE, by default, never requires a data copy.
         %
         %   SQUEEZE is implemented through RESHAPE, and can cause some
         %   dimension flipping due to the way that data is stored in MATLAB:
         %   if size is [1 N M], the squeezed size is [M N]. See the DIPimage
         %   User Guide for more information
         %
         %   The 'CheapSqueeze' setting (see DIPSETPREF) can be turned off to
         %   change the behavior of SQUEEZE. In this mode, SQUEEZE is
         %   implemented through PERMUTE, and will not flip dimensions.
         %   However, it potentially copies the data.
         %
         %   SQUEEZE(A,T) overrides the 'CheapSqueeze' setting with T. If T
         %   evaluates to TRUE, the cheap version (RESHAPE) is always used.
         %   if it evaluates to FALSE, the expensive version (PERMUTE) is used.
         %
         %   See also dip_image.reshape, dip_image.permute
         if nargin < 2
            use_cheap = dipgetpref('CheapSqueeze');
         end
         sz = imsize(im);
         nd = numel(sz);
         ind = 1:nd;
         expected_ind = ind(sz>1);
         if nd>1
            sz = sz([2,1,3:end]);
            ind = ind([2,1,3:end]);
         end
         ind(sz==1) = [];
         if numel(ind)>1
            ind = ind([2,1,3:end]);
         end
         if isequal(ind,expected_ind)
            use_cheap = true; % we don't care what the user asks for, the cheap version does the right thing.
         elseif nargin < 2
            use_cheap = dipgetpref('CheapSqueeze');
         end
         if use_cheap
            % use the cheap squeeze: reshape
            sz = imsize(im);
            sz = sz(ind);
            pxsz = ensurePixelSizeDimensionality(im.PixelSize,nd);
            im = reshape(im,sz);
            im.PixelSize = pxsz(ind);
         else
            % use the expensive squeeze: permute
            im = permute(im,expected_ind);
         end
      end

      function im = flip(im,dim)
         %FLIP   Flips an image along specified dimension, same as FLIPDIM.
         if nargin~=2
            error('Requires two arguments.')
         end
         if ~isnumeric(dim) || numel(dim)~=1 || fix(dim)~=dim || dim<1
            error('DIM must be a positive integer.')
         end
         if im.NDims < dim
            error('Cannot flip along non-existent dimension.')
         end
         if dim == 2
            dim = 1;
         elseif dim == 1 && im.NDims > 1
            dim = 2;
         end
         im.Data = flip(im.Data,dim+2);
      end

      function im = flipdim(im,dim)
         %FLIPDIM   Flips an image along specified dimension, same as FLIP.
         im = flip(im,dim);
      end

      function im = fliplr(im)
         %FLIPLR   Flips an image left/right.
         if im.NDims < 1
            error('Cannot flip left/right with less than one dimension.')
         end
         im.Data = flip(im.Data,4);
      end

      function im = flipud(im)
         %FLIPUD   Flips an image up/down.
         if im.NDims < 2
            error('Cannot flip up/down with less than two dimensions.')
         end
         im.Data = flip(im.Data,3);
      end

      function im = rot90(im,k)
         %ROT90  Rotate image 90 degrees.
         %   ROT90(A) is the 90 degree clockwise rotation of image A.
         %   ROT90(A,K) is the K*90 degree rotation of A, K = +-1,+-2,...
         %
         %   Note that the direction of rotation is reversed w.r.t. the
         %   built-in function ROT90 for matrices. This is due to the
         %   different dimension order of images and matrices. The rotation
         %   here is consistent with the DIPimage function ROTATION.
         if nargin == 1
             k = 1;
         else
            if ~isnumeric(k) || numel(k)~=1 || fix(k)~=k
               error('k must be a scalar.');
            end
            k = mod(k,4);
         end
         if k == 1
            im = swapdim(im,1,2);
            im = flipdim(im,1);
         elseif k == 2
            im = flipdim(flipdim(im,1),2);
         elseif k == 3
            im = flipdim(im,1);
            im = swapdim(im,1,2);
         % else k==0, we do nothing.
         end
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
         in = varargin;
         in(cellfun(@isempty,in)) = [];
         if isempty(in)
            out = dip_image([]);
            return
         end
         out = in{1};
         % Collect all images in cell array
         n = numel(in);
         if n == 1
            return
         end
         oneD = true;
         nExtraDims = 0;
         pxsz = cell(1,n);
         for ii = 1:n
            img = in{ii};
            if ~isa(img,'dip_image')
               img = dip_image(img);
            end
            in{ii} = img.Data;
            if img.NDims == 1
               sz = size(in{ii});
               sz(3:4) = [1,sz(3)];
               in{ii} = reshape(in{ii},sz);
            else
               oneD = false;
            end
            nExtraDims = max(nExtraDims,img.TrailingSingletons);
            pxsz{ii} = img.PixelSize;
         end
         % Find the correct output datatype
         compl = size(in{1},1) == 2;
         out_type = di_findtypex(class(in{1}),class(in{1}),compl);
         for ii = 2:n
            compl = compl | (size(in{ii},1) == 2);
            out_type = di_findtypex(out_type,class(in{ii}),compl);
         end
         pxsz(cellfun('isempty',pxsz)) = [];
         if ~isempty(pxsz)
            pxsz = pxsz{1};
         end
         % Convert images to the output type
         for ii = 1:n
            if ~strcmp(class(in{ii}),out_type)
               in{ii} = array_convert_datatype(in{ii},out_type);
            end
            if compl && size(in{ii},1) == 1
               in{ii}(2,1) = 0; % make it complex by adding zeros
            end
         end
         % Call CAT
         if dim == 1
            dim = 2;
         else
            oneD = false;
            if dim == 2
               dim = 1;
            end
         end
         out.Data = cat(dim+2,in{:});
         out.TrailingSingletons = nExtraDims;
         if oneD
            sz = size(out.Data);
            sz(3) = [];
            out.Data = reshape(out.Data,sz);
         end
         if ~isempty(pxsz)
            out.PixelSize = pxsz;
         end
      end

      function a = horzcat(varargin)
         %HORZCAT   Equal to CAT(1,...).
         a = cat(1,varargin{:});
      end

      function a = vertcat(varargin)
         %VERTCAT   Equal to CAT(2,...).
         a = cat(2,varargin{:});
      end

      function im = repmat(im,varargin)
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
            n = varargin{1}(:).';
            if ~isint(n) || any(n<1)
               error('Size vector must be a vector with positive integer elements')
            end
         end
         if isempty(n)
            error('Size argument is an empty array')
         end
         nd = max(numel(n),ndims(im));
         if numel(n)==1
            n = [1,n];
         else
            n = n([2,1,3:end]);
         end
         sz = size(im.Data);
         if ndims(im)==1
            % Special case for a 1D image: it's stored along the Y-axis,
            % the code below won't work correctly
            k = sz(3);
            im.Data = reshape(im.Data,[sz(1:2),1,k]);
            im.Data = repmat(im.Data,[1,1,n]);
         else
            im.Data = repmat(im.Data,[1,1,n]);
            k = 1;
         end
         if nd==1
            im.Data = reshape(im.Data,[sz(1:2),n(2)*k]);
         end
         im.NDims = nd;
      end

      function out = split(in,stepsize)
         %SPLIT   Split an image into subsampled versions
         %   B = SPLIT(A,N) reduces size of image A by an integer factor N, by
         %   subsampling. The subsampling factor must evenly divide the image.
         %   The output image has an additional dimension, where all possible
         %   subsampling shifts are present. See SPLITIM for more information.
         out = splitim(in,stepsize);
      end

      % ------- OPERATORS -------

      function out = plus(lhs,rhs)
         %PLUS   Overload for operator +
         out = dip_operators('+',lhs,rhs,dipgetpref('KeepDataType'));
      end

      function out = minus(lhs,rhs)
         %MINUS   Overload for operator -
         out = dip_operators('-',lhs,rhs,dipgetpref('KeepDataType'));
      end

      function out = mtimes(lhs,rhs)
         %MTIMES   Overload for operator *
         %   Computes the matrix multiplication of the tensors at each corresponding pixel.
         out = dip_operators('*',lhs,rhs,dipgetpref('KeepDataType'));
      end

      function out = times(lhs,rhs)
         %TIMES   Overload for operator .*
         out = dip_operators('.',lhs,rhs,dipgetpref('KeepDataType'));
      end

      function out = mrdivide(lhs,rhs)
         %MRDIVIDE   Overload for operator /
         %   Only implemented for scalar RHS.
         if ~isscalar(rhs)
            error('Not implented');
         end
         out = dip_operators('/',lhs,rhs,dipgetpref('KeepDataType'));
      end

      function out = rdivide(lhs,rhs)
         %RDIVIDE   Overload for operator ./
         out = dip_operators('/',lhs,rhs,dipgetpref('KeepDataType'));
      end

      function out = mod(lhs,rhs)
         %MOD   Modulus after division.
         out = dip_operators('%',lhs,rhs);
      end

      function out = power(lhs,rhs)
         %POWER   Overload for operator .^
         if isequal(rhs,2)
            out = dip_operators('.',lhs,lhs,dipgetpref('KeepDataType')); % a.*a is about four times faster than a.^2
         else
            out = dip_operators('^',lhs,rhs);
         end
      end

      function out = mpower(lhs,rhs)
         %MPOWER   Overload for operator ^
         %   For powers other than 2, not implemented for non-scalar images.
         if isequal(rhs,2)
            out = dip_operators('*',lhs,lhs,dipgetpref('KeepDataType')); % a^2 is implemented as a*a
         else
            if ~isscalar(lhs) || ~isscalar(rhs)
               error('Not implented');
            end
            out = dip_operators('^',lhs,rhs);
         end
      end

      function out = eq(lhs,rhs)
         %EQ   Overload for operator ==
         out = dip_operators('=',lhs,rhs);
      end

      function out = gt(lhs,rhs)
         %GT   Overload for operator >
         out = dip_operators('>',lhs,rhs);
      end

      function out = lt(lhs,rhs)
         %LT   Overload for operator <
         out = dip_operators('<',lhs,rhs);
      end

      function out = ge(lhs,rhs)
         %GE   Overload for operator >=
         out = dip_operators('g',lhs,rhs);
      end

      function out = le(lhs,rhs)
         %LE   Overload for operator <=
         out = dip_operators('l',lhs,rhs);
      end

      function out = ne(lhs,rhs)
         %NE   Overload for operator ~=
         out = dip_operators('n',lhs,rhs);
      end

      function out = and(lhs,rhs)
         %AND   Overload for operator &
         if ~islogical(lhs)
            lhs = dip_image(lhs,'bin');
         end
         if ~islogical(lhs)
            rhs = dip_image(rhs,'bin');
         end
         out = dip_operators('&',lhs,rhs);
      end

      function out = or(lhs,rhs)
         %OR   Overload for operator |
         if ~islogical(lhs)
            lhs = dip_image(lhs,'bin');
         end
         if ~islogical(lhs)
            rhs = dip_image(rhs,'bin');
         end
         out = dip_operators('|',lhs,rhs);
      end

      function out = xor(lhs,rhs)
         %XOR   Overload for operator XOR
         if ~islogical(lhs)
            lhs = dip_image(lhs,'bin');
         end
         if ~islogical(lhs)
            rhs = dip_image(rhs,'bin');
         end
         out = dip_operators('x',lhs,rhs);
      end

      function im = not(im)
         %NOT   Overload for unary operator ~
         if ~islogical(im)
            im = dip_image(im,'bin');
         end
         im = dip_operators('m~',im);
      end

      function out = bitand(lhs,rhs)
         %BITAND   Bitwise AND for integer-valued images.
         if isa(lhs,'dip_image')
            dt = class(lhs.Data);
            if ~isintclass(dt)
               error('BITAND only defined for integer-valued images')
            end
            if isa(rhs,'dip_image')
               dt = class(rhs.Data);
               if ~isintclass(dt)
                  error('BITAND only defined for integer-valued images')
               end
            else
               rhs = dip_image(rhs,dt);
            end
         else % the 2nd image must be a dip_image!
            dt = class(rhs.Data);
            if ~isintclass(dt)
               error('BITAND only defined for integer-valued images')
            end
            lhs = dip_image(lhs,dt);
         end
         out = dip_operators('&',lhs,rhs);
      end

      function out = bitor(lhs,rhs)
         %BITOR   Bitwise OR for integer-valued images.
         if isa(lhs,'dip_image')
            dt = class(lhs.Data);
            if ~isintclass(dt)
               error('BITOR only defined for integer-valued images')
            end
            if isa(rhs,'dip_image')
               dt = class(rhs.Data);
               if ~isintclass(dt)
                  error('BITOR only defined for integer-valued images')
               end
            else
               rhs = dip_image(rhs,dt);
            end
         else % the 2nd image must be a dip_image!
            dt = class(rhs.Data);
            if ~isintclass(dt)
               error('BITOR only defined for integer-valued images')
            end
            lhs = dip_image(lhs,dt);
         end
         out = dip_operators('|',lhs,rhs);
      end

      function out = bitxor(lhs,rhs)
         %BITXOR   Bitwise XOR for integer-valued images.
         if isa(lhs,'dip_image')
            dt = class(lhs.Data);
            if ~isintclass(dt)
               error('BITXOR only defined for integer-valued images')
            end
            if isa(rhs,'dip_image')
               dt = class(rhs.Data);
               if ~isintclass(dt)
                  error('BITXOR only defined for integer-valued images')
               end
            else
               rhs = dip_image(rhs,dt);
            end
         else % the 2nd image must be a dip_image!
            dt = class(rhs.Data);
            if ~isintclass(dt)
               error('BITXOR only defined for integer-valued images')
            end
            lhs = dip_image(lhs,dt);
         end
         out = dip_operators('x',lhs,rhs);
      end

      function im = bitcmp(im)
         %BITCMP   Bitwise complement for integer-valued images.
         dt = class(im.Data);
         if ~isintclass(dt)
            error('BITCMP only defined for integer-valued images')
         end
         im = dip_operators('m~',im);
      end

      function im = uminus(im)
         %UMINUS   Overload for unary operator -
         %   For unsigned integer types, -A is the same as INTMAX(DATATYPE(A))-A.
         im = dip_operators('m-',im);
      end

      function im = uplus(im)
         %UPLUS   Overload for unary operator +
         %   Converts a binary image to UINT8. Has no effect for other data types.
         if islogical(im.Data)
            im.Data = uint8(im.Data);
         end
      end

      function im = ctranspose(im)
         %CTRANSPOSE   Overload for unary operator '
         %   Returns the complex conjugate transpose of the tensor at each pixel.
         im = conj(transpose(im));
      end

      function im = transpose(im)
         %TRANSPOSE   Overload for unary operator .'
         %   Returns the non-conjugate transpose of the tensor at each pixel.
         %   No data is copied, the transposition is accomplished by changing the
         %   TensorShape property of the image.
         switch im.TensorShapeInternal
            case 'column vector'
                im.TensorShapeInternal = 'row vector';
            case 'row vector'
                im.TensorShapeInternal = 'column vector';
            case 'column-major matrix'
                im.TensorShapeInternal = 'row-major matrix';
            case 'row-major matrix'
                im.TensorShapeInternal = 'column-major matrix';
            case 'diagonal matrix'
                %im.TensorShapeInternal = 'diagonal matrix';
            case 'symmetric matrix'
                %im.TensorShapeInternal = 'symmetric matrix';
            case 'upper triangular matrix'
                im.TensorShapeInternal = 'lower triangular matrix';
            case 'lower triangular matrix'
                im.TensorShapeInternal = 'upper triangular matrix';
         end
         im.TensorSizeInternal = im.TensorSizeInternal([2,1]);
      end

      function im = conj(im)
         %CONJ   Complex conjugate.
         if iscomplex(im)
            im.Data(2,:) = -im.Data(2,:);
         end
      end

      function im = real(im)
         %REAL   Complex real part.
         if iscomplex(im)
            sz = size(im.Data);
            sz(1) = 1;
            im.Data = reshape(im.Data(1,:),sz);
         end
      end

      function im = imag(im)
         %IMAG   Complex imaginary part.
         if iscomplex(im)
            sz = size(im.Data);
            sz(1) = 1;
            im.Data = reshape(im.Data(2,:),sz);
         end
      end

      function im = abs(im)
         %ABS   Absolute value.
         %   See also DIP_IMAGE/HYPOT and DIP_IMAGE/NORM.
         im = dip_operators('ma',im);
      end

      function im = angle(im)
         %ANGLE   Phase angle of complex values, or angle of vector.
         %   ANGLE(C), with C a complex-valued image, returns the phase angle of the
         %   values in C.
         %
         %   ANGLE(V), with V a real-valued 2-vector image, returns the angle of the
         %   vector to the x-axis. If V is a real-valued 3-vector image, returns an
         %   image with two tensor components, representing the angles PHI and THETA of
         %   the vector -- PHI being the angle to the x-axis within the x-y plane
         %   (azimuth), and THETA being the angle to the z-axis (inclination).
         %
         %   See also DIP_IMAGE/ATAN2.
         im = dip_operators('mc',im);
      end

      function im = phase(im)
         %PHASE   Phase angle of complex values, alias to DIP_IMAGE/ANGLE.
         im = dip_operators('mc',im);
      end

      function im = ftz(im)
         %FTZ   Flushes denormal sample values to zero.
         if isfloat(im)
            im = dip_operators('mq',im);
         end
      end

      function im = round(im)
         %ROUND   Round to nearest integer.
         if isfloat(im)
            im = dip_operators('md',im);
         end
      end

      function im = ceil(im)
         %CEIL   Round up.
         if isfloat(im)
            im = dip_operators('me',im);
         end
      end

      function im = floor(im)
         %FLOOR   Round down.
         if isfloat(im)
            im = dip_operators('mf',im);
         end
      end

      function im = fix(im)
         %FIX   Round towards zero.
         if isfloat(im)
            im = dip_operators('mg',im);
         end
      end

      function im = sign(im)
         %SIGN   Signum function.
         if ~isreal(im)
            error('SIGN defined only for real-valued images')
         end
         if issigned(im) || isfloat(im)
            im = dip_operators('mh',im);
         else
            im.Data(:) = 1;
         end
      end

      function im = isnan(im)
         %ISNAN   True for samples that are NaN.
         im = dip_operators('mi',im);
      end

      function im = isinf(im)
         %ISINF   True for samples that are +/- Inf.
         im = dip_operators('mj',im);
      end

      function im = isfinite(im)
         %ISFINITE   True for samples that are not NaN nor Inf.
         im = dip_operators('mk',im);
      end

      function im = det(im)
         %DET   Determinant of a tensor image.
         %   DET(A) returns the determinant of the square tensors in the
         %   tensor image A.
         im = dip_operators('ml',im);
      end

      function im = inv(im)
         %INV   Inverse of a square tensor image.
         %   INV(A) returns the inverse of A in the sense that INV(A)*A is equal
         %   to EYE(A). A must be a square matrix image.
         %
         %   It is possible that the inverse does not exist for a pixel. In that
         %   case output values can be Inf or NaN. Use PINV for a more robust
         %   inverse, which also works for non-square matrices.
         %
         %   See also: DIP_IMAGE/PINV
         im = dip_operators('mm',im);
      end

      function im = norm(im)
         %NORM   Computes the Eucledian norm of a vector image.
         %   NORM(V) returns the norm of the vectors in V.
         im = dip_operators('mn',im);
      end

      function im = trace(im)
         %TRACE   Sum of the diagonal elements.
         %   TRACE(A) is the sum of the diagonal elements of the tensor image.
         im = dip_operators('mo',im);
      end

      function im = pinv(im,tol)
         %PINV   Pseudoinverse or a tensor image.
         %   X = PINV(A,TOL)
         %
         %   X is of dimension A' and fullfills A*X*A = A, X*A*X = X. The computation
         %   is based on SVD(A) and any singular values less than a TOL are treated
         %   as zero. By default, TOL = 1e-6.
         %
         %   See also: DIP_IMAGE/INV, DIP_IMAGE/SVD
         if nargin < 2
            tol = 1e-6;
         end
         im = dip_operators('mp',im,tol);
      end

      function im = cos(im)
         %COS   Cosine.
         im = dip_operators('mA',im);
      end

      function im = sin(im)
         %SIN   Sine.
         im = dip_operators('mB',im);
      end

      function im = tan(im)
         %TAN   Tangent.
         im = dip_operators('mC',im);
      end

      function im = acos(im)
         %ACOS   Inverse cosine.
         im = dip_operators('mD',im);
      end

      function im = asin(im)
         %ASIN   Inverse sine.
         im = dip_operators('mE',im);
      end

      function im = atan(im)
         %ATAN   Inverse tangent.
         im = dip_operators('mF',im);
      end

      function im = cosh(im)
         %COSH   Hyperbolic cosine.
         im = dip_operators('mG',im);
      end

      function im = sinh(im)
         %SINH   Hyperbolic sine.
         im = dip_operators('mH',im);
      end

      function im = tanh(im)
         %TANH   Hyperbolic tangent.
         im = dip_operators('mI',im);
      end

      function im = sqrt(im)
         %SQRT   Square root.
         im = dip_operators('m1',im);
      end

      function im = exp(im)
         %EXP   Base e power.
         im = dip_operators('m2',im);
      end

      function im = pow10(im)
         %POW10   Base 10 power.
         im = dip_operators('m3',im);
      end

      function im = pow2(im)
         %POW2   Base 2 power.
         im = dip_operators('m4',im);
      end

      function im = log(im)
         %LOG   Natural (base e) logarithm.
         im = dip_operators('m5',im);
      end

      function im = log10(im)
         %LOG10   Base 10 logarithm.
         im = dip_operators('m6',im);
      end

      function im = log2(im)
         %LOG2   Base 2 logarithm.
         im = dip_operators('m7',im);
      end

      function im = erf(im)
         %ERF   Error function.
         im = dip_operators('m!',im);
      end

      function im = erfc(im)
         %ERFC   Complementary error function.
         im = dip_operators('m@',im);
      end

      function im = gammaln(im)
         %GAMMALN   Logarithm of gamma function.
         im = dip_operators('m#',im);
      end

      function res = atan2(y,x)
         %ATAN2   Four quadrant inverse tangent of y/x.
         res = dip_operators('A',y,x);
      end

      function res = hypot(a,b)
         %HYPOT   Robust computation of the square root of the sum of squares.
         res = dip_operators('H',a,b);
      end

      function res = cross(a,b)
         %CROSS   Cross product of two vector images.
         %   CROSS(A,B) returns the cross product of the vector images A and B.
         %   The cross product results in a vector image, and is only defined
         %   for vectors with 2 or 3 components.
         %
         %   For 2-vectors, we define the cross product as the z component of
         %   the cross product of 3-vectors with 0 z-component. That is, we add
         %   a 0 as the 3rd vector component, and compute the cross product
         %   ignoring the two first components of the result, which are zero.
         %
         %   Either A or B can be a normal vector such as [1,0,0].
         %
         %   See also: DIP_IMAGE/DOT, DIP_IMAGE/MTIMES
         res = dip_operators('C',a,b);
      end

      function res = dot(a,b)
         %DOT   Dot product of two vector images.
         %   DOT(A,B) returns the dot product of the vector images A and B.
         %   If both A and B are column vectors, this is the same as A'*B.
         %   The dot product results in a scalar image.
         %
         %   Either A or B can be a normal vector such as [1,0,0].
         %
         %   See also: DIP_IMAGE/CROSS, DIP_IMAGE/MTIMES
         res = dip_operators('D',a,b);
      end

   end

   % ------- STATIC METHODS -------

   methods(Static)

      function n = numberchannels(col)
         %NUMBERCHANNELS   Number of channels for a color space.
         %   N = DIP_IMAGE.NUMBERCHANNELS(COL) returns the number of channels
         %   expected for color space COL.
         n = colorspacemanager(col);
      end

      function empty(varargin)
         error('Cannot create empty dip_image object')
      end
      % The function above defined to prevent its use.

      function img = loadobj(img)
         %LOADOBJ   This function is called when loading dip_image objects from file.
         if isa(img,'dip_image')
            return
         end
         % It's a struct. Likely because the MAT-file was saved with a DIPimage 2 object
         in = img;
         img = cell(1,numel(in));
         for ii = 1:numel(in)
            img{ii} = expanddim(dip_image(in(ii).data),in(ii).dims);
            img{ii}.PixelSize = struct('magnitude',num2cell(in(ii).physDims.PixelSize),'units',in(ii).physDims.PixelUnits);
         end
         if numel(img) == 1
            % Scalar image
            img = img{1};
            return
         end
         try
            % Try to convert the cell array to a tensor image
            img = dip_image(img);
         catch
            % We can't. Let's leave it as a cell array
            return
         end
         % It was a tensor image. Let's try to recover the color space
         if isfield(in,'color') && isfield(in(1).color,'space')
            img.ColorSpace = in(1).color.space;
         end
         % And let's try to recover the tensor shape
         if numel(size(in)) <= 2
            img.TensorShape = size(in);
         end
      end

   end

end

% ------- PRIVATE FUNCTIONS -------

function res = isstring(in)
   res = ischar(in) && isrow(in);
end

function res = isint(in)
   res = isnumeric(in) && all(fix(in)==in);
end

function res = isintscalar(in)
   res = isint(in) && numel(in)==1;
end

function in = array_convert_datatype(in,class)
   %#function int8, uint8, int16, uint16, int32, uint32, int64, uint64, single, double, logical
   in = feval(class,in);
end

function res = isintclass(dt)
   res = any(strcmp(dt,{'uint8','uint16','uint32','uint64','int8','int16','int32','int64'}));
end

% Gives a DIPlib data type string for the data in the image.
function str = datatypestring(in)
   str = class(in.Data);
   switch str
      case 'logical'
         str = 'binary';
      case {'uint8','uint16','uint32','uint64'}
         % nothing to do, it's OK.
      case 'int8'
         str = 'sint8';
      case 'int16'
         str = 'sint16';
      case 'int32'
         str = 'sint32';
      case 'int64'
         str = 'sint64';
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
      case {'logical','binary','bin','bin8'}
         str = 'logical';
      case {'uint32','uint'}
         str = 'uint32';
      case {'uint8','uint16','uint64'}
         % nothing to do, it's OK.
      case {'int8','sint8'}
         str = 'int8';
      case {'int16','sint16'}
         str = 'int16';
      case {'int32','sint32','int'}
         str = 'int32';
      case {'int64','sint64'}
         str = 'int64';
      case {'single','sfloat','float'}
         str = 'single';
      case {'double','dfloat'}
         str = 'double';
      case {'scomplex','complex'}
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
            dt1_size = str2double(dt1(4:end));
         else
            assert(all(dt1(1:4)=='uint'))
            dt1_signed = false;
            dt1_size = str2double(dt1(5:end));
         end
         if dt2(1:3)=='int'
            dt2_signed = true;
            dt2_size = str2double(dt2(4:end));
         else
            assert(all(dt2(1:4)=='uint'))
            dt2_signed = false;
            dt2_size = str2double(dt2(5:end));
         end
         if dt1_signed && dt2_signed
            % Both signed
            dt_out = ['int',num2str(max(dt1_size,dt2_size))];
         elseif ~dt1_signed && ~dt2_signed
            % Both unsigned
            dt_out = ['uint',num2str(max(dt1_size,dt2_size))];
         else
            if dt1_signed
               [dt1_size,dt2_size] = deal(dt2_size,dt1_size); % swap dt1 and dt2
            end
            % dt1 unsigned, dt2 signed
            if dt2_size > dt1_size
               sz = dt2_size;
            else
               switch dt1_size
                  case 8
                     sz = 16;
                  case 16
                     sz = 32;
                  case {32,64}
                     sz = 64;
                  otherwise
                     error('Unknown integer type found')
               end
            end
            dt_out = ['int',num2str(sz)];
         end
      end
   end
   if comp
      if strcmp(dt_out,'double') || strcmp(dt_out,'int64') || strcmp(dt_out,'sint64') || strcmp(dt_out,'int32') || strcmp(dt_out,'sint32')
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
   elseif isint(str) && numel(str) >= 1 && numel(str) <= 2
      res = true;
   else
      res = false;
   end
end

% Validates and converts numbers to a PixelSize struct
function pxsz = validatePixelSize(pxsz)
   if isnumeric(pxsz)
      pxsz = struct('magnitude',num2cell(pxsz(:)),'units',repmat({'m'},numel(pxsz),1));
   elseif ~isstruct(pxsz) || ~isfield(pxsz,'magnitude') || ~isfield(pxsz,'units') || ...
          ~all(arrayfun(@(pxsz)isnumeric(pxsz.magnitude) && isscalar(pxsz.magnitude) && isstring(pxsz.units), pxsz))
          % Record for longest expression???
      error('Illegal value for pixel size')
   end
   pxsz = reshape(pxsz,[],1); % We try to keep this being a column vector
end

% Makes sure that the PixelSize array has at least the given number of elements by replicating the last element
function pxsz = ensurePixelSizeDimensionality(pxsz,dim)
   if isempty(pxsz)
      pxsz(1:dim,1) = defaultPixelSize;
   elseif numel(pxsz) < dim
      pxsz(end+1:dim,1) = pxsz(end);
   end
end

% Inserts an element at location `dim` in the `img.PixelSize` array
function img = insertPixelSizeElement(img,dim,newelem)
   pxsz = img.PixelSize;
   pxsz = ensurePixelSizeDimensionality(pxsz,dim);
   pxsz = [pxsz(1:dim-1);validatePixelSize(newelem);pxsz(dim:end)];
   img.PixelSize = pxsz;
end

% Removes the element at location `dim` in the `img.PixelSize` array
function img = removePixelSizeElement(img,dim)
   pxsz = img.PixelSize;
   if numel(pxsz) > dim % Not `>=`: we don't want to remove the last element in the array, as that is implicitly also the value for subsequent elements
      pxsz(dim) = [];
      img.PixelSize = pxsz;
   end
end

% The default value for pixel sizes: 1 px
function pxsz = defaultPixelSize
   pxsz = struct('magnitude',1,'units','px');
end

% Figures out how to index into an image, used by subsref and subsasgn
function [s,tsz,tsh,ndims] = construct_subs_struct(s,sz,a)
   tensorindex = 0;
   imageindex = 0;
   for ii=1:numel(s)
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
         tsz = [numel(telems),1];
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
         tsz = [numel(ii),numel(jj)];
         lut = dip_tensor_indices(a);
         [ii,jj] = ndgrid(ii,jj);
         telems = lut(ii + (jj-1)*stride) + 1;
         telems = telems(:).'; % make into row vector
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
   ndims = numel(sz);
   if imageindex
      s = s(imageindex);
      if numel(s.subs) == 1 % (this should produce a 1D image)
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
               ind = find(ind.Data(:));
            end
         elseif isnumeric(ind)
            ind = ind+1;
            if any(ind > prod(sz)) || any(ind < 1)
               error('Index exceeds image dimensions')
            end
         elseif islogical(ind)
            msz = size(ind);
            msz = msz([2,1,3:end]);
            % We compare the "squeezed" sizes
            msz(msz==1) = [];
            sz(sz==1) = [];
            if ~isequal(msz,sz)
               error('Mask image must match image size when indexing')
            end
            ind = find(ind(:));
         % else it's ':'
         end
         s.subs{1} = ind;
      elseif numel(s.subs) == numel(sz)
         for ii=1:numel(sz)
            ind = s.subs{ii};
            if islogical(ind)
               error('Illegal indexing')
            elseif isa(ind,'dip_image')
               if all(imsize(ind)==1) % Added for Piet (BR)
                  ind=double(ind);
               else
                  error('Illegal indexing');
               end
            end
            if isnumeric(ind)
               ind = ind(:)+1; % Linearized the indices, so the next line doesn't error. Output doesn't have the right shape anyway.
                               % TODO: Make it so that the output has the same shape as the matrix `ind`.
               if any(ind > sz(ii)) || any(ind < 1)
                  error('Index exceeds image dimensions')
               end
            end
            s.subs{ii} = ind;
         end
         s.subs = s.subs([2,1,3:end]);
      else
         error('Number of indices not the same as image dimensionality')
      end
   else
      s = substruct('()',repmat({':'},1,numel(sz)));
   end
   % Combine spatial and tensor indexing, add indexing into complex dimension
   s.subs = [{':'},{telems},s.subs];
end

% Assigns b into dip_image data segement a. b is the data matrix extracted from a dip_image
function a = subsasgn_dip(a,s,b)
   %fprintf('subsasgn_dip: size(b,1) = %d, size(a,1) = %d\n', size(b,1), size(a,1))
   if size(b,1) == 1 % Assigning real values into a
      % Assign b into real part of a
      s.subs{1} = 1;
      a = subsasgn_core(a,s,b);
      if size(a,1) > 1
         % Clear imaginary part of a
         s.subs{1} = 2;
         a = subsasgn_core(a,s,0);
      end
   else % Assigning complex values into a
      if size(a,1) == 1
         error('Cannot assign complex data into real-valued image')
      end
      a = subsasgn_core(a,s,b);
   end
end

% Assigns b into dip_image data segment a. b is a standard MATLAB matrix
function a = subsasgn_mat(a,s,b)
   %fprintf('subsasgn_mat: size(b,1) = %d, size(a,1) = %d\n', size(b,1), size(a,1))
   if isreal(b) % Assigning real values into a
      % Assign b into real part of a
      s.subs{1} = 1;
      a = subsasgn_core(a,s,b);
      if size(a,1) > 1
         % Clear imaginary part of a
         s.subs{1} = 2;
         a = subsasgn_core(a,s,0);
      end
   else % Assigning complex values into a
      if size(a,1) == 1
         error('Cannot assign complex data into real-valued image')
      end
      s.subs{1} = 1;
      a = subsasgn_core(a,s,real(b));
      s.subs{1} = 2;
      a = subsasgn_core(a,s,imag(b));
   end
end

% Assigns b into dip_image data segment a. b is a standard MATLAB matrix
function a = subsasgn_core(a,s,b)
   %fprintf('subsasgn_core: size(b,2) = %d, size(a,2) = %d\n', size(b,2), size(a,2))
   % Here we assign matrix B into matrix A using S.
   % SUBSASGN doesn't do singleton expansion, we want to do that, at least
   % in the case of assigning a 0D tensor image to multiple pixels of A.
   % so we call SUBSASGN on each of the tensor elements of B.
   telemsA = s.subs{2};
   ntelemsB = size(b,2);
   if ntelemsB == numel(telemsA)
      % Insert b(:,ii,:,:,:,...) into tensor element telemsA(ii)
      sb = substruct('()',repmat({':'},1,ndims(b)));
      for ii = 1:numel(telemsA)
         s.subs{2} = telemsA(ii);
         sb.subs{2} = ii;
         b2 = subsref(b,sb);
         %fprintf('Assigning array %s into array %s using:\n', mat2str(size(b2)), mat2str(size(a)))
         %disp(s)
         a = subsasgn(a,s,b2);
      end
   elseif ntelemsB == 1
      % Insert b into each tensor element
      for ii = telemsA
         s.subs{2} = ii;
         %fprintf('Assigning array %s into array %s using:\n', mat2str(size(b)), mat2str(size(a)))
         %disp(s)
         a = subsasgn(a,s,b);
      end
   else
      error('Subscripted assignment tensor sizes mismatch')
   end
end
