%% OpticalFlow_main.m
clear all; close all;

%% Read in image sequence
if isunix
    folder = '~/workspaces/matlab/vfx/Data/Richard1/images_BW/';
else
    folder = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Calib1\Right\images';
end
imgType = '.png';
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
vx = zeros(480, 640, numImgs-1);
vy = vx;

disp('Computing optical flow...');
for i = 1:(numImgs-1)
    disp(i);
    [vx(:,:,i),vy(:,:,i)] = Coarse2FineTwoFrames(imArray{i},imArray{i+1},para);
end
disp('Done!');

%%  Save optical flow
% save([folder '\vx'], 'vx');
% save([folder '\vy'], 'vy');