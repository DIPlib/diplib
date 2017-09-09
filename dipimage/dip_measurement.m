%dip_measurement   Contains measurement results
%   An object of this class is returned by the MEASURE function.
%
%   T = TABLE(M), and A = DOUBLE(M) will turn the object into a different
%   form, but will not preserve all metadata (such as units of
%   measurement).
%
%   A = M.FeatureName returns a double matrix with the values only for the
%   given feature. M2 = M(O) returns a new DIP_MEASUREMENT object with only
%   the objects specified in O. These two forms of indexing can be combined:
%   M(O).FeatureName.
%
%   It is not possible to change any of the data in the object, but it is
%   possible to change the feature names by adding a prefix. See the
%   example below.
%
%   Two DIP_MEASUREMENT objects can be concatenated into a single object,
%   either vertically (if they measure the same features on different
%   objects) or horizontally (if they measure different feature on the same
%   objects). For example, if we were to measure intensities in separate R,
%   G and B channels of an image, we could merge the results into a single
%   DIP_MEASUREMENT object:
%
%      m1 = measure(bin,grey{1},'mean');
%      m2 = measure(bin,grey{2},'mean');
%      m3 = measure(bin,grey{3},'mean');
%      m1.prefix = 'R_';
%      m2.prefix = 'G_';
%      m3.prefix = 'B_';
%      m = [m1,m2,m3]
%
%   Produces the output:
%           ID         R_Mean         G_Mean         B_Mean
%       ----------------------------------------------------
%            1     201.519380     100.031008      97.674419
%            2     153.091962      56.946185      52.890424
%            3      62.988679       6.535849       4.696855
%           ...

% DIPimage 3.0
%
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

