%% KLT_main.m
clear all; close all; clc;

%% Read in stereo image sequence
disp('Reading stereo image sequence...');
imgType = '.jpg';
numImgs = 500;

% Left images
if isunix
    folder = '~/workspaces/matlab/vfx/Data/Richard2/Left/images_rect/';
else
    folder = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Left\images_rect';
end
imArray_left = ReadImgs(folder, imgType, numImgs);

% Right images
if isunix
    folder = '~/workspaces/matlab/vfx/Data/Richard2/Left/images_rect/';
else
    folder = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Right\images_rect';
end
imArray_right = ReadImgs(folder, imgType, numImgs);
disp('Done!');

%% Display first frame
figure(1);
im_left = imArray_left{1};
im_right = imArray_right{1};
imshow([im_left,im_right]); hold on;
Mov(1) = im2frame(flipud([im_left,im_right]), gray(256));

%% Load features in left image (if exist!)
load('fStore_left');
numFeatures = size(fStore,2);

% Plot left features
figure(1);
plot(fStore(1,:),fStore(2,:),'yO')

% Display feature numbers
for i = 1:numFeatures
    figure(1);
    text(fStore(1,i),fStore(2,i),num2str(i));
end

%% Find corresponding features in right image
im1_left = im2double(im_left);
im1_right = im2double(im_right);
fStore_right = fStore;

w = 5; % Image patch size = (2*w + 1) x (2*w + 1) pixels
NC = NaN(1,640);
for i = 1:numFeatures % For each feature point in left image...
    col = round(fStore(1,i)); % x-coordinate
    row = round(fStore(2,i)); % y-coordinate
    window_left = im1_left(row-w:row+w, col-w:col+w); % Left image patch
    mean_left = mean(window_left(:)); % Mean of image patch
    std_left = std(window_left(:)); % Std dev. of image patch
    window_left_shift = window_left - mean_left; % Patch shifted by mean
        
    for j = col-20:col+60 % For points in right image on same row...
        window_right = im1_right(row-w:row+w, j-w:j+w); % Right image patch
        mean_right = mean(window_right(:)); % Mean of image patch
        std_right = std(window_right(:)); % Std dev. of image patch
        window_right_shift = window_right - mean_right; % Patch shifted by mean
        
        % Normalised cross-correlation
        NC(j) = sum(sum(window_left_shift.*window_right_shift))/(std_left*std_right);
    end    
    
    [val,ind] = max(NC); % Max cross-correlation
    % While distance between feature points is greater than 60px...
    while(abs(ind-col) > 60) 
        NC(ind) = NaN;
        [val,ind] = max(NC); % Choose next best...
    end   
    
    fStore_right(1,i) = ind; % Store right feature point
end

% Plot right features
figure(1); hold on; 
plot(fStore_right(1,:)+640,fStore_right(2,:),'rO')

% Display feature numbers
for i = 1:numFeatures
    figure(1); hold on;
    text(fStore_right(1,i)+640,fStore_right(2,i),num2str(i));
end

