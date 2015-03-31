% The sparse Richard mesh is warped so that it matched the hand-picked
% points on Victor's face.
% 
% At the moment it uses the Finite ICP:
%   http://www.mathworks.com/matlabcentral/fileexchange/24301-finite-iterative-closest-point
%
% However, we should probably be using this:
%   http://www.mathworks.com/matlabcentral/fileexchange/41396-nonrigidicp
% Unfortunately, this wants us to give it a mesh, so we'd have to choose a
% lot of point, do a triangulation, then do the non-rigid IPC.
%
% For the 3D thin plate Spline Warping, try:
%   http://www.mathworks.com/matlabcentral/fileexchange/37576-3d-thin-plate-spline-warping-function
% At the moment this is used for testing, but we may use this later to warp
% Victor's mesh.


% Import the data.
clearvars -except OBJ;
close all; clc;

% Choose vertices
select_vertices = false;

if ~(exist('OBJ', 'var'))
    OBJ=read_wobj('victorHead.obj');
end

T = OBJ.objects.data.vertices;
v = OBJ.vertices;

% hgload('neutralface.fig');

%% Select marker points.

if select_vertices
%     figure(1);
%     imshow('Richard_neutral_pic.jpg')
    fig = figure(2);
    hold on
    trisurf(T(2:end,:), v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
    axis equal
    zoom(1.3)
    num_vertices = 97;
    disp('Click anywhere on the surface, then hit return')
    
    v_store = zeros(3,num_vertices);
    
    % Store and display the nearest vertex.
    for i = 1:num_vertices
        pause
        [p vn] = select3d;
        v_store(:,i) = vn;
        scatter3(vn(1),vn(2),vn(3), 100, 'MarkerEdgeColor',[1 0 0],...
            'MarkerFaceColor',[1 0 0]);
        disp(i)
    end
else
    fig = figure(2);
    hold on
    trisurf(T(2:end,:), v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
    axis equal
    zoom(1.3)
    load('Victor_97vertices.mat');
    % Display feature numbers
    numFeatures = size(v_store,2);
    for i = 1:numFeatures
        text(v_store(1,i),v_store(2,i),v_store(3,i)+0.05, num2str(i),'color', [1 0 0], 'FontSize', 18)
    end
end
% figure(3);
% v_store = v_store';
% scatter3(v_store(:,1),v_store(:,2),v_store(:,3))



%% Load Richard neutral face.
load('Richard_neutral_9pts');
Rich_neutral = X(1:3,:);

%% Arrange points in terms of increasing x.
Rich_neutral = sortrows(Rich_neutral',1);
v_store = sortrows(v_store',1);

%% Translate Richard points with respect to the point on the nose.
% diff = Rich_neutral(5,:) - v_store(5,:);
% Rich_neutral = Rich_neutral - diff;

%% ICP.
[Points_Moved,M] = ICP_finite(v_store, Rich_neutral,struct('Registration','Rigid'));

%% Move Rich_neutral according to the ICP.
Rich_affine = movepoints(M, Rich_neutral);

%% Thin plate spline for testing.
[wobject] = TPS3D(Rich_affine,v_store,  v);

%% Plot mesh for testing.
figure(4);
hold on
trisurf(T(2:end,:), v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
axis([-2 2 15.5 20.25])
figure(5);
hold on
trisurf(T(2:end,:), wobject(:, 1), wobject(:,2), wobject(:,3), ones(1,size(v,1)));


%% Other plots.
fig = figure(6);
hold on
trisurf(wobject(2:end,:), Rich_neutral(:, 1), Rich_neutral(:,2), Rich_neutral(:,3), ones(1,size(v,1)));
axis([-2 2 15.5 20.25])

%% Save .obj.
OBJ_new = OBJ;
OBJ_new.vertices = wobject;
write_wobj(OBJ_new, 'transformed_data.obj');
