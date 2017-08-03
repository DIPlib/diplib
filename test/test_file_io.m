a = readics('trui.ics');
a.PixelSize(1)=struct('units','mm','magnitude',0.1);

writetiff(a,'trui2')
writetiff(a,'trui3','deflate')
writetiff(a,'trui4','LZW')
writetiff(a,'trui5','PackBits')
writetiff(a,'trui6','JPEG',100)
writetiff(a,'trui7','JPEG',80)
writetiff(a,'trui8','JPEG',50)
writetiff(a,'trui9','none')

a2 = readtiff('trui2');
if ~all(a2==a) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui2.tif'), end
a2 = readtiff('trui3');
if ~all(a2==a) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui3.tif'), end
a2 = readtiff('trui4');
if ~all(a2==a) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui4.tif'), end
a2 = readtiff('trui5');
if ~all(a2==a) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui5.tif'), end
a2 = readtiff('trui6');
if errormeasure(a,a2,[],'MAE')>0.1 || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui6.tif'), end
a2 = readtiff('trui7');
if errormeasure(a,a2,[],'MAE')>1.7 || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui7.tif'), end
a2 = readtiff('trui8');
if errormeasure(a,a2,[],'MAE')>2.5 || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui8.tif'), end
a2 = readtiff('trui9');
if ~all(a2==a) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui9.tif'), end

writeics(a,'trui2')
writeics(a,'trui3',{},0,'v1')
writeics(a,'trui4',{},0,{'v1','uncompressed'})
writeics(a,'trui5',{},0,{'v2','uncompressed'})

a2 = readics('trui2');
if ~all(a2==a) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui2.ics'), end
a2 = readics('trui3');
if ~all(a2==a) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui3.ics'), end
a2 = readics('trui4');
if ~all(a2==a) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui4.ics'), end
a2 = readics('trui5');
if ~all(a2==a) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! trui5.ics'), end

a = hessian(a);

writetiff(a,'hessian1')
a2 = readtiff('hessian1');
if ~all(tensortospatial(a2)==tensortospatial(a)) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! hessian1.tif'), end

writeics(a,'hessian1')
a2 = readics('hessian1');
if ~all(logical(all(a2==a))) || ~isequal(a2.PixelSize,a.PixelSize), disp('Error! hessian1.ics'), end
