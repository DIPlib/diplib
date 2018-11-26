%WRAP   Wraps (circular shifts) an image
%  Equivalent (but faster) to SHIFT with the 'nearest' interpolation method
%  and the 'periodic' boundary condition.
%
% SYNOPSIS:
%  image_out = wrap(image_in,shift)
%
% PARAMETERS:
%  shift: array containing an integer shift for each dimension
%
% SEE ALSO:
%  shift, dip_image/circshift
%
% DIPlib:
%  This function calls the DIPlib function dip::Wrap.

% (c)2017-2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
% Based on original DIPimage code: (c)2010-2011, Cris Luengo
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

function out = wrap(varargin)
out = dip_geometry('wrap',varargin{:});
