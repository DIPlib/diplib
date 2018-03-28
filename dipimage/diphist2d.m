%DIPHIST2D   Generates a 2D histogram
%
% SYNOPSIS:
%  [out,h] = diphist2d(in1,in2,mask,range1,range2,bin1,bin2,...
%         contourhist,logdisp,contourfill,contourlab,Nl)
%
% PARAMETERS:
%  in1:     first input image
%  in2:     second input image
%  mask:    region where histogram should be evaluted
%  range1:  [min max] two values specifying the data range
%  range2:  [min max] two values specifying the data range
%  bin1:    number of bins for first image
%  bin2:    number of bins for second image
%  contourhist: should a contour plot be made ('yes','no')
%  logdisp:     log plot for axes ('none','x-axis','y-axis','z-axis','xy-axes','xyz-axes')
%  contourfill: fill the contour plot? ('yes','no')
%  contourlab : label the contour lines?  ('yes','no')
%  Nl:      number of contour lines, -1 for automatic
%
% DEFAULTS:
%  range1: [] (the minimum and maximum value of image 1 is taken)
%  range2: [] (the minimum and maximum value of image 2 is taken)
%  bins1: 100 (change to smaller (+-50) values if result is too sparse)
%  bins2: 100
%  contourhist: 'No'
%  logdisp: 'none'
%  contourfill: 'No'
%  contourlab: 'No'
%  Nl: -1
%
% OUTPUT:
%  out:  overlay image of the backmap with the original
%  h:    handle to the contour plot
%
% HINTS: 
%  Use Mappings -> Custom -> 'jet' for a nice colormap.
%  Use axis xy to flip the axis.
%
% SEE ALSO:
%  hist2image, diphist, mdhistogram

% (c)2004, Max-Planck-Institute for Biophysical Chemistry (G?ttingen), written by Bernd Rieger.
% (c)2017, Cris Luengo.
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

function [out,hf] = diphist2d(ch1,ch2,mask,range1,range2,bins1,bins2,...
   contourhist,logdisp,contourfill,contourlab,Nl)

if numtensorel(ch1)~=1 || numtensorel(ch2)~=1
   error('Input images must be scalar')
end
if nargin<12
   Nl = -1;
   if nargin<11
      contourlab = false;
      if nargin<10
         contourfill = false;
         if nargin<9
            logdisp = 'none';
            if nargin<8
               contourhist = false;
               if nargin<7
                  bins2 = 100;
                  if nargin<6
                     bins1 = 100;
                     if nargin<5
                        range2 = [];
                        if nargin<4
                           range1 = [];
                           if nargin<3
                              mask = [];
                           end
                        end
                     end
                  end
               end
            end
         end
      end
   end
end
if ~isempty(mask) && ~islogical(mask)
   error('MASK must be binary')
end
if isempty(range1) || range1(1) == -1
   range1 = getmaximumandminimum(ch1,mask);
elseif numel(range1)~=2 || ~isnumeric(range1)
   error('RANGE1 must be a 2-element numeric vector')
end
if isempty(range2) || range2(1) == -1
   range2 = getmaximumandminimum(ch2,mask);
elseif numel(range2)~=2 || ~isnumeric(range2)
   error('RANGE2 must be a 2-element numeric vector')
end
if ischar(contourhist)
   contourhist = strcmpi(contourhist,'Yes') || strcmpi(contourhist,'On');
end
if ischar(contourfill)
   contourfill = strcmpi(contourfill,'Yes') || strcmpi(contourfill,'On');
end
if ischar(contourlab)
   contourlab = strcmpi(contourlab,'Yes') || strcmpi(contourlab,'On');
end

[out,bins] = mdhistogram(newtensorim(ch1,ch2),mask,...
      {{'lower',range1(1),'upper',range1(2),'bins',bins1,'lower_abs','upper_abs'},...
       {'lower',range2(1),'upper',range2(2),'bins',bins2,'lower_abs','upper_abs'}});
out = dip_image(out);

hf = [];

if contourhist
   hf = figure;
   x = bins{1};
   y = bins{2};
   if strcmp(logdisp,'xyz-axes') || strcmp(logdisp,'z-axis')
      o2 = log10(double(out));
   else
      o2 = double(out);
   end
   if contourfill
      if Nl < 0
         [c,h] = contourf(x,y,o2);
      else
         [c,h] = contourf(x,y,o2,Nl);
      end
   else
      if Nl < 0
         [c,h] = contour(x,y,o2);
      else
         [c,h] = contour(x,y,o2,Nl);
      end
   end
   if contourlab
      clabel(c,h);
   end
   axis ij
   ha = gca;
   switch logdisp
      case {'xy-axes','xyz-axes'}
         set(ha,'XScale','log');
         set(ha,'YScale','log');
      case 'x-axis'
         set(ha,'XScale','log');
      case 'y-axis'
         set(ha,'YScale','log');
   end

   hc = colorbar;

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
   if strcmp(logdisp,'xyz-axes') || strcmp(logdisp,'z-axis')
      he = round(10.^he);
      s = cell(length(he),1);
      for ii=1:length(he)
         s{ii} = num2str(he(ii));
      end
      set(hc,'YTickLabel',s);
   end

end
