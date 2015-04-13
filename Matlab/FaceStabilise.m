function [minerr, transform] = FaceStabilise(X, Xnew)

n = size(X,1); % Number of points
ind = rem(floor((0:2^n - 1)'*pow2((-n+1):0)),2); % All combinations
ind = logical(ind(sum(ind,2)>4,:)); % Need at least 4 points
N = size(ind,1); % Number of possible combinations
err = zeros(N,1);

for i = 1:N % For all combinations
    j = ind(i,:);
    [err(i), ~, ~] = procrustes(X(j,:), Xnew(j,:));
end

% Choose combination with least error
[~,index] = min(err); 
j = ind(index,:);
[minerr, ~, transform] = procrustes(X(j,:), Xnew(j,:));

end

