clear all;
close all;

data_path = '~/workspaces/matlab/vfx/Data/skinRender/microgeometry/original/';

choose_new_points = false;

% Points from macro lens
%x = [2186, 1316, 1312, 299, 161, 1357, 1649, 1395, 1666, 2800];
%y = [2905, 1584, 1012, 1830, 1721, 1063, 2484, 1579, 1304, 2250];
x = 1.0e+03 * [1.5117, 1.8452, 1.5659, 1.6076, 1.4992, 1.2532, 1.6618, 1.3908, 1.0323, 1.5034];
y = 1.0e+03 * [2.1423, 2.2632, 2.3507, 2.3299, 2.2590, 2.0714, 2.2507, 2.2090, 1.7170, 2.1714];
patch_size = 600;

% Blur an image
for i = 1:10
    I = imread([data_path 'A1_' int2str(i) '.JPG']);
    
    if choose_new_points
        figure(1);
        imshow(I); hold on;
        while(true)
            [y_aux, x_aux, button] = ginput(1);
            if button ~= 1
                break;
            end
            x(i) = x_aux;
            y(i) = y_aux;
            imshow(I); hold on;
            patch_x = [x(i)+patch_size, x(i), x(i), x(i)+patch_size, x(i)+patch_size];
            patch_y = [y(i)+patch_size, y(i)+patch_size, y(i), y(i), y(i)+patch_size];
            plot(patch_y, patch_x, 'b-', 'LineWidth', 3);
            hold off;
        end
    end
    
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
    
    Igray = rgb2gray(I);
    Igray = imadjust(Igray,stretchlim(Igray));
    imshow(Igray);
    imwrite(Igray, [data_path 'A1_' int2str(i) '.png']);
    imwrite(I, [data_path 'A1_' int2str(i) 'c.png']);
    
    %Igray = imread('~/A0gt.png');
    %imshow(Igray);
    
    %H = fspecial('disk',10);
    %H = fspecial('gaussian',[5 5],2);
    %B = imfilter(Igray,H,'replicate');
    
    kernelStd = 1.5;
    kernelRadius = ceil(5 * kernelStd);
    kernelLength = (2 * kernelRadius ) + 1;
    H = fspecial('gaussian', [kernelLength, kernelLength], kernelRadius);
    B = imfilter(Igray,H,'replicate');
    
    %B = imgaussfilt(Igray,2,'FilterSize',[3 3]);
    imshow(B);
    imwrite(B, [data_path 'A0_' int2str(i) '.png']);
    
    B = imfilter(I,H,'replicate');
    imshow(B);
    imwrite(B, [data_path 'A0_' int2str(i) 'c.png']);
end