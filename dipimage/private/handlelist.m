%HANDLELIST   Returns a list of handles and titles for the selected type.
%   [HANDLES,TITLES] = HANDLELIST(SELECTION), with SELECTION a cell array
%   with strings or a single string, each string representing a substring
%   of the Tag element of the figure windows to return data about. TITLES
%   is a cell array with strings.
%
%   Only handles for figure windows created by DIPSHOW are returned.
%
%   Examples for SELECTION:
%      {'2D','3D'}
%      {'Color','Grey','Binary'}
%      {'1D_Color','2D_Binary','3D_Grey'}
%   SELECTION is not case-sensitive.

% (c)2017, Cris Luengo.
% (c)1999-2014, Delft University of Technology.
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

function [handles,titles] = handlelist(selection)

if nargin==0 || isempty(selection)
   selection = '';
elseif ~ischar(selection) && ~iscellstr(selection)
   error('Invalid SELECTION parameter.')
end

% Get list of defined handles
handles = get(0,'Children');
if ~isnumeric(handles)
    handles = [handles.Number];
end
if ~isempty(handles)
   tags = get(handles,'Tag');
   I = strncmp(tags,'DIP_Image',9);
   handles = handles(I);
   if ~isempty(handles)
      if ~isempty(selection)
         if length(I)>1, tags = tags(I); end % else Tags is a string, not a cell array!
         I = contain(lower(tags),lower(selection));
         handles = handles(I);
      end
      if nargout>1
         N = length(handles);
         if N>0
            titles = get(handles,'name');
            if N==1, titles = {titles}; end
            for ii=1:N
               if ~isempty(titles{ii})
                  titles{ii} = ['Figure No. ',num2str(handles(ii)),': ',titles{ii}];
               else
                  titles{ii} = ['Figure No. ',num2str(handles(ii))];
               end
            end
            [titles,I] = sort(titles);
            handles = handles(I);
         end
      end
   end
end
if nargout>1 && isempty(handles)
   titles = {};
end

% Returns true for each element in str1 that contains any of str2
function res = contain(str1,str2)
if iscell(str1)
   N = numel(str1);
   res = logical(zeros(N,1));
   for ii=1:N
      res(ii) = contains(str1{ii},str2);
   end
else
   res = contains(str1,str2);
end

% Returns true if str1 contains any of str2
function res = contains(str1,str2)
if iscell(str2)
   res = 0;
   for ii=1:numel(str2)
      res = res || ~isempty(findstr(str1,str2{ii}));
      if res, return, end
   end
else
   res = ~isempty(findstr(str1,str2));
end
