%% BlendshapeSequence.m
clearvars -except OBJ numfiles; close all; clc;
saveon = false;

%% Load Blendshapes
if ~(exist('OBJ', 'var'))
    numfiles = 68;
    load('blendshapes_OBJ');
end

%% Get neutral and displacements
load('X_emily');
sparse_neutral = X;
num_sparse_vertices = size(sparse_neutral,1);

% Dense blendshapes
num_vertices = size(OBJ{1, 1}.vertices,1);
neutral_pose = reshape(OBJ{1, 1}.vertices, 3*num_vertices, 1);
key_poses = zeros(num_vertices*3,numfiles-1);
key_disp = zeros(num_vertices*3,numfiles-1);
for i = 1:numfiles
    key_poses(:,i) = reshape(OBJ{1, i+1}.vertices, 3*num_vertices, 1);
    key_disp(:,i) =  bsxfun(@minus, key_poses(:,i), neutral_pose);
end

T = OBJ{1, 1}.objects.data.vertices;
v = OBJ{1, 1}.vertices;
figure(1);
trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
axis equal; axis([-15 15 -15 10 -15 15]); view([0 90]); hold on;
scatter3(X(:,1),X(:,2),X(:,3), 50, 'MarkerEdgeColor',[1 0 0],...
            'MarkerFaceColor',[1 0 0]);

% Find indeces of markers
idx = zeros(size(sparse_neutral,1),1);
for i = 1:size(sparse_neutral,1)
    fx = OBJ{1, 1}.vertices(:, 1);
    fy = OBJ{1, 1}.vertices(:, 2);
    fz = OBJ{1, 1}.vertices(:, 3);
    valx = sparse_neutral(i,1);
    valy = sparse_neutral(i,2);
    valz = sparse_neutral(i,3);
    tmp = abs(fx-valx) + abs(fy-valy) + abs(fz-valz);
    [~,idx(i)] = min(tmp);
end

% Sparse blendshapes
sparse_OBJ = cell(1, numfiles);
for i = 1:numfiles
    sparse_OBJ{i} = OBJ{1, i+1}.vertices(idx,:);
end

% Compute displacements of markers
sparse_neutral = reshape(sparse_neutral, 3*num_sparse_vertices, 1);
sparse_key_poses = zeros(num_sparse_vertices*3,numfiles);
sparse_key_disp = zeros(num_sparse_vertices*3,numfiles);
for i = 1:numfiles
    sparse_key_poses(:,i) = reshape(sparse_OBJ{1,i}, 3*num_sparse_vertices, 1);
    sparse_key_disp(:,i) =  bsxfun(@minus, sparse_key_poses(:,i), sparse_neutral);
end

%% Plot sparse blendshapes
for i = 1:numfiles;
    v = OBJ{1, i+1}.vertices;
    figure(1);
    trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
    axis equal; axis([-15 15 -15 10 -15 15]); view([0 90]); hold on;
    scatter3(sparse_OBJ{1, i}(:,1),sparse_OBJ{1, i}(:,2),sparse_OBJ{1, i}(:,3), 50, 'MarkerEdgeColor',[1 0 0],...
        'MarkerFaceColor',[1 0 0]);
    hold off;
    pause(0.1);
end

%% Blendshape sequence
load('Emily_sequence');
numFrames = size(Emily_sequence,3);
w = zeros(numfiles,numFrames);

for frame = 1:numFrames
% for frame = 1
    sparse_new = Emily_sequence(:,:,frame);
    x = reshape(sparse_new, 3*num_sparse_vertices, 1);
    displacement = bsxfun(@minus, x, sparse_neutral);
    
    % Solve for weights
    lb = 0*ones(2*num_sparse_vertices,1);
    ub = 1*ones(2*num_sparse_vertices,1);
    [w(:,frame),resnorm2] = lsqlin(sparse_key_disp,displacement,[],[],[],[],lb,ub);
    
    % Plot
    OBJ_blend = OBJ{1};
    v_temp = bsxfun(@plus, key_disp*w(:,frame), neutral_pose);
    OBJ_blend.vertices = reshape(v_temp, num_vertices,3);
    v = OBJ_blend.vertices;
    figure(2);
    trisurf(T, v(:,1),v(:,2),v(:,3), ones(1,size(v,1)));
    axis equal; axis([-15 15 -15 10 -15 15]); view([0 90]);
    
    % Save OBJ
    if saveon == true
        write_wobj(OBJ_blend, ['Emily_', num2str(frame,'% 05.f'), '.obj']);
    end    
end

% weight_array = reshape(w,numFrames*numfiles,1);
% save weights.txt weight_array -ASCII

%% Show all together
load('X_sequence');
load('Emily_sequence');
tri = delaunay(X_sequence(:,1,1),X_sequence(:,2,1)); % Delaunay triangulation
for frame = 1:numFrames  
    figure(1);
    subplot(1,3,1);
    trisurf(tri,X_sequence(:,1,frame),X_sequence(:,2,frame),X_sequence(:,3,frame),...
        'LineWidth', 1.5);
    axis equal; axis([-60 60 -80 80 -80 0]); view([0 90]);
    colormap(cool); light; lighting gouraud; material dull;
    
    figure(1);  
    subplot(1,3,2);
    trisurf(tri,Emily_sequence(:,1,frame),Emily_sequence(:,2,frame),Emily_sequence(:,3,frame),...
        'LineWidth', 1.5);
    axis equal; axis([-10 10 -15 10 -5 5]); view([0 90]);
    colormap(cool); light; lighting gouraud; material dull;
    
    figure(1);  
    subplot(1,3,3);
    OBJ_blend = OBJ{1};
    v_temp = bsxfun(@plus, key_disp*w(:,frame), neutral_pose);
    OBJ_blend.vertices = reshape(v_temp, num_vertices,3);
    v = OBJ_blend.vertices;
    trisurf(T, v(:,1),v(:,2),v(:,3), ones(1,size(v,1)));
    axis equal; axis([-15 15 -15 10 -15 15]); view([0 90]);
end
