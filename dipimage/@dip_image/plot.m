%PLOT   plots a one dimensional image
%   This function simply converts all dip_image objects in the input
%   to DOUBLE and passes on all the arguments to PLOT.

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

function out = plot(varargin)

for ii=1:nargin
   if isa(varargin{ii},'dip_image');
      varargin{ii} = double(varargin{ii});
   end
end

if nargout
   out = plot(varargin{:});
else
   plot(varargin{:});
end
