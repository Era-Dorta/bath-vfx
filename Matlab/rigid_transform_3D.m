% This function finds the optimal Rigid/Euclidean transform in 3D space
% It expects as input a Nx3 matrix of 3D points.
% It returns R, t

% expects row data
function [R, t] = rigid_transform_3D(A, B)
if nargin ~= 2
    error('Missing parameters');
end

centroid_A = mean(A);
centroid_B = mean(B);

N = size(A,1);

H = (A - repmat(centroid_A, N, 1))' * (B - repmat(centroid_B, N, 1));

[U,~,V] = svd(H);

R = V*U';

if det(R) < 0
    V(:,3) = V(:,3) * -1;
    R = V*U';
end

t = -R*centroid_A' + centroid_B';
end

