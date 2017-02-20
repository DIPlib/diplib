%DIPFIG_TITLEBAR(FIG,UDATA)
%    Updates the title bar of FIG.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

function dipfig_titlebar(fig,udata)
tit = udata.imname;
if isfield(udata,'curslice')
   tit = [tit,' (',num2str(udata.curslice),')'];
end
if isfield(udata,'curtime')
   tit = udata.imname;
   tit = [tit,' (' num2str(udata.curslice) ',' num2str(udata.curtime),')'];
end
if isfield(udata,'zoom') & ~isempty(udata.zoom) & udata.zoom~=0
   tit = [tit,' (',num2str(udata.zoom*100),'%)'];
end
set(fig,'Name',tit);
