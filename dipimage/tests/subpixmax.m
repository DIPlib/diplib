function subpixmax()
% Tests findmaxima and findminima

% Separable methods (in 4D)
N = 4;
p = [10.33, 20.52, 15.89, 23.001];
q = round(p);
a = newim(ones(1, N) * 30);
a = gaussianblob(a, p, 1, 1);
m = a > 1e-7;
v = (2*pi)^(-N/2); % theoretical height of the blob, the estimate should be below this
t = max(a);        % largest pixel value, the estimate should be above this

[x, y] = findmaxima(a, m, 'linear');
assert_true(all(abs(x - p) < 0.2) && all(y <= v) && all(y >= t))
[x, y] = findmaxima(a, m, 'parabolic');
assert_true(all(abs(x - p) < 0.05) && all(y <= v) && all(y >= t))
[x, y] = findmaxima(a, m, 'gaussian');
assert_true(all(abs(x - p) < 1e-6) && all(y <= v) && all(y >= t))

[x, y] = findminima(-a, m, 'linear');
assert_true(all(abs(x - p) < 0.2) && all(-y <= v) && all(-y >= t))
[x, y] = findminima(-a, m, 'parabolic');
assert_true(all(abs(x - p) < 0.05) && all(-y <= v) && all(-y >= t))
[x, y] = findminima(-a, m, 'gaussian');
assert_true(all(abs(x - p) < 1e-6) && all(-y <= v) && all(-y >= t))

% Non-separable methods, 3D version
N = 3;
p = p(1:N);
a = newim(ones(1, N) * 30);
a = gaussianblob(a, p, 1, 1);
v = (2*pi)^(-N/2);
t = max(a);

[x, y] = findmaxima(a, [], 'parabolic_nonseparable');
assert_true(all(abs(x - p) < 0.05) && all(y >= t))
[x, y] = findmaxima(a, [], 'gaussian_nonseparable');
assert_true(all(abs(x - p) < 1e-6) && all(y >= t))

% Non-separable methods, 2D version
N = 2;
p = p(1:N);
a = newim(ones(1, N) * 30);
a = gaussianblob(a, p, 1, 1);
v = (2*pi)^(-N/2);
t = max(a);

[x, y] = findmaxima(a, [], 'parabolic_nonseparable');
assert_true(all(abs(x - p) < 0.05) && all(y >= t))
[x, y] = findmaxima(a, [], 'gaussian_nonseparable');
assert_true(all(abs(x - p) < 1e-6) && all(y >= t))
