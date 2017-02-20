%IS_VALID_VARNAME(NAME)
%    Check variable name for validity.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

function v = is_valid_varname(name)

start = '_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
other = [start,'0123456789'];

v = 0;
if ~ischar(name), return, end
if length(name)<1, return, end
if ~any(start==name(1)), return, end
if ~all(ismember(name(2:end),other)), return, end
v = 1;
