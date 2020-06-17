%LOCALSHIFT   Shifts/warps an image by a shift vector per pixel/grid point
%
% SYNOPSIS:
%  out = localshift(in, sv, interp, coord)
%
% PARAMETERS:
%  in:     2D image
%  sv:     shift vector per pixel as a dip_image_array
%  interp: Interplotation option
%          'bilinear': osize is ignored=ConstFoV, loop is done over output image
%          '3-cubic': osize is ignored=ConstFoV, loop is done over output image
%  coord:  optional coordinate grid image (2D) as meshgrid, if the shift vector is not
%          per pixel as the output from findlocalshift
%
% NOTE:
%  only implemented for 2D but could be nD
%
% SEE ALSO:
%  findlocalshift, orientation/vectorplot

% (c)2020, Delft University of Technology, by Yan Guo
% Based on original DIPimage code: (c)2005, Delft University of Technology.
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

function out = localshift(in,sv,inter,mc)

if ndims(in)~=2
    error('Only implemented for 2D.');
end
in = dip_image(in,'double'); % data casting to prevent integer output
if nargin < 2
    error('Required parameter missing');
end
if ~isa(sv,'dip_image')
    sv = spatialtotensor(dip_image(sv));
end
% if nargin < 3
%     recut = 'FullSize';
% end
if nargin < 3
    inter = 'bilinear';
end
if nargin < 4
    mc = [];
    mc = dip_image(mc);
end

% dip:ResampleAt works differently than what was used in diplib2.0
sv{1} = -sv{1}+xx(size(in),'corner');
sv{2} = -sv{2}+yy(size(in),'corner');

if size(in) ~= size(sv{1})
    if isempty(mc)
        error('No coordinate grid given and sizes of image and shift image not identical');
    end
    %assume mc is a rectangular grid inside the image borders
    %diplib 3 no longer support ex_slice, use arrangeslices instead
    o1 = min(arrangeslices(mc(:,:,0)));
    o2 = max(arrangeslices(mc(:,:,0)));
    o3 = min(arrangeslices(mc(:,:,1)));
    o4 = max(arrangeslices(mc(:,:,1)));
    szt = [o2-o1+1 o4-o3+1];
    ns = szt./size(sv{1});
    %first interpolate new shift values in the regular grid as from findlocalshift, then
    %do the borders
    sv{1} = resample(sv{1},ns,[0 0],'3-cubic');
    sv{2} = resample(sv{2},ns,[0 0],'3-cubic');

    % do borders, outside the grid generate new shift values by normalized
    % averaging
    tmp = newim(in,'bin');
    tmp(o1:o2,o3:o4)=1;
    sv1 = newim(in,'dfloat');sv2 = newim(in,'dfloat');
    sv1(tmp) = sv{1};sv2(tmp)=sv{2};
    sg = (o1/3 + (size(in,2)-o2)/3 )/2;
    sv{1} = normalized_averaging(sv1,tmp,sg);
    sv{2} = normalized_averaging(sv2,tmp,sg);
    clear tmp sv1 sv2
end

switch inter
%     case 'zoh'
%         pos = newtensorim(xx(in,'corner'),yy(in,'corner'));
%         model = pos + sv;
%         zo =1; %gobal zoom
%         %in the following the model contains the ABSOLUTE positions on the new grid
%
%         lefttop = floor( [min(model{1}) min(model{2})] * zo );
%         rightbottom = ceil( [max(model{1}) max(model{2})] * zo );
%
%         switch recut
%             case 'Recut'
%                 offset = -lefttop';      % offset of 1st frame in the HR grid
%                 outsize = rightbottom - lefttop + 1;
%             case {'FullSize'}
%                 offset = [0 0];
%                 outsize = rightbottom + 1;
%             case 'ConstFoV'
%                 offset = [0 0];
%                 outsize = size(in);
%             otherwise
%                 error('Should not happen.');
%         end
%         %fprintf('offset %d %d\n',offset);
%         %fprint f('lefttop %d %d\n',lefttop);
%         %fprintf('rightbottom %d %d\n',rightbottom);
%         %fprintf('outsize %d %d\n',outsize);
%         I = round(model{1}*zo + offset(1)) + round(model{2}*zo + offset(2))*outsize(1);
%         param = {{'lower',0,'upper',prod(outsize),'bins',prod(outsize),...
%             'lower_abs','upper_abs'}};
%         tot = reshape(mdhistogram(I,in,param), outsize);
%         cnt = reshape(mdhistogram(I,size(in)*0,~newim(in,'bin'),param), outsize);
%
%         out = normalized_averaging(tot,cnt,1);
    case 'bilinear'
        out = warp_subpixel(in,sv,'linear');
    case '3-cubic'
        out = warp_subpixel(in,sv,'3-cubic');
    otherwise
        error('Unknown interpolation option.');
end

function out = normalized_averaging(tot,cnt,s)
% Normalized the accumulated samples/weights, fill-in NaN if necessary
out = tot/(cnt+eps);    % add epsilon to avoid NaN (=0/0)
mask = cnt==0;          % fill-in empty pixels, if any
if sum(mask)>0
   tot2 = gaussf(tot,s);
   cnt2 = gaussf(cnt,s);
   out(mask) = tot2(mask)/(cnt2(mask)+eps);
end
