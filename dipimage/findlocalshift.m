%FINDLOCALSHIFT   Estimates locally the shift
%
% SYNOPSIS:
%  [ms, mc] = findlocalshift(in1, in2, boxsize, boxgrid, expec, zoom, sigma, mask,  CRlB)
%
% PARAMETERS:
%  in1:  2D input image
%  in2:  2D input image
%  boxsize: either a scalar for the pixel size of the box between which the shift is
%           computed, or an image with boxsize x in slice 0 and boxsize y in slice 1.
%  boxgrid: either a scalar e.g. 5 for a 5x5 grid points or
%           an image with the grid points in pixel [0,size-1], i.e.
%           in slice 0 the x position and in slice 1 the y corresponding y position
%  expec:   the expected shift. The correlation is computed between boxes that are
%           shifted relative to each other, an image with the expected shift vector
%           per point or per boxgrid point
%  scale:   scale the box before doing the cross-correlation?
%           1: blow up, -1 shrink (only if expec is given for all input pixels)
%  sigma:   smoothing for the shift finding
%  mask:    mask image, shift is only computed if the box corners do not lie inside
%           the mask
%  CRlB:    compute Cramer-Rao lower bound for the std of the shift iter estimate as
%           Pham (2005) [yes/no]
%
% DEFAULTS:
%  boxsize: 31
%  boxgrid: 13
%  expec:   0
%  scale:   1
%  sigma:   1
%  mask:    []
%  CRlB:    0
%
% OUTPUT:
%  ms: the shift vectors per grid box (pixel) [x: slice0, y: slice1 ]
%  mc: grid box center coordinates (pixel) [slice 0: x coord as with meshgrid]
%  mask_bgrid: the mask at the grid box center positions
%
% EXAMPLE:
%  a = readim
%  b = shift(a,[-1.5 3])
%  findlocalshift(a,b)
%  bs = rr([13 13])+5;
%  bs = cat(3,bs,bs);
%  bg = cat(3,xx([13 13],'corner')+25, yy([13 13],'corner')+25);
%  findlocalshift(a,b,bs,bg)
%
% SEE ALSO:
%  localshift, orientation/vectorplot
%
% LITERATURE:
%  T.Q. Pham, M. Bezuijen, L.J. van Vliet, K. Schutte and C.L. Luengo Hendriks
%  Performance of optimal registration estimators, Z. Rahman, R.A. Schowengerdt
%  and S.E. Reichenbach, editors. Volume 5817 of Proceedings of SPIE, pp. 133-144,
%  March 29-30 2005.


% (C) Copyright 2005               FEI Electron Optics - Building AAEp
%     All rights reserved          PO Box 80066
%                                  5600 KA Eindhoven
%                                  The Netherlands
% Bernd Rieger, May 2005
% Sep 2006, added image input for variable boxsize

function varargout = findlocalshift(in1, in2, boxsize, boxgrid, expshift, scaling,sigma, mask, CRlB)

if nargin < 3
    boxsize = dip_image(31);
else
    boxsize = dip_image(boxsize);
end
if nargin < 4
    boxgrid = dip_image(13);
else
    boxgrid = dip_image(boxgrid);
end
if nargin < 5
    expshift = dip_image(0);
else 
    expshift = dip_image(expshift);
end
if nargin < 6
    scaling = 0;
end
if nargin < 7
    sigma = 1;
end
if nargin < 8
    mask = dip_image([]);
else 
    mask = dip_image(mask);
end
if nargin < 9
    CRlB = 0;
end

sz1 = size(in1);
sz2 = size(in2);
szm = size(mask);
if ndims(in1) ~=2 | ndims(in2) ~=2
    error('Images must be 2D.');
end
if sz1 ~= sz2
    error('Image are not same size.');
end


%compute pixel coordinates of the box grid points
if ndims(boxgrid)==0
    %keep 10% safty margin to each borders
    perc = .8;
    boxgrid = double(boxgrid);
    nsz = perc * sz1;
    pc_d = nsz./boxgrid;
    [x,y]=meshgrid(1:boxgrid,1:boxgrid);
    pc_x = sz1(1)*(1-perc)/2 - pc_d(1)/2 + x*pc_d(1);
    pc_y = sz1(2)*(1-perc)/2 - pc_d(2)/2 + y*pc_d(2);
    tx = reshape(pc_x,1,prod(size(pc_x)));
    ty = reshape(pc_y,1,prod(size(pc_y)));
    mc = round(cat(1,tx,ty));
