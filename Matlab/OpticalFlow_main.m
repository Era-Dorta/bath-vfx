%% OpticalFlow_main.m
clear all; close all;

% Requires C. Lui's optical flow code
addpath(genpath('optical_flow_cliu'));

%% Read in image sequence
if isunix
    folder = '~/workspaces/matlab/vfx/Data/Richard1/images_BW/';
else
    folder = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Right\images_rect';
end
imgType = '.jpg';
numImgs = 5;
imArray = ReadImgs(folder, imgType, numImgs);

%% Set optical flow parameters (C.Lui)
alpha = 0.012;
ratio = 0.75;
minWidth = 20;
nOuterFPIterations = 7;
nInnerFPIterations = 1;
nSORIterations = 30;
para = [alpha,ratio,minWidth,nOuterFPIterations,...
        nInnerFPIterations,nSORIterations];

%% Compute optical flow
Richard2_right_vx500 = zeros(480, 640, numImgs-1);
Richard2_right_vy500 = Richard2_right_vx500;

disp('Computing optical flow...');
for i = 1:(numImgs-1)
    disp(i);
    [Richard2_right_vx500(:,:,i),Richard2_right_vy500(:,:,i)] = Coarse2FineTwoFrames(imArray{i},imArray{i+1},para);
end
disp('Done!');

%%  Save optical flow
% save([folder '\vx'], 'vx');
% save([folder '\vy'], 'vy');