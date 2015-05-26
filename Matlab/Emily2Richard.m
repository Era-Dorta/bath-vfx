%% Emily2Richard.m
clear; close all; clc;
addpath(genpath('TPS3D'));
addpath(genpath('WOBJ_toolbox_Version2b'));

%% Load Emily blendshapes sparse
if ~(exist('OBJ', 'var'))
    numfiles = 68;
    load('blendshapes_OBJ');
end
T = OBJ{1, 1}.objects.data.vertices;
v = OBJ{1, 1}.vertices;

%% Get neutral
load('X_emily');
E_neutral = X;
% load('X_neutral');
load('Xs_neutral');
R_neutral = X;

figure(1);
trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
axis equal; axis([-15 15 -15 10 -15 15]);
view([0 90]); alpha(1.0);
hold on;
scatter3(E_neutral(:,1),E_neutral(:,2),E_neutral(:,3), 50,...
    'MarkerEdgeColor',[1 0 0],...
    'MarkerFaceColor',[1 0 0]);

% Find indeces of markers.
idx = zeros(size(E_neutral,1),1);
for i = 1:size(E_neutral,1)
    fx = OBJ{1, 1}.vertices(:, 1);
    fy = OBJ{1, 1}.vertices(:, 2);
    fz = OBJ{1, 1}.vertices(:, 3);
    valx = E_neutral(i,1);
    valy = E_neutral(i,2);
    valz = E_neutral(i,3);
    tmp = abs(fx-valx) + abs(fy-valy) + abs(fz-valz);
    [value idx(i)] = min(tmp);
end

% Store sparse blendshapes.
sparse_OBJ = cell(1, numfiles);
for j = 1:numfiles
    sparse_OBJ{j} = OBJ{1, j+1}.vertices(idx,:);
end
hold off;

%% Plot sparse points
disp('Press any key...');
waitforbuttonpress;
for i = 1:numfiles;
    v = OBJ{1, i+1}.vertices;
    figure(1);
    trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
    axis equal; axis([-15 15 -15 10 -15 15]); view([0 90]); hold on;
    scatter3(sparse_OBJ{1, i}(:,1),sparse_OBJ{1, i}(:,2),sparse_OBJ{1, i}(:,3), 50, 'MarkerEdgeColor',[1 0 0],...
        'MarkerFaceColor',[1 0 0]);
    hold off;
    pause(0.01);
end

%% Transform Emily neutral to Richard neutral using TPS
[w_neutral, param] = TPS3D(E_neutral, R_neutral, E_neutral);
figure(2);
tri = delaunay(w_neutral(:,1),w_neutral(:,2));
trisurf(tri, w_neutral(:,1), w_neutral(:,2), w_neutral(:,3), ones(1,size(w_neutral,1)), ...
        'LineWidth', 1.5); 
axis equal; axis([-60 60 -80 80 -80 0]); view([0 90]);
colormap(cool); light; lighting gouraud; material dull;

%% Transform all Emily blendshapes to Richard blendshapes using same transformation
w_blends = cell(1,numfiles);
for i = 1:numfiles
    w_blends{i} = TSP3DTransformPoints(param, E_neutral, sparse_OBJ{i});
end

% Plot transformed sparse blendshapes
disp('Press any key...');
waitforbuttonpress;
for i = 1:numfiles;
    figure(2);
    w_blend = w_blends{i};
    trisurf(tri, w_blend(:,1), w_blend(:,2), w_blend(:,3), ones(1,size(w_blend,1)), ...
        'LineWidth', 1.5); 
    axis equal; axis([-60 60 -80 80 -80 0]); view([0 90]);
    colormap(cool); light; lighting gouraud; material dull;
    hold off;
    pause(0.1);
end

%% Save
% save('w_neutral','w_neutral');
% save('w_blends','w_blends');
% save('ws_neutral','w_neutral');
% save('ws_blends','w_blends');

