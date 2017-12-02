%DISPLAYLABELNUMBERS   Overlay the label numbers on a figure window
%
% SYNOPSIS:
%  displaylabelnumbers(figure_handle,label_image);
%
% PARAMETERS:
%  figure_handle: handle of the figure window to draw in
%  label_image:   image containing the labels
%
% EXAMPLE
%  a = label(readim('cermet')<128);
%  h = dipshow(a,'labels');
%  displaylabelnumbers(h,a);

% UNDOCUMENTED BEHAVIOR:
%  If you request an output, it'll give a cell array with the commands
%  and options that create the output figure.
%
%  If FIGURE_HANDLE is 0, do not draw anything. Use in combination with
%  an output argument.
%
%  Additional parameters after the default two are key/value pairs, with
%  keys 'measure' and 'grey'. The first one indicates which feature to
%  measure, the second one supplies a grey-value image that might be
%  necessary to measure the feature. The feature values will be drawn on
%  the image, instead of label IDs. For example:
%     a = readim('cermet');
%     b = label(a<128);
%     h = dipshow(b,'labels');
%     displaylabelnumbers(h,b,'measure','minval','grey',a);

% (c)2017, Cris Luengo.
% (C)2006, Michael van Ginkel.
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

function out = displaylabelnumbers(fh,imlabels,varargin)

if ~isinteger(imlabels)
   error('Only label images supported');
end

img = [];
msrwhat = {};
nvargs = nargin-2;
ai = 1;
while ai<=nvargs
   switch varargin{ai}
      case 'grey'
         if (ai+1)>nvargs
            error('No argument to option "grey"');
         end
         img = varargin{ai+1};
         ai = ai+1;
      case 'measure'
         if (ai+1)>nvargs
            error('No argument to option "measure"');
         end
         msrwhat = varargin{ai+1};
         ai = ai+1;
      otherwise
         error(['Unknown option: ',varargin{ai}]);
   end
   ai = ai+1;
end

if isempty(msrwhat)
   msr = measure(imlabels,[],{'center'});
   msrwhat = 'id';
else
   msr = measure(imlabels,img,{'center',msrwhat});
end

tlabels = strtrim(cellstr(num2str(msr.(msrwhat))));
if nargout>0
   if isempty(msr)
      out={};
   else
      out={{'text',msr.center(:,1),msr.center(:,2),tlabels},...
           {'option','FontUnits','pixels'},...
           {'option','FontSize',8},...
           {'option','horizontalalignment','center'},...
           {'option','verticalalignment','middle'},...
           {'option','backgroundcolor','white'},...
           {'option','edgecolor','black'},...
           {'option','margin',1}};
   end
end
if ~isequal(fh,0)
   if ~isfigh(fh)
      error('First argument must be a figure handle')
   end
   if ~isempty(msr)
      th = text(msr.center(:,1),msr.center(:,2),tlabels,'parent',get(fh,'CurrentAxes'));
      set(th,'FontUnits','pixels','FontSize',8,'horizontalalignment','center','verticalalignment','middle');
      set(th,'backgroundcolor','white','edgecolor','black','margin',1)
   end
end
