function assert_approx_equal(a, b, tol, mode)
% Checks that all(A == B), but approximately, using relative tolerance TOL.
% Or, if MODE is 'abs', using absolute tolerance TOL.
% TOL defaults to 1e-6.
if nargin < 3
   tol = 1e-6;
end
if nargin >= 4 and strcmp(mode, 'abs')
   diff = a - b;
else
   diff = 2 * (a - b) / (a + b);
end
diff = max(abs(diff(:)));
assert_true(diff < tol, 2)
