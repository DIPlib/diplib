function assert_true(a, stack_depth)
% Checks that the value A evaluates to true. A must be scalar.
if a
   global dip_test_good
   dip_test_good = dip_test_good + 1;
else
   if nargin < 2, stack_depth = 1; end
   stack_depth = stack_depth + 1;
   stack = dbstack;
   line = stack(stack_depth).line;
   disp("   -> test failed on line number " + line)
   global dip_test_bad
   dip_test_bad = dip_test_bad + 1;
end
