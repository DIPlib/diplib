%VIEWSLICE   Shows image slices in an interactive display window
%   VIEWSLICE(B) displays the image in B.
%   VIEWSLICE(B,TITLE) displays the image in B with title TITLE.
%
%   H = VIEWSLICE(B,...) returns a Java object of class
%   org.diplib.viewer.Viewer, which extends javax.swing.JFrame.
%
%   VIEWSLICE(H, B) where H is a Java object returned by VIEWSLICE
%   displays the image in B in H's window.
%
% HINTS:
%   If there is any active window, the mex file is locked to prevent it
%   from being unloaded. Call VIEWSLICE without arguments to unlock it
%   after closing all windows.
%
%   You can call any standard javax.swing.JFrame member function on
%   the returned handle, e.g.
%
%      h = viewslice(readim);
%      h.setLocation(0, 0);
%      h.setSize(800, 600);
%
%   To close programatically, use the DISPOSE member function:
%
%      h = viewslice(readim);
%      h.dispose()
%
%   See https://diplib.org/diplib-docs/group__viewer.html#details
%   for mouse interaction and keyboard shortcuts.

% (c)2018, Wouter Caarls
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
