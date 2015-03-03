%% KLT_main.m
% Requires Matlab Computer Vision toolbox!
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
[height, width] = size(im_left);
imshow([im_left,im_right]); hold on;
Mov(1) = im2frame(flipud([im_left,im_right]), gray(256));

%% Load features in left image (if exist!)
load('points_left');
numFeatures = size(points_left,2);

% Plot left features
figure(1);
plot(points_left(1,:),points_left(2,:),'yO')

% Display feature numbers
for i = 1:numFeatures
    figure(1);
    text(points_left(1,i),points_left(2,i),num2str(i));
end

%% Find corresponding features in right image
imdouble_left = im2double(im_left);
imdouble_right = im2double(im_right);
points_right = points_left;

w = 5; % Image patch size = (2*w + 1) x (2*w + 1) pixels
NC = NaN(1,640);
for i = 1:numFeatures % For each feature point in left image...
    col = round(points_left(1,i)); % x-coordinate
    row = round(points_left(2,i)); % y-coordinate
    window_left = imdouble_left(row-w:row+w, col-w:col+w); % Left image patch
    mean_left = mean(window_left(:)); % Mean of image patch
    std_left = std(window_left(:)); % Std dev. of image patch
    window_left_shift = window_left - mean_left; % Patch shifted by mean
        
    for j = col-20:col+60 % For points in right image on same row...
        window_right = imdouble_right(row-w:row+w, j-w:j+w); % Right image patch
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
    
    points_right(1,i) = ind; % Store right feature point
end

% Plot right features
figure(1); hold on; 
plot(points_right(1,:)+640,points_right(2,:),'rO')

% Display feature numbers
for i = 1:numFeatures
    figure(1); hold on;
    text(points_right(1,i)+width,points_right(2,i),num2str(i));
end

%% Initialise point trackers
pObj_left = cornerPoints(points_left'); % Create points object
pObj_right = cornerPoints(points_right');

% Create a point tracker and enable the bidirectional error constraint to
% make it more robust in the presence of noise and clutter
pointTracker_left = vision.PointTracker('MaxBidirectionalError', inf);
pointTracker_right = vision.PointTracker('MaxBidirectionalError', inf);

% Initialize the tracker with the initial point locations and the initial
% video frame
pObj_left = pObj_left.Location;
pObj_right = pObj_right.Location;
initialize(pointTracker_left, pObj_left, im_left);
initialize(pointTracker_right, pObj_right, im_right);

%% Track points
disp('Tracking points...');

% Make a copy of the points to be used for computing the geometric
% transformation between the points in the previous and the current frames
oldPoints_left = pObj_left;
oldPoints_right = pObj_right;

for frame = 2:numImgs
    % Get next frame
    newImg_left = imArray_left{frame};
    newImg_right = imArray_right{frame};

    % Track the points. Note that some points may be lost
    % Left image
    [pObj_left, isFound] = step(pointTracker_left, newImg_left);
    visiblePoints_left = pObj_left(isFound, :);
    oldInliers_left = oldPoints_left(isFound, :);
    
    % Right image
    [pObj_right, isFound] = step(pointTracker_right, newImg_right);
    visiblePoints_right = pObj_right(isFound, :);
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
        disp(['Points lost on frame ', num2str(frame),'! Please re-estimate points']);
        break;
    end
    points_left(:,:,frame) = visiblePoints_left';
    points_right(:,:,frame) = visiblePoints_right';
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

%% Apply epipolar constraint
points_left(2,:) = (points_left(2,:) + points_right(2,:))./2; % Set y-coordinate to average
points_right(2,:) = points_left(2,:);

%% 3D reconstruction
X = Reconstruct(P_left, P_right, points_left(:,:,1), points_right(:,:,1));
VisualiseScene(P_left, P_right, X); % Visualise

%% Reconstruct first frame
figure('units','normalized','outerposition',[0 0 1 1]);

x_offset = X(1,54); % Keep nose position at x = 0
y_offset = X(3,54); % Keep nose position at y = 0
z_offset = -X(2,54); % Keep nose position at z = 0
    
x = X(1,:)-x_offset;
y = X(3,:)-y_offset;
z = -X(2,:)-z_offset;

% Display 3D points
tri = delaunay(x, z); % Delaunay triangulation
trisurf(tri, x, y, z, 'LineWidth', 1.5);
axis equal; 

% Shading properties
shading interp;
colormap(cool);
light; % create a light
lighting gouraud
material dull
% alpha(0.8) 

%% Normalise face
% So nose is always vertical
figure('units','normalized','outerposition',[0 0 1 1]);

% Angle correction
sintheta = x(48)/z(48); 
costheta = sqrt(1 - sintheta^2);
Ry = [costheta, 0, sintheta; 0, 1, 0; -sintheta, 0, costheta];
X = [x;y;z];
Xnew = Ry' * X; % Rotate points

% Display 3D points
trisurf(tri, Xnew(1,:), Xnew(2,:), X(3,:), 'LineWidth', 1.5);
axis equal

% Shading properties
shading interp;
colormap(cool);
light; % create a light
lighting gouraud
material dull
% alpha(0.8) 

%% Reconstruct all frames
X = NaN(4,numFeatures,numImgs);
for frame = 1:numImgs
    X(:,:,frame) = Reconstruct(P_left, P_right, points_left(:,:,frame), points_right(:,:,frame));
end

%% Display 4D plot
for frame = 1:numImgs
    % Keep tip of nose at [0,0,0]'
    x_offset = X(1,54,frame); % Keep nose position at x = 0
    y_offset = X(3,54,frame); % Keep nose position at y = 0
    z_offset = -X(2,54,frame); % Keep nose position at z = 0
    
    x = X(1,:,frame)-x_offset;
    y = X(3,:,frame)-y_offset;
    z = -X(2,:,frame)-z_offset;
    
    % Display 3D points
    figure(7);
    trisurf(tri, x, y, z, 'LineWidth', 1.5);
    axis equal; axis([-50 50 0 80 -60 80]); axis off;
    view([90,-180,45]);
    
    % Shading properties
    % shading interp
    colormap(cool);
    camlight right; light; % create a light
    lighting gouraud;
    material dull;
end

%% Normalise face
% So nose is always vertical
for frame = 1:numImgs
    % Keep tip of nose at [0,0,0]'
    x_offset = X(1,54,frame); % Keep nose position at x = 0
    y_offset = X(3,54,frame); % Keep nose position at y = 0
    z_offset = -X(2,54,frame); % Keep nose position at z = 0
    
    x = X(1,:,frame)-x_offset;
    y = X(3,:,frame)-y_offset;
    z = -X(2,:,frame)-z_offset;
    
    % Angle correction
    sintheta = x(48)/z(48); 
    costheta = sqrt(1 - sintheta^2);
    Ry = [costheta, 0, sintheta; 0, 1, 0; -sintheta, 0, costheta]; 
    XX = [x;y;z];
    XXnew = Ry' * XX; % Rotate points
    
    % Display 3D points
    figure(8);
    trisurf(tri, XXnew(1,:), XXnew(2,:), XX(3,:), 'LineWidth', 1.5);
    axis equal; axis([-50 50 0 80 -60 80]); axis off;
    view([90,-180,45]);
    
    % Shading properties
    % shading interp
    colormap(cool);
    camlight right; light; % create a light
    lighting gouraud;
    material dull;
end