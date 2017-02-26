%DIPFIG_SETPOINTER(FIG,POINTER)
%    Sets the custom pointers used throughout.

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

function dipfig_setpointer(fig,pointer)

switch pointer
   case 'cross'
      pointershape = [...
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2 NaN   2 NaN NaN NaN NaN NaN NaN NaN
           2   2   2   2   2   2 NaN NaN NaN   2   2   2   2   2   2 NaN
           1   1   1   1   1 NaN NaN NaN NaN NaN   1   1   1   1   1 NaN
           2   2   2   2   2   2 NaN NaN NaN   2   2   2   2   2   2 NaN
         NaN NaN NaN NaN NaN NaN   2 NaN   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN ...
      ];
      set(fig,'PointerShapeCData',pointershape,'PointerShapeHotSpot',[8,8],'pointer','custom');
   case 'loupe'
      pointershape = [...
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   2   2 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN   2   2   1   1   1   2   2 NaN NaN NaN NaN NaN
         NaN NaN NaN   2   1   1   2   2   2   1   1   2 NaN NaN NaN NaN
         NaN NaN NaN   2   1   2 NaN NaN NaN   2   1   2 NaN NaN NaN NaN
         NaN NaN   2   1   2 NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN
         NaN NaN   2   1   2 NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN
         NaN NaN   2   1   2 NaN NaN NaN NaN NaN   2   1   2 NaN NaN NaN
         NaN NaN NaN   2   1   2 NaN NaN NaN   2   1   2 NaN NaN NaN NaN
         NaN NaN NaN   2   1   1   2   2   2   1   1   2 NaN NaN NaN NaN
         NaN NaN NaN NaN   2   2   1   1   1   2   2   1   2 NaN NaN NaN
         NaN NaN NaN NaN NaN NaN   2   2   2 NaN NaN   2   1   2 NaN NaN
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN   2   1   2 NaN
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN   2   1   2
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN   2   2 ...
      ];
      set(fig,'PointerShapeCData',pointershape,'PointerShapeHotSpot',[8,8],'pointer','custom');
   case 'hand_open'
      pointershape = [...
         NaN NaN NaN NaN NaN NaN NaN   1   1 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN   1   1 NaN   1   2   2   1   1   1 NaN NaN NaN NaN
         NaN NaN   1   2   2   1   1   2   2   1   2   2   1 NaN NaN NaN
         NaN NaN   1   2   2   1   1   2   2   1   2   2   1 NaN   1 NaN
         NaN NaN NaN   1   2   2   1   2   2   1   2   2   1   1   2   1
         NaN NaN NaN   1   2   2   1   2   2   1   2   2   1   2   2   1
         NaN   1   1 NaN   1   2   2   2   2   2   2   2   1   2   2   1
           1   2   2   1   1   2   2   2   2   2   2   2   2   2   2   1
           1   2   2   2   1   2   2   2   2   2   2   2   2   2   1 NaN
         NaN   1   2   2   2   2   2   2   2   2   2   2   2   2   1 NaN
         NaN NaN   1   2   2   2   2   2   2   2   2   2   2   2   1 NaN
         NaN NaN   1   2   2   2   2   2   2   2   2   2   2   1 NaN NaN
         NaN NaN NaN   1   2   2   2   2   2   2   2   2   2   1 NaN NaN
         NaN NaN NaN NaN   1   2   2   2   2   2   2   2   1 NaN NaN NaN
         NaN NaN NaN NaN NaN   1   2   2   2   2   2   2   1 NaN NaN NaN
         NaN NaN NaN NaN NaN   1   2   2   2   2   2   2   1 NaN NaN NaN ...
      ];
      set(fig,'PointerShapeCData',pointershape,'PointerShapeHotSpot',[8,8],'pointer','custom');
   case 'hand_closed'
      pointershape = [...
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN   1   1 NaN   1   1 NaN   1   1 NaN NaN NaN
         NaN NaN NaN NaN   1   2   2   1   2   2   1   2   2   1   1 NaN
         NaN NaN NaN NaN   1   2   2   2   2   2   2   2   2   1   2   1
         NaN NaN NaN NaN NaN   1   2   2   2   2   2   2   2   2   2   1
         NaN NaN NaN NaN   1   1   2   2   2   2   2   2   2   2   2   1
         NaN NaN NaN   1   2   2   2   2   2   2   2   2   2   2   2   1
         NaN NaN NaN   1   2   2   2   2   2   2   2   2   2   2   1 NaN
         NaN NaN NaN NaN   1   2   2   2   2   2   2   2   2   2   1 NaN
         NaN NaN NaN NaN NaN   1   2   2   2   2   2   2   2   1 NaN NaN
         NaN NaN NaN NaN NaN NaN   1   2   2   2   2   2   2   1 NaN NaN
         NaN NaN NaN NaN NaN NaN   1   2   2   2   2   2   2   1 NaN NaN ...
      ];
      set(fig,'PointerShapeCData',pointershape,'PointerShapeHotSpot',[8,8],'pointer','custom');
   case 'hand_finger'
      pointershape = [...
         NaN NaN NaN NaN NaN NaN   1   1 NaN NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN   1   2   2   1 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN   1   2   2   1 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN   1   2   2   1 NaN NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN   1   2   2   1   1 NaN NaN NaN NaN NaN NaN
         NaN NaN NaN NaN NaN   1   2   2   1   2   1   1 NaN NaN NaN NaN
         NaN   1   1 NaN NaN   1   2   2   1   2   1   2   1   1 NaN NaN
           1   2   2   1 NaN   1   2   2   1   2   1   2   1   2   1 NaN
         NaN   1   2   2   1   1   2   2   2   2   2   2   1   2   1 NaN
         NaN NaN   1   2   2   1   2   2   2   2   2   2   2   2   1 NaN
         NaN NaN   1   2   2   1   2   2   2   2   2   2   2   2   1 NaN
         NaN NaN NaN   1   2   2   2   2   2   2   2   2   2   2   1 NaN
         NaN NaN NaN   1   2   2   2   2   2   2   2   2   2   1 NaN NaN
         NaN NaN NaN NaN   1   2   2   2   2   2   2   2   2   1 NaN NaN
         NaN NaN NaN NaN NaN   1   2   2   2   2   2   2   1 NaN NaN NaN
         NaN NaN NaN NaN NaN   1   2   2   2   2   2   2   1 NaN NaN NaN ...
      ];
      set(fig,'PointerShapeCData',pointershape,'PointerShapeHotSpot',[2,7],'pointer','custom');
   otherwise
      error('Unknown pointer shape.')
end
