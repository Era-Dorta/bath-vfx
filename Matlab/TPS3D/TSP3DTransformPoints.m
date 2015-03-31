function [wobject] = TSP3DTransformPoints(param, points, object)

npnts = size(points,1);
pntsNum=size(object,1); 
K = zeros(pntsNum, npnts);
gx=object(:,1);
gy=object(:,2);
gz=object(:,3);
for nn = 1:npnts
    K(:,nn) = (gx - points(nn,1)).^2 + (gy - points(nn,2) ).^2 + (gz - points(nn,3) ).^2; % R^2
end;
K = max(K,1e-320); 
K = sqrt(K); %|R| for 3D

P = [ones(pntsNum,1), gx, gy, gz];
L = [K, P];
wobject = L * param;  
wobject(:,1)=round(wobject(:,1)*10^3)*10^-3;
wobject(:,2)=round(wobject(:,2)*10^3)*10^-3;
wobject(:,3)=round(wobject(:,3)*10^3)*10^-3;

end