%DIRNAME = GETDIR(TITLE)
%    Asks the user for a dirname using a dialog box.

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

function dirname = getdir(title)

if nargin ~= 1 || ~ischar(title)
   error('Invalid input.')
end

defdir = dipgetpref('CurrentImageFileDir');
if isempty(defdir)
   % Go to first directory specified in path.
   defdir = convertpath(dipgetpref('imagefilepath'));
   if ~isempty(defdir) && ~isempty(defdir{1})
      defdir = defdir{1};
   else
      defdir = '';
   end
end
dirname = uigetdir(defdir,title);
if ~ischar(dirname)
   % uigetdir returns 0 when the user presses CANCEL.
   dirname = '';
else
   dipsetpref('CurrentImageFileDir',dirname);
end
