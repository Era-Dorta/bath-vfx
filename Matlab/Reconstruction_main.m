%% Reconstruction_main.m
clear; close all;

%% Read a pair of rectified stereo images
folder_left = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Left\images_rect\';
folder_right = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Right\images_rect\';
im_left = imread([folder_left, 'Richard2_left_rect_0001.jpg']);
im_right = imread([folder_right, 'Richard2_right_rect_0001.jpg']);

figure(1);
subplot(1,2,1); imshow(im_left);
subplot(1,2,2); imshow(im_right);

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

% Clear variables
clearvars -except P_left P_right K_left K_right R t im_left im_right

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Detect feature points in left image
im_left_sift = single(im_left);

% Select region of interest
disp('Select region of interest');
figure(1); subplot(1,2,1); hold on; 
M = single(roipoly(im_left));
im_left_sift = im_left_sift.*M;

% Detect features
[f,d] = vl_sift(im_left_sift, 'Octaves', 2, 'Levels', 5, 'FirstOctave', -1,...
    'PeakThresh', 3, 'EdgeThresh', 5, 'NormThresh', 1, ...
    'Magnif', 1, 'WindowSize', 10);

% Plot features
figure(1); subplot(1,2,1); hold on;
vl_plotframe(f(:,:));
plot(f(1,:),f(2,:),'r*');

% Store features
clear fStore;
fStore(:,:,1) = f(1:2,:);

% Remove duplicate features
x = fStore(1,:); y = fStore(2,:);
indx = diff(x)==0; indy = diff(y)==0;
ind = bsxfun(@and,indx,indy);
fStore(:,ind) = [];
numFeatures = size(fStore,2);

%% Add points manually
disp('Adding points, right click to exit');
addPoint = true;
figure(1); subplot(1,2,1); hold on;
i = numFeatures + 1;
while(true)
    [fStore(1,i,1), fStore(2,i,1), button] = ginput(1);
    if button ~= 1
        break;
    end
    plot(fStore(1,i,1),fStore(2,i,1),'r*');
    i = i + 1;
end
numFeatures = size(fStore,2);
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
    k = dsearchn(fStore',p);
    fStore(:,k) = [];
    imshow(im_left); subplot(1,2,1); plot(fStore(1,:),fStore(2,:),'r*');
end
numFeatures = size(fStore,2);
disp('Finished removing points');

%% Remove points close to each other
clearvars index dist
kdtree = vl_kdtreebuild(fStore);
n = 2; % 2 nearest neighbours (including the point itself!)
for i = 1:numFeatures
    [index(:,i), dist(:,i)] = vl_kdtreequery(kdtree, fStore, fStore(:,i), 'NumNeighbors', n);   
end
distID = dist(2,:) < 2;
fStore(:,distID) = [];
numFeatures = size(fStore,2);
subplot(1,2,1); imshow(im_left); plot(fStore(1,:),fStore(2,:),'r*');

%% Order features
[fStore(1,:), sortID] = sort(fStore(1,:));
fStore(2,:) = fStore(2,sortID);

% Display feature numbers
for i = 1:numFeatures
    figure(1); subplot(1,2,1); hold on;
    text(fStore(1,i),fStore(2,i),num2str(i));
end

%% Save features
% save('fStore_left','fStore');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Load features in left image (if exist!)
load('fStore_left');
numFeatures = size(fStore,2);

% Plot left features
figure(1);
subplot(1,2,1); hold on; 
plot(fStore(1,:),fStore(2,:),'yO')

% Display feature numbers
for i = 1:numFeatures
    figure(1); subplot(1,2,1); hold on;
    text(fStore(1,i),fStore(2,i),num2str(i));
end

%% Find corresponding features in right image
im_left = im2double(im_left);
im_right = im2double(im_right);
fStore_right = fStore;

w = 5; % Image patch size = (2*w + 1) x (2*w + 1) pixels
NC = NaN(1,640);
for i = 1:numFeatures % For each feature point in left image...
    col = round(fStore(1,i)); % x-coordinate
    row = round(fStore(2,i)); % y-coordinate
    window_left = im_left(row-w:row+w, col-w:col+w); % Left image patch
    mean_left = mean(window_left(:)); % Mean of image patch
    std_left = std(window_left(:)); % Std dev. of image patch
    window_left_shift = window_left - mean_left; % Patch shifted by mean
        
    for j = 1+w:640-w % For points in right image on same row...
        window_right = im_right(row-w:row+w, j-w:j+w); % Right image patch
        mean_right = mean(window_right(:)); % Mean of image patch
        std_right = std(window_right(:)); % Std dev. of image patch
        window_right_shift = window_right - mean_right; % Patch shifted by mean
        
        % Normalised cross-correlation
        NC(j) = sum(sum(window_left_shift.*window_right_shift))/(std_left*std_right);
    end    
    
    [val,ind] = max(NC); % Max cross-correlation
    % While distance between feature points is greater than 50px...
    while(abs(ind-col) > 50) 
        NC(ind) = NaN;
        [val,ind] = max(NC); % Choose next best...
    end   
    
    fStore_right(1,i) = ind; % Store right feature point
end

%% Plot right features
figure(1);
subplot(1,2,2); hold on; 
plot(fStore_right(1,:),fStore_right(2,:),'rO')

% Display feature numbers
for i = 1:numFeatures
    figure(1); subplot(1,2,2); hold on;
    text(fStore_right(1,i),fStore_right(2,i),num2str(i));
end

%% 3D reconstruction
X = Reconstruct(P_left, P_right, fStore, fStore_right);

%% Visualise
VisualiseScene(P_left, P_right, X);

%% Delaunay triangulation
figure('units','normalized','outerposition',[0 0 1 1]);
x = X(1,:);
y = X(3,:);
z = -X(2,:);
tri = delaunay(x, z);
trimesh(tri, x, y, z);
axis equal