%% Initialise point trackers
points_left = cornerPoints(fStore'); % Create points object
points_right = cornerPoints(fStore_right');

% Create a point tracker and enable the bidirectional error constraint to
% make it more robust in the presence of noise and clutter
pointTracker_left = vision.PointTracker('MaxBidirectionalError', inf);
pointTracker_right = vision.PointTracker('MaxBidirectionalError', inf);

% Initialize the tracker with the initial point locations and the initial
% video frame
points_left = points_left.Location;
points_right = points_right.Location;
initialize(pointTracker_left, points_left, im_left);
initialize(pointTracker_right, points_right, im_right);

%% Track points
disp('Tracking points...');

% Make a copy of the points to be used for computing the geometric
% transformation between the points in the previous and the current frames
oldPoints_left = points_left;
oldPoints_right = points_right;

for frame = 2:numImgs
    % Get next frame
    newImg_left = imArray_left{frame};
    newImg_right = imArray_right{frame};

    % Track the points. Note that some points may be lost
    [points_left, isFound] = step(pointTracker_left, newImg_left);
    visiblePoints_left = points_left(isFound, :);
    oldInliers_left = oldPoints_left(isFound, :);
    
    [points_right, isFound] = step(pointTracker_right, newImg_right);
    visiblePoints_right = points_right(isFound, :);
    oldInliers_right = oldPoints_right(isFound, :);
    
    % Estimate the geometric transformation between the old points
    % and the new points and eliminate outliers
    [xform_left, oldInliers_left, visiblePoints_left] = estimateGeometricTransform(...
        oldInliers_left, visiblePoints_left, 'similarity', 'MaxDistance', 4);
    [xform_right, oldInliers_right, visiblePoints_right] = estimateGeometricTransform(...
            oldInliers_right, visiblePoints_right, 'similarity', 'MaxDistance', 4);
    
    % Display tracked points
    newImg_left = insertMarker(newImg_left, visiblePoints_left, '+', ...
            'Color', 'white');
    newImg_right = insertMarker(newImg_right, visiblePoints_right, '+', ...
            'Color', 'white');

    % Reset the points
    oldPoints_left = visiblePoints_left;
    oldPoints_right = visiblePoints_right;
    setPoints(pointTracker_left, oldPoints_left);
    setPoints(pointTracker_right, oldPoints_right);       
   
    % Store frame
    newImg = [newImg_left(:,:,1),newImg_right(:,:,1)];
    Mov(frame) = im2frame(flipud(newImg), gray(256)); 
    
    % Store points
    % If points are lost, break loop
    if size(visiblePoints_left,1)<numFeatures || size(visiblePoints_right,1)<numFeatures
        numImgs = frame-1;
        disp('Points lost! Please re-estimate points');
        break;
    end
    fStore(:,:,frame) = visiblePoints_left';
    fStore_right(:,:,frame) = visiblePoints_right';
end
disp('Done!')

%% Play movie
figure; imshow([im_left,im_right]);
movie(Mov,1,120);

%% Load calibration data
load('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Calib_Results_stereo_rectified');

% Intrinsic camera matrices
K_left = KK_left_new;
K_right = KK_right_new;

% External parameters
R = R_new;
t = T_new;

% Camera matrices
P_left = K_left * eye(3,4);
P_right = K_right * [R, t];

%% 3D reconstruction
X = Reconstruct(P_left, P_right, fStore(:,:,1), fStore_right(:,:,1));

%% Visualise
VisualiseScene(P_left, P_right, X);

%% Delaunay triangulation
figure('units','normalized','outerposition',[0 0 1 1]);
x = X(1,:);
y = X(3,:);
z = -X(2,:);
tri = delaunay(x, z);
trisurf(tri, x, y, z, 'LineWidth', 2);
axis equal
shading interp
colormap(cool)
% camlight right
light % create a light
lighting gouraud
material dull
% alpha(0.8) 

%% Apply epipolar constraint
fStore(2,:) = (fStore(2,:) + fStore_right(2,:))./2; % Set y-coordinate to average
fStore_right(2,:) = fStore(2,:);

%% Reconstruct all frames
X = NaN(4,numFeatures,numImgs);
for frame = 1:numImgs
    X(:,:,frame) = Reconstruct(P_left, P_right, fStore(:,:,frame), fStore_right(:,:,frame));
end

%% Show 4D plot
for frame = 1:numImgs
    x_offset = X(1,54,frame); % Keep nose position at x = 0
    y_offset = X(3,54,frame); % Keep nose position at y = 0
    z_offset = -X(2,54,frame); % Keep nose position at z = 0
    
    x = X(1,:,frame)-x_offset;
    y = X(3,:,frame)-y_offset;
    z = -X(2,:,frame)-z_offset;
    
    figure(6);
    trisurf(tri, x, y, z, 'LineWidth', 1.5);
    axis equal
    axis([-50 50 0 80 -60 80])
    axis off
%     shading interp
    colormap(cool)
    camlight right
    light % create a light
    lighting gouraud
    material dull
    view([90,-160,45]);
end