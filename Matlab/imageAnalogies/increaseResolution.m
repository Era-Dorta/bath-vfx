%% Clean up environment
clear all;
close all;

%% Loading and initialization

% Uncoment to recompute all the skin samples
% imblur

num_seg = 10;

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

%Switch x,y for matlab indexing
segments_centres = [segments_centres(:,2), segments_centres(:,1)];

data_path = '~/workspaces/matlab/vfx/Data/skinRender/microgeometry';
face_segmented = imread([data_path '/face_segmented.png']);
face_tex = imread('~/workspaces/matlab/vfx/Data/skinRender/3dscans/vfx_richard3_face_simplified_0.png');

create_bump_map = false;

if create_bump_map
    bump_map = histeq(rgb2gray(face_tex));
    imwrite(bump_map, [data_path '/synthesized/bump_map.png']);
    imshow(bump_map);
else
    bump_map = imread([data_path '/synthesized/bump_map.png']);
end

% 0 is background, 1 are lines
face_segmented = ~face_segmented;

% Texture up-sampling
%% Save the original images for each segment
save_files = false;
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
    [indx, indy] = find(region_pixels{i} == 1);
    max_indx(i) = max(indx);
    min_indx(i) = min(indx);
    max_indy(i) = max(indy);
    min_indy(i) = min(indy);
    
    if save_files
        x = [max_indx(i), min_indx(i), min_indx(i), max_indx(i), max_indx(i)];
        y = [max_indy(i), max_indy(i), min_indy(i), min_indy(i), max_indy(i)];
        
        bump_map1 = bump_map;
        bump_map1(region_pixels{i}) = 0;
        imshow(bump_map1);
        hold on;
        plot(y, x, 'b-', 'LineWidth', 3);
        hold off;
        
        % Get the image of the texture region
        region_ori_tex = face_tex(min_indx(i):max_indx(i), min_indy(i):max_indy(i));
        path_b0 = [data_path '/synthesized/B0_' int2str(i) '.png'];
        imwrite(region_ori_tex, path_b0);
    end
end

%TODO Execute the script here rather than outside

%% Read the generated textures
new_tex_seg = cell(num_seg,1);
for i = 1:num_seg
    path_b1 = [data_path '/synthesized/B1_' int2str(i) '.png'];
    new_tex_seg{i} = imread(path_b1);
end

%% Compose the generated textures
new_bump_map = bump_map;

for i = 1:num_seg
    new_bump_map(region_pixels{i}) = new_tex_seg{i}(region_pixels{i}(min_indx(i):max_indx(i), min_indy(i):max_indy(i)));
end

%% Fill in the gaps with lineal interpolation

new_bump_map(face_segmented) = NaN;
[border_row, border_col] = find(face_segmented == 1);

imshow(new_bump_map);