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

% (c)2018-2021, Cris Luengo.
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

% Find the DIPlib installation directory, which is three directories up from
% where this M-file lives.
dippath = fileparts(fileparts(fileparts(mfilename('fullpath'))));

if ispc
   % On Windows we pass the name of the DIPlib library file explicitly.
   dipopts = { fullfile(dippath,'lib','DIP.lib') };
else
   % On Linux and macOS we use the -l and -L options to link to the DIPlib library.
   dipopts = { '-lDIP',...
               ['-L"',fullfile(dippath,'lib'),'"'] };
end

% The remaining options are the same for all platforms.
%
% Note that on Windows (ispc) we need to set either CXXFLAGS or COMPFLAGS,
% depending on the compiler that `mex` is configured to use. We cannot know
% what that compiler is. CXXFLAGS is for GCC and Clang, COMPFLAGS is for MSCV.
%
% We add the -largeArrayDims option, which is necessary for older versions of
% MATLAB. It is the default option since R2010b or R2011a (release notes are
% ambiguous). Since R2018a we have the -R2017b option that we could use instead.
% This option implies -largeArrayDims, and also affects graphics handles, but
% we don't care about that here.
dipopts = [dipopts, { ['-I"',fullfile(dippath,'include',''),'"'],...
                      'CXXFLAGS=$CXXFLAGS -std=c++14',...
                      'COMPFLAGS=$COMPFLAGS /std:c++14',...
                      '-largeArrayDims' }];

mex(varargin{:},dipopts{:});
