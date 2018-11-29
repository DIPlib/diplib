%COORDINATES   Creates an image with general coordinates
%
% SYNOPSIS:
%  image_out = coordinates(sizes,value,origin,options)
%
% PARAMETERS:
%  sizes:   The sizes of the output image.
%  value:   Which value to write to the output image. One of:
%    - n:             An integer indicating the index of the cartesian coordinate
%                     (1 is x coordinate, 2 is y coordinate, etc.).
%    - 'cartesian':   The output is a tensor image with all cartesian coordinate
%                     components.
%    - 'radius':      The distance to the origin (R of the polar/spherical
%                     coordinate system), generalizes to any number of dimensions.
%    - 'phi':         The angle to the x-axis in the x-y plane (2D and 3D only).
%    - 'theta':       The angle to the z-axis (3D only).
%    - 'spherical':   The output is a tensor image with all polar or spherical
%                     coordinate components (2D or 3D only).
%  origin:  The location of the origin. One of:
%    - 'left':        The pixel to the left of the true center.
%    - 'right':       The pixel to the right of the true center (default).
%    - 'true':        The true center, between pixels if required.
%    - 'corner':      The pixel on the upper left corner (indexed at (0,0)).
%    - 'frequency':   Uses frequency domain coordinates, in the range [-0.5,0.5),
%                     corresponds to coordinate system used by FT.
%           Note that the first three are identical if the size is odd.
%  options: A cell array containing zero or more of the following strings:
%    - 'radial':      When 'frequency' is selected as the origin, causes it to
%                     use radial frequencies instead, making the range [-pi,pi).
%    - 'math':        Let the Y coordinate increase upwards instead of downwards.
%
% DEFAULTS:
%  sizes = [256,256]
%  value = 'cartesian'
%  origin = 'right'
%  options = {}
%
% NOTES:
%  An image can be passed instead of the SIZES parameter, causing the output
%  to use its sizes, and copy its pixel size as well.
%
%  The output image is always of type SINGLE (SFLOAT)
%
%  It is possible to set ORIGIN to 'radfreq', which combines 'frequency' with
%  'radial'.
%
%  Prepending an 'm' to any option for ORIGIN is equivalent to setting the
%  'math' option. That is, 'mleft' is equivalent to 'left' and 'math'.
%
% SEE ALSO:
%  ramp, xx, yy, zz, rr, phiphi
%
% DIPlib:
%  This function calls the DIPlib functions dip::FillCoordinates, dip::FillRamp,
%  dip::FillRadiusCoordinate, dip::FillPhiCoordinate, or dip::FillThetaCoordinate

% (c)2017-2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function out = coordinates(varargin)
out = dip_generation('coordinates',varargin{:});
