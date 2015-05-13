%% RichardBlendshapes.m
clear; close all; clc;

%% Load X_blendshpes
load('X_blendshapes');
Xn = X_blendshapes{1};
X_blendshapes = X_blendshapes(2:end);
numBlends = size(X_blendshapes,2);

%% load Richard seqeunce
load('X_sequence');
numFrames = size(X_sequence, 3);

%% Solve for frames
for i = 1:numFrames
    Xnew = X_sequence(:,:,i);
    numVertices = size(Xnew,1);
    x = reshape(Xnew, 3*numVertices, 1);
    xn = reshape(Xn, 3*numVertices, 1);
    disp = bsxfun(@minus, x, xn);
    
    key_poses = zeros(numVertices*3,numBlends);
    key_disp = zeros(numVertices*3,numBlends);
    for j = 1:numBlends
        key_poses(:,j) = reshape(X_blendshapes{1,j}, 3*numVertices, 1);
        key_disp(:,j) =  bsxfun(@minus, key_poses(:,j), xn);
    end
    
%     w = key_poses\x;
    
    lb = 0*ones(2*numVertices,1);
    ub = 1*ones(2*numVertices,1);
    [w,resnorm2] = lsqlin(key_disp,disp,[],[],[],[],lb,ub);
    
%     options = optimset('Display','notify','TolX',1e-15);
%     [w,resnorm] = lsqnonneg(key_poses,x,options);
    
    bb = bsxfun(@plus, key_disp*w, xn);
    bb = reshape(bb, 97, 3);
    
%     figure(1);
%     scatter3(Xnew(:,1),Xnew(:,2),Xnew(:,3), 20, 'MarkerEdgeColor',[1 0 0],...
%         'MarkerFaceColor',[1 0 0]);
%     axis equal; axis([-90 90 -80 80 -150 0]); view([0 90]); hold on;
%     scatter3(bb(:,1),bb(:,2),bb(:,3), 20, 'MarkerEdgeColor',[0 1 1]);
%     hold off;
    
    if i==1
        tri = delaunay(Xnew(:,1),Xnew(:,2));
    end
    figure(1);
    trisurf(tri,Xnew(:,1),Xnew(:,2),Xnew(:,3), ones(1,size(Xnew,1)), ...
        'LineWidth', 1.5); 
    axis equal; axis([-90 90 -80 80 -150 0]); view([0 90]); alpha(0.5);
    colormap(cool); light; lighting gouraud; material dull;
    hold on;
    trisurf(tri,bb(:,1),bb(:,2),bb(:,3), ones(1,size(bb,1)), ...
        'LineWidth', 1.5); 
    axis equal; axis([-90 90 -80 80 -150 0]); view([0 90]); alpha(0.5);
    colormap(cool); light; lighting gouraud; material dull;
    hold off;
end