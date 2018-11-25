%DIPHIST2D   Generates a 2D histogram
%
% SYNOPSIS:
%  [hist,handle,bins] = diphist2d(in1,in2,mask,range1,range2,bin1,bin2,...
%                       contourhist,logdisp,contourfill,contourlab,Nl)
%
% PARAMETERS:
%  in1, in2:    Input images, must be of equal sizes
%  mask:        region where histogram should be evaluted
%  range1:      [min max] two values specifying the data range for IN1, or []
%  range2:      [min max] two values specifying the data range for IN2, or []
%  bin1:        number of bins for IN1
%  bin2:        number of bins for IN2
%  contourhist: should a contour plot be made ('yes','no')
%  logdisp:     log plot for axes ('none','x-axis','y-axis','z-axis','xy-axes','xyz-axes')
%  contourfill: fill the contour plot? ('yes','no')
%  contourlab : label the contour lines?  ('yes','no')
%  Nl:          number of contour lines, -1 for automatic
%
% DEFAULTS:
%  range1: []
%  range2: []
%  bins1: 100 (change to smaller (+-50) values if result is too sparse)
%  bins2: 100
%  contourhist: 'No'
%  logdisp: 'none'
%  contourfill: 'No'
%  contourlab: 'No'
%  Nl: -1
%
% OUTPUTS:
%  hist:   image containing histogram bins
%  handle: handle to the figure window with the contour plot
%  bins:   cell array with bin centers for the two dimensions.
%
% HINTS: 
%  Use Mappings -> Custom -> 'parula' for a nice colormap.
%  Use axis xy to flip the axis.
%
% SEE ALSO:
%  hist2image, diphist, mdhistogram

% (c)2004, Max-Planck-Institute for Biophysical Chemistry (Göttingen), written by Bernd Rieger.
% (c)2017-2018, Cris Luengo.
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

function [hist,hf,bins] = diphist2d(ch1,ch2,mask,range1,range2,bins1,bins2,...
   contourhist,logdisp,contourfill,contourlab,Nl)

if numtensorel(ch1)~=1 || numtensorel(ch2)~=1
   error('Input images must be scalar')
end
if nargin<12
   Nl = -1;
end
if nargin<11
   contourlab = false;
end
if nargin<10
   contourfill = false;
end
if nargin<9
   logdisp = 'none';
end
if nargin<8
   contourhist = false;
end
if nargin<7
   bins2 = 100;
end
if nargin<6
   bins1 = 100;
end
if nargin<5
   range2 = [];
end
if nargin<4
   range1 = [];
end
if nargin<3
   mask = [];
end
if ~isempty(mask) && ~islogical(mask)
   error('MASK must be binary')
end
if isempty(range1) || isequal(range1, -1)
   range1 = getmaximumandminimum(ch1,mask);
elseif numel(range1)~=2 || ~isnumeric(range1)
   error('RANGE1 must be a 2-element numeric vector')
end
if isempty(range2) || isequal(range2, -1)
   range2 = getmaximumandminimum(ch2,mask);
elseif numel(range2)~=2 || ~isnumeric(range2)
   error('RANGE2 must be a 2-element numeric vector')
end
if ischar(contourhist)
   contourhist = strcmpi(contourhist,'Yes') || strcmpi(contourhist,'On');
end
xlog = false;
ylog = false;
zlog = false;
if ischar(logdisp)
   if ~strcmp(logdisp,'none')
      s = regexp(logdisp,'^([x-z]+)-ax[ei]s$','tokens');
      if isempty(s)
         error('LOGDISP string not recognied');
      end
      s = s{1}{1};
      xlog = any(s=='x');
      ylog = any(s=='y');
      zlog = any(s=='z');
   end
else
   error('LOGDISP must be a string');
end
if ischar(contourfill)
   contourfill = strcmpi(contourfill,'Yes') || strcmpi(contourfill,'On');
end
if ischar(contourlab)
   contourlab = strcmpi(contourlab,'Yes') || strcmpi(contourlab,'On');
end

[hist,bins] = mdhistogram(newtensorim(ch1,ch2),mask,...
      {{'lower',range1(1),'upper',range1(2),'bins',bins1,'lower_abs','upper_abs'},...
       {'lower',range2(1),'upper',range2(2),'bins',bins2,'lower_abs','upper_abs'}});

hf = [];

if contourhist
   hf = figure;
   ha = axes(hf);
   x = bins{1};
   y = bins{2};
   dhist = double(hist);
   if zlog
      dhist = log10(dhist);
   end
   if contourfill
      if Nl < 0
         [c,h] = contourf(ha,x,y,dhist);
      else
         [c,h] = contourf(ha,x,y,dhist,Nl);
      end
   else
      if Nl < 0
         [c,h] = contour(ha,x,y,dhist);
      else
         [c,h] = contour(ha,x,y,dhist,Nl);
      end
   end
   set(ha,'YDir','reverse')
   if contourlab
      clabel(c,h);
   end
   if xlog
      set(ha,'XScale','log');
   end
   if ylog
      set(ha,'YScale','log');
   end
   xlabel(ha,'Image 1')
   ylabel(ha,'Image 2')

   hc = colorbar('peer',ha);

   n = length(h);
   % Get height lines and mark them on the colorbar with ticks
   if isprop(h(1),'LevelList')
      % change R13->14 in [c,h]=contour what the handles to contours is, arg
      he = h.LevelList;
   else
      he = zeros(1,n);
      for ii=1:n
         he(ii) = get(h(ii),'UserData');
      end
      he = unique(he);
   end
   set(hc,'YTick',he);
   if zlog
      he = round(10.^he);
      s = cell(length(he),1);
      for ii=1:length(he)
         s{ii} = num2str(he(ii));
      end
      set(hc,'YTickLabel',s);
   end

end
