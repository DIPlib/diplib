%DIPMAXASPECT   Undocumented till usefulness proved

% (C) 2006- Michael van Ginkel
% 16 Aug 2006 - Created. I use this in preparation for "print" statements...
%               (Hence to attempt to re-position the window; centering it
%                on the screen is probably the only useful option if that
%                would be desired). Bits and bops nicked from dipgetimage/
%                diptruesize

function out=dipmaxaspect( fh )

if nargin == 1
   if ischar(fh) & strcmp(fh,'DIP_GetParamList')
      out = struct('menu','none');
      return
   end
end

tag = get(fh,'Tag');
if ~strncmp(tag,'DIP_Image',9)
   error('dipmaxaspect only works on images displayed using DIPSHOW.')
end

rootUnits = get(0,'Units');
set(0,'Units','pixels');
scrsz = get(0,'ScreenSize');
scrsz = scrsz(3:4);
set(0,'Units',rootUnits);

udata = get(fh,'UserData');
pos=get(fh,'Position');
if length(udata.imsize)==1
   pos(3)=scrsz(1);
   set(fh,'Position',pos);
   diptruesize(fh,'off');
else
   pos(3)=scrsz(1);
   pos(4)=floor(scrsz(1)*udata.imsize(2)/udata.imsize(1));
   if pos(4)>=scrsz(2)
      pos(4)=scrsz(2);
      pos(3)=floor(scrsz(2)*udata.imsize(1)/udata.imsize(2));
   end
   set(fh,'Position',pos);
   diptruesize(fh,'off');
end
