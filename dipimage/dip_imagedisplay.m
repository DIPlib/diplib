%dip_imagedisplay   For internal use of DIPSHOW

% This file is here, not in the private/ subdirectory, because it is not legal to define private classes.

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

classdef dip_imagedisplay < handle
   properties (GetAccess=public,SetAccess=private)
      handle = []
   end
   methods
      function obj = dip_imagedisplay(bla,h)
         if ~isequal(bla,'create'), error('Wrong input'), end
         if ~isnumeric(h) || ~isscalar(h) || fix(h)~=h, error('Illegal handle'), end
         obj.handle = h;
      end
      function delete(obj)
         try
            imagedisplay(obj,'clear')
         catch
            % let's just ignore this error.
         end
      end
   end
end
