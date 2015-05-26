%% Reconstruction_main.m
close all; clc;
clearvars -except Richard2_left_vx500 Richard2_left_vy500 ...
                    Richard2_right_vx500 Richard2_right_vy500;
% Requires vlfeat toolbox
addpath(genpath('vlfeat-0.9.20'));

%% Read a pair of rectified stereo images
folder_left = 'C:\Users\Richard\Desktop\CDE\Semester2\Visual_Effects\Data\Richard2\Left\images_rect\';
folder_right = 'C:\Users\Richard\Desktop\CDE\Semester2\Visual_Effects\Data\Richard2\Right\images_rect\';
im_left = imread([folder_left, 'Richard2_left_rect_1000.jpg']);
im_right = imread([folder_right, 'Richard2_right_rect_1000.jpg']);

figure(1);
% subplot(1,2,1); 
imshow(im_left);
% subplot(1,2,2); 
% imshow(im_right);

%% Load calibration data
load('C:\Users\Richard\Desktop\CDE\Semester2\Visual_Effects\Data\Richard2\Calib_Results_stereo_rectified');

% Intrinsic camera matrices
K_left = KK_left_new;
K_right = KK_right_new;

% External parameters
R = R_new;
t = T_new;

% Camera matrices
P_left = K_left * eye(3,4);
P_right = K_right * [R, t];

% Clear variables
clearvars -except P_left P_right K_left K_right R t im_left im_right ...
                    Richard2_left_vx500 Richard2_left_vy500 ...
                    Richard2_right_vx500 Richard2_right_vy500;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Detect feature points in left image
im_left_sift = single(im_left);

% Select region of interest
disp('Select region of interest');
figure(1); % subplot(1,2,1); 
hold on; 
M = single(roipoly(im_left));
im_left_sift = im_left_sift.*M;

% Detect features
[f,d] = vl_sift(im_left_sift, 'Octaves', 2, 'Levels', 5, 'FirstOctave', -1,...
    'PeakThresh', 2, 'EdgeThresh', 10, 'NormThresh', 1, ...
    'Magnif', 1, 'WindowSize', 10);

% Plot features
figure(1); % subplot(1,2,1); 
hold on;
vl_plotframe(f(:,:));
% plot(f(1,:),f(2,:),'r*');

% Store features
clear points_left;
points_left(:,:,1) = f(1:2,:);

% Remove duplicate features
x = points_left(1,:); y = points_left(2,:);
indx = diff(x)==0; indy = diff(y)==0;
ind = bsxfun(@and,indx,indy);
points_left(:,ind) = [];
numFeatures = size(points_left,2);

%% Add points manually
disp('Adding points, right click to exit');
addPoint = true;
figure(1); % subplot(1,2,1); 
hold on;
i = numFeatures + 1;
while(true)
    [points_left(1,i,1), points_left(2,i,1), button] = ginput(1);
    if button ~= 1
        break;
    end
    plot(points_left(1,i,1),points_left(2,i,1),'r*');
    i = i + 1;
end
numFeatures = size(points_left,2);
disp('Finished adding points');

