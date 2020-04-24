a = newim(10,20,30);

dipsetpref('cheapsqueeze',1)
b = squeeze(a(0,:,:));
assert(isequal(imsize(b),[30,20]))
assert(isequal(b.pixelsize,[1,1]))
b = squeeze(a(:,0,:));
assert(isequal(imsize(b),[30,10]))
assert(isequal(b.pixelsize,[1,1]))
b = squeeze(a(:,:,0));
assert(isequal(imsize(b),[10,20]))
assert(isequal(b.pixelsize,[1,1]))

dipsetpref('cheapsqueeze',0)
b = squeeze(a(0,:,:));
assert(isequal(imsize(b),[20,30]))
assert(isequal(b.pixelsize,[1,1]))
b = squeeze(a(:,0,:));
assert(isequal(imsize(b),[10,30]))
assert(isequal(b.pixelsize,[1,1]))
b = squeeze(a(:,:,0));
assert(isequal(imsize(b),[10,20]))
assert(isequal(b.pixelsize,[1,1]))


a.PixelSize = 1;

dipsetpref('cheapsqueeze',1)
b = squeeze(a(0,:,:));
assert(isequal(imsize(b),[30,20]))
assert(isequal(b.pixelsize,[1,1]))
b = squeeze(a(:,0,:));
assert(isequal(imsize(b),[30,10]))
assert(isequal(b.pixelsize,[1,1]))
b = squeeze(a(:,:,0));
assert(isequal(imsize(b),[10,20]))
assert(isequal(b.pixelsize,[1,1]))

dipsetpref('cheapsqueeze',0)
b = squeeze(a(0,:,:));
assert(isequal(imsize(b),[20,30]))
assert(isequal(b.pixelsize,[1,1]))
b = squeeze(a(:,0,:));
assert(isequal(imsize(b),[10,30]))
assert(isequal(b.pixelsize,[1,1]))
b = squeeze(a(:,:,0));
assert(isequal(imsize(b),[10,20]))
assert(isequal(b.pixelsize,[1,1]))


a.PixelSize = [1,2,3];

dipsetpref('cheapsqueeze',1)
b = squeeze(a(0,:,:));
assert(isequal(imsize(b),[30,20]))
assert(isequal(b.pixelsize,[3,2]))
b = squeeze(a(:,0,:));
assert(isequal(imsize(b),[30,10]))
assert(isequal(b.pixelsize,[3,1]))
b = squeeze(a(:,:,0));
assert(isequal(imsize(b),[10,20]))
assert(isequal(b.pixelsize,[1,2]))

dipsetpref('cheapsqueeze',0)
b = squeeze(a(0,:,:));
assert(isequal(imsize(b),[20,30]))
assert(isequal(b.pixelsize,[2,3]))
b = squeeze(a(:,0,:));
assert(isequal(imsize(b),[10,30]))
assert(isequal(b.pixelsize,[1,3]))
b = squeeze(a(:,:,0));
assert(isequal(imsize(b),[10,20]))
assert(isequal(b.pixelsize,[1,2]))
