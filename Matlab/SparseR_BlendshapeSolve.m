%% SparseR_BlendshapeSolve.m
clear all; close all; clc;

%% Read in sparse neutrals
% load('X_neutral');
load('Xs_neutral');
R_neutral = X;

load('X_emily');
E_neutral = X;

%% Read in sparse R blendshapes
load('ws_neutral');
load('ws_blends');

%% Load sequence
% load('X_sequence');
load('Xs_sequence2');
numFrames = size(X_sequence,3);

%% Solve for weight on each frame
w_neutral = reshape(w_neutral, 3*97, 1);
key_poses = zeros(3*97,68);
key_disp = zeros(3*97,68);
for i = 1:68
    key_poses(:,i) = reshape(w_blends{i}, 3*97, 1);
    key_disp(:,i) =  bsxfun(@minus, key_poses(:,i), w_neutral);
    
    if (i == 12 || i ==13 || i ==14 || i ==15 || i ==18 || i ==19 )
        key_disp(:,i) = zeros(3*97,1);
    end
end

lb = zeros(68,1);
ub = 1*ones(68,1);
ub(1:7) = 2;
ub(8:9) = 1.5;
weights = zeros(68,numFrames);
options = optimset('Display','notify','TolX',1e-15);
weight_array = [];

for frame = 1:numFrames
    if (frame > 2500 && frame < 2660)
       ub(24) = 0.1; 
       ub(25) = 0.1;
       ub(37) = 0.5;
       ub(22) = 0.1;
    elseif (frame > 2670 && frame < 2710)
       ub(7) = 0.1;
       ub(43) = 0.1; 
       ub(37) = 0.1;
    else 
        ub = 1*ones(68,1);
        ub(1:7) = 2;
        ub(8:9) = 1.5;   
    end
    

    
% for frame = 1:400
    X_frame = X_sequence(:,:,frame);
    X_frame = reshape(X_frame, 3*97, 1);
    displacement = bsxfun(@minus, X_frame, w_neutral);
    
    % Solve for weights
%     weights(:,frame) = key_poses\X_frame;
%     [weights(:,frame),resnorm2] = lsqlin(key_poses,X_frame,[],[],[],[],lb,ub);
    [weights(:,frame),resnorm2] = lsqlin(key_disp,displacement,[],[],[],[],lb,ub);
%     [weights(:,frame),resnorm] = lsqnonneg(key_disp,displacement,options);
%     [weights(:,frame),resnorm] = lsqnonneg(key_poses,X_frame,options);
    weight_array = [weight_array; weights(:,frame)];           
end
% save weights.txt weight_array -ASCII

%% Apply weights to sparse sequence
w_sequence = zeros(size(X_sequence));
for frame = 1:numFrames
    w_temp = bsxfun(@plus, key_disp*weights(:,frame), w_neutral);
    w_sequence(:,:,frame) = reshape(w_temp,97,3);
end

%% Plot Sequence
for frame = 1:numFrames
    figure(1);
    scatter3(X_sequence(:,1,frame),X_sequence(:,2,frame),X_sequence(:,3,frame), ...
        30, 'MarkerEdgeColor',[1 0 0]);
    axis equal; axis([-60 60 -80 80 -60 0]); view([0 90]); 
    hold on;
   
    scatter3(w_sequence(:,1,frame),w_sequence(:,2,frame),w_sequence(:,3,frame), ...
        10, 'MarkerEdgeColor',[0 0 1], 'MarkerFaceColor',[0 0 1]);
    hold off;
end

%% Apply same weights to dense Emily blendshape model
load('blendshapes_OBJ');
T = OBJ{1, 1}.objects.data.vertices;
numFiles = 68;

% Dense blendshapes
numVertices = size(OBJ{1, 1}.vertices,1);
dense_neutral = reshape(OBJ{1, 1}.vertices, 3*numVertices, 1);
dense_key_poses = zeros(numVertices*3,numFiles);
dense_key_disp = zeros(numVertices*3,numFiles);
for i = 1:numFiles
    dense_key_poses(:,i) = reshape(OBJ{1, i+1}.vertices, 3*numVertices, 1);
    dense_key_disp(:,i) =  bsxfun(@minus, dense_key_poses(:,i), dense_neutral);
end

dense_sequence = zeros(numVertices,3,numFrames);
for frame = 1:numFrames
    dense_temp = bsxfun(@plus, dense_key_disp*weights(:,frame), dense_neutral);
    dense_sequence(:,:,frame) = reshape(dense_temp,numVertices,3);
end

%% Visualise
for frame = 1:numFrames
    figure(2);
    trisurf(T, dense_sequence(:,1,frame),dense_sequence(:,2,frame),dense_sequence(:,3,frame), ones(1,size(dense_sequence,1)));
    axis equal; axis([-15 15 -15 10 -15 15]); view([0 90]);
end

%% Apply same weights to dense Richard blendshape model
load('blendshapes_R_OBJ');
T_R = R_OBJ{1, 1}.objects(5).data.vertices;
numFiles = 68;

% Dense blendshapes
numVertices_R = size(R_OBJ{1, 1}.vertices,1);
dense_neutral_R = reshape(R_OBJ{1, 1}.vertices, 3*numVertices_R, 1);
dense_key_poses_R = zeros(numVertices_R*3,numFiles);
dense_key_disp_R = zeros(numVertices_R*3,numFiles);
for i = 1:numFiles
    dense_key_poses_R(:,i) = reshape(R_OBJ{1, i+1}.vertices, 3*numVertices_R, 1);
    dense_key_disp_R(:,i) =  bsxfun(@minus, dense_key_poses_R(:,i), dense_neutral_R);
end

dense_sequence_R = zeros(numVertices_R,3,numFrames);
for frame = 1:numFrames
    dense_temp_R = bsxfun(@plus, dense_key_disp_R*weights(:,frame), dense_neutral_R);
    dense_sequence_R(:,:,frame) = reshape(dense_temp_R,numVertices_R,3);
end

%% Visualise
for frame = 1:numFrames
    figure(3);
    trisurf(T_R, dense_sequence_R(:,1,frame),dense_sequence_R(:,2,frame),dense_sequence_R(:,3,frame), ones(1,size(dense_sequence_R,1)));
    axis equal; axis([-10 10 -10 10 -10 10]); view([0 90]);
end
