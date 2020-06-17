% (c)2018, Cris Luengo.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
% Originally written by Bernd Rieger.
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

function out = gst_subsampled(in,sg,st)

n = ndims (in);
if numel(sg)==1
   sg = repmat(sg,1,n);
elseif numel(sg)~=n
   error(['SG has wrong length, should have ',num2str(n),' elements'])
else
   sg = sg(:).';
end
if numel(st)==1
   st = repmat(st,1,n);
elseif numel(sg)~=n
   error(['ST has wrong length, should have ',num2str(n),' elements'])
else
   st = st(:).';
end
st = max(st,sg);
st = max(st,1); % Make sure we don't pick one that is too small

grad = cell(n,1);
order = zeros(1,n);
for ii = 1:n
   order(ii) = 1;
   grad{ii} = derivative(in,order,sg);
   order(ii) = 0;
end
out = cell(n*(n+1)/2,1);
for ii = 1:n
   out{ii} = subsample(gaussf(grad{ii}*grad{ii},st),floor(st));
end
index = n;
for jj = 2:n
   for ii = 1:jj-1
      index = index + 1;
      out{index} = subsample(gaussf(grad{ii}*grad{jj},st),floor(st));
   end
end
out = dip_image(out,'symmetric matrix');
