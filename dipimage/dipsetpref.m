%DIPSETPREF   Sets a DIPimage preference
%   DIPSETPREF('name',value) sets the value of the named DIPimage
%   preference.
%
%   DIPSETPREF('name1',value1,'name2',value2,'name3',value3,...)
%   sets multiple values at once.
%
%   Notice that these values are not stored from one session to the
%   next. You can add a DIPSETPREF command to your STARTUP file
%   to customize your copy of DIPimage.
%
%   The property names and values are described in the user manual.
%
%   See also: DIPGETPREF

% Undocumented syntax: DIPSETPREF -UNLOAD
%   Unlocks and clears the MEX-file private/dippreferences.

% (c)1999-2013, Delft University of Technology
% (c)2017, Cris Luengo

function menu_out = dipsetpref(varargin)

if nargin==1 && ischar(varargin{1}) && strcmpi(varargin{1},'-unload')
   dippreferences('unload');
   clear private/dippreferences
return
end
if nargin<2 | mod(nargin,2)
   error('Need name-value pairs.')
end
for ii=1:2:nargin
   dippreferences('set',varargin{ii},varargin{ii+1});
end