else
    sz = size(boxgrid);
    tx = reshape(double(slice_ex(boxgrid,0)),1,prod(sz(1:2)));
    ty = reshape(double(slice_ex(boxgrid,1)),1,prod(sz(1:2)));
    mc = round(cat(1,tx,ty));
    if size(mc,1) ~=2
        error('Box coordinates not in format [Nx2]')
    end
    boxgrid = sz(1);
end


if ndims(boxsize)==0
    bx2 = round((double(boxsize)-1)/2);
    bx2 = repmat(bx2,2,size(mc,2));
else
    szS = size(boxsize);
    boxsize = round((boxsize-1)/2);
    tx = reshape(double(slice_ex(boxsize,0)),1,prod(szS(1:2)));
    ty = reshape(double(slice_ex(boxsize,1)),1,prod(szS(1:2)));
    bx2 = cat(1,tx,ty);
    if size(bx2,1) ~=2 | any(size(bx2)~=size(mc))
        error('Box sizes not in format [Nx2] or does not match given box coordinates.')
    end
end

if ndims(expshift)==0
    mc_es = mc;
    ES = zeros(size(mc));
    scaling = 0;
else
    if ndims(expshift)~=3
        error('Expected shift needs x and y componets in one 3D image.');
    end
    expshiftI = round(expshift); %either handle perdicted integer shift by box shifting or 'dewarp' by interpolation (chose 1)
    szE = size(expshift);
    if any(szE(1:2)~=size(in1))
        tx = reshape(double(slice_ex(expshiftI,0)),1,prod(szE(1:2)));
        ty = reshape(double(slice_ex(expshiftI,1)),1,prod(szE(1:2)));
        scaling =0;
    else
        mc_o1 = dip_image(reshape(mc(1,:),boxgrid,boxgrid),'dfloat');
        mc_o2 = dip_image(reshape(mc(2,:),boxgrid,boxgrid),'dfloat');
        co1 = [flatten(mc_o1) flatten(mc_o2)];
        ind1 = coord2image(co1,szE(1:2));
        ind1 = cat(3,ind1,ind1);
        tshift = expshiftI(ind1); %only get the expected shifts at the boxgird coordiantes
        tshift = permute(reshape(tshift,boxgrid,boxgrid,2),[2,1,3]);
        tx = reshape(double(slice_ex(tshift,0)),1,boxgrid^2);
        ty = reshape(double(slice_ex(tshift,1)),1,boxgrid^2);
    end
    clear expshiftI
    ES = cat(1,tx,ty);
    mc_es = mc + ES;
    if size(ES,1) ~=2 | any(size(ES)~=size(mc))
        error('Expected shift not in format [Nx2] or does not match given box coordinates.')
    end
end

if CRlB
    sig = noisestd(in1);   %sig = 1; %Gaussian noise estimate
end

Nbox = size(mc,2);
ms1 = zeros(size(mc));
%ms = newim(size(mc),'dfloat');

S1 = newim([2*bx2(1,1)+1,2*bx2(2,1)+1,Nbox]);
S2 = newim([2*bx2(1,1)+1,2*bx2(2,1)+1,Nbox]);
S3 = newim([2*bx2(1,1)+1,2*bx2(2,1)+1,Nbox]);

