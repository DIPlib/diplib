%MEASURE   Do measurements on objects in an image
%
% SYNOPSIS:
%  msr = measure(object,grey,measurmentIDs,objectIDs,connectivity)
%
% PARAMETERS:
%  object: binary or labeled image holding the objects.
%  grey: (original) grey value image of object_in. It is needed for
%        several types of measurements. Otherwise you can use [].
%  featureIDs: measurements to be performed, either a single string
%              or a cell array with strings (e.g.: {'Size','Perimeter'} ).
%  objectIDs: labels of objects to be measured. Use [] to measure all
%             objects
%  connectivity: defines which pixels are considered neighbours. Match
%                this value to the one used to label object_in.
%
% DEFAULTS:
%  grey_in = [];
%  featureIDs = 'size'
%  objectIDs = []
%  connectivity = 0 (equivalent to NDIMS(OBJECT_IN))
%
% RETURNS:
%  msr: a dip_measurement object containing the results.
%
% EXAMPLE:
%  img = readim('cermet')
%  msr = measure(img<100, img, ({'size', 'perimeter','mean'}))
%
% NOTE:
%  MEASURE HELP prints a list of all measurement features available in this
%  function.
%
%  F = MEASURE('features') returns a struct array with feature names and
%  descriptions, this is used by the DIPimage GUI.
%
% NOTE:
%  Several measures use the boundary chain code (i.e. 'Feret', 'Perimeter',
%  'BendingEnergy', 'P2A' and 'PodczeckShapes'). These measures will fail
%  if the object is not compact (one chain code must represent the whole
%  object). If the object is not compact under the connectivity chosen, only
%  one part of the object will be measured.
%
% NOTE:
%  See the user guide for the definition of connectivity in DIPimage.
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/dip-MeasurementTool.html#dip-MeasurementTool-Measure-Image-CL-Image-CL-StringArray--UnsignedArray-CL-dip-uint--C">dip::MeasurementTool::Measure</a>.

% (c)2017, Cris Luengo.
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
