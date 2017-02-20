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

% (C) Copyright 1999-2005               Pattern Recognition Group
%     All rights reserved               Faculty of Applied Physics
%                                       Delft University of Technology
%                                       Lorentzweg 1
%                                       2628 CJ Delft
%                                       The Netherlands
%
% Bernd Rieger, July 2001
% 16-26 August 2001 (CL): Changed DIPSHOW. This function changes accordingly.
% 22 September 2001 (CL): Changing the windows we link to immediately.
%                         New syntax: DIPLINK(H,LIST).
% Dec 2002 (BR), extended help (diplink only works on registered images names)
% 2 February 2005 (CL):   DIPLINK no longer toggles the state, but always shows
%                         the dialog box to specify new linlked windows.
% 17 october 2011 (BR):   Added linking for 2D displays

function menu_out = diplink(arg1,arg2)

% Avoid being in menu
if nargin == 1 & ischar(arg1) & strcmp(arg1,'DIP_GetParamList')
   menu_out = struct('menu','none');
   return
end

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
   if ischar(arg2) & any(strcmpi(arg2,{'on','off','toggle'}))
      action = lower(arg2);
   elseif isempty(arg2)
      action = 'on';
   else
      list = arg2;
      action = 'on';
   end
end
if strncmp(get(fig,'Tag'),'DIP_Image_2D',12) | strncmp(get(fig,'Tag'),'DIP_Image_3D',12) | strncmp(get(fig,'Tag'),'DIP_Image_4D',12)
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
nD = length(udata.imsize);
if isnumeric(list) & isnan(list)
   switch nD
      case {2,3,4}
         newlist = handleselect(['Select a ' num2str(nD) 'D image display'],fig,udata.linkdisplay,[ num2str(nD) 'D']);
      otherwise
         error('Should not happen.')
   end
   if ischar(newlist)
      return
   end
else
   newlist = [];
   
   if iscell(list)
      jj = 1;
      for ii=1:prod(size(list))
         try
            newlist(jj) = getfigh(list{ii});
            jj = jj+1;
         end
      end
   elseif isnumeric(list)
      jj = 1;
      for ii=1:prod(size(list))
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
set(fig,'UserData',[]);% Solve MATLAB bug!
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
set(fig,'UserData',[]);% Solve MATLAB bug!
set(fig,'UserData',udata);
set(findobj(fig,'Tag','diplink'),'Checked','off');
