    %% FaceReconstructSequence.m
clearvars -except OBJ; close all; clc;
% Requires TPS3D
addpath(genpath('TPS3D'));
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

% Load feature point sequence
firstFrame = 5150;
lastFrame = 5880;
load('points_left_5150_5880');
load('points_right_5150_5880');
numFeatures = size(points_left,2);
numFrames = size(points_left,3);

%% Reconstruct sequence
% Appy epipolar constraint
for frame = 1:numFrames
    points_left(2,:,frame) = (points_left(2,:,frame) + points_right(2,:,frame))./2; % Set y-coordinate to average
    points_right(2,:,frame) = points_left(2,:,frame);
end

% Neutral face
% load('X_neutral');
load('Xs_neutral');
Xn = X;
tri = delaunay(Xn(:,1),Xn(:,2)); % Delaunay triangulation
figure(1);
trisurf(tri, Xn(:,1), Xn(:,2), Xn(:,3), 'LineWidth', 1.5); axis equal; view([0 90]);
colormap(cool); light; lighting gouraud; material dull;

for frame = 1:numFrames
    Xtemp = Reconstruct(P_left, P_right, points_left(:,:,frame), points_right(:,:,frame));
    x_offset = Xtemp(1,54); % Keep nose position at x = 0
    y_offset = -Xtemp(2,54); % Keep nose position at y = 0
    z_offset = -Xtemp(3,54); % Keep nose position at z = 0
    x = Xtemp(1,:)-x_offset;
    y = -Xtemp(2,:)-y_offset;
    z = -Xtemp(3,:)-z_offset;
    X(:,:,frame) = [x;y;z]';
    figure(1);
    tri = delaunay(X(:,1),X(:,2)); % Delaunay triangulation
    trisurf(tri, x, y, z, 'LineWidth', 1.5); 
    axis equal; axis([-60 60 -80 80 -80 0]); view([0 90]);
    colormap(cool); light; lighting gouraud; material dull;
end
X_sequence = X;

%% Procrustes
% load('X_neutral_rigid');
% Xn_rigid = X;
ind = [98,99,53,54,44];
Xn = X;
Xn_rigid = Xn(ind,:);

err = zeros(1,numFrames);
for frame = 1:numFrames
    Xnew = X_sequence(1:97,:,frame);
    Xnew_rigid = X_sequence(ind,:,frame);
%     [err(frame), transform] = FaceStabilise(Xn_rigid, Xnew_rigid);
%     [err(frame), Z, transform] = procrustes(Xn(rigidPoints,:), Xnew(rigidPoints,:));
    [err(frame), Z, transform] = procrustes(Xn_rigid, Xnew_rigid);
    X_sequence(1:97,:,frame) = transform.b * Xnew * transform.T + repmat(transform.c(1,:),numFeatures,1);
    figure(1);
    trisurf(tri,X_sequence(:,1,frame),X_sequence(:,2,frame),X_sequence(:,3,frame),...
        'LineWidth', 1.5); 
    axis equal; axis([-60 60 -80 80 -80 0]); view([0 90]);
    colormap(cool); light; lighting gouraud; material dull;
end
avgerr = sum(err)/(numFrames);
disp(['Average error = ', num2str(avgerr)]);

%% Smooth point trajectory
X_sequence = X_sequence(1:97,:,:);
for i = 1:numFeatures
    X_sequence(i,1,:) = smooth(X_sequence(i,1,:));
    X_sequence(i,2,:) = smooth(X_sequence(i,2,:));
    X_sequence(i,3,:) = smooth(X_sequence(i,3,:));
end

for frame = 1:numFrames
    figure(1);
    if frame == 1
        tri = delaunay(X_sequence(:,1,frame),X_sequence(:,2,frame));
    end        
    trisurf(tri,X_sequence(:,1,frame),X_sequence(:,2,frame),X_sequence(:,3,frame),...
        'LineWidth', 1.5); 
    axis equal; axis([-60 60 -80 80 -80 0]); view([0 90]);
    colormap(cool); light; lighting gouraud; material dull;
end

%% Transform sparse points
load('X_sequence');
load('X_neutral'); % Richard neutral (sparse)
Xn = X; 
load('X_emily'); % Emily neutral (sparse)
X_emily = X;
[w, param] = TPS3D(Xn, X_emily, Xn);

for frame = 1:numFrames
% for frame = 1
    w(:,:,frame) = TSP3DTransformPoints(param, Xn, X_sequence(:,:,frame));
    figure(2);
    trisurf(tri, w(:,1,frame), w(:,2,frame), w(:,3,frame), ones(1,size(w,1)), ...
        'LineWidth', 1.5); 
    axis equal; axis([-10 10 -15 10 -5 5]); view([0 90]);
    colormap(cool); light; lighting gouraud; material dull;
end

%% Transform dense mesh
saveon = false;
% Load Emily OBJ (dense)
if ~(exist('OBJ', 'var'))
    OBJ = read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Emily\EmilyFace.obj');
%     OBJ = read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Emily\shapeBS\E_nomouth\E_nomouth.0001.obj');
end
T = OBJ.objects(5).data.vertices;
v = OBJ.vertices;

for frame = 1:numFrames
    [w2, param2] = TPS3D(X_emily, w(:,:,frame), v);
    figure(3);
    trisurf(T, w2(:, 1), w2(:,2), w2(:,3), ones(1,size(w2,1)));
    axis equal; axis([-15 15 -15 10 -15 15]); view([0 90]); alpha(1.0);
    % Save OBJ
    if saveon == true
        OBJnew = OBJ;
        OBJnew.vertices = w2;
        write_wobj(OBJnew, ['Emily_', num2str(frame,'% 05.f'), '.obj']);
    end
end
