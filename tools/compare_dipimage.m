n = dir('/Users/cris/newdip/dipimage/*.m');
n2 = dir('/Users/cris/newdip/dipimage/alias/*.m');
n = cat(1,n,n2);
n = {n.name}';

o = dir('/Users/cris/dip/diptree/dipimage/*files/*.m');
o = {o.name}';

added = setdiff(n,o)
missing = setdiff(o,n)

f = fopen('/Users/cris/newdip/dipimage/TODO','w');
fprintf(f,'Functions in DIPimage 3 that didn''t exist in DIPimage 2:\n\n');
fprintf(f,'%s\n',added{:});
fprintf(f,'\n\n\nFunctions in DIPimage 2 that are not yet ported:\n\n');
fprintf(f,'%s\n',missing{:});
fclose(f);
