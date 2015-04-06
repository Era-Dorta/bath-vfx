%% Clean up environment
clear all;
close all;

%% Loading and initialization

% Uncoment to recompute all the skin samples
% imblur

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
bump_map = histeq(rgb2gray(face_tex));
%imshow(bum_map);

% 0 is background, 1 are lines
face_segmented = ~face_segmented;

%% Texture up-sampling

sec_index = 1;
% Get the first section
filled_with_bg = imfill(face_segmented,segments_centres(sec_index,:));

% Delete the extra lines, i.e. subtract common pixels in both images
only_region = logical(filled_with_bg - (filled_with_bg & face_segmented));

% Find the smallest square that contains the region
[indx, indy] = find(only_region == 1);
max_indx = max(indx);
min_indx = min(indx);
max_indy = max(indy);
min_indy = min(indy);

x = [max_indx, min_indx, min_indx, max_indx, max_indx];
y = [max_indy, max_indy, min_indy, min_indy, max_indy];

bump_map1 = bump_map;
bump_map1(only_region) = 0;
imshow(bump_map1);
hold on;
plot(y, x, 'b-', 'LineWidth', 3);
hold off;

% Get the image of the texture region
region_ori_tex = face_tex(min_indx:max_indx, min_indy:max_indy);
path_b0 = [data_path '/synthesized/B0_' int2str(sec_index) '.png'];
imwrite(region_ori_tex, path_b0);

path_b1 = [data_path '/synthesized/B1_' int2str(sec_index) '.png'];
path_a0 = [data_path '/original/A0_' int2str(sec_index) '.png'];
path_a1 = [data_path '/original/A1_' int2str(sec_index) '.png'];

pathToScript = '~/workspaces/github/vfx/python/image_analogies_parallel.py';

cmdStr = ['~/workspaces/github/vfx/python/run.sh' ' ' path_a0 ' ' path_a1 ' ' path_b0 ' ' path_b1];

system(cmdStr);

synt_im = imread(path_b1);

imshow(synt_im);