newdip_dir = '/Users/cris/src/diplib';
olddip_dir = '/Users/cris/src/old_dip';

n = dir(fullfile(newdip_dir, 'dipimage', '*.m'));
n2 = dir(fullfile(newdip_dir, 'dipimage', 'alias', '*.m'));
n = cat(1, n, n2);
n = {n.name}';

o = dir(fullfile(olddip_dir, 'diptree', 'dipimage', '*files', '*.m'));
o = {o.name}';

added = setdiff(n, o);
missing = setdiff(o, n);

f = fopen('TODO.txt', 'w');
fprintf(f, 'Functions in DIPimage 3 that didn''t exist in DIPimage 2:\n\n');
fprintf(f, '%s\n', added{:});
fprintf(f, '\n\n\nFunctions in DIPimage 2 that are not yet ported:\n\n');
fprintf(f, '%s\n', missing{:});
fclose(f);
