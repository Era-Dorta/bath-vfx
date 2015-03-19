clear all;
close all;

% Load A, A', B
A0 = imread('~/workspaces/matlab/vfx/Data/skinRender/imageAnalogies/A0.jpg');
A1 = imread('~/workspaces/matlab/vfx/Data/skinRender/imageAnalogies/A1.jpg');
B0 = imread('~/workspaces/matlab/vfx/Data/skinRender/imageAnalogies/B0.jpg');

% Conver to gray scale and normalize


A0 = im2double(rgb2gray(A0));
A1 = im2double(rgb2gray(A1));
B0 = im2double(rgb2gray(B0));

k = 1;
L = 3;

B1 = createImageAnalogy(A0, A1, B0, k, L);

B1test = imread('~/workspaces/matlab/vfx/Data/skinRender/imageAnalogies/B1.jpg');