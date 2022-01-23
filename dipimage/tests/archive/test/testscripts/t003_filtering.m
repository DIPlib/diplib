%filtering
disp('t003_filtering.m');

a=readim('cermet');

b=gaussf(a,10,'fir');
c=gaussf(a,10,'iir');
if max(abs(b-c))>1
   error('Too large a difference between FIR and IIR Gauss filters.')
end
n=dipgetpref('NumberOfThreads');
dipsetpref('NumberOfThreads',1);
if max(abs(gaussf(a,10,'fir')-b))>1e-6
   error('Single- and multi-threaded versions of FIR Gauss produce different results.')
end
if max(abs(gaussf(a,10,'iir')-c))>1e-6
   error('Single- and multi-threaded versions of FIR Gauss produce different results.')
end
dipsetpref('NumberOfThreads',n);

c=readim('chromo3d.ics');
kuwahara(c)

curvature(c);
m = threshold(a);
bopening(m,5,-1,1);
bclosing(m,5,2,0);
dt(m);
m = label(m);
measure(m,a,{'Size','Mass','Inertia'},[],1,50,1e5);
watershed(-a,1,10,100);
[o,e,a,l1,l2] = structuretensor(a,1,5,{'orientation','energy','anisotropy','l1','l2'});
[l1,phi1,theta1,l2,phi2,theta2,l3,phi3,theta3,e,f,g] = structuretensor3d(c,1,5,{'l1','phi1','theta1','l2','phi2','theta2','l3','phi3','theta3','energy','cylindrical','planar'});
aniso(a,20,10,0.25);
bilateralf(a,2,30,2,'xysep');
canny(a,1,0.5,0.9);
resample(c,[1 .5 2],[1 0 3],'3-cubic');
hist_equalize(a);
dx(a);
dxy(c);
findshift(a,shift(a,[1.5 7]),'iter');
testobject(a,'ellipsoid',255,30,[1 2],1,0,3,1);
noise(a);
noise(c,'poisson',50);
radialmean(a);
h=diphist(c);
percf(c,76);
radialmean(a,[],1);
x=squeeze(a(:,1));
y=squeeze(a(:,10));
joinchannels('RGB',x,y);

clear a b c e f g h m n o l1 l2 l3 phi1 theta1 phi2 theta2 phi3 theta3 x y ans
