%% DenseReconstruction_main.m
clear; close all; clc;
% Requires vlfeat toolbox
addpath(genpath('vlfeat-0.9.20'));
addpath(genpath('WOBJ_toolbox_Version2b'));

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
clearvars -except P_left P_right K_left K_right R t;

%% Read in a pair of stereo images
% Left image folder
if isunix
    folder_left = '~/workspaces/matlab/vfx/Data/Richard2/Left/images_rect/';
else
    folder_left = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Left\images_seg\';
end
% Right image folder
if isunix
    folder_right = '~/workspaces/matlab/vfx/Data/Richard2/Right/images_rect/';
else
    folder_right = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Right\images_seg\';
end
im_left = imread([folder_left, 'Richard2_left_seg_0001.jpg']);
im_right = imread([folder_right, 'Richard2_right_seg_0001.jpg']);
[m,n] = size(im_left);
figure(1);
subplot(1,2,1); imshow(im_left);
subplot(1,2,2); imshow(im_right);

%% Load sparse features in left image
if isunix
    load('~/workspaces/matlab/vfx/Data/Richard2/points_left');
else
    load('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\points_left');
end
numFeatures = size(points_left,2);
% Plot left features
figure(1); subplot(1,2,1); hold on;
plot(points_left(1,:),points_left(2,:),'r*')
% Display feature numbers
for i = 1:numFeatures
    figure(1); subplot(1,2,1); hold on;
    text(points_left(1,i),points_left(2,i),num2str(i));
end

%% Find sparse features in right image
im_left = double(im_left);
im_right = double(im_right);
points_right = points_left;

w = 5; % Image patch size = (2*w + 1) x (2*w + 1) pixels
for i = 1:numFeatures % For each feature point in left image...
    NC = NaN(1,640);
    col = round(points_left(1,i)); % x-coordinate
    row = round(points_left(2,i)); % y-coordinate
    window_left = im_left(row-w:row+w, col-w:col+w); % Left image patch
    mean_left = mean(window_left(:)); % Mean of image patch
    window_left = window_left - mean_left; % Patch shifted by mean
    std_left = std(window_left(:)); % Std dev. of image patch
    maxNC = sum(sum(window_left.*window_left))/(std_left*std_left);
    
    for j = col-20 : col+66 % For points in right image on same row...
        window_right = im_right(row-w:row+w, j-w:j+w); % Right image patch
        mean_right = mean(window_right(:)); % Mean of image patch
        window_right = window_right - mean_right; % Patch shifted by mean
        std_right = std(window_right(:)); % Std dev. of image patch
                
        % Normalised cross-correlation
        NC(j) = sum(sum(window_left.*window_right))/(std_left*std_right);
    end
    NC = NC./maxNC; 
    [val,ind] = max(NC); % Max cross-correlation
    points_right(1,i) = ind; % Store right feature point
end

% Plot right features
figure(1); subplot(1,2,2); hold on; 
plot(points_right(1,:),points_right(2,:),'r*')
% Display feature numbers
for i = 1:numFeatures
    figure(1); subplot(1,2,2); hold on;
    text(points_right(1,i),points_right(2,i),num2str(i));
end

%% Sparse 3D reconstruction
X = Reconstruct(P_left, P_right, points_left(:,:,1), points_right(:,:,1));
xsparse = X(1,:)';
ysparse = X(3,:)';
zsparse = -X(2,:)';
Xsparse = [xsparse,ysparse,zsparse];
% Display 3D points
figure;
tri = delaunay(xsparse,zsparse); % Delaunay triangulation
trisurf(tri, xsparse, ysparse, zsparse, 'LineWidth', 1.5);
axis equal; 
% Shading properties
shading interp;
colormap(cool);
light; % create a light
lighting gouraud;
material dull;
% Y-range
yrange = max(ysparse) - min(ysparse);

%% Load matte
load([folder_left, 'Richard2_left_seg_0001.mat']);
load([folder_right, 'Richard2_right_seg_0001.mat']);
alpha_left = logical(Richard2_left_seg_0001(:));
alpha_right = logical(Richard2_right_seg_0001(:));

% Find left matte boundary
Ifill_left = imfill(Richard2_left_seg_0001,'holes');
B = bwboundaries(Ifill_left);
b_left = fliplr(B{1});
bxmin_left = min(b_left(:,1));
bxmax_left = max(b_left(:,1));
centroid = regionprops(Ifill_left,'Centroid');
cx_left = centroid.Centroid(1,1); % x-coordinate of centroid
cy_left = centroid.Centroid(1,2); % y-coordinate of centroid
figure(1); subplot(1,2,1); hold on; plot(b_left(:,1),b_left(:,2),'gO','LineWidth',1); % boundary
plot(cx_left,cy_left,'bO','LineWidth',5); % centroid

% Find right matte boundary
Ifill_right = imfill(Richard2_right_seg_0001,'holes');
B = bwboundaries(Ifill_right);
b_right = fliplr(B{1});
bxmin_right = min(b_right(:,1));
bxmax_right = max(b_right(:,1));
centroid = regionprops(Ifill_right,'Centroid');
cx_right = centroid.Centroid(1,1); % x-coordinate of centroid
cy_right = centroid.Centroid(1,2); % y-coordinate of centroid
figure(1); subplot(1,2,2); hold on; plot(b_right(:,1),b_right(:,2),'gO','LineWidth',1); % boundary
plot(cx_right,cy_right,'bO','LineWidth',5); % centroid

