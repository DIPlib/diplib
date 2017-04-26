%DIPANIMATE   Animates a 3D image in a display window
%   DIPANIMATE(H) animates the 3D image in window with handle H. It shows
%   all slices in sequence, starting at 0 till the end. Interrupt with Esc.
%   H defaults to the current figure window.
%
%   DIPANIMATE(H,T) waits T seconds between slices. T must be at least 0.05
%   (20 frames per second). This minimum is imposed because a smaller pause
%   is useless. H cannot be left out in this syntax.
%
%   DIPANIMATE(...,'loop') loops. It continues going until the user interrupts
%   with the Esc key. The looping is performed by going from slice 0 to the
%   last slice, then backwards, then forward again, etc. To skip the reverse
%   sequence use 'loopfwd'.
%
%   Note: If you feel the need to interrupt this function with Ctrl-C,
%   you will need to refresh the display (by re-displaying the image
%   or changing the 'Actions' state).
%
%   See also DIPSHOW.

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

function dipanimate(varargin)

% Parse input
loop = 0;
t = 0.05;
N = nargin;
if N>3
   error('Too many input arguments.')
end
if N>=1
   arg = varargin{N};
   if ischar(arg)
      if any(strcmpi(arg,{'loop','yes'}))
         loop = 1;
         N = N-1;
      elseif strcmpi(arg,{'loopfwd'})
         loop = 2;
         N = N-1;
      end
   end
end
if N>=1
   try
      fig = getfigh(varargin{1});
   catch
      error('Argument must be a valid figure handle.')
   end
   ii = 2;
   if ii<=N
      arg = varargin{ii};
      if isnumeric(arg) && length(arg)==1
         if arg>t
            t = arg;
         end
         ii = ii+1;
      end
   end
   if ii<=N
      error('Illegal input argument.')
   end
else
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
end

tag = get(fig,'Tag');
if ~strncmp(tag,'DIP_Image_3D',12) && ~strncmp(tag,'DIP_Image_4D',12)
   error('DIPANIMATE only works on 3/4D images displayed using DIPSHOW.')
end

figure(fig);

% Disable all callbacks
wbdF = get(fig,'WindowButtonDownFcn');
wbuP = get(fig,'WindowButtonUpFcn');
wbmF = get(fig,'WindowButtonMotionFcn');
bdF = get(fig,'ButtonDownFcn');
kpF = get(fig,'KeyPressFcn');
set(fig,'WindowButtonDownFcn','',...
        'WindowButtonUpFcn','',...
        'WindowButtonMotionFcn','',...
        'ButtonDownFcn','',...
        'KeyPressFcn','');

% Do your thing
udata = get(fig,'UserData');
Nd = length(udata.imsize);
is3D =  Nd==3;
N = udata.imsize(Nd);
set(fig,'CurrentCharacter',' '); % Avoid it containing 'Esc'.
if loop==1
   sequence = [0:(N-1),(N-2):-1:1];
else % loop==0 || loop==2
   sequence = 0:(N-1);
end
while 1
   for ii=sequence
      if is3D
         dipshow(fig,'ch_slice',ii);
      else
         dipshow(fig,'ch_time',ii);
      end
      drawnow;
      pause(t);
      newch = get(fig,'CurrentCharacter');
      if ~isempty(newch) && double(newch)==27 % 'Esc' terminates.
         loop = 0;
         break
      end
   end
   if loop==0
      break
   end
end

% Restore old settings
set(fig,'WindowButtonDownFcn',wbdF,...
        'WindowButtonUpFcn',wbuP,...
        'WindowButtonMotionFcn',wbmF,...
        'ButtonDownFcn',bdF,...
        'KeyPressFcn',kpF);
