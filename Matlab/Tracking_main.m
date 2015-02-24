%% Tracking_main.m
clearvars -except vx vy; close all;

%% Read in image sequence
if isunix
    folder = '~/workspaces/matlab/vfx/Data/Richard1/images_BW/';
else
    folder = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard1\images_BW';
end
imgType = '.png';
numImgs = 500;
imArray = ReadImgs(folder, imgType, numImgs);

%% SIFT
im1 = imArray{1};
im1_sift = single(im1);

% Select region of interest
disp('Select region of interest');
M = single(roipoly(im1));
im1_sift = im1_sift.*M;

% Detect features
im1 = imArray{1};
[f,d] = vl_sift(im1_sift, 'Octaves', 3, 'Levels', 5, 'FirstOctave', -1,...
    'PeakThresh', 6, 'EdgeThresh', 6, 'NormThresh', 1, ...
    'Magnif', 1, 'WindowSize', 10);

% Plot features
figure(1); hold on;
vl_plotframe(f(:,:));
plot(f(1,:),f(2,:),'r*');

% Store features
numFeatures = size(f,2);
clear fStore;
fStore(:,:,1) = f(1:2,:);

%% Add points manually
disp('Adding points, right click to exit');
addPoint = true;
figure(1); hold on;
i = numFeatures + 1;
while(true)
    [fStore(1,i,1), fStore(2,i,1), button] = ginput(1);
    if button ~= 1
        break;
    end
    plot(fStore(1,i,1),fStore(2,i,1),'r*');
    i = i+1;
end
numFeatures = size(fStore,2);
disp('Finished adding points');

%% Remove points manually
disp('Removing points, right click to exit');
removePoint = true;
figure(1); hold on;
i = numFeatures;
while(true)
    [px, py, button] = ginput(1);
    if button ~= 1
        break;
    end
    p = [px,py];
    k = dsearchn(fStore',p);
    fStore(:,k) = [];
    imshow(im1); plot(fStore(1,:),fStore(2,:),'r*');
end
numFeatures = size(fStore,2);
disp('Finished removing points');

%% Optical flow
% C.Lui
% flow = estimate_flow_interface(im1, im2, 'classic+nl-fast');
% figure(2); plotflow(flow);

% M.Black
% Set optical flow parameters
% alpha = 0.012;
% ratio = 0.75;
% minWidth = 20;
% nOuterFPIterations = 7;
% nInnerFPIterations = 1;
% nSORIterations = 30;
% para = [alpha,ratio,minWidth,nOuterFPIterations,...
%         nInnerFPIterations,nSORIterations];
%
% vx = zeros(size(im1_sift));
% vy = vx;
% for i = 1:(numImgs-1)
%     disp(i);
%     [vx(:,:,i),vy(:,:,i)] = Coarse2FineTwoFrames(imArray{i},imArray{i+1},para);
%     % flow(:,:,1) = vx; flow(:,:,2) = vy;
% end

%% Load optical flow
if ~(exist('vx', 'var') && exist('vy', 'var'))
    disp('Loading vx and vy');
    load('vx_500.mat');
    load('vy_500.mat');
    vx = vx_500;
    vy = vy_500;
    clear vx_500 vy_500;
end

%% Track features
disp('Tracking features');
newImg = imArray{1};
for j = 1:numFeatures
    newImg(round(fStore(2,j,1)) - 2:2 + round(fStore(2,j,1)), ...
        round(fStore(1,j,1)) - 2:round(fStore(1,j,1)) + 2 ) = 255;
end

Mov(1) = im2frame(newImg, gray(256));
for frame = 2:numImgs
    newImg = imArray{frame};
    for j = 1:numFeatures
        idx = round(fStore(1,j,frame-1));
        idy = round(fStore(2,j,frame-1));
        fStore(1,j,frame) = fStore(1,j,frame-1) + vx(idy,idx,frame-1);
        fStore(2,j,frame) = fStore(2,j,frame-1) + vy(idy,idx,frame-1);
        
        newImg(round(fStore(2,j,frame)) - 2:2 + round(fStore(2,j,frame)), ...
            round(fStore(1,j,frame)) - 2:round(fStore(1,j,frame)) + 2 ) = 255;
    end
    Mov(frame) = im2frame(newImg, gray(256));
end

%% Play movie
figure(2); imshow(im1);
movie(Mov,1,30);

%% Delaunay triangulation
TRI = delaunay(fStore(1,:,1),fStore(2,:,1));
for frame = 1:numImgs
    figure(3);
    triplot(TRI, fStore(1,:,frame), fStore(2,:,frame))
    axis([1 640 1 480]); set(gca,'YDir','reverse');
    Mov2(frame) = getframe;
end

%% Play movie
figure(4);
axis([1 640 1 480]); set(gca,'YDir','reverse');
movie(Mov2,1,60);

%% Test
for frame = 1:numImgs
    figure(5);
    imshow(imArray{frame}); hold on;
    triplot(TRI, fStore(1,:,frame), fStore(2,:,frame))
    Mov3(frame) = getframe;
end

%% Play movie
figure(6); imshow(im1);
movie(Mov3,1,30);