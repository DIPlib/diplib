% Finds and runs all the tests scripts in the current directory

% Yes, globals are bad, but this makes writing test scripts a lot simpler,
% and the simpler that is, the more likely we are of actually writing them!
global dip_test_good dip_test_bad

total_tests = 0;
total_failures = 0;
exceptions = 0;

files = dir('*.m');
files = sort({files.name});  % sort so that we get a deterministic run order
for name = files
   name = name{1}(1:end-2);  % remove the '.m' extension, which is guarnateed to be there
   if strncmp(name, 'assert_', 7) || any(strcmp(name, {'run_tests', 'startup', 'approx_equal'}))
      continue
   end
   disp(newline + "Running test " + name)
   try
      dip_test_good = 0;
      dip_test_bad = 0;
      eval(name);
      disp("   " + (dip_test_good + dip_test_bad) + " tests, " + dip_test_bad + " failed.")
      total_tests = total_tests + dip_test_good + dip_test_bad;
      total_failures = total_failures + dip_test_bad;
   catch ME
      if isempty(ME.identifier)
         disp("   Exception of unidentified type: " + ME.message)
      else
         disp("   Exception of type " + ME.identifier + ": " + ME.message)
      end
      exceptions = exceptions + 1;
   end
end

disp(newline + "Totals: " + total_tests + " tests, " + total_failures + " failed; " + exceptions + " test scripts threw an exception")
