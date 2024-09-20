function filtering()
% Tests assorted filtering functions

% 2D test image
a = readim('trui');

% Test similarity between FIR, IIR and FT versions of Gaussian filter
g_fir = gaussf(a, 10, 'fir');
g_iif = gaussf(a, 10, 'iir');
assert_true(max(abs(g_fir - g_iif)) <= 1)
g_ft = gaussf(a, 10, 'ft');
assert_true(max(abs(g_fir - g_ft)) <= 1)

% Test single vs multi-threaded results
n = dipgetpref('NumberOfThreads');
dipsetpref('NumberOfThreads', 1);
assert_approx_equal(gaussf(a, 10, 'fir'), g_fir)
assert_approx_equal(gaussf(a, 10, 'iir'), g_iif)
assert_approx_equal(gaussf(a, 10, 'ft'), g_ft)
dipsetpref('NumberOfThreads', n);

% 3D test image
c = readim('chromo3d.ics');

% Just run a bunch of filters? This is not much of a test!

kuwahara(c);
curvature(c);

m = threshold(a);
bopening(m, 5, -1, 1);
bclosing(m, 5, 2, 0);
dt(m);
m = label(m);
measure(m, a, {'Size', 'Mass', 'Inertia'});

watershed(-a, 1, 10, 100);

[orientation, energy, anisotropy, l1, l2] = structuretensor(a, 1, 5, {'orientation', 'energy', 'anisotropy', 'l1', 'l2'});
[l1, phi1, theta1, l2, phi2, theta2, l3, phi3, theta3, energy, cylindrical, planar] = structuretensor(c, 1, 5, {'l1', 'phi1', 'theta1', 'l2', 'phi2', 'theta2', 'l3', 'phi3', 'theta3', 'energy', 'cylindrical', 'planar'});

aniso(a, 20, 10, 0.25);
bilateralf(a, 2, 30, 2, 'xysep');
canny(a, 1, 0.5, 0.9);

resample(c, [1 .5 2], [1 0 3], '3-cubic');

hist_equalize(a);

dx(a);
dxy(c);

findshift(a, shift(a, [1.5 7]), 'iter');

testobject('ellipsoid', imsize(a), [30 60], generationMethod='fourier', objectAmplitude=200, randomShift=1, pointSpreadFunction='gaussian', oversampling=3, backgroundValue=10, signalNoiseRatio=10);
noise(a);
noise(c, 'poisson', 50);
radialmean(a);
h = diphist(c);
percf(c, 76);
radialmean(a, [], 1);