%% Remove points manually
disp('Removing points, right click to exit');
removePoint = true;
figure(1); subplot(1,2,1); hold on;
i = numFeatures;
while(true)
    [px, py, button] = ginput(1);
    if button ~= 1
        break;
    end
    p = [px,py];
    k = dsearchn(points_left',p);
    points_left(:,k) = [];
    imshow(im_left); subplot(1,2,1); plot(points_left(1,:),points_left(2,:),'r*');
end
numFeatures = size(points_left,2);
disp('Finished removing points');

%% Remove points close to each other
clearvars index dist
kdtree = vl_kdtreebuild(points_left);
n = 2; % 2 nearest neighbours (including the point itself!)
for i = 1:numFeatures
    [index(:,i), dist(:,i)] = vl_kdtreequery(kdtree, points_left, points_left(:,i), 'NumNeighbors', n);   
end
distID = dist(2,:) < 2;
points_left(:,distID) = [];
numFeatures = size(points_left,2);
subplot(1,2,1); imshow(im_left); plot(points_left(1,:),points_left(2,:),'r*');

%% Order features
[points_left(1,:), sortID] = sort(points_left(1,:));
points_left(2,:) = points_left(2,sortID);

% Display feature numbers
for i = 1:numFeatures
    figure(1); subplot(1,2,1); hold on;
    text(points_left(1,i),points_left(2,i),num2str(i));
end

%% Save features
% save('points_left','points_left');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Load features in left image (if exist!)
load('points_left');
numFeatures = size(points_left,2);

% Plot left features
figure(2);
subplot(1,2,1); hold on; 
plot(points_left(1,:),points_left(2,:),'r+')

% Display feature numbers
for i = 1:numFeatures
    figure(2); subplot(1,2,1); hold on;
    text(points_left(1,i),points_left(2,i),num2str(i));
end

%% Find corresponding features in right image
im_left = im2double(im_left);
im_right = im2double(im_right);
points_right = points_left;

w = 5; % Image patch size = (2*w + 1) x (2*w + 1) pixels
NC = NaN(1,640);
for i = 1:numFeatures % For each feature point in left image...
    col = round(points_left(1,i)); % x-coordinate
    row = round(points_left(2,i)); % y-coordinate
    window_left = im_left(row-w:row+w, col-w:col+w); % Left image patch
    mean_left = mean(window_left(:)); % Mean of image patch
    std_left = std(window_left(:)); % Std dev. of image patch
    window_left_shift = window_left - mean_left; % Patch shifted by mean
        
    for j = col-20:col+60 % For points in right image on same row...
        window_right = im_right(row-w:row+w, j-w:j+w); % Right image patch
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

%% Plot right features
figure(1);
subplot(1,2,2); hold on; 
plot(points_right(1,:),points_right(2,:),'rO')

% Display feature numbers
for i = 1:numFeatures
    figure(1); subplot(1,2,2); hold on;
    text(points_right(1,i),points_right(2,i),num2str(i));
end

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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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

%% Load optical flow
folder = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\';
disp('Loading vx and vy...');
load([folder 'Richard2_left_vx500']);
load([folder 'Richard2_left_vy500']);
load([folder 'Richard2_right_vx500']);
load([folder 'Richard2_right_vy500']);
disp('Done!');
    
%% Track features in left image
disp('Tracking features');
newImg = imArray_left{1};
for j = 1:numFeatures
    newImg(round(points_left(2,j,1)) - 2:2 + round(points_left(2,j,1)), ...
        round(points_left(1,j,1)) - 2:round(points_left(1,j,1)) + 2 ) = 255;
end

if isunix
    Mov(1) = im2frame(newImg, gray(256));
else
    Mov(1) = im2frame(flipud(newImg), gray(256));
end

for frame = 2:numImgs
    newImg = imArray_left{frame};
    for j = 1:numFeatures
        idx = round(points_left(1,j,frame-1));
        idy = round(points_left(2,j,frame-1));
        points_left(1,j,frame) = points_left(1,j,frame-1) + Richard2_left_vx500(idy,idx,frame-1);
        points_left(2,j,frame) = points_left(2,j,frame-1) + Richard2_left_vy500(idy,idx,frame-1);
        
        newImg(round(points_left(2,j,frame)) - 2:2 + round(points_left(2,j,frame)), ...
            round(points_left(1,j,frame)) - 2:round(points_left(1,j,frame)) + 2 ) = 255;
    end
    
    if isunix
        Mov(frame) = im2frame(newImg, gray(256));
    else
        Mov(frame) = im2frame(flipud(newImg), gray(256));
    end
end

%% Play movie
figure; imshow(imArray_left{1});
movie(Mov,1,60);

%% Track features in right image
disp('Tracking features');
newImg = imArray_right{1};
for j = 1:numFeatures
    newImg(round(points_right(2,j,1)) - 2:2 + round(points_right(2,j,1)), ...
        round(points_right(1,j,1)) - 2:round(points_right(1,j,1)) + 2 ) = 255;
end

if isunix
    Mov(1) = im2frame(newImg, gray(256));
else
    Mov(1) = im2frame(flipud(newImg), gray(256));
end

for frame = 2:numImgs
    newImg = imArray_right{frame};
    for j = 1:numFeatures
        idx = round(points_right(1,j,frame-1));
        idy = round(points_right(2,j,frame-1));
        points_right(1,j,frame) = points_right(1,j,frame-1) + Richard2_right_vx500(idy,idx,frame-1);
        points_right(2,j,frame) = points_right(2,j,frame-1) + Richard2_right_vy500(idy,idx,frame-1);
        
        newImg(round(points_right(2,j,frame)) - 2:2 + round(points_right(2,j,frame)), ...
            round(points_right(1,j,frame)) - 2:round(points_right(1,j,frame)) + 2 ) = 255;
    end
    
    if isunix
        Mov(frame) = im2frame(newImg, gray(256));
    else
        Mov(frame) = im2frame(flipud(newImg), gray(256));
    end
end

%% Play movie
figure; imshow(imArray_right{1});
movie(Mov,1,60);

%% Apply epipolar constraint
points_left(2,:) = (points_left(2,:) + points_right(2,:))./2; % Set y-coordinate to average
points_right(2,:) = points_left(2,:);

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