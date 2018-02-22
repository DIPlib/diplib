A = rand(2); % an arbitrary matrix

[U,D,V] = svd(A);
V = V'; % A = U*D*V' => A = U*D*V for simplicity
scale = max(1,min(diag(D))); % separate out any upscaling
D = D/scale;

% A is V + D + U + scale (not arithmetic addition, the sequence of
%                         transformations)

% V is a flip + rotation
% D is a scaling in all dimensions but one
% U is a flip + rotation
% scale is a uniform upscaling, put at end to reduce computational
%    cost of other transformations. By taking the minimum of all
%    scalings, we won't downscale in the middle and upscale back at
%    the end, losing information.

% All rotations are flip + dimension swap + 3 * shear (in 2D) or
%    more shears in higher dimensions

flipU = U(1)*U(4)<0; % means flip 2nd axis
rotU = atan2(U(2),U(1));
flipV = V(1)*V(4)<0; % means flip 2nd axis
rotV = atan2(V(2),V(1));

% A is flipV + rotV + D + flipU + rotU + scale

scale * [cos(rotU),-((-1).^flipU)*sin(rotU);sin(rotU),((-1).^flipU)*cos(rotU)] * D * [cos(rotV),-((-1).^flipV)*sin(rotV);sin(rotV),((-1).^flipV)*cos(rotV)]

