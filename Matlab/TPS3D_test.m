%% TPS3D_test.m
clearvars -except OBJ; close all; clc;
% Requires TPS3D
addpath(genpath('TPS3D')); 
addpath(genpath('WOBJ_toolbox_Version2b')); 

%% Load sparse points
load('X_0300'); % Richard neutral (sparse)
load('X_emilyface'); % Victor neutral (sparse)
Xn = X;
X_emily = X_emilyface;
tri = delaunay(Xn(:,1),Xn(:,2)); % Delaunay triangulation

% Show Richard neutral (sparse)
figure(1); 
trisurf(tri, Xn(:,1),Xn(:,2),Xn(:,3), 'LineWidth', 1.5); 
axis equal; colormap(cool); light; lighting gouraud; material dull;
for i = 1:97
    text(Xn(i,1),Xn(i,2),Xn(i,3)+5,num2str(i));
end

% Show Victor neutral (sparse)
figure(2);
trisurf(tri, X_emily(:,1),X_emily(:,2),X_emily(:,3), 'LineWidth', 1.5);
axis equal; colormap(cool); light; lighting gouraud; material dull;
for i = 1:97
    text(X_emily(i,1),X_emily(i,2),X_emily(i,3)+0.1,num2str(i));
end

%% Load Victor OBJ (dense)
if ~(exist('OBJ', 'var'))
%     OBJ = read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Victor\victorHead.obj');
%     OBJ = read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Emily\Emily.obj');
    OBJ = read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Emily\EmilyFace.obj');
end
% T = OBJ.objects.data.vertices;
% T = OBJ.objects(5).data.vertices;
T = OBJ.objects(6).data.vertices;
v = OBJ.vertices;
figure(3);
trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
axis equal; axis([-15 15 -15 10 -15 15]); view([0 90]); alpha(1.0);

%% TPS3D 
% Transfofrm Richard neutral (sparse) to Victor neutral (sparse)
[w, param] = TPS3D(Xn, X_emily, Xn);
figure(4);
trisurf(tri, w(:, 1), w(:,2), w(:,3), ones(1,size(w,1)));
axis equal; alpha(1.0);

%% Brow raise
load('X_1556'); % Load Richard brow raise (sparse)
Xnew = X;
% Transform Richard brow raise (sparse) to Victor brow raise (sparse) 
wtemp = TSP3DTransformPoints(param, Xn, Xnew);
figure(5);
trisurf(tri, wtemp(:, 1), wtemp(:,2), wtemp(:,3), ones(1,size(wtemp,1)));
axis equal; alpha(1.0);

% Transform Victor brow raise (sparse) to Victor brow raise (dense)
[w2, param2] = TPS3D(X_emily, wtemp, v);
figure(6);
trisurf(T, w2(:, 1), w2(:,2), w2(:,3), ones(1,size(w2,1)));
axis equal; axis([-15 15 -15 10 -15 15]); view([0 90]); alpha(1.0);