%% PCA_main.m
clear; close all; clc;

%% Read image
folder = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard2\Left\images_rect\';
im1 = im2double(imread([folder, 'Richard2_left_rect_0001.jpg']));
figure(1); imshow(im1,[]);
[rows, cols] = size(im1);

%% Initialise
row_mean = mean(im1,2); % compute row mean
X = im1 - repmat(row_mean, 1, cols); % subtract row mean to obtain X
Z = 1/sqrt(cols - 1)*X'; % create matrix, Z
covZ = Z'*Z; % covariance matrix of Z

%% Singular value decomposition
[U,S,V] = svd(covZ);
variances = diag(S).*diag(S); % compute variances
figure(2); 
bar(variances(1:40),'b') % plot of variances
xlim([0 40]); xlabel('eigenvector number'); ylabel('eigenvalue');

%% Extract first 40 principal components
PCs = 40;
VV = V(:,1:PCs);
Y = VV'*X; % project data onto PCs
ratio = rows/(2*PCs+1); % compression ratio

%% Convert back to original basis
XX = VV*Y; 
XX = XX + repmat(row_mean, 1, cols); % add the row means 
figure(3); imshow(XX,[]);

%% Show a selection of PCs
figure(4);
i = 1;
for PCs = [2 6 10 14 20 30 40 60 90 120 150 180]
    VV = V(:,1:PCs);
    Y = VV'*X;
    XX = VV*Y;
    XX = XX + repmat(row_mean, 1, cols);
    subplot(4,3,i)
    i = i + 1;
    imshow(XX,[])
    axis off
    title({[num2str(round(10*rows/(2*PCs+1))/10) ':1 compression'];...
    [num2str(PCs) ' principal components']})
end