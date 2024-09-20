function dip_image_squeeze()
% Tests the dip_image.squeeze() method, both in the "cheap" and the default modes

a = newim(10,20,30);

dipsetpref('cheapsqueeze',1)
b = squeeze(a(0,:,:));
assert_equal(imsize(b),[30,20])
assert_equal(b.pixelsize,[1,1])
b = squeeze(a(:,0,:));
assert_equal(imsize(b),[30,10])
assert_equal(b.pixelsize,[1,1])
b = squeeze(a(:,:,0));
assert_equal(imsize(b),[10,20])
assert_equal(b.pixelsize,[1,1])

dipsetpref('cheapsqueeze',0)
b = squeeze(a(0,:,:));
assert_equal(imsize(b),[20,30])
assert_equal(b.pixelsize,[1,1])
b = squeeze(a(:,0,:));
assert_equal(imsize(b),[10,30])
assert_equal(b.pixelsize,[1,1])
b = squeeze(a(:,:,0));
assert_equal(imsize(b),[10,20])
assert_equal(b.pixelsize,[1,1])


a.PixelSize = 1;

dipsetpref('cheapsqueeze',1)
b = squeeze(a(0,:,:));
assert_equal(imsize(b),[30,20])
assert_equal(b.pixelsize,[1,1])
b = squeeze(a(:,0,:));
assert_equal(imsize(b),[30,10])
assert_equal(b.pixelsize,[1,1])
b = squeeze(a(:,:,0));
assert_equal(imsize(b),[10,20])
assert_equal(b.pixelsize,[1,1])

dipsetpref('cheapsqueeze',0)
b = squeeze(a(0,:,:));
assert_equal(imsize(b),[20,30])
assert_equal(b.pixelsize,[1,1])
b = squeeze(a(:,0,:));
assert_equal(imsize(b),[10,30])
assert_equal(b.pixelsize,[1,1])
b = squeeze(a(:,:,0));
assert_equal(imsize(b),[10,20])
assert_equal(b.pixelsize,[1,1])


a.PixelSize = [1,2,3];

dipsetpref('cheapsqueeze',1)
b = squeeze(a(0,:,:));
assert_equal(imsize(b),[30,20])
assert_equal(b.pixelsize,[3,2])
b = squeeze(a(:,0,:));
assert_equal(imsize(b),[30,10])
assert_equal(b.pixelsize,[3,1])
b = squeeze(a(:,:,0));
assert_equal(imsize(b),[10,20])
assert_equal(b.pixelsize,[1,2])

dipsetpref('cheapsqueeze',0)
b = squeeze(a(0,:,:));
assert_equal(imsize(b),[20,30])
assert_equal(b.pixelsize,[2,3])
b = squeeze(a(:,0,:));
assert_equal(imsize(b),[10,30])
assert_equal(b.pixelsize,[1,3])
b = squeeze(a(:,:,0));
assert_equal(imsize(b),[10,20])
assert_equal(b.pixelsize,[1,2])
