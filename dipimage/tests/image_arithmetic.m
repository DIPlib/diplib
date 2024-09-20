function image_arithmetic()
% Tests scalar arithmetic with dip_image objects

a = newim(10,10) + 10;
assert_equal(datatype(a), 'sfloat')
assert_true(all(a == 10))
b = newim([10,10]) + 2;
assert_equal(datatype(b), 'sfloat')
assert_true(all(b == 2))
c = a + b;
assert_equal(datatype(c), 'sfloat')
assert_true(all(c == 12))
c = a .* b;
assert_equal(datatype(c), 'sfloat')
assert_true(all(c == 20))
c = a .^ b;
assert_equal(datatype(c), 'sfloat')
assert_true(all(c == 100))
c = a .^ 2.1;
assert_equal(datatype(c), 'sfloat')
assert_approx_equal(c, 10 .^ 2.1)
c = a * 2.3;
assert_equal(datatype(c), 'sfloat')
assert_true(all(c == 23))
c = a * 1i;
assert_equal(datatype(c), 'scomplex')
assert_true(all(c == 10i))

b = newim([10,10], 'dfloat') + 2;
assert_true(all(b == 2))
c = a + b;
assert_equal(datatype(c), 'dfloat')
assert_true(all(c == 12))
c = a .* b;
assert_equal(datatype(c), 'dfloat')
assert_true(all(c == 20))
c = a .^ b;
assert_equal(datatype(c), 'dfloat')
assert_true(all(c == 100))

b = mat2im(rand(10,10));
assert_equal(datatype(b), 'dfloat')
c = dip_image(b * 10, 'int8');
assert_equal(datatype(c), 'sint8')

a = newim([10,10], 'complex');
a(1,1) = 4 + 4i;
assert_equal(datatype(a), 'scomplex')
b = rand(10,10) + 1i * rand(10,10);
assert_equal(class(b), 'double')
assert_true(~isreal(b))
c = a + b;
assert_equal(datatype(c), 'dcomplex')
c = a * b;
assert_equal(datatype(c), 'dcomplex')

c = a + 1;
assert_equal(datatype(a), 'scomplex')
c = a * 1;
assert_equal(datatype(a), 'scomplex')
c = a * 1i;
assert_equal(datatype(a), 'scomplex')

a = newim([64 64], 'uint8');
a(:) = 10;
assert_equal(datatype(a), 'uint8')
assert_true(all(a == 10))
b = a * 10;
assert_equal(datatype(b), 'sfloat')
assert_true(all(b == 100))
b = a - 50;
assert_equal(datatype(b), 'sfloat')
assert_true(all(b == -40))
b = a + 50;
assert_equal(datatype(b), 'sfloat')
assert_true(all(b == 60))
b = a / 5.3;
assert_equal(datatype(b), 'sfloat')
assert_true(all(b == 10 / 5.3))

dipsetpref('KeepDataType','yes')
b = a * 10;
assert_true(all(b == 100))
assert_equal(datatype(b), 'uint8')
b = a * -1;
assert_equal(datatype(b), 'uint8')
assert_true(all(b == 0))
b = a - 50;
assert_equal(datatype(b), 'uint8')
assert_true(all(b == 0))
b = a + 50;
assert_equal(datatype(b), 'uint8')
assert_true(all(b == 60))
b = a / 5.3;
assert_equal(datatype(b), 'uint8')
assert_true(all(b == 1))
dipsetpref('KeepDataType','no')

a = newim(10,10) + 10;
b = a / 0;
assert_true(all(isinf(b)))
b = a ./ newim(10,10);
assert_true(all(isinf(b)))

a = noise(newim(12,10,5));
b = mean(a, [], [1 2]);
assert_equal(imsize(b), [1, 1, 5])
c = a / b;
assert_equal(imsize(a), imsize(c))
