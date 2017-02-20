%DIPFIG_SETPOINTER(FIG,POINTER)
%    Sets the custom pointers used throughout.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

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
