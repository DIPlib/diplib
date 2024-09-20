% This startup file will avoid the user's custom configurations, it adds
% DIPimage to the path, and it adds ../../examples to the image path.

% Obviously this line needs to be configured to the installation directory. How???
addpath('/Users/cris/src/diplib/target2/dip/share/DIPimage')

p = fileparts(mfilename('fullpath'));  % <root>/dipimage/tests
p = fileparts(fileparts(p));           % <root>
p = fullfile(p, 'examples');
dipsetpref('imagefilepath', p)