% Compute max translation
max_t = max(abs([(bxmin_right-bxmin_left),(bxmax_right-bxmax_left)]));

%% Take a sample of pixels
numSamples = 20000; % Number of samples
ind = find(alpha_left);
[px_left(2,:),px_left(1,:)] = ind2sub([m,n],ind);
numpx = size(px_left,2);
index = randperm(numpx,numSamples);
px_left = px_left(:,index);
figure(1); subplot(1,2,1); hold on; 
plot(px_left(1,:),px_left(2,:),'b.');

%% Find corresponding features in right image
im_left = double(im_left);
im_right = double(im_right);

w = 11; % Image patch size = (2*w + 1) x (2*w + 1) pixels
% Remove points too close to edge
temp_top = (px_left(2,:)-w)<1;
px_left(:,temp_top)=[];
temp_bottom = (px_left(2,:)+w)>m;
px_left(:,temp_bottom)=[];
numSamples = size(px_left,2);

% Find min/max boundary x-coordinates
xmin = zeros(1,numSamples);
xmax = zeros(1,numSamples);
for i = 1:numSamples
    by = (b_right(:,2)==px_left(2,i));
    bx = b_right(by,1);
    xmin(i) = min(bx);
    xmax(i) = max(bx);
end

% Initialise right image points
px_right = px_left;
cx_shift = cx_right - cx_left;

for i = 1:numSamples % For each feature point in left image...
    disp(i);
    NC = NaN(1,640);
    col = px_left(1,i); % x-coordinate
    row = px_left(2,i); % y-coordinate
    window_left = im_left(row-w:row+w, col-w:col+w); % Left image patch
    mean_left = mean(window_left(:)); % Mean of image patch
    window_left = window_left - mean_left; % Patch shifted by mean
    std_left = std(window_left(:)); % Std dev. of image patch
    maxNC = sum(sum(window_left.*window_left))/(std_left*std_left);
    
    for j = col-max_t : col+max_t % For points in right image on same row...
        window_right = im_right(row-w:row+w, j-w:j+w); % Right image patch
        mean_right = mean(window_right(:)); % Mean of image patch
        window_right = window_right - mean_right; % Patch shifted by mean
        std_right = std(window_right(:)); % Std dev. of image patch
                
        % Normalised cross-correlation
        NC(j) = sum(sum(window_left.*window_right))/(std_left*std_right);
    end
%     NC = NC./maxNC; 
    
%     centre = round(col+cx_shift);
%     peak = 2;
%     m = peak/(xmax(i)-centre);
%     NC(centre:xmax(i)) = ((-m) * (0:(xmax(i)-centre)) + 1) .* NC(centre:xmax(i));
%     c = peak-m*(centre-xmin(i));
%     NC(xmin(i):(centre-1)) = ((m) * (0:(centre-xmin(i)-1)) + c) .* NC(xmin(i):(centre-1));

    [val,ind] = max(NC); % Max cross-correlation

%     While distance between feature points is greater than 70px...
%     while(abs(ind-col) > 70)
%         disp(['i = ', num2str(i)]);
%         disp(abs(ind-col));
%         NC(ind) = NaN;
%         [val,ind] = max(NC); % Choose next best...
%     end   
    
    px_right(1,i) = ind; % Store right feature point
end

%% Remove points outside of right matte
in = inpolygon(px_right(1,:),px_right(2,:),b_right(:,1)',b_right(:,2)');
px_right = px_right(:,in);
px_left = px_left(:,in);
numSamples = size(px_right,2);

% Plot right features
figure(1); subplot(1,2,2); hold on;
plot(px_right(1,:),px_right(2,:),'b.'); hold on;

%% 3D reconstruction
X = Reconstruct(P_left, P_right, px_left(:,:,1), px_right(:,:,1));
VisualiseScene(P_left, P_right, X); % Visualise
x = X(1,:)';
y = X(3,:)';
z = -X(2,:)';
X = [x,y,z];

% Remove points by depth
ind = y > (min(ysparse)-2);
px_left = px_left(:,ind);
px_right = px_right(:,ind);
x = x(ind);
y = y(ind);
z = z(ind);
X = [x,y,z];

%% Scatter plot
ind_left = sub2ind([m,n],px_left(2,:),px_left(1,:));
ind_right = sub2ind([m,n],px_right(2,:),px_right(1,:));
c_left = im_left(ind_left)';
c_right = im_right(ind_right)';
c = (c_left + c_right)./2;
c = repmat(c,1,3)./(max(c));
figure; scatter3(x,y,z,[],c,'s','flat'); axis equal; axis off;

%% Create obj
obj.vertices = X;
obj.materials = [];
obj.vertices_point = [];
obj.vertices_normal = [];
obj.vertices_texture = [];
obj.objects = [];
write_wobj(obj, 'test.obj');
