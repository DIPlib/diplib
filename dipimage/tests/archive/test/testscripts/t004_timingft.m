%timing ft
disp('t004_timingft.m');

a=readim('cermet');
tic;for ii=1:100;ft(a);end;toc;

b=double(a);
tic;for ii=1:100;fft(b);end;toc;

c=readim('chromo3d.ics');
tic;for ii=1:20;ft(c);end;toc;
