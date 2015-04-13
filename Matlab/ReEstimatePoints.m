function [newPoints] = ReEstimatePoints(I, points, inliers)

newPoints = points;
plotPoints = [];

figure; imshow(I); hold on;
plot(points(1,:),points(2,:),'y+');
plot(inliers(1,:),inliers(2,:),'gO');
hold off;

s = 30;
scale = 5;
count = 1;

while(true)    
    [c,r,button] = ginput(1);
    if button ~= 1
        break;
    end
    p = [c,r];
    k = dsearchn(points',p);
    
    wc1 = c-s; wc2 = c+s;
    wr1 = r-s; wr2 = r+s;
    w = I(wr1:wr2,wc1:wc2);
    w = imresize(w,scale);
    imshow(w,[]); 
        
    [c,r,button] = ginput(1);
    if button ~= 1
        break;
    end
    c = wc1 + c/scale - 0.5;  
    r = wr1 + r/scale - 0.5;
    
    newPoints(:,k) = [c,r]';
    plotPoints(:,count) = [c,r]';
    count = count + 1;
    
    imshow(I,[]); hold on;
    plot(points(1,:),points(2,:),'y+');
    plot(inliers(1,:),inliers(2,:),'gO');
    plot(plotPoints(1,:),plotPoints(2,:),'r*');
    hold off;
end

imshow(I); hold on;
plot(newPoints(1,:),newPoints(2,:),'rO');
% Display feature numbers
for i = 1:size(newPoints,2)
    text(newPoints(1,i),newPoints(2,i),num2str(i));
end


end

