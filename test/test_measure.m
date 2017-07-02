% Old DIPlib

a2 = readim('cermet');
b2 = label(~threshold(a2));
correct2 = measure(b2,a2,{'GreyMu','Mu'});

a3 = readim('chromo3d');
b3 = label(threshold(a3));
correct3 = measure(b3,a3,{'GreyMu','Mu'});

a2 = dip_array(a2);
b2 = dip_array(b2);
a3 = dip_array(a3);
b3 = dip_array(b3);

correct2 = double(correct2);
correct3 = double(correct3);

save measure_test
switch_dip
load measure_test

a2 = dip_image(a2);
b2 = dip_image(b2,'uint32');
a3 = dip_image(a3);
b3 = dip_image(b3,'uint32');

new2 = measure(b2,a2,{'GreyMu','Mu'})';
diff2 = ( [correct2(:,[1,3,2 , 4,6,5]) - new2] ) ./ new2;
max(abs(diff2(~isnan(diff2))))

new3 = measure(b3,a3,{'GreyMu','Mu'})';
diff3 = ( correct3(:,[1,4,6,2,3,5 , 7,10,12,8,9,11]) - new3 ) ./ new3;
max(abs(diff3(~isnan(diff3))))
