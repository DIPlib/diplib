% 2D

a = readim('cermet');
b = dip_image(label(~threshold(a)),'uint32');
correct = measure(b,a,{'size','perimeter','feret','gravity','p2a'});
test = mextest_measure(b,a);

( correct.feret - test(1:5,:) ) % not identical, two cases have different sizes, not sure why?
( correct.gravity - test(6:7,:) ) % perfect!
( correct.p2a - test(8,:) ) % OK
( correct.size - test(9,:) ) % perfect!
( correct.perimeter - test(10,:) ) % not identical because we fixed a bug (we sometimes have one more corner count: -0.0910

% 3D

a = readim('chromo3d');
b = dip_image(label(threshold(a)),'uint32');
correct = measure(b,a,{'size','surfacearea','gravity','p2a'});
test = mextest_measure(b,a);

( correct.gravity - test(1:3,:) ) % perfect!
( correct.p2a - test(4,:) ) % perfect!
( correct.size - test(5,:) ) % perfect!
( correct.surfacearea - test(6,:) ) % perfect!
