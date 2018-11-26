%HIST2IMAGE   Backmaps a 2D histogram ROI to the images
%  The ROI can be specified by a coordinate array or interactively
%  selected from the histrogram
%
% SYNOPSIS:
%  [out1,out2,roi,histsel] = hist2image(in1,in2,coords,mask,bins1,bins2,range1,range2)
%
% PARAMETERS:
%  in1, in2: Scalar input images, 2D. Must have same sizes
%  coords:   Coordinate array representing a polygon within the histogram:
%            two columns, one row for each vertex. If empty, allows for an
%            interactive selection using DIPROI. If two rows given,
%            represents the top left and bottom right corners of a box.
%            The coordinates are in the two images' grey value space, not
%            histogram bins.
%  mask:     Region where histogram should be evaluted
%  bins1:    Number of bins for first image
%  bins2:    Number of bins for second image
%  range1:   [min,max] two values specifying the data range for IN1, or []
%            to use the minimum and maximum values in the image
%  range2:   Idem for IN2
%
% DEFAULTS:
%  coords: []
%  bins1: 100
%  bins2: 100
%  range1: []
%  range2: []
%
% OUTPUTS:
%  out1,out2: Output images, containing ROI overlaid on IN1 and IN2
%  roi:       Output binary image with the pixels selected by COORDS within
%             the histogram
%  histsel:   Binary image containing the histogram bins selected by COORDS
%
% SEE ALSO:
%  hist2d, mdhistogram, mdhistogrammap, diproi

% (c)2007, Max-Planck-Institute for Biophysical Chemistry (Göttingen), written by Bernd Rieger.
% (c)2018, Cris Luengo.
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

function [out1,out2,roi,histsel] = hist2image(in1,in2,coords,mask,bins1,bins2,range1,range2)
if ~isa(in1,'dip_image')
   in1 = dip_image(in1);
end
in1 = squeeze(in1);
if ~isa(in2,'dip_image')
   in2 = dip_image(in2);
end
in2 = squeeze(in2);
if ~isequal(size(in1),size(in2))
   error('Input images not of same size')
end
if ndims(in1)~=2
   error(['Interactive backmapping only implemented for 2D images. '...
           'Use MDHISTOGRAMMAP instead.']);
end

if nargin < 3
   coords = [];
elseif ~isempty(coords) && size(coords,2)~=2
   error('COORDS must have two columns');
end

if nargin < 4
   mask = [];
end
if nargin < 5
   bins1 = 100;
end
if nargin < 6
   bins2 = 100;
end
if nargin < 7
   range1 = [];
end
if nargin < 8
   range2 = [];
end

if isempty(coords)

   % We're depending on DIPHIST2D doing input argument checking here.

   fprintf('Select a region in the histogram image, double click to end\n');
   [hist,cont_handle,bins] = diphist2d(in1,in2,mask,range1,range2,bins1,bins2,'yes','z-axis');
   h = dipshow(hist,'log');
   [histsel,v] = diproi(h,'polygon');
   dipshow(h,overlay(stretch(hist),histsel)); % keep the histogram + selection
   h = findobj(cont_handle,'type','axes');
   h = line(h,bins{1}(v([1:end,1],1)+1),bins{2}(v([1:end,1],2)+1));
   set(h,'Color','red');
   set(h,'LineWidth',2);
   roi = mdhistogrammap(newtensorim(in1,in2),+histsel,bins) > 0;

else

   if isempty(range1) || isequal(range1, -1)
      range1 = getmaximumandminimum(in1,mask);
   elseif numel(range1)~=2 || ~isnumeric(range1)
      error('RANGE1 must be a 2-element numeric vector')
   end
   if isempty(range2) || isequal(range2, -1)
      range2 = getmaximumandminimum(in2,mask);
   elseif numel(range2)~=2 || ~isnumeric(range2)
      error('RANGE2 must be a 2-element numeric vector')
   end
   histsel = newim([bins1,bins2],'bin');
   if size(coords,1) == 2
      m1 = threshold(in1,'double',coords(:,1));
      m2 = threshold(in2,'double',coords(:,2));
      roi = m1 & m2;
      coords(:,1) = round((coords(:,1)-range1(1))*(bins1/diff(range1)));
      coords(:,2) = round((coords(:,2)-range2(1))*(bins2/diff(range2)));
      coords = round(coords);
      histsel(coords(1,1):coords(2,1),coords(1,2):coords(2,2)) = true;
   else
      coords(:,1) = round((coords(:,1)-range1(1))*(bins1/diff(range1)));
      coords(:,2) = round((coords(:,2)-range2(1))*(bins2/diff(range2)));
      histsel = drawpolygon(histsel,coords,1,'filled');
      bins = {linspace(range1(1),range1(2),bins1),linspace(range2(1),range2(2),bins2)};
      roi = mdhistogrammap(newtensorim(in1,in2),+histsel,bins) > 0;
   end

end
out1 = overlay(stretch(in1),roi);
out2 = overlay(stretch(in2),roi);
