%% Load poses.
clearvars -except OBJ numfiles; close all; clc;


if ~(exist('OBJ', 'var'))
    numfiles = 68;
    load('blendshapes_OBJ');
%     OBJ = cell(1, numfiles);
% 
%     for k = 0:numfiles
%       myfilename = sprintf('shapeBS/E_%d.obj', k);
%       OBJ{k+1} = read_wobj(myfilename);
%     end
end

%% Get neutral and displacements.

num_vertices = size(OBJ{1, 1}.vertices,1);
neutral_pose = reshape(OBJ{1, 1}.vertices, 3*num_vertices, 1);

key_poses = zeros(num_vertices*3,numfiles-1);
key_disp = zeros(num_vertices*3,numfiles-1);
for i = 1:numfiles
    key_poses(:,i) = reshape(OBJ{1, i+1}.vertices, 3*num_vertices, 1);
    key_disp(:,i) =  bsxfun(@minus, key_poses(:,i), neutral_pose);
end

%% TODO Get neutral and displacements for sparse shapes.
load('Emily_97');
sparse_neutral = v_store';

T = OBJ{1, 1}.objects.data.vertices;
v = OBJ{1, 1}.vertices;
figure;
trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
axis equal; % axis([-15 15 -15 10 -15 15]); 
view([0 90]); alpha(1.0);
hold on
scatter3(v_store(1,:),v_store(2,:),v_store(3,:), 50, 'MarkerEdgeColor',[1 0 0],...
            'MarkerFaceColor',[1 0 0]);

% Find indeces of markers.
idx = zeros(size(sparse_neutral,1),1);
for i = 1:size(sparse_neutral,1)
    fx = OBJ{1, 1}.vertices(:, 1);
    fy = OBJ{1, 1}.vertices(:, 2);
    fz = OBJ{1, 1}.vertices(:, 3);
    valx = sparse_neutral(i,1);
    valy = sparse_neutral(i,2);
    valz = sparse_neutral(i,3);
    tmp = abs(fx-valx) + abs(fy-valy) + abs(fz-valz);
    [value idx(i)] = min(tmp);
end

% Store sparse blendshapes.
sparse_OBJ = cell(1, numfiles);
for j = 1:numfiles
    sparse_OBJ{j} = OBJ{1, j+1}.vertices(idx,:);
end

%% Plot sparse points.
k = 10;
T = OBJ{1, 10+1}.objects.data.vertices ;
v = OBJ{1, k+1}.vertices;
figure;
trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
axis equal; % axis([-15 15 -15 10 -15 15]); 
view([0 90]); alpha(1.0);
hold on
scatter3(sparse_OBJ{1, k}(:,1),sparse_OBJ{1, k}(:,2),sparse_OBJ{1, k}(:,3), 50, 'MarkerEdgeColor',[1 0 0],...
            'MarkerFaceColor',[1 0 0]);

%% CHOOSE new frame/pose.
OBJ_new = read_wobj('Emily_blendshapes/E_new8.obj');
sparse_new = OBJ_new.vertices(idx,:);

% x = reshape(OBJ_new.vertices, 3*num_vertices, 1);
num_sparse_vertices = size(v_store,2);
x = reshape(sparse_new, 3*num_sparse_vertices, 1);
sparse_neutral = reshape(sparse_neutral, 3*num_sparse_vertices, 1);
displacement = bsxfun(@minus, x, sparse_neutral);

sparse_key_poses = zeros(num_sparse_vertices*3,numfiles);
sparse_key_disp = zeros(num_sparse_vertices*3,numfiles);
for i = 1:numfiles
    sparse_key_poses(:,i) = reshape(sparse_OBJ{1,i}, 3*num_sparse_vertices, 1);
    sparse_key_disp(:,i) =  bsxfun(@minus, sparse_key_poses(:,i), sparse_neutral);
end
%%  Solve for weights.
w1 = sparse_key_poses\x;

options = optimset('Display','notify','TolX',1e-10);
[w2,resnorm] = lsqnonneg(sparse_key_poses,x,options);

% Aeq = ones(1, numfiles);
% beq = 1;
lb = 0*ones(2*num_sparse_vertices,1);
ub = 1*ones(2*num_sparse_vertices,1);
[w3,resnorm2] = lsqlin(sparse_key_disp, displacement,[], [], [],[],lb,ub);

% OBJ_blend = OBJ_new;
% v_temp = bsxfun(@plus, sparse_key_disp*w1, sparse_neutral);
% OBJ_blend.vertices = reshape( v_temp, num_sparse_vertices,3);
% OBJ_blend2 = OBJ_new;
% v_temp = bsxfun(@plus, sparse_key_disp*w2, sparse_neutral);
% OBJ_blend2.vertices = reshape( v_temp, num_sparse_vertices,3);
% OBJ_blend3 = OBJ_new;
% v_temp = bsxfun(@plus, sparse_key_disp*w3, sparse_neutral);
% OBJ_blend3.vertices = reshape( v_temp, num_sparse_vertices,3);
%% Plot.

OBJ_blend = OBJ_new;
v_temp = bsxfun(@plus, key_disp*w1, neutral_pose);
OBJ_blend.vertices = reshape( v_temp, num_vertices,3);
OBJ_blend2 = OBJ_new;
v_temp = bsxfun(@plus, key_disp*w2, neutral_pose);
OBJ_blend2.vertices = reshape( v_temp, num_vertices,3);
OBJ_blend3 = OBJ_new;
v_temp = bsxfun(@plus, key_disp*w3, neutral_pose);
OBJ_blend3.vertices = reshape( v_temp, num_vertices,3);


T = OBJ_blend.objects(1,5).data.vertices;
v = OBJ_blend.vertices;
v2 = OBJ_blend2.vertices;
v3 = OBJ_blend3.vertices;
v_old = OBJ_new.vertices;
v_neutral = OBJ{1, 1}.vertices;

figure(1);
trisurf(T, v(:,1),v(:,2),v(:,3), ones(1,size(v,1)));
view([0 90]);axis equal
figure(2);
trisurf(T, v2(:,1),v2(:,2),v2(:,3), ones(1,size(v,1)));
view([0 90]);axis equal
figure(3);
trisurf(T, v3(:,1),v3(:,2),v3(:,3), ones(1,size(v,1)));
view([0 90]);axis equal
figure(4);
trisurf(T, v_old(:,1),v_old(:,2),v_old(:,3), ones(1,size(v,1)));
view([0 90]);axis equal
figure(5);
trisurf(T, v_neutral(:,1),v_neutral(:,2),v_neutral(:,3), ones(1,size(v,1)));
view([0 90]);axis equal

%% Save data as txt.
for i = 1:size(w3,1)
    if w3(i) < 1e-3
        w3(i) = 0;
    end
end
% myfilename2 = sprintf('data/weights_%d.txt', k);

save data/weights_1.txt w3 -ASCII