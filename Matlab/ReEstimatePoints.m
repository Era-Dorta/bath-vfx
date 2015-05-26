function [newPoints] = ReEstimatePoints(I, points, inliers)

newPoints = points;
plotPoints = [];
Itemp = I;

figure; imshow(I); hold on;
plot(points(1,:),points(2,:),'y+');
plot(inliers(1,:),inliers(2,:),'gO');
hold off;

% s = 30;
s = 20;
scale = 5;
count = 1;

while(true)    
    % Pick a point...
    [c,r,button] = ginput(1);
    if button ~= 1
        break;
    end
    p = [c,r]; % Point picked
    k = dsearchn(points',p); % Nearest point
    
    p = newPoints(:,k);
    pc = round(p(1)); pr = round(p(2));
    I(pr,pc) = 255; % Insert marker
    % I = insertMarker(I, p', '+', 'color', 'white', 'size', 1); 

    % Window around picked point
    wc1 = pc-s; wc2 = pc+s;
    wr1 = pr-s; wr2 = pr+s;    
    w = I(wr1:wr2,wc1:wc2);
    
    % Zoom window
    w = imresize(w,scale);
    imshow(w,[]); 
    I = Itemp; % Reset image
        
    % Pick a new point on zoomed window
    [c,r,button] = ginput(1);
    pointselected = true;
    if button ~= 1
        pointselected = false;
    end
    
    if pointselected == true 
        c = wc1 + c/scale - 0.5;  
        r = wr1 + r/scale - 0.5;
    
        % Replace point
        newPoints(:,k) = [c,r]';
        plotPoints(:,count) = [c,r]';
        count = count + 1;
    end
    
    % Plot...
    imshow(I,[]); hold on;
    plot(points(1,:),points(2,:),'y+');
    plot(inliers(1,:),inliers(2,:),'gO');
    if isempty(plotPoints) == false
        plot(plotPoints(1,:),plotPoints(2,:),'r*');
    end
    hold off;
end

imshow(I); hold on;
plot(newPoints(1,:),newPoints(2,:),'rO');
% Display feature numbers
for i = 1:size(newPoints,2)
    text(newPoints(1,i),newPoints(2,i),num2str(i));
end

close;

end

