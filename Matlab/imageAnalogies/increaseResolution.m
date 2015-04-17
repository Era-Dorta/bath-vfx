%% Clean up environment
clear all;
close all;

%% Loading and initialization

% Uncoment to recompute all the skin samples
% imblur

num_seg = 10;

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

data_path = '~/workspaces/matlab/vfx/Data/skinRender/microgeometry';
face_segmented = imread([data_path '/face_segmented.png']);
face_tex = imread('~/workspaces/matlab/vfx/Data/skinRender/3dscans/vfx_richard3_face_simplified_0.png');

% Set to false to do the displacement map
do_texture = true;

tex_extra = '';

if do_texture;
    tex_extra = 'c';
end

create_disp_map = false;

if create_disp_map
    % Create a displacement map by taking the texture to gray and doing a
    % histogrami equalization
    disp_map = histeq(rgb2gray(face_tex));
    imwrite(disp_map, [data_path '/synthesized/disp_map.png']);
    imshow(disp_map);
else
    disp_map = imread([data_path '/synthesized/disp_map.png']);
end

if do_texture
    disp_map = face_tex;
end

imshow(disp_map);
% Negate the image so 0 is background, 1 are lines
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
    [indx, indy, indz] = find(region_pixels{i} == 1);
    max_indx(i) = max(indx);
    min_indx(i) = min(indx);
    max_indy(i) = max(indy);
    min_indy(i) = min(indy);
    
    region_pixels{i}(:,:,2) = region_pixels{i}(:,:,1);
    region_pixels{i}(:,:,3) = region_pixels{i}(:,:,1);
    
    if save_files
        x = [max_indx(i), min_indx(i), min_indx(i), max_indx(i), max_indx(i)];
        y = [max_indy(i), max_indy(i), min_indy(i), min_indy(i), max_indy(i)];
        
        % Display the sqaure
        disp_map1 = disp_map;
        disp_map1(region_pixels{i}) = 0;
        imshow(disp_map1);
        hold on;
        plot(y, x, 'b-', 'LineWidth', 3);
        hold off;
        
        % Save an image as big as the square
        % Add two extra pixels so the borders can be computed as well
        region_ori_disp = disp_map(min_indx(i)-2:max_indx(i)+2, min_indy(i)-2:max_indy(i)+2,:);
        path_b0 = [data_path '/synthesized/B0_' int2str(i) tex_extra '.png'];
        imwrite(region_ori_disp, path_b0);
    end
end

%TODO Execute the script here rather than outside

%% Read the generated textures
new_disp_seg = cell(num_seg,1);
for i = 1:num_seg
    path_b1 = [data_path '/synthesized/B1_' int2str(i) tex_extra '.png'];
    new_disp_seg{i} = imread(path_b1);
    % Take out the two extra pixels for the borders
    new_disp_seg{i} = new_disp_seg{i}(3:end-2,3:end-2,:);
end

%% Compose the generated textures
new_disp_map = double(disp_map);

for i = 1:num_seg
    new_disp_map(region_pixels{i}) = ...
        new_disp_seg{i}(region_pixels{i}(min_indx(i):max_indx(i), min_indy(i):max_indy(i),:));
end

%% Fill in the gaps with lineal interpolation

% Set the borders initialy to not a number
new_disp_map(face_segmented) = NaN;
[border_row, border_col] = find(face_segmented == 1);

index_fill = ones(size(border_row));

% Interpolate with the vertical or horizontal neighbour pixels
while( sum(index_fill) > 0)
    for i=1:size(border_row,1)
        if index_fill(i)
            c_row = border_row(i);
            c_col = border_col(i);
            if ~isnan(new_disp_map(c_row+1, c_col)) && ~isnan(new_disp_map(c_row-1, c_col))
                new_disp_map(c_row, c_col,:) = 0.5 * (new_disp_map(c_row+1, c_col,:) + new_disp_map(c_row-1, c_col,:));
                index_fill(i) = 0;
            else
                if ~isnan(new_disp_map(c_row, c_col+1)) && ~isnan(new_disp_map(c_row, c_col-1))
                    new_disp_map(c_row, c_col,:) = 0.5 * (new_disp_map(c_row, c_col+1,:) + new_disp_map(c_row, c_col-1,:));
                    index_fill(i) = 0;
                end
            end
        end
    end
    % Uncomment to show the reamining pixels to be interpolated
    %disp(sum(index_fill));
end

%% Save the generated texture map
new_disp_map = uint8(new_disp_map);
if do_texture
    out_name = 'new_text_map.png';
else
    out_name = 'disp_map.png';
end
imwrite(new_disp_map, [data_path '/synthesized/' out_name]);