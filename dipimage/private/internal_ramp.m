%INTERNAL_RAMP   Creates an image with one cartesian coordinate
%   This implements parameter parsing for functions xx, yy, etc.,
%   then calls COORDINATES.

% (c)2017-2020, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function out = internal_ramp(value,mode,varargin)
% VALUE is either 1, 2, 3, 'radius', 'phi', 'theta' or 'ramp', depending on which function called this
% MODE is either 'full' or 'skinny' (XX vs XX1)
% VARARGIN is all parameters to the called function, one of the two formats (all params optional):
%   - { img, 'origin', {options} }
%   - { [xsz,ysz,...], 'origin', {options} }
%   - { xsz,ysz,..., 'origin', {options} }
% If VALUE is 'ramp', then VARARGIN is (again all params optional, but there's no DIM if there's no SZ):
%   - { img, dim, 'origin', {options} }
%   - { [xsz,ysz,...], dim, 'origin', {options} }
%   - { xsz,ysz,..., dim, 'origin', {options} }

% Default values
sz = [256,256]; % default sizes
origin = 'right';
options = {};

% Get OPTIONS parameter
if ~isempty(varargin) && iscell(varargin{end})
   options = varargin{end};
   varargin(end) = [];
end

% Get ORIGIN parameter
if ~isempty(varargin) && ischar(varargin{end})
   origin = varargin{end};
   varargin(end) = [];
end
if ~isempty(varargin) && ischar(varargin{end})
   % whoa, we've got two string arguments at the end, likely one was meant as OPTIONS:
   if ~isempty(options)
      error('Too many string arguments')
   end
   options = {origin};
   origin = varargin{end};
   varargin(end) = [];
end

% Get DIM parameter (in VALUE)
if strcmp(value,'ramp')
   if numel(varargin)>1 % this parameter is only present if there's a size parameter too
      value = varargin{end};
      varargin(end) = [];
   else
      value = 1; % default value
   end
end

% Get SZ parameter -- could be an image, one vector or a set of scalars
if numel(varargin)>1
   % Set of scalars
   for ii=1:numel(varargin)
      if ~isnumeric(varargin{ii}) || ~isscalar(varargin{ii})
         error('SZ argument must be either a dip_image, an array, or a set of numbers')
      end
   end
   sz = [varargin{:}];
elseif ~isempty(varargin)
   % One vector or an image
   sz = varargin{end};
   if isa(sz,'dip_image')
      if isempty(sz)
         out = dip_image;
         return
      end
      sz = imsize(sz);
   elseif ~isvector(sz)
      error('First input argument expected to be a dip_image or a size array')
   end
end
if strcmp(mode,'skinny')
   % Set all elements of SZ to 1 except the selected dimension
   if ~isnumeric(value)
      error('Value must be numeric')
   end
   nd = length(sz);
   I = true(1,nd);
   if value <= nd
      I(value) = false;
   end
   sz(I) = 1;
end

% Call COORDINATES
out = coordinates(sz,value,origin,options);
