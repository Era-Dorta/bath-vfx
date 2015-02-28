function [K,R,t] = Decompose_P(P)
% Decompose_P - decompose camera projection matrix P into intrinsic
% matrix K, rotation matrix R, and translation vector t
%
% Input:
%           P - 3x4 camera projection matrix
%
% Output:
%           K - 3x3 intrinsic matrix
%           R - 3x3 rotation matrix
%           t - 3x1 translation vector

Q = inv(P(1:3, 1:3));
[R,K] = qr(Q);

%% Check signs of elements of K
if (K(1,1) < 0)
    S = [-1 0 0;0 1 0;0 0 1];
    R = R*S;
    K = S*K;
end

if (K(2,2) < 0)
    S = [1 0 0;0 -1 0;0 0 1];
    R = R*S;
    K = S*K;
end

if (K(3,3) < 0)
    S = [1 0 0;0 1 0;0 0 -1];
    R = R*S;
    K = S*K;
end

%% Translation vector
t = K*P(1:3,4);

%% Check det(R)=1
if det(R) < 0 
    t = -t;
    R = -R;
end

%% Rotation matrix
R = inv(R);

%% Intrinsic matrix
K = inv(K);
K = K./K(3,3);