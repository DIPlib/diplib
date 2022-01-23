%gaussf
disp('t006_gaussf.m');

for ii=1:10
  sz = round( [100+rand(1)*20 100+rand(1)*20 100+rand(1)*20]);
  %sz = [100 110 120]
  a = newim(sz);
  sg = round(2*rand(1)+1);
  gaussf(a,sg);
end
