function [newPoints] = ReEstimatePoints(I, points)

h = figure; imshow(I); hold on;
plot(points(1,:),points(2,:),'y+');
newPoints = points;

while(true)
    [px, py, button] = ginput(1);
    if button ~= 1
        break;
    end
    p = [px,py];
    k = dsearchn(points',p);
    plot(points(1,k),points(2,k),'rO'); hold on;
    
    [px, py, button] = ginput(1);
    if button ~= 1
        break;
    end
    newPoints(:,k) = [px,py]';
    plot(newPoints(1,k),newPoints(2,k),'r*'); hold on;
end
imshow(I); hold on;
plot(newPoints(1,:),newPoints(2,:),'rO'); hold on;
% Display feature numbers
for i = 1:size(newPoints,2)
    text(newPoints(1,i),newPoints(2,i),num2str(i));
end


end