classdef dip_measurement

   % ------- PROPERTIES -------

   properties (GetAccess=public,SetAccess=private,Hidden=true)
      %Data - Measurement data. Each row is one object. Each measurement
      %   feature spans several columns. Each column is a value.
      Data = []
      %Objects - List of object IDs, in same order as the rows of Data.
      %   Objects(ii) is the ID for the measurements in Data(ii,:).
      Objects = []
      %Features - Struct array with feature names.
      %   Features(ii).Name: string, feature name
      %   Features(ii).StartColumn: start column in Data.
      %   Features(ii).NumberValues: number of columns in Data.
      Features = struct('Name',{},'StartColumn',{},'NumberValues',{})
      %Values - Struct array with value (column) names and units.
      %   Values(ii).Name: string, value name
      %   Values(ii).Units: string, units for the value
      %   Values(ii) represents information about Data(:,ii).
      Values = struct('Name',{},'Units',{})
   end
   % These are all public read properties, but the subsref method prevents
   % M-code from actually reading them. However, MEX-files can use
   % mxGetProperty to read any property. (We actually use the undocumented
   % mxGetPropertyShared to prevent data copy, and to be able to modify
   % the Data property. Don't do this at home!)

   % ------- METHODS -------

   methods (Access=private)

      % ------- PRIVATE METHODS -------

      function I = FindFeature(obj,name)
         I = find(strcmpi(name,{obj.Features.Name}),1,'first');
         if isempty(I)
            error('Feature not available')
         end
      end

      function J = FindeatureColumns(obj,I)
         J = obj.Features(I).StartColumn + (0:obj.Features(I).NumberValues-1);
      end

   end

   methods

      % ------- CONSTRUCTOR -------

      function msr = dip_measurement(objects,features,values)
         %dip_measurement   Constructor
         %   Construct an object with:
         %      M = DIP_MEASUREMENT(OBJECTS,FEATURES,VALUES)
         %   OBJECTS, FEATURES and VALUES are as the private properties
         %   with the same names. A DATA array will be created with all
         %   zeros. Note that this is totally useless... This constructor
         %   is used by the MEASURE MEX-file, which can modify the values
         %   in the DATA aray.
         if nargin < 3, error('Too few arguments'); end
         if isempty(objects) || isempty(features)
            % Create an empty measurment
            return
         end
         if ~isnumeric(objects) || ~isvector(objects) || any(mod(objects,1))
            error('OBJECTS input incorrect');
         end
         if ~isstruct(features) || ~isfield(features,'Name') || ~isfield(features,'StartColumn') || ~isfield(features,'NumberValues')
            error('FEATURES input incorrect');
         end
         index = 1;
         for ii=1:numel(features)
            if ~ischar(features(ii).Name) || size(features(ii).Name,1)~=1
               error('FEATURES.Name must be a string');
            end
            if ~isnumeric(features(ii).StartColumn) || ~isscalar(features(ii).StartColumn)
               error('FEATURES.StartColumn must be a scalar integer');
            end
            if features(ii).StartColumn ~= index
               error('FEATURES.StartColumn doesn''t match expected index');
            end
            if ~isnumeric(features(ii).NumberValues) || ~isscalar(features(ii).NumberValues) || mod(features(ii).NumberValues,1) || features(ii).NumberValues<1
               error('FEATURES.NumberValues must be a positive scalar integer');
            end
            index = index + features(ii).NumberValues;
         end
         cols = index-1;
         if ~isstruct(values) || ~isfield(values,'Name') || ~isfield(values,'Units')
            error('VALUES input incorrect');
         end
         if cols~=length(values)
            error('Number of VALUES differs from described by FEATURES');
         end
         for ii=1:numel(values)
            if ~ischar(values(ii).Name) || ~(isempty(values(ii).Name) || size(values(ii).Name,1)==1)
               error('VALUES.Name must be a string');
            end
            if ~ischar(values(ii).Units) || ~(isempty(values(ii).Units) || size(values(ii).Units,1)==1)
               error('VALUES.Units must be a string');
            end
         end
         msr.Objects = objects(:)';
         msr.Features = features;
         msr.Values = values;
         msr.Data = zeros(length(objects),cols);
      end

      % ------- GET PROPERTIES -------

      function varargout = size(obj,dim)
         %SIZE   Size of the data in the measurement object.
         %   SIZE(M) returns a 1x2 array with the number of objects and
         %   the number of values (columns).
         %
         %   SIZE(M,DIM), for DIM equal to 1 or 2, returns only the given
         %   element of the size array.
         %
         %   [O,V] = SIZE(M) returns the number of objects in O and
         %   the number of values (columns) in V.
         %
         %   SIZE(M,'featureID') returns the number of measurement values
         %   per object for feature 'featureID'.
         if nargout > 1
            if nargin ~= 1, error('Unknown command option'); end
            varargout = cell(1,2);
            [varargout{:}] = size(obj.Data);
         else
            if nargin > 1
               if ischar(dim)
                  I = FindFeature(obj,dim);
                  varargout{1} = obj.Features(I).NumberValues;
               else
                  varargout{1} = size(obj.Data,dim);
               end
            else
               varargout{1} = size(obj.Data);
            end
         end
      end

      function n = length(obj)
         %LENGTH   Returns the numer of objects in the measurement object.
         %   LENGTH(M) returns the number of label IDs, and is equivalent
         %   to SIZE(M,1) or LENGTH(M.ID).
         n = numel(obj.Objects);
      end

      function n = numel(obj)
         %NUMEL   Returns the number of values in the object.
         n = numel(obj.Data);
      end

      function res = isempty(obj)
         %ISEMPTY   Returns true if there are no values in the image.
         res = isempty(obj.Data);
      end

      function out = fieldnames(obj)
         %FIELDNAMES   Get measurement names.
         %   NAMES = FIELDNAMES(M) returns the names of the measurements
         %   in the dip_measurement object M, as a cell array of strings.
         %
         %   These names can be used by evaluating M.NAME.
         out = {obj.Features.Name};
      end

      function f = isfield(obj,id)
         %ISFIELD   True if measurement is in dip_measurement object
         %   ISFIELD(M,'featureID') returns true if 'featureID' is the
         %   name of a measurement in dip_measurement object M.
         if nargin<2
            error('Feature name required')
         end
         f = strcmpi('id',id) || any(strcmpi(id,{obj.Features.Name}));
      end

      function obj = rmfield(obj,id)
         %RMFIELD   Remove a feature from a dip_measurement object
         %   M = RMFIELD(M,'featureID') removes 'featureID' from the
         %   dip_measurement object M.
         if nargin<2
            error('Feature name required')
         end
         I = FindFeature(obj,id);
         J = FindeatureColumns(obj,I);
         obj.Data(:,J) = [];
         obj.Features(I) = [];
         obj.Values(J) = [];
      end

      function disp(obj)
         % DISP   Display the DIP_MEASUREMENT object.
         if isempty(obj)
            disp('Empty measurement structure');
            return
         end
         Nobj = length(obj.Objects);
         Nmsr = length(obj.Features);
         % Print measurement labels and creating format string for printing the table
         line1 = '     ID';
         line2 = '       ';
         line3 = '       ';
         fmtstr = ' %6d';
         linelength = 7;
         index = 1;
         for jj=1:Nmsr
            ldat = obj.Features(jj).NumberValues;
            for kk=1:ldat
               line1 = [line1,sprintf(' %14s',obj.Features(jj).Name)];
               line2 = [line2,sprintf(' %14s',obj.Values(index).Name)];
               if ~isempty(obj.Values(index).Units)
                  line3 = [line3,sprintf(' %14s',['(',obj.Values(index).Units,')'])];
               else
                  line3 = [line3,sprintf(' %14s','')];
               end
               index = index+1;
            end
            fmtstr = [fmtstr,repmat(' %14.6f',1,ldat)];
            linelength = linelength + ldat*15;
         end
         fprintf('%s\n',line1);
         if any(line2~=' ')
            fprintf('%s\n',line2);
         end
         if any(line3~=' ')
            fprintf('%s\n',line3);
         end
         fprintf(' %s\n',repmat('-',1,linelength));
         fmtstr = [fmtstr,'\n'];
         % Print measurements for each object
         for ii=1:Nobj
            fprintf(fmtstr,obj.Objects(ii),obj.Data(ii,:));
         end
      end

      % ------- EXTRACT DATA -------

      function out = double(obj)
         %DOUBLE   Convert dip_measurement object to double matrix.
         %   A = DOUBLE(M) converts the measurement data M to a double
         %   precision matrix. To extract a specific measurement, use the
         %   syntax A = M.NAME.
         out = obj.Data;
      end

      function out = table(obj)
         %TABLE   Convert dip_measurement object to table object.
         %   T = TABLE(M) converts the measurement data M to a table.
         %   Features become columns, feature names are the column headers.
         %   Value headers and units are lost.
         variableNames = {'ID',obj.Features.Name};
         data = cell(1,1+numel(obj.Features));
         data{1} = obj.Objects(:);
         for ii = 1:numel(obj.Features)
            J = obj.Features(ii).StartColumn+(0:obj.Features(ii).NumberValues-1);
            data{ii+1} = obj.Data(:,J);
         end
         out = table(data{:},'VariableNames',variableNames);
      end
      
      function [m,id] = min(obj,name)
         %MIN   Finds the maximum value for each measurement.
         %   MIN(M) returns a row vector with the minimum value for each
         %   measurement, and is equivalent to MIN(DOUBLE(M)).
         %
         %   [Y,I] = MIN(M) also returns the object ID of the minimum values.
         %   This is not the same as [Y,I] = MIN(DOUBLE(M)), because object
         %   IDs do not have to be consecutive or sorted.
         %
         %   MIN(M,'featureID') computes the minimum values only for the
         %   given feature.
         if nargin==1
            values = obj.Data;
         else
            I = FindFeature(obj,name);
            J = FindeatureColumns(obj,I);
            values = obj.Data(:,J);
         end
         if nargout>1
            [m,I] = min(values);
            id = obj.Objects(I);
         else
            m = min(values);
         end
      end
      
      function [m,id] = max(obj,name)
         %MAX   Finds the maximum value for each measurement.
         %   MAX(M) returns a row vector with the maximum value for each
         %   measurement, and is equivalent to MAX(DOUBLE(M)).
         %
         %   [Y,I] = MAX(M) also returns the object ID of the maximum values.
         %   This is not the same as [Y,I] = MAX(DOUBLE(M)), because object
         %   IDs do not have to be consecutive or sorted.
         %
         %   MAX(M,'featureID') computes the maximum values only for the
         %   given feature.
         if nargin==1
            values = obj.Data;
         else
            I = FindFeature(obj,name);
            J = FindeatureColumns(obj,I);
            values = obj.Data(:,J);
         end
         if nargout>1
            [m,I] = max(values);
            id = obj.Objects(I);
         else
            m = max(values);
         end
      end
      

      % ------- INDEXING -------

      function n = numArgumentsFromSubscript(~,~,~)
         %numArgumentsFromSubscript   Overload for internal use by MATLAB
         n = 1; % Indexing always returns a single object.
      end

      function out = subsref(in,s)
         %SUBSREF   Overload for B=A(I).NAME.
         %   Valid syntaxes:
         %   - M = M(I) : returns a new dip_measurement with selected
         %     objects. I can be either one or more object IDs or a logical
         %     array over the rows of the table.
         %   - A = M.NAME : returns a double array with the values for
         %     feature NAME.
         %   - U = M.units.NAME : returns the units for the feature NAME.
         %     If the feature has multiple values, it returns a cell array.
         %   - O = M.ID : returns a list of object IDs.
         %   - A combination of the above, and additional indexing into
         %     the returned array/object.
         N = length(s);
         %if N==1 && strcmp(s(1).type,'.')
         %   out = builtin('subsref',in,s);
         %   return
         %end
         ii = 1;
         out = in;
         if strcmp(s(ii).type,'()')
            % Select objects
            I = s(ii).subs;
            if length(I) ~= 1
               try
                  I = cat(2,I{:});
               catch
                  error('Illegal indexing')
               end
            else
               I = I{1};
            end
            if ~(islogical(I) || isnumeric(I)) || ~isvector(I) || isempty(I)
               error('Illegal indexing')
            end
            I = I(:); % Make sure it is a column vector
            if islogical(I)
               if length(I) ~= length(out.Objects)
                  error('Incorrect indexing array')
               end
               out.Objects = out.Objects(I);
               out.Data = out.Data(I,:);
            else
               [~,I] = ismember(I,out.Objects);
               if any(I==0)
                  error('Object ID not available')
               end
               out.Objects = out.Objects(I);
               out.Data = out.Data(I,:);
            end
            ii = ii+1;
         end
         if ii>N, return, end
         if strcmp(s(ii).type,'.') && strcmpi(s(ii).subs,'units')
            returnunits = true;
            ii = ii+1;
            if ii>N
               error('Feature ID needed to return units strings')
            end
         else
            returnunits = false;
         end
         if strcmp(s(ii).type,'.')
            % Select measurement
            name = s(ii).subs;
            if strcmpi(name,'ID')
               if returnunits
                  error('Feature ID needed to return units strings')
               end
               out = out.Objects;
            else
               I = FindFeature(out,name);
               J = FindeatureColumns(out,I);
               if returnunits
                  if length(J)>1
                     out = {out.Values(J).Units};
                  else
                     out = out.Values(J).Units;
                  end
               else
                  out = out.Data(:,J);
               end
            end
            ii = ii+1;
            if ii>N, return, end
            % Some other referencing being done on the result. Let MATLAB handle it...
            out = subsref(out,s(ii:N));
         elseif ii<=N
            % There are no more indexing operations posible...
            error('Illegal indexing operation');
         end
      end

      function ii = end(a,~,n)
         %END   Overload for using END in indexing.
         if n~=1
            error('Illegal indexing');
         end
         ii = a.Objects(end);
      end

      function a = subsasgn(a,s,b)
         %SUBSASGN   Overloaded operator for A.NAME=B.
         %   Valid syntaxes:
         %   - M.ID = O : changes the object IDs. O must have LENGTH(M)
         %     elements.
         %   - M.prefix = STR : adds a prefix to the feature names.
         %
         %   Modifications that are allowed are meant to make it possible
         %   to concatenate dip_measurement objects. See DIP_MEASUREMENT/HORZCAT
         %   and DIP_MEASUREMENT/VERTCAT.
         if length(s)==1 && strcmp(s.type,'.')
            name = s.subs;
            if strcmpi(name,'ID')
               % Change object IDs for the measured objects.
               % All must change at the same time:
               if ~isnumeric(b) || numel(b)~=length(a.Objects) || any(mod(b,1)) || any(b<1)
                  error('Invalid object ID array')
               end
               a.Objects = b(:)';
               return
            elseif strcmpi(name,'prefix')
               % Prefix names with b.
               if ~ischar(b) || isempty(b) || size(b,1)~=1
                  error('Invalid prefix.')
               end
               for ii=1:length(a.Features)
                  a.Features(ii).Name = [b,a.Features(ii).Name];
               end
               return
            end
         end
         error('Do not mess with the dip_measurement object!')
      end

      % ------- CONCATENATION -------

      function out = horzcat(out,varargin)
         %HORZCAT   Overloaded operator for [a b] or [a,b].
         %   [M!,M2] joins two measurement objects with the same object IDs,
         %   but different measurements. If some measurements are repeated,
         %   or if the object IDs don't match, an error is generated.
         %
         %   See DIP_MEASUREMENT/SUBSASGN for information on how to modify
         %   the objects so they can be concatenated.
         if ~isa(out,'dip_measurement')
            error('Only dip_measurement objects can be concatenated together');
         end
         % We sort the indices of all the measurement structures,
         % so that they will overlap.
         [out.Objects,I] = sort(out.Objects);
         out.Data = out.Data(I,:);
         for ii=1:length(varargin)
            if ~isa(varargin{ii},'dip_measurement')
               error('Only dip_measurement objects can be concatenated together');
            end
            % The names may not be repeated
            if ~isempty(intersect(lower({out.Features.Name}),lower({varargin{ii}.Features.Name})))
               error('Measurement objects contain repeated features.')
            end
            % The object IDs should be identical
            [varargin{ii}.Objects,I] = sort(varargin{ii}.Objects);
            if ~isequal(out.Objects,varargin{ii}.Objects)
               error('Measurement objects do not contain same object IDs.')
            end
            % Keep data in same order as object IDs
            varargin{ii}.Data = varargin{ii}.Data(I,:);
            % Join
            out.Data = [out.Data,varargin{ii}.Data];
            out.Features = [out.Features,varargin{ii}.Features];
            out.Values = [out.Values,varargin{ii}.Values];
         end
      end

      function out = vertcat(out,varargin)
         %VERTCAT   Overloaded operator for [a;b].
         %   [M1;M2] joins two measurement objects with the same measurements,
         %   on different object IDs. If some IDs are repeated, or if the
         %   measurements don't match, an error is generated.
         %
         %   See DIP_MEASUREMENT/SUBSASGN for information on how to modify
         %   the objects so they can be concatenated.
         if ~isa(out,'dip_measurement')
            error('Only dip_measurement objects can be concatenated together');
         end
         features = lower({out.Features.Name});
         for ii=1:length(varargin)
            if ~isa(varargin{ii},'dip_measurement')
               error('Only dip_measurement objects can be concatenated together');
            end
            % The object IDs may not be repeated
            if ~isempty(intersect(out.Objects,varargin{ii}.Objects))
               error('Measurement objects contain repeated object IDs.')
            end
            % The features should be identical
            [~,I] = ismember(features,lower({varargin{ii}.Features.Name}));
            if length(varargin{ii}.Features) ~= length(features) || any(I==0)
               error('Measurement objects do not contain same measurements.')
            end
            % Sort features to match out
            features = varargin{ii}.Features(I);
            J = [];
            for jj=1:length(features)
               J = [J,features(jj).StartColumn+(0:features(jj).NumberValues-1)];
            end
            % Join
            out.Data = [out.Data;varargin{ii}.Data(:,J)];
            out.Objects = [out.Objects,varargin{ii}.Objects];
         end
      end

   end
end
