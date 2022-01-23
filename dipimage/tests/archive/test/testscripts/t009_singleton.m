disp('t009_singleton.m');

a=newim+10;
b=a/0;
if any(~isinf(b))
   error('division by zero does not give Inf');
end
b = a./newim;
if any(~isinf(b))
   error('division by zero does not give Inf');
end

a = noise(newim(128,100,50));
b = mean(a,[],[1 2]);
c = a/b;
if any(imsize(a)~=imsize(c))
   error('singleton dim test 1');
end


