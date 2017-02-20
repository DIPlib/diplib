% Labeled image color map.
% The first item is [0,0,0] for background.
% Label 1 gets red, matches what happens with binary images.
% The first 6 labels are the purest colors, to maximize
% differences when there's only a few labels.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

function g = label_colormap(m)
if nargin<1
   m = 256; % length of color map
end
% Greens are closer together perceptually, so we reduce the
% number of green entries in this map.
col = [
    1.0000         0         0
         0    1.0000         0
         0         0    1.0000
    1.0000    1.0000         0
         0    1.0000    1.0000
    1.0000         0    1.0000
    1.0000    0.3333         0
    0.6667    1.0000         0
         0    0.6667    1.0000
    0.3333         0    1.0000
    1.0000         0    0.6667
    1.0000    0.6667         0
         0    1.0000    0.5000
         0    0.3333    1.0000
    0.6667         0    1.0000
    1.0000         0    0.3333
];
n = size(col,1);
g = [0,0,0;col(mod((2:m)-2,n)+1,:)];
