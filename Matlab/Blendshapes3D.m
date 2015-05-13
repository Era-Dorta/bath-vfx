%% Load poses.
clearvars -except numfiles; close all; clc;


if ~(exist('OBJ', 'var'))
    numfiles = 68; % 68;
    load('blendshapes_OBJ');
    %     OBJ = cell(1, numfiles);
    %
    %     for k = 0:numfiles
    %       myfilename = sprintf('shapeBS/E_%d.obj', k);
    %       OBJ{k+1} = read_wobj(myfilename);
    %     end

end

% Use 20 blendshapes:
% load('blendshapes_19.mat');
% OBJ = new;
% numfiles = 19;

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
% load('Emily_97');
load('X_emily');
sparse_neutral = X;

T = OBJ{1, 1}.objects.data.vertices;
v = OBJ{1, 1}.vertices;
figure;
trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
axis equal; % axis([-15 15 -15 10 -15 15]);
view([0 90]); alpha(1.0);
hold on
scatter3(X(:,1),X(:,2),X(:,3), 50, 'MarkerEdgeColor',[1 0 0],...
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

% sparse_Blend1 = Blend1.vertices(idx,:);

%% Plot sparse points.
% for k = 2: 30;
% T = OBJ{1, k+1}.objects.data.vertices ;
% v = OBJ{1, k+1}.vertices;
% figure(k);
% trisurf(T, v(:, 1), v(:,2), v(:,3), ones(1,size(v,1)));
% axis equal; % axis([-15 15 -15 10 -15 15]);
% view([0 90]); alpha(1.0);
% hold on
% scatter3(sparse_OBJ{1, k}(:,1),sparse_OBJ{1, k}(:,2),sparse_OBJ{1, k}(:,3), 50, 'MarkerEdgeColor',[1 0 0],...
%     'MarkerFaceColor',[1 0 0]);
% end
% %% CHOOSE new frame/pose.
% load('Emily_sequence');
% numFrames = size(Emily_sequence,3);
% weight_array = [];
% error_store = zeros(numFrames,1);
% 
% % CHOOSE SOLVER: 1, 2, or 3:
% solver = 3;
% if solver == 3
%     w3 = zeros(1,numfiles);
% end
% 
% for i = 1:numFrames
%     %myfilename = sprintf('Emily_blendshapes/E_new%d.obj', 2);
%     %OBJ_new = read_wobj(myfilename);
%     %sparse_new = OBJ_new.vertices(idx,:);
%     sparse_new = Emily_sequence(:,:,i);
%     
%     % x = reshape(OBJ_new.vertices, 3*num_vertices, 1);
%     num_sparse_vertices = size(X,1);
%     x = reshape(sparse_new, 3*num_sparse_vertices, 1);
%     sparse_neutral = reshape(sparse_neutral, 3*num_sparse_vertices, 1);
%     displacement = bsxfun(@minus, x, sparse_neutral);
%     
%     sparse_key_poses = zeros(num_sparse_vertices*3,numfiles);
%     sparse_key_disp = zeros(num_sparse_vertices*3,numfiles);
%     for j = 1:numfiles
%         sparse_key_poses(:,j) = reshape(sparse_OBJ{1,j}, 3*num_sparse_vertices, 1);
%         sparse_key_disp(:,j) =  bsxfun(@minus, sparse_key_poses(:,j), sparse_neutral);
%     end
%     
%     %  Solve for weights.
%     switch solver
%         case 1
%             w1 = sparse_key_poses\x;
%             
%             weight_array = [weight_array; w1];
%             bb = bsxfun(@plus, sparse_key_disp*w1, sparse_neutral);
%             bb = reshape(bb, 97, 3);
%             
%         case 2
%             options = optimset('Display','notify','TolX',1e-15);
%             [w2,resnorm] = lsqnonneg(sparse_key_poses,x,options);
%             
%             weight_array = [weight_array; w2];
%             bb = bsxfun(@plus, sparse_key_disp*w2, sparse_neutral);
%             bb = reshape(bb, 97, 3);
%             
%         case 3
%             % Aeq = ones(1, numfiles); Force sum = 1.
%             % beq = 1;
%             lb = 0*ones(2*num_sparse_vertices,1);
%             ub = 1*ones(2*num_sparse_vertices,1);
%             x0 = w3;
%             options = optimoptions('lsqlin','MaxIter', 300, 'MaxPCGIter', 100, 'TolFun', 1e-20, 'TolPCG', 0.01);
%             [w3,resnorm2] = lsqlin(sparse_key_disp, displacement,[], [], [],[],lb,ub,x0,options);
%             
%             weight_array = [weight_array; w3];
%             bb = bsxfun(@plus, sparse_key_disp*w3, sparse_neutral);
%             bb = reshape(bb, 97, 3);
%             
%     end
%     
%     % Plot Error.
%     if (mod(i,200) == 0)
%         figure;
%         axis equal;
%         view([0 90]); alpha(1.0);
%         hold on
%         scatter3(sparse_new(:,1),sparse_new(:,2),sparse_new(:,3), 50, 'MarkerEdgeColor',[1 0 0],...
%             'MarkerFaceColor',[1 0 0]);
%         scatter3(bb(:,1),bb(:,2),bb(:,3), 80, 'MarkerEdgeColor',[0 1 1]);
%     end
%     
%     % Quantify error.
%     temp = abs(sparse_new - bb);
%     temp = mean(temp);
%     error = norm(temp);
% 
%     error_store(i) = error;
%     
%     % OBJ_blend = OBJ_new;
%     % v_temp = bsxfun(@plus, sparse_key_disp*w1, sparse_neutral);
%     % OBJ_blend.vertices = reshape( v_temp, num_sparse_vertices,3);
%     % OBJ_blend2 = OBJ_new;
%     % v_temp = bsxfun(@plus, sparse_key_disp*w2, sparse_neutral);
%     % OBJ_blend2.vertices = reshape( v_temp, num_sparse_vertices,3);
%     % OBJ_blend3 = OBJ_new;
%     % v_temp = bsxfun(@plus, sparse_key_disp*w3, sparse_neutral);
%     % OBJ_blend3.vertices = reshape( v_temp, num_sparse_vertices,3);
%     
% end
% average_error = mean(error_store);
% disp(average_error);
% 
% %% Save data as txt.
% % myfilename2 = sprintf('data/weights_%d.txt', k);
% 
% save data/weights_w2.txt weight_array -ASCII

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

%% CHOOSE new frame/pose.
v_store = X';
load('Emily_sequence');
numFrames = size(Emily_sequence,3);
weight_array = [];
error_store = zeros(numFrames,1);

% CHOOSE SOLVER: 1, 2, or 3:
solver = 1;

for i = 1:numFrames
    %myfilename = sprintf('Emily_blendshapes/E_new%d.obj', 2);
    %OBJ_new = read_wobj(myfilename);
    %sparse_new = OBJ_new.vertices(idx,:);
    sparse_new = Emily_sequence(:,:,i);
    
    % x = reshape(OBJ_new.vertices, 3*num_vertices, 1);
    num_sparse_vertices = size(v_store,2);
    x = reshape(sparse_new, 3*num_sparse_vertices, 1);
    sparse_neutral = reshape(sparse_neutral, 3*num_sparse_vertices, 1);
    displacement = bsxfun(@minus, x, sparse_neutral);
    
    sparse_key_poses = zeros(num_sparse_vertices*3,numfiles);
    sparse_key_disp = zeros(num_sparse_vertices*3,numfiles);
    for j = 1:numfiles
        sparse_key_poses(:,j) = reshape(sparse_OBJ{1,j}, 3*num_sparse_vertices, 1);
        sparse_key_disp(:,j) =  bsxfun(@minus, sparse_key_poses(:,j), sparse_neutral);
    end
    
    %  Solve for weights.
    switch solver
        case 1
            w1 = sparse_key_poses\x;
            
            weight_array = [weight_array; w1];
            bb = bsxfun(@plus, sparse_key_disp*w1, sparse_neutral);
            bb = reshape(bb, 97, 3);
            
        case 2
            options = optimset('Display','notify','TolX',1e-15);
            [w2,resnorm] = lsqnonneg(sparse_key_poses,x,options);
            
            weight_array = [weight_array; w2];
            bb = bsxfun(@plus, sparse_key_disp*w2, sparse_neutral);
            bb = reshape(bb, 97, 3);
            
        case 3
            % Aeq = ones(1, numfiles); Force sum = 1.
            % beq = 1;
            lb = 0*ones(2*num_sparse_vertices,1);
            ub = 1*ones(2*num_sparse_vertices,1);
            [w3,resnorm2] = lsqlin(sparse_key_disp, displacement,[], [], [],[],lb,ub);
            
            weight_array = [weight_array; w3];
            bb = bsxfun(@plus, sparse_key_disp*w3, sparse_neutral);
            bb = reshape(bb, 97, 3);
            
    end
    
    % Plot Error.
    
%     if (mod(i,200) == 0)
    if i ==1
        figure;
        axis equal;
        view([0 90]); alpha(1.0);
        hold on
        scatter3(sparse_new(:,1),sparse_new(:,2),sparse_new(:,3), 50, 'MarkerEdgeColor',[1 0 0],...
            'MarkerFaceColor',[1 0 0]);
        scatter3(bb(:,1),bb(:,2),bb(:,3), 80, 'MarkerEdgeColor',[0 1 1]);
    end
    
    % Quantify error.
    error = sum(sum(abs(sparse_new - bb),1),2);

    error_store(i) = error;
    
    % OBJ_blend = OBJ_new;
    % v_temp = bsxfun(@plus, sparse_key_disp*w1, sparse_neutral);
    % OBJ_blend.vertices = reshape( v_temp, num_sparse_vertices,3);
    % OBJ_blend2 = OBJ_new;
    % v_temp = bsxfun(@plus, sparse_key_disp*w2, sparse_neutral);
    % OBJ_blend2.vertices = reshape( v_temp, num_sparse_vertices,3);
    % OBJ_blend3 = OBJ_new;
    % v_temp = bsxfun(@plus, sparse_key_disp*w3, sparse_neutral);
    % OBJ_blend3.vertices = reshape( v_temp, num_sparse_vertices,3);
    
end
average_error = mean(error_store);
disp(average_error);

%% Save data as txt.
% myfilename2 = sprintf('data/weights_%d.txt', k);
% save data/weights_w2.txt weight_array -ASCII

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


%% Error:
% w1_error = 64.1
% w2_error = 93.1
% w3_error = 74.7

% With all blendshapes and X_emily:
% w1_error = 44.1 / 0.1898
% w2_error = 56.2 / 0.262
% w3_error = 56.7 / 0.233

% With 19 blendshapes and X_emily:
% w1_error = 40.4 / 0.251
% w2_error = 50.1 / 0.305
% w3_error = 42.4 / 0.262

% With 17 blendshapes and X_emily:
% w1_error = 40.8 / 0.253
% w2_error = 50.1 / 0.305
% w3_error = 42.4 / 0.263

% With all blendshapes and X_emily:
% x0 = zeros(1,numfiles);
% options = optimoptions('lsqlin','MaxIter', 300, 'MaxPCGIter', 70, 'TolFun', 1e-20, 'TolPCG', 0.01);
% w3_error = 0.2333

% With all blendshapes and X_emily:
% x0 = w3 (from previous iteration);
% options = optimoptions('lsqlin','MaxIter', 300, 'MaxPCGIter', 70, 'TolFun', 1e-20, 'TolPCG', 0.01);
% w3_error = 0.2333
