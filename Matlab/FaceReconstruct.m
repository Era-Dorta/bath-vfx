%% FaceReconstruct.m
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
clearvars -except P_left P_right K_left K_right R t;

% Load feature points
if isunix
    folder_left = '~/workspaces/matlab/vfx/Data/Richard2/Left/expressions/'; % Left image folder
    folder_right = '~/workspaces/matlab/vfx/Data/Richard2/Right/expressions/'; % Right image folder
else
    folder_left = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Left\expressions\';
    folder_right = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Right\expressions\';
end
frame = 2270;
% load([folder_left, 'points_left_' num2str(frame,'% 05.f') '.mat']);
% load([folder_right,'points_right_' num2str(frame,'% 05.f') '.mat']);
load(['points_left_' num2str(frame,'% 05.f') '.mat']);
load(['points_right_' num2str(frame,'% 05.f') '.mat']);
numFeatures = size(points_left,2);

%% 3D reconstruction
X = Reconstruct(P_left, P_right, points_left, points_right);
x_offset = X(1,54); % Keep nose position at x = 0
y_offset = -X(2,54); % Keep nose position at y = 0
z_offset = -X(3,54); % Keep nose position at z = 0
x = X(1,:)-x_offset;
y = -X(2,:)-y_offset;
z = -X(3,:)-z_offset;
X = [x;y;z]';
tri = delaunay(x,y); % Delaunay triangulation
figure(1); 
trisurf(tri, x, y, z, 'LineWidth', 1.5); axis equal; 
colormap(cool); light; lighting gouraud; material dull;

%% Load Victor
% if ~(exist('OBJ', 'var'))
%     OBJ=read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Victor\victorHead.obj');
% end
% T = OBJ.objects.data.vertices;
% v = OBJ.vertices;
% figure(2);
% trisurf(T(2:end,:), v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
% axis equal; alpha(0.5);
