disp('t010_arithmetic.m');

clear i
a=newim(10,10)+1;
b=newim([10,10])+2;
c=a+b;
c=a.*b;
c=a.^b;
c=a.^2.1;
c=a*2.3;
c=a*i;


b=newim([10,10],'dfloat')+2;
c=a+b;
c=a.*b;
c=a.^b;
if ~isequal(datatype(c),'dfloat'), error('Wrong data type!'); end

a+1;
a-1.1;
a*10;
a/10.5;

b = mat2im(rand(10,10));
c = dip_image(b*10,'int8');

a = newim([10,10],'complex');
a(1,1) = 4+4*i;
b = rand(10,10)+i*rand(10,10);
c = a+b;
c = a*b;

c = a+1;
c = a*1;
c = a*i;

dipsetpref('KeepDataType','yes')
a = newim([64 64],'uint8')+10;
b = a*10;
if ~strcmp(datatype(b),'uint8'), error('Wrong data type!'); end
b = a*-1;
if any(b~=0),error('b should be all 0');end

b = a-50;
if ~strcmp(datatype(b),'uint8'), error('Wrong data type!'); end
b = a+50;
if ~strcmp(datatype(b),'uint8'), error('Wrong data type!'); end
b = a/5.3;.3;.3;
if ~strcmp(datatype(b),'uint8'), error('Wrong data type!'); end


dipsetpref('KeepDataType','no')

