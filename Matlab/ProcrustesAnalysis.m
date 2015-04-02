%% ProcrustesAnalysis.m
clearvars -except OBJ; close all; clc;

%% Load sparse points
load('X_0300'); % neutral face
Xn = X;
load('X_1190'); % comparison face
Xnew = X;
tri = delaunay(Xn(:,1),Xn(:,2)); % Delaunay triangulation

% Show neutral (sparse)
figure(1); 
trisurf(tri, Xn(:,1),Xn(:,2),Xn(:,3), 'LineWidth', 1.5); 
axis equal; axis([-50 50 -80 80 -60 0]); view([0 90]);
colormap(cool); light; lighting gouraud; material dull;
for i = 1:97
    text(Xn(i,1),Xn(i,2),Xn(i,3)+5,num2str(i));
end

% Show comparison (sparse)
figure(2);
trisurf(tri, Xnew(:,1),Xnew(:,2),Xnew(:,3), 'LineWidth', 1.5);
axis equal; axis([-50 50 -80 80 -60 0]); view([0 90]);
colormap(cool); light; lighting gouraud; material dull;
for i = 1:97
    text(Xnew(i,1),Xnew(i,2),Xnew(i,3)+5,num2str(i));
end

%% Procrustes
% rigidPoints = [43,54,58,38,59];
rigidPoints = [43,54,58,30,69,9,84];
[d, Z, transform] = procrustes(Xn(rigidPoints,:), Xnew(rigidPoints,:));
disp(['error = ', num2str(d)]);

% Rigid transformation
Xa = transform.b * Xnew * transform.T;
% Xa = transform.b * Xnew * transform.T + repmat(transform.c(1,:),97,1);

% Show result
figure(3);
trisurf(tri, Xa(:,1),Xa(:,2),Xa(:,3), 'LineWidth', 1.5);
axis equal; axis([-50 50 -80 80 -60 0]); view([0 90]);
colormap(cool); light; lighting gouraud; material dull;
for i = 1:97
    text(Xa(i,1),Xa(i,2),Xa(i,3)+5,num2str(i));
end

figure(4);
scatter3(Xn(rigidPoints,1),Xn(rigidPoints,2),Xn(rigidPoints,3),50,...
    'MarkerFaceColor',[0 0 1]); hold on; 
scatter3(Xnew(rigidPoints,1),Xnew(rigidPoints,2),Xnew(rigidPoints,3),50,...
    'MarkerFaceColor',[0 1 0]);
scatter3(Xa(rigidPoints,1),Xa(rigidPoints,2),Xa(rigidPoints,3),50,...
    'MarkerFaceColor',[1 0 0]); axis equal 
