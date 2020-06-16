%OPTICFLOW   Computes the optic flow of a 2D/3D time series
%  Image_in is a time series of 2D/3D dip_images, ordering x,y,t/ x,y,z,t
%  stored in one spatiotemporal dip_image 3D/4D.
%  The flow is computed via; dI/dt =0
%  v = -(\nabla I \nabla I^t)^{-1} \nabla I I_time
%
% SYNOPSIS:
%  out = opticflow(image_in, SpatialSig, TimeSig, TensorSig, Frame, Method)
%
% PARAMETERS:
%  SpatialSig   sigma of the spatial derivatives
%  TimeSig      sigma for the time derivatives
%  TensorSig    smoothing of the matrix of derivatives
%  Frame        frame number at which to compute the optic flow, otherwise
%               for all frames the flow is computed at once; a 3*TensorSig
%               window to both sides of the current time slice is used to
%               compute the flow
%  Method       'direct', fast but high memory usage
%               'sequential' slow but lower memory usage
%               the first and last 3*TensorSig frames are not computed
%
% DEFAULTS:
%  SpatialSig = 1
%  TimeSig    = 1
%  TensorSig  = 4
%  Frame      = -1 (all frames)
%  Method     = 'direct'
%
%  A 2D/3D vector images is returned with the x-coordinate
%  of the velocity field in the first component out{1}, etc.
%
% LITERATURE:
% Lucas, B.D. and Kanade, T.,
% An Iterative Image Registration Technique with an Application to Stereo Vision
% Proc. Image Understanding Workshop, p. 121-130, 1981

% (C) Copyright 1999-2007               Pattern Recognition Group
%     All rights reserved               Faculty of Applied Physics
%                                       Delft University of Technology
%                                       Lorentzweg 1
%                                       2628 CJ Delft
%                                       The Netherlands
%
% Bernd Rieger, Apr 2001.
% Bernd, Aug 2001, 6sigma window for time slice computation
%                  option to compute flow sequential/direct
% Bernd, March 2002, changed computation of slices to use less time
% Bernd, Jan 2004, removed 4D dimension limit, KvW implemented higher dim inverse
% MvG, Jun 2005, added undocumented feature: for (x,y,t) [3D] input only,
%                a confidence measure is optionally returned.
% 10 October 2007: Calling DERIVATIVE instead of DIP_GAUSS directly. (CL)

function [out,conf] = opticflow(in, sigma, stime, stensor, t0, method)

if nargin < 2
    sigma = 1;
end
if nargin < 3
    stime = 1;
end
if nargin < 4
    stensor = 4;
end
if nargin < 5
    t0 = -1;
end
if nargin < 6
    method = 'direct';
end
if ~isa(in,'dip_image')
    in = dip_image(in);
end

di = ndims(in); %dimesions of the input image
stensor = repmat(stensor,1,di);

if t0<0
    s = whos('in');
    me = s.bytes*(di-1);
    fprintf('Memory for output vector image +- %d MB\n',round(me/2^20));
    if round(me/2^20)>500
        ant=input('Are you sure the 4D image has reasonable size?[y/n]','s');
        if strcmp(ant,'n')
            return
        end
    end
end
if strcmp(method,'sequential') & t0>=0
    method = 'direct';
end
if strcmp(method,'sequential')
    fprintf('Sequential computation, get some coffee...\n');
    out = opt_step(in, sigma, stime, stensor);
    return;
end
if t0>=0
    sT = stensor(end);
    if t0 < 3*sT | t0 > size(in,di)-3*sT
        error(['You request a time slice that cannot be calculated' ...
            ' as it is within  3 sigma tensor form the image borders, 99% confidence.']);
    end
    % compute 6sigma window indices and use this as input
    s = repmat({':'},1,di-1);
    in = in(s{:},t0-3*sT:3*sT+t0);
end

sigma = [repmat(sigma,1,di-1) stime];
Ix = gradientvector(in,sigma,'best','zero order');
It = Ix{di};
Ix = Ix{1:di-1};

%mem_usage/2^20
A = gaussf(Ix*Ix',stensor);
b = gaussf(Ix.*It,stensor);

out = -1*inv(A)*b;

if (nargout>1) & (di==3)
    It2 = gaussf( It*It, stensor );
    conf = dip_symmetriceigensystem3(...
        A{1,1},A{1,2},b{1},A{2,2},b{2},It2,{'cylindrical'});
end

if t0>=0
    out = slice_ex(out,3*sT);
end
end

function out = opt_step(in,sg,stime,sT)
sz = size(in);
di = ndims(in); %spatial + time

out = newtensorim(di-1,sz);
s1 = repmat({':'},1,di-1);

%for ii=0:sz(end)-1
sTt =sT(end);
fprintf('Computation from time slice %d to %d\n',3*sTt,sz(end)-1-3*sTt);
for ii=3*sTt:sz(end)-1-3*sTt
    tmp = opticflow(in,sg,stime,sT(1),ii,'direct');
    for jj = 1:di-1
        out{jj}(s1{:},ii) = tmp{jj};
    end
end
end