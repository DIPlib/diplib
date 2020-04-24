%AUTOCORRELATION   Computes the auto-correlation of an image
%
% SYNOPSIS:
%  image_out = autocorrelation(image_in)
%
% NOTE:
%  The 'FtOption' preference (see DIPSETPREF) can affect the output of this function.
%  See FT for more information.

% (c)2017-2018, Cris Luengo.
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

function out = autocorrelation(in)
opts = {};
if isreal(in)
   opts = {'real'};
end
fa = ft(in);
out = ift(fa*conj(fa),opts);
