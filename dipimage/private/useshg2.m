function res = useshg2
persistent newmatlab;
if isempty(newmatlab)
   newmatlab = [100, 1] * sscanf(version, '%d.', 2) >= 804;
end
res = newmatlab;
