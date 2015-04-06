clear all;
close all;

data_path = '~/workspaces/matlab/vfx/Data/skinRender/microgeometry/original/';
x = [2186, 1316, 1312, 299, 161, 1357, 1649, 1395, 1666, 2800];
y = [2905, 1584, 1012, 1830, 1721, 1063, 2484, 1579, 1304, 2250];
patch_size = 600;

% Blur an image
for i = 1:10
    I = imread([data_path 'A0_' int2str(i) '.JPG']);
    
    figure(1);
    imshow(I);
    hold on;
    patch_x = [x(i)+patch_size, x(i), x(i), x(i)+patch_size, x(i)+patch_size];
    patch_y = [y(i)+patch_size, y(i)+patch_size, y(i), y(i), y(i)+patch_size];
    plot(patch_y, patch_x, 'b-', 'LineWidth', 3);
    hold off;
    
    I = I(x(i):x(i) + patch_size, y(i):y(i) + patch_size,:);
    
    figure(2);
    imshow(I);
    I = rgb2gray(I);
    Igray = imadjust(I,stretchlim(I));
    imshow(Igray);
    imwrite(Igray, [data_path 'A0_' int2str(i) '.png']);
    
    %Igray = imread('~/A0gt.png');
    %imshow(Igray);
    
    %H = fspecial('disk',10);
    %H = fspecial('gaussian',[5 5],2);
    %B = imfilter(Igray,H,'replicate');
    
    kernelStd = 2;
    kernelRadius = ceil(5 * kernelStd);
    kernelLength = (2 * kernelRadius ) + 1;
    H = fspecial('gaussian', [kernelLength, kernelLength], kernelRadius);
    B = imfilter(Igray,H,'replicate');
    
    %B = imgaussfilt(Igray,2,'FilterSize',[3 3]);
    imshow(B);
    imwrite(B, [data_path 'A1_' int2str(i) '.png']);
end