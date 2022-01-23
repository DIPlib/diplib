% file I/O
disp('t002_fileio.m');

img = readim;

writeim(img,[tmpdir 'blablub_1'],'ics',0);
if any(readim([tmpdir 'blablub_1'])~=img), error('Read data not same as written data!'); end

writeim(img,[tmpdir 'blablub_2'],'ics',1);
if any(readim([tmpdir 'blablub_2'])~=img), error('Read data not same as written data!'); end

writeim(img,[tmpdir 'blablub_3'],'tif',0);
if any(readim([tmpdir 'blablub_3'])~=img), error('Read data not same as written data!'); end

writeim(img,[tmpdir 'blablub_4'],'tif',1);
if any(readim([tmpdir 'blablub_4'])~=img), error('Read data not same as written data!'); end

writeim(img,[tmpdir 'blablub_5'],'gif');
if any(readim([tmpdir 'blablub_5'])~=img), error('Read data not same as written data!'); end

img = readim('chromo3d.ics');
writeavi(img,[tmpdir 'blablub_6'],15,'None');
% once readavi works again, we should add that here!

img = readim('flamingo.tif');
writeim(img,[tmpdir 'blablub_7'],'tif',1);
if ~isequal(readim([tmpdir 'blablub_7']),img), error('Read data not same as written data!'); end

delete([tmpdir 'blablub_*']);

clear img
