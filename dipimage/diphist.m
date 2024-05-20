%DIPHIST   Displays a histogram
%   DIPHIST(B) displays the histogram for image B, using the
%   default interval, in a new figure window.
%
%   DIPHIST(B,[MIN,MAX]) displays the histogram of the pixels
%   in the range [MIN MAX].
%
%   DIPHIST(B,'all') displays the histogram using the complete
%   interval present in the image. DIPHIST(B,[]) is the same
%   thing.
%
%   DIPHIST(B,...,n) displays the histogram using n bins.
%
%   DIPHIST(B,...,'optimal') displays the histogram with bin
%   sizes determined according to the Freedman-Diaconis rule.
%   Do not combine with the 'n' input above.
%
%   DIPHIST(B,...,mode) displays the histogram using a different
%   plotting method instead of the default stem plot. MODE can be
%   one of: 'stem' (the default), 'bar', 'line'.
%
%   [HISTOGRAM,BINS] = DIPHIST(B,...) returns the histogram
%   values and bin centers. The histogram is not displayed if
%   an output value is given, unless one of the plot mode
%   strings is also given.
%
%   See also: MDHISTOGRAM, DIPHIST2D

% (c)2017-2024, Cris Luengo.
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

function [outhist,outbins] = diphist(in,varargin)

% Default values
n = 256;
mini = 0;
maxi = 255;
stretch = 0;
has_range = false;
mode = '';

% Parse input
if nargin > 1
   ii = 1;
   while ii<nargin
      arg = varargin{ii};
      if ischar(arg)
         switch lower(arg)
            case 'all'
               stretch = 1;
            case 'optimal'
               n = 'optimal';
            case {'stem','bar','line'}
               mode = lower(arg);
            otherwise
               error('Unknown command option.')
         end
      elseif isnumeric(arg)
         arg = double(arg);
         if length(arg)==2
            mini = arg(1);
            maxi = arg(2);
            has_range = true;
         elseif length(arg)==1
            n = arg;
         elseif isempty(arg)
            stretch = 1;
         else
            error('Error in arguments.')
         end
      else
         error('Error in arguments.')
      end
      ii = ii+1;
   end
end

% Convert input image
if ~isa(in,'dip_image')
   in = dip_image(in);
end
if numtensorel(in) ~= 1
   error('Scalar image expected.')
end
if islogical(in)
   % Binary images always generate 2-bin histograms!
   in = +in;
   mini = 0; maxi = 1; n = 2;
   stretch = 0;
end

% Create configuration parameter
if stretch
   mini = 0;
   maxi = 100;
   has_range = true;
end
if ischar(n)  % n == 'optimal'
   conf = {n};
else
   conf = {'bins',n};
   has_range = true;
end
if has_range
   conf = [conf,{'lower',mini,'upper',maxi}];
   if ~stretch
      conf = [conf,{'lower_abs','upper_abs'}];
   end
end

% Do histogram.
[histogram,bins] = mdhistogram(in,[],conf);
histogram = dip_array(histogram);

if nargout>0
   % Give output data.
   outhist = double(histogram);
   outbins = bins;
end

if nargout==0 || ~isempty(mode)
   % Draw it.
   figure;
   switch mode
      case {'','stem'}
         stem(bins,histogram,'b.-');
      case 'bar'
         bar(bins,histogram,1,'b');
      case 'line'
         plot(bins,histogram,'b-');
   end
   dx = (bins(2)-bins(1))/2;
   set(gca,'xlim',[bins(1)-dx,bins(end)+dx]);
end
