function v=initenv(env,value)
v=getenv(env);
if isempty(v)
  v=value;
end
