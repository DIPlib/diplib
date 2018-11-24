%PATHCLOSING   Path closing or opening
%
% SYNOPSIS:
%  image_out = pathclosing(image_in,filterParams,polarity,mode)
%
% See PATHOPENING for documentation. This function is identical but has
% POLARITY set to 'closing' by default.

% (c)2017-2018, Cris Luengo.
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

function out = pathopening(varargin)
out = dip_morphology('pathclosing',varargin{:});
