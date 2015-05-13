%% ExpressionReconstruct.m
clear; close all; clc;
addpath(genpath('WOBJ_toolbox_Version2b'));

%% Initialise
% Load calibration data
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
clearvars -except P_left P_right K_left K_right R t OBJ;

% Specify image folders
if isunix
    folder_left = '~/workspaces/matlab/vfx/Data/Richard2/Left/images_rect/'; % Left image folder
    folder_right = '~/workspaces/matlab/vfx/Data/Richard2/Right/images_rect/'; % Right image folder
else
    folder_left = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Left\expressions\';
    folder_right = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Right\expressions\';
end
imgType = 'jpg';
sDir_left =  dir( fullfile(folder_left ,['*' imgType]) );
sDir_right =  dir( fullfile(folder_right ,['*' imgType]) );
numImgs = size(sDir_left, 1);

% % Load feature points
% num = 300;
% load(['points_left_', num2str(num,'% 05.f')]);
% load(['points_right_', num2str(num,'% 05.f')]);
% numFeatures = size(points_left,2);
% 
% % Appy epipolar constraint
% points_left(2,:) = (points_left(2,:) + points_right(2,:))./2; % Set y-coordinate to average
% points_right(2,:) = points_left(2,:);
% 
% im_left = imread([folder_left, 'Richard2_left_rect_', num2str(num,'% 05.f'), '.jpg']);
% im_right = imread([folder_right, 'Richard2_right_rect_', num2str(num,'% 05.f'), '.jpg']);
% [height, width] = size(im_left);
% figure(1); imshow([im_left,im_right],[]); hold on;
% 
% % Plot features
% plot(points_left(1,:),points_left(2,:),'yO');
% plot(points_right(1,:)+width,points_right(2,:),'yO');
% % Display feature numbers
% for i = 1:numFeatures
%     text(points_left(1,i),points_left(2,i),num2str(i));
%     text(points_right(1,i)+width,points_right(2,i),num2str(i));
% end
% points_left = ReEstimatePoints(im_left, points_left, points_left);
% points_right = ReEstimatePoints(im_right, points_right, points_right);
% 
% % Epipolar constraint
% points_left(2,:) = (points_left(2,:) + points_right(2,:))./2; 
% points_right(2,:) = points_left(2,:);
% 
% figure(2); imshow([im_left,im_right],[]); hold on;
% % Plot features
% plot(points_left(1,:),points_left(2,:),'yO');
% plot(points_right(1,:)+width,points_right(2,:),'yO');
% % Display feature numbers
% for i = 1:numFeatures
%     text(points_left(1,i),points_left(2,i),num2str(i));
%     text(points_right(1,i)+width,points_right(2,i),num2str(i));
% end
% 
% %% Select rigid points
% figure(3); imshow(im_left,[]); hold on;
% [x,y] = ginput(2);
% p_left = [x';y'];
% p_left = ReEstimatePoints(im_left, p_left, p_left);
% % ind = [54,53,38,59,9,44,84];
% % rigidPoints_left = [p_left, points_left(:,ind)];
% plot(p_left(1,:),p_left(2,:),'yO');
% 
% figure(4); imshow(im_right,[]); hold on;
% [x,y] = ginput(2);
% p_right = [x';y'];
% p_right = ReEstimatePoints(im_right, p_right, p_right);
% % ind = [54,53,38,59,9,44,84];
% % rigidPoints_right = [p_right, points_right(:,ind)];
% plot(p_right(1,:),p_right(2,:),'yO');
% 
% % Epipolar constraint
% p_left(2,:) = (p_left(2,:) + p_right(2,:))./2; 
% p_right(2,:) = p_left(2,:);
% 
% %% Save points
% save(['points_left_', num2str(num,'% 05.f')],'points_left');
% save(['points_right_', num2str(num,'% 05.f')],'points_right');
% save(['p_left_', num2str(num,'% 05.f')],'p_left');
% save(['p_right_', num2str(num,'% 05.f')],'p_right');
% disp('saved');

%% Reconstruct
load('X_neutral_with_ears'); Xn = X;
tri = delaunay(Xn(:,1),Xn(:,2)); % Delaunay triangulation
figure;
trisurf(tri, Xn(:,1), Xn(:,2), Xn(:,3), 'LineWidth', 1.5);
axis equal; axis([-90 90 -80 80 -150 0]); view([0 90]);
colormap(cool); light; lighting gouraud; material dull;

% Load feature points
num = 8990;
load(['points_left_', num2str(num,'% 05.f')]);
load(['points_right_', num2str(num,'% 05.f')]);
load(['p_left_', num2str(num,'% 05.f')]);
load(['p_right_', num2str(num,'% 05.f')]);
numFeatures = size(points_left,2);

allPoints_left = [points_left,p_left];
allPoints_right = [points_right,p_right];

Xtemp = Reconstruct(P_left, P_right, allPoints_left, allPoints_right);
x_offset = Xtemp(1,54); % Keep nose position at x = 0
y_offset = -Xtemp(2,54); % Keep nose position at y = 0
z_offset = -Xtemp(3,54); % Keep nose position at z = 0
x = Xtemp(1,:)-x_offset;
y = -Xtemp(2,:)-y_offset;
z = -Xtemp(3,:)-z_offset;
X = [x;y;z]';
% figure;
% trisurf(tri, x, y, z, 'LineWidth', 1.5);
% axis equal; axis([-90 90 -80 80 -150 0]); view([0 90]);
% colormap(cool); light; lighting gouraud; material dull;

%% Procrustes
ind = [98,99,53,54,44,48];
% ind = [98,99,54,53,48,21,80];
% ind = [98,99,54,53,48,21,80,30,69];
% ind = [98,99,54,53,38,59];
% ind = [54,53,38,59];

Xnew = X;
Xnew_rigid = X(ind,:);
Xn_rigid = Xn(ind,:);

[err, Z, transform] = procrustes(Xn_rigid, Xnew_rigid);
disp(err)
Xt = transform.b * Xnew * transform.T + repmat(transform.c(1,:),99,1);
figure;
trisurf(tri,Xt(:,1),Xt(:,2),Xt(:,3),'LineWidth', 1.5); 
axis equal; axis([-90 90 -80 80 -150 0]); view([0 90]);
colormap(cool); light; lighting gouraud; material dull;
