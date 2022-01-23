homedir=initenv('HOME', '/home/diplib/');
dipdir=initenv('DIPDIR', [homedir, '/dip/dip/']);
testdir=initenv('TESTDIR', pwd);
logfile=initenv('LOGFILE', [homedir, '/testdip.log']);
scriptdir=initenv('SCRIPTDIR', [testdir, '/testscripts/']);
imgdir=initenv('IMGDIR', [homedir, '/images/']);
tmpdir=initenv('TMPDIR', [homedir, '/tmp/']);
os=initenv('OS', '');
donefile=initenv('DONEFILE', [tmpdir, 'matlab_done']);

fid=fopen(logfile,'w');

fprintf(fid, '%s - Testing %s with Matlab %s...\n', datestr(now), dipdir, version('-release'));

cd(scriptdir);
files=dir('t*.m');
[dummy,index]=sort({files.name}); % sort the name fields
files=files(index); % ... and re-index with the sort
no_files=length(files);
for i=1:no_files
  fname=files(i).name;
%%% the fileparts function outputs three parts (starting R2014b) instead of four...
%  [fpath, fbase, fext, fversn] = fileparts(fname);
  [fpath, fbase, fext] = fileparts(fname);
  fprintf(fid, '%s -   %s\n', datestr(now), fbase);
  try
    eval(fbase);
  catch
    [msg,msgid]=lasterr;
    fprintf(fid, '%s - Error in script %s: %s\n', datestr(now), fname, msg);
  end
end

fprintf(fid,'%s - done.\n', datestr(now));
fclose(fid)

% Duh...
if strcmp(os,'Cygwin')
  fid=fopen(donefile,'w');
  fclose(fid);
end

exit
