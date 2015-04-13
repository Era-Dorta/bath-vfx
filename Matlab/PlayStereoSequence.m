function PlayStereoSequence(folder_left,folder_right,points_left,points_right,firstFrame,lastFrame)

% First frame
imgType = 'jpg';
sDir_left =  dir( fullfile(folder_left ,['*' imgType]) );
sDir_right =  dir( fullfile(folder_right ,['*' imgType]) );
im_left = imread([folder_left '/' sDir_left(1).name]);
[height, width] = size(im_left);

count = 0;
for frame = firstFrame:lastFrame
    count = count + 1;
    im_left = imread([folder_left '/' sDir_left(frame).name]);
    im_right = imread([folder_right '/' sDir_right(frame).name]);
    figure(1); imshow([im_left,im_right],[]); hold on;
    % Plot features
    plot(points_left(1,:,count),points_left(2,:,count),'yO');
    plot(points_right(1,:,count)+width,points_right(2,:,count),'yO');
end

end

