% Testing dip_subpixellocation, dip_subpixelmaxima, dip_subpixelminima
disp('t007_subpixmax.m');

N = 5;

p = [10.33,20.52,15.89,6.13,23.001];
q = round(p);
a = newim(ones(1,N)*30);
a = gaussianblob(a,p,1,1);
m = a>1e-7;
v = (2*pi)^(-N/2);
t = max(a);

[x,y] = dip_subpixelmaxima(a,m,'linear');
if any(abs(x-p)>0.2) | y>v | y<t
   error('dip_subpixelmaxima/linear failed');
end
[x,y] = dip_subpixelmaxima(a,m,'parabolic');
if any(abs(x-p)>0.05) | y>v | y<t
   error('dip_subpixelmaxima/parabolic failed');
end
[x,y] = dip_subpixelmaxima(a,m,'gaussian');
if any(abs(x-p)>1e-6) | y>v | y<t
   error('dip_subpixelmaxima/gaussian failed');
end
[x,y] = dip_subpixelmaxima(a,m,'bspline');
if any(abs(x-p)>0.05) | y>v | y<t
   error('dip_subpixelmaxima/bspline failed');
end

[x,y] = dip_subpixelminima(-a,m,'linear');
if any(abs(x-p)>0.2) | y<-v | y>-t
   error('dip_subpixelmaxima/linear failed');
end
[x,y] = dip_subpixelminima(-a,m,'parabolic');
if any(abs(x-p)>0.05) | y<-v | y>-t
   error('dip_subpixelmaxima/parabolic failed');
end
[x,y] = dip_subpixelminima(-a,m,'gaussian');
if any(abs(x-p)>1e-6) | y<-v | y>-t
   error('dip_subpixelmaxima/gaussian failed');
end
[x,y] = dip_subpixelminima(-a,m,'bspline');
if any(abs(x-p)>0.05) | y<-v | y>-t
   error('dip_subpixelmaxima/bspline failed');
end

N = 3;
p = p(1:N);
q = round(p);
a = newim(ones(1,N)*30);
a = gaussianblob(a,p,1,1);
v = (2*pi)^(-N/2);
t = max(a);

[x,y] = dip_subpixelmaxima(a,[],'parabolic_nonseparable');
if any(abs(x-p)>0.05) | y<t
   error('dip_subpixelmaxima/parabolic_nonseparable/3D failed');
end
[x,y] = dip_subpixelmaxima(a,[],'gaussian_nonseparable');
if any(abs(x-p)>1e-6) | y<t
   error('dip_subpixelmaxima/gaussian_nonseparable/3D failed');
end

N = 2;
p = p(1:N);
q = round(p);
a = newim(ones(1,N)*30);
a = gaussianblob(a,p,1,1);
v = (2*pi)^(-N/2);
t = max(a);

[x,y] = dip_subpixelmaxima(a,[],'parabolic_nonseparable');
if any(abs(x-p)>0.05) | y<t
   error('dip_subpixelmaxima/parabolic_nonseparable/2D failed');
end
[x,y] = dip_subpixelmaxima(a,[],'gaussian_nonseparable');
if any(abs(x-p)>1e-6) | y<t
   error('dip_subpixelmaxima/gaussian_nonseparable/2D failed');
end

clear a m p q v t N x y
