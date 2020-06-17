%DIPMEX   Compile a MEX-file that uses DIPlib
%
% SYNOPSIS:
%  dipmex filename.c
%  dipmex filename.c extrasource.c /path/lib/linkalib.a -I/path/include
%  dipmex('filename.c','extrasource.c',...)
%
%  You can add any option you can pass to the command MEX. This function
%  simply calls MEX with the given options and adds options to link in
%  the DIPlib libraries. The first source file name given will be the
%  name for the resulting MEX-file, unless an "-output" argument is
%  supplied.
%
% NOTEs:
%  This function uses the MEX function, which first needs to be
%  configured by
%     mex -setup C++
%
%  On Windows, the MSVC compiler is assumed. On Linux and MacOS, either
%  GCC or CLang compilers can be used. If you use a different compiler,
%  edit this file to adjust the options passed to the MEX command, in
%  particular the switch to tell the compiler to use C++14.
%
%  Do note that you most likely need to configure MEX to use the exact
%  same compiler make and version as was used to compile DIPlib. You will
%  see linker errors if there is a mismatch.

% (c)2018, Cris Luengo.
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

function dipmex(varargin)

if nargin<1 || any(~cellfun('isclass',varargin,'char'))
   error('filename expected');
end

dippath = mfilename('fullpath');
dippath = fileparts(fileparts(fileparts(dippath)))

if ispc
   dipopts = { fullfile(dippath,'lib','DIP.lib'),...
               ['-I"',fullfile(dippath,'include',''),'"'],...
               'COMPFLAGS=$COMPFLAGS /std:c++14'}; % MSVC-specific flag
else
   dipopts = { '-lDIP',...
               ['-L"',fullfile(dippath,'lib'),'"'],...
               ['-I"',fullfile(dippath,'include'),'"'],...
               'CXXFLAGS=$CXXFLAGS -std=c++14'}; % Works with GCC and CLang
end
% TODO: add -largeArrayDims for older MATLABs. It is the default option since R2010b or R2011a (release notes are ambiguous)

mex(varargin{:},dipopts{:});
