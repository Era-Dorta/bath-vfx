%% Clean up environment
clear all;
close all;

%% Loading and initialization
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


face_segmented = imread('~/workspaces/matlab/vfx/Data/skinRender/microgeometry/face_segmented.png');
face_tex = imread('~/workspaces/matlab/vfx/Data/skinRender/3dscans/vfx_richard3_face_simplified_0.png');
bump_map = histeq(rgb2gray(face_tex));
%imshow(bum_map);

% 0 is background, 1 are lines
face_segmented = ~face_segmented;

%% Texture up-sampling

% Get the first section
filled_with_bg = imfill(face_segmented,segments_centres(1,:));

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
