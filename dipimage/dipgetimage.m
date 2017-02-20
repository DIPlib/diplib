%DIPGETIMAGE   Retrieves an image from a display
%   B = DIPCROP(H) returns the image in figure window H in B.
%
%   See also DIPSHOW, DIPCROP.

% (C) Copyright 1999-2001               Pattern Recognition Group
%     All rights reserved               Faculty of Applied Physics
%                                       Delft University of Technology
%                                       Lorentzweg 1
%                                       2628 CJ Delft
%                                       The Netherlands
%
% Cris Luengo, September 2001
% 22 September 2001: Changed specs of 1D image display (again, *sigh*).

function out = dipgetimage(fig)

% Parse input
if nargin == 0
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
else % nargin == 1
   if ischar(fig) & strcmp(fig,'DIP_GetParamList')
      out = struct('menu','Display',...
                   'display','Retrieve image from display',...
                   'inparams',struct('name',       {'fig'},...
                                     'description',{'Figure window'},...
                                     'type',       {'handle'},...
                                     'dim_check',  {0},...
                                     'range_check',{[]},...
                                     'required',   {0},...
                                     'default',    {[]}...
                                    ),...
                   'outparams',struct('name',{'out'},...
                                      'description',{'Output image'},...
                                      'type',{'image'}...
                                      )...
                  );
      return
   end
   try
      fig = getfigh(fig);
   catch
      error('Argument must be a valid figure handle.')
   end
end

tag = get(fig,'Tag');
if ~strncmp(tag,'DIP_Image',9)
   error('DIPGETIMAGE only works on images displayed using DIPSHOW.')
end

udata = get(fig,'UserData');
switch length(udata.imsize)
   case {3,4}
      out = udata.slices;
   case {2,1}
      if ~isempty(udata.colspace)
         out = udata.colordata;
      else
         out = udata.imagedata;
      end
   otherwise
      error('Cannot handle dimensionality of image data!')
end
