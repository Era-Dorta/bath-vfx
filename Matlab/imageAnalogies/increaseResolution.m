clear all;
close all;

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

% 0 is background, 1 are lines
face_segmented = ~face_segmented;

% Get the first section
filled_with_bg = imfill(face_segmented,segments_centres(1,:));
% Delete the extra lines, i.e. substract common pixels in both images
only_region = filled_with_bg - (filled_with_bg & face_segmented);
imshow(only_region);