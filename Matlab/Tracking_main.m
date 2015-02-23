%% Tracking_main.m
clear; close all;

%% Read in image sequence
folder = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard1\images_BW';
imgType = '.png';
numImgs = 500;
imArray = ReadImgs(folder, imgType);

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
numFeatures = length(f);
clear fStore;
fStore(:,:,1) = f(1:2,:);
Mov(1) = getframe;

%% Add points manually
prompt = 'Do you want to add more points? y/n: ';
str = input(prompt,'s');
if str == 'y'
    addPoint = true;
    figure(1); hold on;
    i = numFeatures + 1;
    while(addPoint == true)
        [fStore(1,i,1),fStore(2,i,1)] = ginput(1);  
        plot(fStore(1,i,1),fStore(2,i,1),'r*');
        i = i+1;
        prompt = 'Do you want to add another point? y/n: ';
        str = input(prompt,'s');
        if str == 'n'
            addPoint = false;
        end
    end
end
numFeatures = length(fStore);

%% Remove points manually
prompt = 'Do you want to remove points? y/n: ';
str = input(prompt,'s');
if str == 'y'
    removePoint = true;
    figure(1); hold on;
    i = numFeatures;
    while(removePoint == true)
        [px,py] = ginput(1);
        p = [px,py];
        k = dsearchn(fStore',p);
        fStore(:,k) = [];
        imshow(im1); plot(fStore(1,:),fStore(2,:),'r*');
        prompt = 'Do you want to remove another point? y/n: ';
        str = input(prompt,'s');
        if str == 'n'
            removePoint = false;
        end
    end
end
numFeatures = length(fStore);

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
load('vx_500.mat');
load('vy_500.mat');
vx = vx_500;
vy = vy_500;
clear vx_500 vy_500;

%% Track features
for frame = 2:numImgs
    for j = 1:numFeatures
        idx = round(fStore(1,j,frame-1));
        idy = round(fStore(2,j,frame-1));
        fStore(1,j,frame) = fStore(1,j,frame-1) + vx(idy,idx,frame-1);
        fStore(2,j,frame) = fStore(2,j,frame-1) + vy(idy,idx,frame-1);
    end
    figure(2);
    imshow(imArray{frame}); hold on; 
    plot(fStore(1,:,frame),fStore(2,:,frame),'r*');
    title(['frame: ', num2str(frame)]);
    Mov(frame) = getframe;
end

%% Play movie
figure(3); imshow(im1);
movie(Mov,3,30);

%% Delaunay triangulation
for frame = 1:numImgs
    TRI = delaunay(fStore(1,:,frame),fStore(2,:,frame));
    figure(4);
    triplot(TRI, fStore(1,:,frame), fStore(2,:,frame))
    axis([1 640 1 480]); set(gca,'YDir','reverse');
    title(['frame: ', num2str(frame)]);
    Mov2(frame) = getframe;
end

%% Play movie
figure(5); 
axis([1 640 1 480]); set(gca,'YDir','reverse');
movie(Mov2,3,60);

%% Test
for frame = 1:numImgs
    figure(6);
    imshow(imArray{frame}); hold on;
    triplot(TRI, fStore(1,:,frame), fStore(2,:,frame))
    title(['frame: ', num2str(frame)]);
    Mov3(frame) = getframe;
end

%% Play movie
figure(7); imshow(im1);
movie(Mov3,3,30);