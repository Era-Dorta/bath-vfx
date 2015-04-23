clear all;
close all;

data_path = '~/workspaces/matlab/vfx/Data/skinRender/microgeometry/original/';
data_path_new = '~/workspaces/matlab/vfx/Data/skinRender/microgeometry/synthesized/';
num_seg = 10;

choose_new_points = false;
do_color_correction = true;

% Points from macro lens
%x = [2186, 1316, 1312, 299, 161, 1357, 1649, 1395, 1666, 2800];
%y = [2905, 1584, 1012, 1830, 1721, 1063, 2484, 1579, 1304, 2250];
x = 1.0e+03 * [1.5117, 2.1985, 1.5659, 1.6076, 1.4992, 1.2532, 1.8905, 1.3908, 1.6846, 1.5034];
y = 1.0e+03 * [2.1423, 2.6785, 2.3507, 2.3299, 2.2590, 2.0714, 2.2865, 2.2090, 1.6865, 2.1714];
patch_size = 600;

% Pixel positions known to be in the senter of each face segment
segments_centres = [2540, 556;
    3260, 2124;
    2212, 3716;
    2612, 1380;
    2412, 2892;
    1996, 804;
    2052, 2076;
    1740, 3196;
    596, 1980;
    1260, 2044];

% Switch x,y for matlab indexing
segments_centres = [segments_centres(:,2), segments_centres(:,1)];

face_segmented = imread('~/workspaces/matlab/vfx/Data/skinRender/microgeometry/face_segmented.png');
face_segmented = ~face_segmented;
region_pixels = cell(num_seg,1);
max_indx = zeros(num_seg,1);
min_indx = zeros(num_seg,1);
max_indy = zeros(num_seg,1);
min_indy = zeros(num_seg,1);
for i = 1:num_seg
    % Get the first section
    filled_with_bg = imfill(face_segmented,segments_centres(i,:));
    
    % Delete the extra lines, i.e. subtract common pixels in both images
    region_pixels{i} = logical(filled_with_bg - (filled_with_bg & face_segmented));
    
    % Find the smallest square that contains the region
    [indx, indy, indz] = find(region_pixels{i} == 1);
    max_indx(i) = max(indx);
    min_indx(i) = min(indx);
    max_indy(i) = max(indy);
    min_indy(i) = min(indy);
end

% Blur an image
for i = 1:num_seg
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
    
    % Compute the mean on each channel and make the A0 and A1 in the same
    % color space/mean as B0
    if do_color_correction
        B0 = imread([data_path_new 'B0_' int2str(i) 'c.png']);
        % Take out extra two pixels from the borders
        B0 = B0(3:end-2,3:end-2,:);
        valid_region = region_pixels{i}(min_indx(i):max_indx(i), min_indy(i):max_indy(i));
        B0sub = B0(:,:,1);
        B0mean(1) = mean(reshape(B0sub(valid_region), 1, []));
        B0sub = B0(:,:,2);
        B0mean(2) = mean(reshape(B0sub(valid_region), 1, []));
        B0sub = B0(:,:,3);
        B0mean(3) = mean(reshape(B0sub(valid_region), 1, []));
        
        Imean(1) = mean(reshape(I(:,:,1), 1, []));
        Imean(2) = mean(reshape(I(:,:,2), 1, []));
        Imean(3) = mean(reshape(I(:,:,3), 1, []));
        
        meanDiff = B0mean - Imean;
        
        I(:,:,1) = I(:,:,1) + meanDiff(1);
        I(:,:,2) = I(:,:,2) + meanDiff(2);
        I(:,:,3) = I(:,:,3) + meanDiff(3);
    end
    
    Igray = rgb2gray(I);
    Igray = histeq(Igray);
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