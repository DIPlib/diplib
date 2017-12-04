%ANS = QUESTIONDIALOG(MESSAGE)
%    Presents a question to the user in a dialog box. Returns 'yes' or
%    'no'.

% (c)2017, Cris Luengo.
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

function out = questiondialog(message)
if nargin~=1 || ~(ischar(message) || iscellstr(message))
   error('Illegal syntax.')
end

bgcolor = get(0,'defaultuicontrolbackgroundcolor');

% Create icon
icon = uint8([...
0 0 0 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 0 0 0
0 3 3 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 3 3 0
0 3 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 3 0
3 1 1 1 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 1 1 1 3
3 1 1 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 2 2 2 2 1 1 3
3 1 1 2 2 2 2 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 2 2 2 2 1 1 3
3 1 1 2 2 2 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 1 1 1 1 1 2 2 2 1 1 3
3 1 1 2 2 2 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 2 2 2 1 1 3
3 1 1 2 2 2 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 2 2 2 1 1 3
3 1 1 2 2 2 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 2 2 2 1 1 3
3 1 1 2 2 2 2 1 1 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 3
3 1 1 1 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 1 1 1 3
0 3 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 3 0
0 3 3 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 3 3 0
0 0 0 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 0 0 0 ...
]);
edgecolor = (bgcolor+[0,0,1])/2; % average of background and blue.
colmap = [...
bgcolor
1,1,1
0,0,1
edgecolor ...
];

string1 = 'Yes';
string2 = 'No';

% Create figure window
fig = figure('Visible','off',...
             'Color',bgcolor,...
             'Colormap',colmap,...
             'KeyPressFcn',['DIP_Image_DLG_ch = double(get(gcbf,''CurrentCharacter''));',...
                            'if ~isempty(DIP_Image_DLG_ch),switch DIP_Image_DLG_ch,',...
                            'case 13,set(gcbf,''userdata'',''',string1,''');',...
                            'case 27,set(gcbf,''userdata'',''',string2,''');',...
                            'end,end;clear DIP_Image_DLG_ch'],...
             'MenuBar','none',...
             'Name','DIPimage',...
             'NumberTitle','off',...
             'Resize','off',...
             'Units','pixels',...
             'WindowStyle','modal');

% Create controls
txth = uicontrol(fig,'Style','text','String',message,'units','pixels',...
                     'HorizontalAlignment','left');
okbtnh = uicontrol(fig,'Style','pushbutton','String',string1,...
                     'Callback',['set(gcbf,''userdata'',''',string1,''');'],'units','pixels');
cnbtnh = uicontrol(fig,'Style','pushbutton','String',string2,...
                     'Callback',['set(gcbf,''userdata'',''',string2,''');'],'units','pixels');
axh = axes('parent',fig,'units','pixels','visible','off');
image('parent',axh,'cdata',icon);

% Set sizes and positions       % (Copied from DIPimage)
sizes.leftmargin = 20;          % Space between left edge and controls
sizes.internalmargin = 10;      % Horizontal space between controls
sizes.rightmargin = 20;         % Space between right edge and controls
sizes.topmargin = 10;           % Space between top edge and controls
sizes.bottommargin = 10;        % Space between bottom edge and controls
sizes.vslack = 4;               % Extra space allotted to controls (around text)
sizes.hslack = 10;              % Extra space allotted to controls (around text)
units = get(0,'units');
set(0,'units','pixels');
screensz = get(0,'screensize');
set(0,'units',units);
iconsz = size(icon); iconsz = iconsz([2,1]);
txtsz = get(txth,'Extent');
truetxtheight = txtsz(4);
txtsz(3:4) = max(txtsz(3:4),iconsz(1).*[8,2]);
set(cnbtnh,'String','Select All');
btnsz = get(cnbtnh,'Extent')+[0,0,sizes.hslack,sizes.vslack];
set(cnbtnh,'String',string2);
figpos = [0,0,sizes.leftmargin+iconsz(1)+sizes.leftmargin+txtsz(3)+sizes.rightmargin,...
          sizes.bottommargin+btnsz(4)+sizes.bottommargin+txtsz(4)+sizes.topmargin];
figpos(1:2) = round(screensz(3:4)-figpos(3:4))/2;
set(fig,'position',figpos);
axpos = [sizes.leftmargin,...
         sizes.bottommargin+btnsz(4)+sizes.bottommargin+(txtsz(4)-iconsz(2))/2,...
         iconsz];
set(axh,'xlim',[0.5,iconsz(1)+0.5],'ylim',[0.5,iconsz(2)+0.5],'ydir','reverse','position',axpos);
btnsz(1:2) = [figpos(3)-sizes.rightmargin-btnsz(3),sizes.bottommargin];
set(okbtnh,'position',btnsz);
btnsz(1) = figpos(3)-sizes.rightmargin-2*btnsz(3)-sizes.internalmargin;
set(cnbtnh,'position',btnsz);
txtsz = [sizes.leftmargin+iconsz(1)+sizes.leftmargin,...
         sizes.bottommargin+btnsz(4)+sizes.bottommargin+ceil((txtsz(4)-truetxtheight)/2),...
         txtsz(3),truetxtheight];
set(txth,'position',txtsz);
set(fig,'Visible','on')

% Wait for the user to press a button...
waitfor(fig,'userdata')
out = get(fig,'userdata');
delete(fig)
