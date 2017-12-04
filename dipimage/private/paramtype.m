%PARAMTYPE   Parameter-type-specific code for DIPIMAGE.
%   The first argument is a command, and the second one is a PARAM
%   structure containing:
%      PARAM.DESCRIPTION  String containing a description to show the user.
%      PARAM.TYPE         String containing the data type.
%      PARAM.DIM_CHECK    Dimensionality check.
%      PARAM.RANGE_CHECK  Range check.
%      PARAM.DEFAULT      Default value used if none specified.
%   All these entries can be interpreted differently for the various
%   parameter types.
%
%   MSG = PARAMTYPE('definition_test',PARAM,PARAMS)
%      Test the PARAM structure entry, returns an error message or ''.
%
%   HANDLES = PARAMTYPE('control_create',PARAM,FIG,INDEX)
%      Create a UI control in DIPimage GUI, return the list of handles.
%      The first handle is the edit control, and if there's another
%      handle it'll be for a push button to the right.
%      FIG is the figure handle. INDEX is the control's index number.
%
%   [VALUE,STRING] = PARAMTYPE('control_value',PARAM,CONTROL)
%      Evaluate the input in a UI control in DIPimage GUI.
%      CONTROL is the handle of the control containing the values.
%
%   The actual code is in sub-functions called 'PARAMTYPE_xxx.M'. To
%   add a parameter type, just copy one of these over, change the name
%   and modify the code to do what you need it to do. It will be
%   automatically registered. However, for the new type to work with
%   the MATLAB compiler, you need to add it to the #function pragma
%   in this file.
%   If the new type becomes part of DIPimage, please edit the User Guide.

% (c)2017, Cris Luengo.
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

function varargout = paramtype(command,param,varargin)

%#function paramtype_anytypearray
%#function paramtype_array
%#function paramtype_boolean
%#function paramtype_cellarray
%#function paramtype_dataset
%#function paramtype_handle
%#function paramtype_image
%#function paramtype_indir
%#function paramtype_infile
%#function paramtype_measureid
%#function paramtype_measurement
%#function paramtype_option
%#function paramtype_optionarray
%#function paramtype_outfile
%#function paramtype_string

% Compile registry, only the first time this function is called.
persistent registry;
if ~iscell(registry)
   % The first time this function is called, registry==[].
   p = fileparts(mfilename('fullpath'));
   p = dir(fullfile(p,'paramtype_*.m'));
   registry = {p.name};
   for ii=1:length(registry)
      registry{ii} = registry{ii}(11:end-2); % remove the 'paramtype_' and '.m' parts.
   end
end

% Call the relevant sub-function
if ~any(strcmp(param.type,registry))
   error('Unknown parameter type');
end
varargout = cell(1,nargout);
[varargout{:}] = feval(['paramtype_',param.type],command,param,varargin{:});
