disp('t001_init.m');

addpath([dipdir, '/common/dipimage']);
dip_initialise;
dipsetpref('ImageFilePath', imgdir);
close all hidden;
t0 = clock;
