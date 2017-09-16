%DIPIMAGE   The DIPimage GUI (not yet implemented)
%
%   For now, all this function does is DIPIMAGE('clear'), which clears from memory
%   some MEX-files that lock themselves in memory to preserve information.

% (c)2017, Cris Luengo.
% (c)1999-2014, Delft University of Technology.
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

function dipimage(varargin)
if nargin==1
   if strcmp(varargin{1},'clear')
      close all
      imagedisplay('unlock')
      clear imagedisplay
      numberofthreads('unlock')
      clear numberofthreads
      dippreferences('unload');
      clear dippreferences
   else
      error('Invalid flag, for now only ''dipimage clear'' is supported')
   end
else
   error('The DIPimage GUI is not yet implemented')
end
