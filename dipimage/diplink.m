%DIPLINK   Linking of displays for 3D images
%   DIPLINK allows the user to select one or more display windows to
%   link the current window with.
%   DIPLINK OFF unlinks the current figure window.
%
%   Linking a window with one or more other windows means that the latter
%   will display the same slice number and orientation as the former.
%   The linking is one way, e.g. if 'a' is linked with 'b', 'b' is not linked
%   with 'a'. Changing the slice of 'a' changes that of 'b', but changing the
%   slice of 'b' does not affect 'a'.
%
%   Also: DIPLINK(H,'ON'), DIPLINK(H,'OFF'), etc. to specify a window handle.
%
%   DIPLINK(H,LIST) links display H with the displays in LIST. LIST is either
%   a numeric array with figure window handles, or a cell array with variable
%   names. The handle H cannot be left out in this syntax.
%
%   NOTE: DIPLINK(H,CHARLIST) only works on registerd windows, i.e.
%   windows that have been bound to a variable using dipfig.
%
%   NOTE: For backwards compatability, DIPLINK ON is the same as DIPLINK.
%
%   See also DIPSHOW, DIPMAPPING, DIPSTEP, DIPZOOM, DIPTEST.

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

function diplink(arg1,arg2)

list = NaN;
if nargin == 0
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
   action = 'on';
elseif nargin == 1
   if ischar(arg1)
      fig = get(0,'CurrentFigure');
      if isempty(fig)
         error('No figure window open to do operation on.')
      end
      if any(strcmpi(arg1,{'on','off','toggle'}))
         action = lower(arg1);
      else
         try
            fig = getfigh(arg1);
         catch
            error('Argument must be a valid figure handle.')
         end
      action = 'on';
      end
   else
      try
         fig = getfigh(arg1);
      catch
         error('Argument must be a valid figure handle.')
      end
      action = 'on';
   end
else
   try
      fig = getfigh(arg1);
   catch
      error('Argument must be a valid figure handle.')
   end
   if ischar(arg2) && any(strcmpi(arg2,{'on','off','toggle'}))
      action = lower(arg2);
   elseif isempty(arg2)
      action = 'on';
   else
      list = arg2;
      action = 'on';
   end
end
tag = get(fig,'Tag');
if strncmp(tag,'DIP_Image_2D',12) || strncmp(tag,'DIP_Image_3D',12) || strncmp(tag,'DIP_Image_4D',12)
   udata = get(fig,'UserData');
   switch (action)
   case 'toggle'
      if isempty(udata.linkdisplay)
         makeDIPlinkObj(fig,udata,list);
      else
         deleteDIPlinkObj(fig,udata);
      end
   case 'on'
      makeDIPlinkObj(fig,udata,list);
   case 'off'
      if ~isempty(udata.linkdisplay)
        deleteDIPlinkObj(fig,udata)
      end
   end
end


%
% Link display
%
function makeDIPlinkObj(fig,udata,list)
tag = get(fig,'Tag');
nD = tag(11);
if isnumeric(list) && isnan(list)
   newlist = handleselect(['Select a ',nD,'D image display'],fig,udata.linkdisplay,[nD,'D']);
   if ischar(newlist)
      return
   end
else
   newlist = [];
   if iscell(list)
      jj = 1;
      for ii=1:numel(list)
         try
            newlist(jj) = getfigh(list{ii});
            jj = jj+1;
         end
      end
   elseif isnumeric(list)
      jj = 1;
      for ii=1:numel(list)
         try
            newlist(jj) = getfigh(list(ii));
            jj = jj+1;
         end
      end
   elseif ischar(list)
      try
         newlist = getfigh(list);
      end
   else
      return
   end
   valid = zeros(size(newlist));
   for ii=1:length(newlist)   
       valid(ii) = strncmp(get(newlist(ii),'Tag'),['DIP_Image_' num2str(nD) 'D'],12);    
   end
   newlist = newlist(logical(valid));
   if isempty(newlist)
      return
   end
end
udata.linkdisplay = newlist;
set(fig,'UserData',[]); % Solve MATLAB bug!
set(fig,'UserData',udata);
if isempty(newlist)
   set(findobj(fig,'Tag','diplink'),'Checked','off');
else
   dipshow(fig,'updatelinked',[]);
   set(findobj(fig,'Tag','diplink'),'Checked','on');
end


%
% Unlink display
%
function deleteDIPlinkObj(fig,udata)
udata.linkdisplay = [];
set(fig,'UserData',[]); % Solve MATLAB bug!
set(fig,'UserData',udata);
set(findobj(fig,'Tag','diplink'),'Checked','off');
