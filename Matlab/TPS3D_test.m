%% TPS3D_test.m
clearvars -except OBJ; close all; clc;
% Requires TPS3D
addpath(genpath('TPS3D')); 
addpath(genpath('WOBJ_toolbox_Version2b')); 

%% Load sparse points
load('X_neutral'); % Richard neutral (sparse)
load('X_victor'); % Victor neutral (sparse)
tri = delaunay(X(:,1),X(:,2)); % Delaunay triangulation

% Show Richard neutral (sparse)
figure(1); 
trisurf(tri, X(:,1),X(:,2),X(:,3), 'LineWidth', 1.5); 
axis equal; colormap(cool); light; lighting gouraud; material dull;
for i = 1:97
    text(X(i,1),X(i,2),X(i,3)+5,num2str(i));
end

% Show Victor neutral (sparse)
figure(2);
trisurf(tri, X_victor(:,1),X_victor(:,2),X_victor(:,3), 'LineWidth', 1.5);
axis equal; colormap(cool); light; lighting gouraud; material dull;
for i = 1:97
    text(X_victor(i,1),X_victor(i,2),X_victor(i,3)+0.1,num2str(i));
end

%% Load Victor OBJ (dense)
if ~(exist('OBJ', 'var'))
    OBJ = read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Victor\victorHead.obj');
end
T = OBJ.objects.data.vertices;
v = OBJ.vertices;
figure(3);
trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
axis equal; alpha(1.0);

%% TPS3D 
% Transfofrm Richard neutral (sparse) to Victor neutral (sparse)
[w, param] = TPS3D(X, X_victor, X);
figure(4);
trisurf(tri, w(:, 1), w(:,2), w(:,3), ones(1,size(w,1)));
axis equal; alpha(1.0);

%% Brow raise
load('X_browsup'); % Load Richard brow raise (sparse)
% Transform Richard brow raise (sparse) to Victor brow raise (sparse) 
wtemp = TSP3DTransformPoints(param, X, X_browsup);
figure(5);
trisurf(tri, wtemp(:, 1), wtemp(:,2), wtemp(:,3), ones(1,size(wtemp,1)));
axis equal; alpha(1.0);

% Transform Victor brow raise (sparse) to Victor brow raise (dense)
[w2, param2] = TPS3D(X_victor, wtemp, v);
figure(6);
trisurf(T, w2(:, 1), w2(:,2), w2(:,3), ones(1,size(w2,1)));
axis equal; alpha(1.0);