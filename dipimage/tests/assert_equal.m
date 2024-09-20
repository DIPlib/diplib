function assert_equal(a, b)
% Checks that the two arrays A and B are numerically equal -- ignores the class,
% but expects the sizes to match. See ISEQUAL.
assert_true(isequal(a, b), 2)
