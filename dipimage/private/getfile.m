%FILENAME = GETFILE(MASK,TITLE)
%    Asks the user for a filename using a dialog box.

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

function filename = getfile(mask,title)
if nargin ~= 2 || (~ischar(mask) || ~ischar(title))
   error('Invalid input.')
end

cp = [pwd,filesep];
defdir = dipgetpref('CurrentImageFileDir');
if ~isempty(defdir)
   % Go to 'current directory'.
   try
      cd(defdir);
   catch
      warning('Non-existing directory referenced.');
      dipsetpref('CurrentImageFileDir','');
   end
else
   % Go to first directory specified in path.
   defdir = convertpath(dipgetpref('imagefilepath'));
   if ~isempty(defdir) && ~isempty(defdir{1})
      try
         cd(defdir{1});
      catch
         warning('Non-existing directory referenced.');
      end
   end
end
if isempty(mask)
   mask = '*.*';
end
if isunix && strcmp(mask,'*.*')
   mask = '*';
end
[filename,p] = uigetfile(mask,title);
% Return to original directory.
cd(cp);
if ~ischar(filename)
   % uigetfile returns [0,0] when the user presses CANCEL.
   filename = '';
else
   % This part removes the path if it is equal to the current path.
   if ~isunix
      p = lower(p);
      cp = lower(cp);
   end
   if ~strcmp(p,cp)
      filename = [p,filename];
   end
   dipsetpref('CurrentImageFileDir',p);
end
