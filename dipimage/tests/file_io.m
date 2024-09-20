function file_io()
% Tests file I/O

dipsetpref('FileWriteWarning', 'no')

img = readim;

writeim(img, [tempdir 'blablub_1'], 'icsv1');
assert_equal(readim([tempdir 'blablub_1']), img);

writeim(img, [tempdir 'blablub_2'], 'icsv2');
assert_equal(readim([tempdir 'blablub_2']), img);

writeim(img, [tempdir 'blablub_3'], 'tif');
assert_equal(readim([tempdir 'blablub_3']), img);

writeim(img, [tempdir 'blablub_4'], 'npy');
assert_equal(readim([tempdir 'blablub_4']), img);

writeim(img, [tempdir 'blablub_5.png'], 'png');
assert_equal(readim([tempdir 'blablub_5.png']).Array, img.Array);
% NOTE: comparing the .Array only because .PixelSize is [0×0 struct] in one and [0×1 struct] in the other, for some reason.


img = readim('chromo3d.ics');

writeim(img, [tempdir 'blablub_6'], 'ics');
assert_equal(readim([tempdir 'blablub_6']), img);

img = readim('DIP.tif');

writeim(img, [tempdir 'blablub_7'], 'icsv2');
assert_equal(readim([tempdir 'blablub_7']), img);

writeim(img, [tempdir 'blablub_8'], 'tif');
assert_equal(readim([tempdir 'blablub_8']), img);

writeim(img, [tempdir 'blablub_9.png'], 'png');
assert_equal(readim([tempdir 'blablub_9.png']).Array, img.Array);
% NOTE: comparing the .Array only because .PixelSize is [0×0 struct] in one and [0×1 struct] in the other, for some reason.

delete([tempdir 'blablub_*']);