mask_boxgrid = zeros(1,Nbox);
Emask = ~isempty(mask);
for ii = 1:Nbox
    bx2_x = bx2(1,ii);
    bx2_y = bx2(2,ii);
    
    %check if box intersects image borders
    if (mc(1,ii)-bx2_x <0) | (mc(1,ii)+bx2_x > sz1(1)-1) | (mc(2,ii)-bx2_y <0) | (mc(2,ii)+bx2_y > sz1(2)-1)
        error('Box intersects image border, use smaller boxsize or less boxgridpoints.');
        %TODO maybe clip border here?
    end
    
    %test if the box corners are inside the provided mask
    if Emask
        %fprintf('mc %f %f\n',mc(1,ii),mc(2,ii));
        if   mask(mc(1,ii)-bx2_x,mc(2,ii)-bx2_y) ...
                | mask(mc(1,ii)-bx2_x,mc(2,ii)+bx2_y) ...
                | mask(mc(1,ii)+bx2_x,mc(2,ii)-bx2_y) ...
                | mask(mc(1,ii)+bx2_x,mc(2,ii)+bx2_y)
            mask_boxgrid(ii) = 1;
            continue;
        end
    end
    %fprintf('ii: %d, %d %d %d %d\n',ii,mc_es(1,ii)-bx2_x, mc_es(1,ii)+bx2_x, mc_es(2,ii)-bx2_y, mc_es(2,ii)+bx2_y);
    if ndims(expshift)==3
        if (mc_es(1,ii)-bx2_x <0) | (mc_es(1,ii)+bx2_x > sz1(1)-1) | (mc_es(2,ii)-bx2_y <0) | (mc_es(2,ii)+bx2_y > sz1(2)-1) ...
                mask_boxgrid(ii) = 1;
            continue;
        end
        if Emask
            if   mask(mc_es(1,ii)-bx2_x, mc_es(2,ii)-bx2_y) ...
                    | mask(mc_es(1,ii)-bx2_x, mc_es(2,ii)+bx2_y) ...
                    | mask(mc_es(1,ii)+bx2_x, mc_es(2,ii)-bx2_y) ...
                    | mask(mc_es(1,ii)+bx2_x, mc_es(2,ii)+bx2_y)
                mask_boxgrid(ii) = 1;
                continue;
            end
        end
    end
    
    %retrieve the two boxes from the image
    tmp1 = in1(mc(1,ii)-bx2_x : mc(1,ii)+bx2_x, mc(2,ii)-bx2_y : mc(2,ii)+bx2_y);
    tmp2 = in2(mc_es(1,ii)-bx2_x : mc_es(1,ii)+bx2_x, mc_es(2,ii)-bx2_y : mc_es(2,ii)+bx2_y);
    %S1(:,:,ii-1)=tmp1;
    %S2(:,:,ii-1)=tmp2;
    
    if scaling
        ts = expshift(mc(1,ii)-bx2_x : mc(1,ii)+bx2_x, mc(2,ii)-bx2_y : mc(2,ii)+bx2_y,:);
        n = norm(im2array(ts));
        zo = 1 + (max(n)-min(n))/boxsize;
        zo = double(sign(scaling) * zo);
        %fprintf('Box %d, zo %f, mean: %f, max: %f, std: %f\n',ii, zo, mean(n),max(n),std(n))
        tmp2 = resample(tmp2,zo,[0 0],'3-cubic');
        tmp2 = cut(tmp2,size(tmp1));
        
    end
    %S3(:,:,ii-1) = tmp2;
    %if ii==50
    %   fprintf('mc: %d %d\n',mc(:,ii));
    %   fprintf('mc_es: %d %d\n',mc_es(:,ii));
    %   dipshow(1,tmp1);diptruesize(1,'off');
    %   dipshow(2,tmp2);diptruesize(2,'off');
    %   pause
    %end
    s = findshift(tmp1,tmp2,'iter',sigma).';
    s = s - ES(:,ii); %correct for preshifted box; conform the findshift usage
    ms1(:,ii) = s;
    
    if CRlB
        %computing Cramer-Rao lower bound for the std of the shift iter estimate
        %as Pham2005
        g=gradient(tmp1);
        G=newtensorim([2,2]);
        G{1,1} = sum(g{1}^2);
        G{2,2} = sum(g{2}^2);
        G{1,2} = sum(g{1}*g{2});
        G{2,1} = G{1,2};
        G = G./sig.^2;
        dG = det(G);
        sx(ii) = double(sqrt( G{2,2}/dG));
        sy(ii) = double(sqrt( G{1,1}/dG));
        fprintf('Processing box: %d, CRlb stdx: %f, stdy %f\n',ii,sx(ii),sy(ii));
    end
end
ms = newim(boxgrid, boxgrid,2,'dfloat');
ms(:,:,0) = dip_image(reshape(ms1(1,:),boxgrid,boxgrid),'dfloat');
ms(:,:,1) = dip_image(reshape(ms1(2,:),boxgrid,boxgrid),'dfloat');

varargout{1} = ms;
if nargout >1
    mc_out = newim(boxgrid, boxgrid,2,'dfloat');
    mc_out(:,:,0) = dip_image(reshape(mc(1,:),boxgrid,boxgrid),'dfloat');
    mc_out(:,:,1) = dip_image(reshape(mc(2,:),boxgrid,boxgrid),'dfloat');
    varargout{2} = mc_out;
end
if nargout >2
    varargout{3} = dip_image(reshape(mask_boxgrid,[boxgrid boxgrid]),'bin');
end

if CRlB
    fprintf('mean CRlB: %f %f\n',mean(sx),mean(sy));
end

end

