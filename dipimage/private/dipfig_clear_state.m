%DIPFIG_CLEAR_STATE(FIG,UDATA)
%    Disables all callbacks for the image in FIG, and updates UDATA.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

function dipfig_clear_state(fig,udata)
udata.state = 'none';

if isfield(udata,'orientationim')
   udata = rmfield(udata,'orientationim');
end
set(fig,'pointer','arrow',...
        'WindowButtonDownFcn','',...
        'WindowButtonUpFcn','',...
        'WindowButtonMotionFcn','',...
        'ButtonDownFcn','',...
        'UserData',[]);   % Solve MATLAB bug!
set(fig,'UserData',udata);
dipfig_set_action_check(fig,udata.state);
