%% SelectMeshPoints.m
clearvars -except OBJ; close all; clc
addpath(genpath('WOBJ_toolbox_Version2b'));
addpath(genpath('SpringLab'));

%% Load mesh
if ~(exist('OBJ', 'var'))
    OBJ = read_wobj('C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Emily\Emily.obj');
end
T = OBJ.objects(5).data.vertices;
v = OBJ.vertices;

%% Show mesh
figure('units','normalized','outerposition',[0 0 1 1]);
trisurf(T(2:end,:), v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
hold on; axis equal; view([0 90]);

%% Select vertices
num_vertices = 97;
v_store = zeros(num_vertices,3);
%%
disp('Click anywhere on the surface, then hit return');
for i = 86
    
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
load('X_emily');
scatter3(X(:,1),X(:,2),X(:,3), 30, 'MarkerEdgeColor',[1 0 0],...
                                'MarkerFaceColor',[1 0 0]);
for i = 1:97
    text(X(i,1),X(i,2),X(i,3)+0.5, num2str(i), 'FontSize', 20);
end


