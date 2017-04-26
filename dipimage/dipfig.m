%DIPFIG   Links a variable name with a figure window
%   DIPFIG A creates a figure window for the variable A. The functional
%   form also accepts other parameters:
%
%   DIPFIG(N,'A') links figure window N with variable name 'A'. It also
%   creates figure window N if it doesn't yet exist.
%
%   DIPFIG(...,POS) puts the (new) figure window at position POS. POS is
%   a 1-by-4 array in the form [left,bottom,width,height], or a 1-by-2
%   array in the form [width,height].
%
%   H = DIPFIG(...) also returns the figure window handle.
%
%   It is possible to link many variables to a single figure window. The
%   last variable send to the display is always the one seen. Note that
%   even the most simple operations cause the variable name to be either
%   ANS or unknown.
%
%   The variable name 'other' is reserved for those variables not associated
%   with a figure window. If more than one figure window is associated
%   with it, they will be used in alternating sequence. Other variable
%   names cannot be associated with more than one figure window.
%
%   DIPFIG is the only way of linking a variable name with a figure window.
%   DIPSHOW(N,A) causes the DIPFIG setting to be overruled, but doesn't link
%   'A' with N.
%
%   DIPFIG -UNLINK unlinks all variables names. This is a reset of the figure
%   handle list.
%
%   H = DIPFIG('-GET','A') returns the handle associated to the variable
%   named 'A'. Returns the handle associated to 'other' if 'A' was not
%   previously registered with DIPFIG, or 0 if no handle is associated to
%   'other'.
%
%   Please note that
%      DIPSHOW(X)
%      H = DIPFIG('-GET','X');
%      COORDS = DIPGETCOORDS(H,1);
%   Need not work (if 'X' is not registered, and 'other' has several handles
%   associated, DIPFIG will return a different handle than that used by DIPSHOW).
%   Instead use the syntax H = DIPSHOW(X).

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

function h = dipfig(varargin)

% Parse input
if nargin < 1
   error('Variable name expected.')
end
fig = 0;
pos = [];
n = 1;
if isnumeric(varargin{n}) && length(varargin{n})==1
   fig = double(varargin{n});
   if ~ishandle(fig) && fix(fig) ~= fig
      error('Argument must be a valid figure handle.')
   end
   n = n+1;
end
if nargin >= n && ischar(varargin{n})
   name = varargin{n};
   if ~isempty(name) && name(1)=='-'
      switch lower(name)
         case '-unlink'
            dip__fig('-unlink');
            return
         case '-get'
            if nargin > n
               name = varargin{n+1};
               if ischar(name)
                  h = dip__fig('-get',name);
                  return
               end
            end
            error('Illegal syntax.')
         otherwise
            error('Unknown flag.')
      end
   end
   if ~is_valid_varname(name)
      error('Invalid variable name.')
   end
   n = n+1;
   if nargin >= n
      pos = varargin{n};
      pos = pos(:)';
      if ~isnumeric(pos) || all(length(pos)~=[2,4])
         error('Position matrix must have 2 or 4 elements.')
      end
   end
else
   error('Variable name expected.')
end

% Create, place and initialize window
if isempty(pos)
   pos = [dipgetpref('DefaultFigureWidth'),dipgetpref('DefaultFigureHeight')];
end
if isfigh(fig) && strncmp(get(fig,'Tag'),'DIP_Image',9)
   if length(pos)==2
      coord = get(fig,'position');
      pos = [coord(1),coord(2)+coord(4)-pos(2),pos];
   end
   set(fig,'position',pos);
else
   fig = dipshow(fig,[],'name',name,'position',pos);
   if ~isnumeric(fig)
      fig = fig.Number;
   end
end
% Link variable name with figure handle
dip__fig('-link',name,fig);
% Pass handle to caller
if nargout > 0
   h = fig;
end

%
% This sub-function manages the data that persists from one call to the next.
%
function h = dip__fig(action,name,fig)

persistent handles;      % list of variable names and figure handles (struct).
persistent otherhandles; % list of figure handles for 'other' variable names.
persistent lasthandle;   % index into OTHERHANDLES for last number returned.

if ~mislocked
   % First time call -- initialise
   mlock
   handles = [];
   otherhandles = [];
   lasthandle = 0;
end

switch action
case '-link'
   if strcmpi(name,'other')
      otherhandles = [otherhandles,fig];
   else
      handles = subsasgn(handles,substruct('.',name),fig);
   end
case '-get'
   if ~isempty(handles) && any(strcmp(fieldnames(handles),name))
      h = subsref(handles,substruct('.',name));
   elseif ~isempty(otherhandles)
      lasthandle = lasthandle+1;
      if lasthandle>length(otherhandles)
         lasthandle = 1;
      end
      h = otherhandles(lasthandle);
   else
      h = 0;
   end
case '-unlink'
   % Clear all persistent variables
   handles = [];
   otherhandles = [];
   lasthandle = 0;
   munlock % allow clearing
otherwise
   error('This shouldn''t have happened.')
end
