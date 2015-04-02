%% SelectMeshPoints.m
clearvars -except OBJ; close all; clc
addpath(genpath('WOBJ_toolbox_Version2b'));
addpath(genpath('SpringLab'));

%% Load mesh
if ~(exist('OBJ', 'var'))
    OBJ = read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Emily\EmilyFace.obj');
end
T = OBJ.objects(6).data.vertices;
v = OBJ.vertices;

%% Show mesh
figure('units','normalized','outerposition',[0 0 1 1]);
trisurf(T(2:end,:), v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
hold on; axis equal; view([0 90]);

%% Select vertices
num_vertices = 97;
v_store = zeros(num_vertices,3);
disp('Click anywhere on the surface, then hit return');
for i = 10
    pause;
    [p,vn] = select3d;
    v_store(i,:) = vn;
    scatter3(vn(1),vn(2),vn(3)+0.5, 30, 'MarkerEdgeColor',[1 0 0],...
                                    'MarkerFaceColor',[1 0 0]);
    text(vn(1),vn(2),vn(3)+0.5, num2str(i), 'FontSize', 20)
    disp(i);
end

%% Load already selected points
figure('units','normalized','outerposition',[0 0 1 1]);
if ~(exist('OBJ', 'var'))
    OBJ = read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Emily\EmilyFace.obj');
end
T = OBJ.objects(6).data.vertices;
v = OBJ.vertices;
trisurf(T(2:end,:), v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
hold on; axis equal; view([0 90]);
load('X_emilyface.mat');
X = X_emilyface;
for i = 1:97
    scatter3(X(i,1),X(i,2),X(i,3)+0.5, 30, 'MarkerEdgeColor',[1 0 0],...
                                    'MarkerFaceColor',[1 0 0]);
    text(X(i,1),X(i,2),X(i,3)+0.5, num2str(i), 'FontSize', 20);
end

