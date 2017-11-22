%DIPINIT   Initialize the working environment
%   DIPINIT is called by DIPIMAGE when starting up. It initializes
%   the working environment. Since it is a script, it is possible
%   to initialize variables too.
%   Note that you can also call this script yourself, to re-set the
%   windows to their initial location.
%
%   The commands herein are only an example. You can copy this file
%   to your local directory (make sure it sits before the DIPimage
%   toolbox directory on your path), and edit it to your liking.

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

dipinit_window10 = 'other'; % for DIPimage up to version 2.3 this was 'a'
dipinit_window11 = 'other'; % for DIPimage up to version 2.3 this was 'b'
dipinit_window12 = 'other'; % for DIPimage up to version 2.3 this was 'c'
dipinit_window13 = 'other'; % for DIPimage up to version 2.3 this was 'd'
dipinit_window14 = 'other'; % for DIPimage up to version 2.3 this was 'ans'
dipinit_window15 = 'other';

% Remove any previous links
dipfig -unlink

% Get window sizes
dipinit_ws = [dipgetpref('DefaultFigureWidth'),dipgetpref('DefaultFigureHeight')];
dipinit_xs = [0,0]; % extra spacing to add around windows

% First link (we'll use this window to measure outer size of windows)
dipinit_h = dipfig(10,dipinit_window10,dipinit_ws);
drawnow; % for the benefit of MATLAB 7.0.1 on Windows XP
pause(0.2) % somehow needed when this file is being called from within DIPimage GUI

% Determine size of elements & spacing of windows
dipinit_sp = get(dipinit_h,'OuterPosition');
dipinit_sp = dipinit_sp(3:4); % size of the window with decorations
dipinit_ss = get(0,'ScreenSize');
dipinit_ss = dipinit_ss(3:4)-dipinit_sp; % position for window on top-right
dipinit_sp = dipinit_sp+dipinit_xs; % spacing between window positions

% Set the position of the first window
% On MacOS it will be pushed down to accomodate the menu bar at the top of
% the screen; adjust dipinit_ss to match
set(dipinit_h,'position',[dipinit_ss-dipinit_sp.*[2,0],dipinit_ws]);
drawnow
pause(0.2)
dipinit_ss = get(dipinit_h,'position');
dipinit_ss = dipinit_ss(1:2)+dipinit_sp.*[2,0];

% Make sure we have enough vertical real estate for these windows
% (we make assumptions about the horizontal sizes here...)
if dipinit_ss(2) < 2*dipinit_sp(2)
   % The screen is too small to fit the three windows
   dipinit_sp(2) = dipinit_ss(2)/2;
end

% Create the rest of the windows
dipfig(11,dipinit_window11,[dipinit_ss-dipinit_sp.*[0,0],dipinit_ws]);
dipfig(12,dipinit_window12,[dipinit_ss-dipinit_sp.*[1,1],dipinit_ws]);
dipfig(13,dipinit_window13,[dipinit_ss-dipinit_sp.*[0,1],dipinit_ws]);
dipfig(14,dipinit_window14,[dipinit_ss-dipinit_sp.*[1,2],dipinit_ws]);
dipfig(15,dipinit_window15,[dipinit_ss-dipinit_sp.*[0,2],dipinit_ws]);

% Message for the benefit of the new user
disp(' ')
disp('   The image display windows you see now are created by dipinit.m.')
disp('   Type HELP DIPINIT to learn how to modify these default windows.')
disp(' ')

% Clear local variables - this is necessary because this is a script
clear dipinit_*
% Note how all variables start with 'dipinit_'. This is to avoid conflicts with
% any variables you might have defined in your base workspace, where this script
% executes.
