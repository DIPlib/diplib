disp('t008_nufft.m');

a = readim;
q = im2array([0.3 0.2; 0.02 0.4]);

e = 0.0001;
data = nufft_type2(a,q);

if abs(double(data(0)) - complex(1.650457664159489e6,-1.775331749572309e6)) > e*abs(double(data(0))) || abs(double(data(1)) - complex(-6.379175036855147e5,5.348848653594245e4)) > e*abs(double(data(0)))
   error('nufft_type2 failed');
end

clear a q e data;
