
%
% Use ia_process to blur an image.
%
% Currently this converts the images into HSV space and just blurs the
% value channel and leaving the hue and saturation channels unchanged.
% This was done to avoid the cost of processing each color channel
% separately (which is what should probably be done).
%
clear all

A0 = imread('~/workspaces/matlab/vfx/Data/skinRender/imageAnalogies/A0.jpg');
A1 = imread('~/workspaces/matlab/vfx/Data/skinRender/imageAnalogies/A1.jpg');
B0 = imread('~/workspaces/matlab/vfx/Data/skinRender/imageAnalogies/B0.jpg');

A0 = imresize(A0, 0.2);
A1 = imresize(A1, 0.2);
B0 = imresize(B0, 0.2);

imA = rgb2hsv(im2double(A0));
imAp = rgb2hsv(im2double(A1));
imB = rgb2hsv(im2double(B0));

% Decrease to increase the image distortion
opt.sourceWeight = 0.3;

imBp = ia_process(imA(:,:,3), imAp(:,:,3), imB(:,:,3), 'testim', opt);

% add the H and S channels to the 
imBp = cat(3, imB(:,:,[1 2]), imBp);

imA = hsv2rgb(imA);
imAp = hsv2rgb(imAp);
imB = hsv2rgb(imB);
imBp = hsv2rgb(imBp);

figure(1); clf;
subplot(221); imshow(imA); title('A')
subplot(222); imshow(imAp); title('A''')
subplot(223); imshow(imB); title('B')
subplot(224); imshow(imBp); title('B''');
