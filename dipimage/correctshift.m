%CORRECTSHIFT   Corrects the shift in a time series
%  All images are aligned with the image st [0,N-1].
%
% SYNOPSIS:
%  [out,s,crlb] = correctshift(in, st, sa, bo)
%
% PARAMETERS:
%  in: dipimage where last axis is time
%  st: number of frame to align [0,N-1]
%  sa: average sa frames to get better align frame st
%      or -1, algin subsequent frames to each other and then
%      align all to st frame (useful if total shift is large)
%  bo: boundary handling option. How image contend is treated that
%      appears new due to the shift
%
% OUT PARAMETERS
%  s : the shift matrix with respect to the given frame st
%  crlb: Cramer-Rao lower bound of the shift estimation accuracy
%
% NOTE:
%  If the images are very noisy, the shift estimation might not give good
%  results. In this case the shift is estimated on low-pass filtered
%  version.
%
% LITERATURE:
%  T.Q. Pham, M. Bezuijen, L.J. van Vliet, K. Schutte and C.L. Luengo Hendriks
%  Performance of optimal registration estimators, Proceedings of SPIE vol 5817,
%  pp. 133-144, 2005.

% (c)2020, Delft University of Technology, by Yan Guo
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

function [out,s,CRlb] = correctshift(in,stf,sif,boundaryoption)

if nargin < 2
    stf = 0;
end
if nargin < 3
    sif = 0;
end
if nargin < 4
    boundaryoption = 'mirror';
end

CRlb_compute = 0;
if nargout==3
    CRlb_compute=1;
end
shift_sig =1;%one pixel for smoothing

ISC =0;
if iscolor(in)
    ISC =1;
    inc = in;
    in = sum(in);
end

err = '';
try

    N = ndims(in);
    sz = size(in);

    if stf>sz(N)-1
        error('Align frame out of bounds.');
    end
    if sif > sz(N)-1
        error('Averaging out of bounds.');
    end

    if CRlb_compute
        sig_noise = noisestd(slice_ex(in,0));   %sig = 1; %Gaussian noise estimate
        CRlb = zeros(sz(N),N-1);
    end

    s = zeros(sz(N),N-1);
    if sif < 0
        %estimate shift on subsequent time slices
        for ii=0:sz(N)-2
            s(ii+1,:) = findshift(slice_ex(in,ii),slice_ex(in,ii+1),'iter',shift_sig)';
            if CRlb_compute
                CRlb(ii+1,:) = crlb_function(slice_ex(in,ii), sig_noise,shift_sig);
            end
        end
        out = newim(in);
        tmp = slice_ex(in,stf);
        out = slice_in(out,tmp,stf);
        s_out = zeros(sz(N),N-1);
        for ii=0:stf-1
            tmp_s = sum(s(ii+1:stf,:),1);
            s_out(ii+1,:) = tmp_s;
            %fprintf('Shift: %f %f\n',tmp_s);
            tmp = shift(slice_ex(in,ii),tmp_s,'bspline',boundaryoption);
            out = slice_in(out,tmp,ii);
        end
        for ii=sz(N)-1:-1:stf+1
            tmp_s = sum(s(stf+1:ii,:),1);
            s_out(ii+1,:) = tmp_s;
            tmp = shift(slice_ex(in,ii),-tmp_s,'bspline',boundaryoption);
            %fprintf('Shift: %f %f\n',tmp_s);
            out = slice_in(out,tmp,ii);
        end
        s = s_out;
    else
        fi = slice_ex(in,stf);
        out = newim(in);
        if sif>0
            st_f = max(stf-round(sif/2),0);
            st_e = min(stf+round(sif/2),sz(N)-1);
            s2 = repmat({':'},1,N-1);
            fi_sum = squeeze(mean(in(s2{:},st_f:st_e),[],N));
        else
            fi_sum = fi;
        end
        for ii=0:sz(N)-1
            s(ii+1,:) = findshift(fi_sum,slice_ex(in,ii),'iter', shift_sig)';
            tmp = shift(slice_ex(in,ii),-s(ii+1,:),'bspline',boundaryoption);
            out = slice_in(out,tmp,ii);
            if CRlb_compute
                CRlb(ii+1,:) = crlb_function(slice_ex(in,ii), sig_noise, shift_sig);
            end
        end

        % any strange shifts found due to noise?
        sm = median(abs(s),1)+std(abs(s));
        sm = repmat(sm,sz(N),1);
        [i,j]= find(abs(s)>sm);
        if i
            fprintf(' Refining the estimation.\n');
            sg=5;
            i = unique(i);
            fi = gaussf(fi_sum,sg);
            for ii=1:length(i)
                idx = i(ii)-1;
                tmp = gaussf(slice_ex(in,idx),sg);
                st = findshift(fi,tmp,'iter')';
                %found siginifcant difference?
                x=any(abs(st) < abs(s(idx+1,:))./5);
                if x
                    s(idx+1,:)   = st;
                    tmp = shift(slice_ex(in,idx),s(idx+1,:),'bspline',boundaryoption);
                    out = slice_in(out,tmp,idx);
                end
            end
        end
    end
    %color image - compute shift on sum of both, apply to each separately
    if ISC
        nc = size(inc,1);
        fprintf(' Shifting the %d color channels\n',nc);
        %nc=2;
        out = newimar(nc);
        for kk=1:nc
            out{kk} = inc{kk};
            for ii=0:sz(N)-1
                out{kk}(:,:,ii) = shift(slice_ex(inc{kk},ii),-s(ii+1,:),'bspline',boundaryoption);
            end
        end
        out = colorspace(out,colorspace(inc));
    end

catch
    err = lasterr;
end

error(err);

end

function out = crlb_function(a,sigN, sigS)
%Thesis Tuan Pham chapeter 2.1

%sigS=0;
g = gradient(a,sigS);
G = zeros(numtensorel(g),numtensorel(g));
for ii=1:numtensorel(g)
    for jj=1:ii
        G(ii,jj) = double(sum(g{ii}*g{jj}));
        %fprintf('%d %d\n',ii,jj)
    end
    for jj=ii-1:-1:1
        G(jj,ii)=G(ii,jj);
        %fprintf('--%d %d\n',ii,jj)
    end
end
G = G./sigN.^2;
G = inv(G);
for ii=1:numtensorel(g)
    out(ii) = sqrt(G(ii,ii));
end
end
