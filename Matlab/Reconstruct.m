function [X] = Reconstruct(P1, P2, x1, x2)
% Reconstruct - computes 3D points given two projection matrices
% and corresponding image points
%
% Input:
%           P1 - 3x4 projection matrix 1
%           P2 - 3x4 projection matrix 2
%           x1 - 2xn points in image 1
%           x2 - 2xn points in image 2
%
% Output:
%           X - 4xn point in space

[r1,c1] = size(x1);
[r2,c2] = size(x2);
n = c1; % number of points

if (r1 == 3) % normalise
    x1(1,:) = x1(1,:)./x1(3,:);
    x1(2,:) = x1(2,:)./x1(3,:);
end

if (r2 == 3) % normalise
    x2(1,:) = x2(1,:)./x2(3,:);
    x2(2,:) = x2(2,:)./x2(3,:);
end

X = NaN(4,n);

if (c1 == c2)
    for i = 1:n
        A1 = [x1(1,i)*P1(3,1)-P1(1,1), x1(1,i)*P1(3,2)-P1(1,2),...
              x1(1,i)*P1(3,3)-P1(1,3), x1(1,i)*P1(3,4)-P1(1,4)]; 
        A2 = [x1(2,i)*P1(3,1)-P1(2,1), x1(2,i)*P1(3,2)-P1(2,2),...
              x1(2,i)*P1(3,3)-P1(2,3), x1(2,i)*P1(3,4)-P1(2,4)]; 
        A3 = [x2(1,i)*P2(3,1)-P2(1,1), x2(1,i)*P2(3,2)-P2(1,2),...
              x2(1,i)*P2(3,3)-P2(1,3), x2(1,i)*P2(3,4)-P2(1,4)]; 
        A4 = [x2(2,i)*P2(3,1)-P2(2,1), x2(2,i)*P2(3,2)-P2(2,2),...
              x2(2,i)*P2(3,3)-P2(2,3), x2(2,i)*P2(3,4)-P2(2,4)];

        A = [A1;A2;A3;A4];
        [~,~,V] = svd(A);

        X(1,i) = V(1,end)/V(end,end);
        X(2,i) = V(2,end)/V(end,end);
        X(3,i) = V(3,end)/V(end,end);
        X(4,i) = V(4,end)/V(end,end);
    end
end

end

