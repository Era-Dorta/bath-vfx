clear all;
close all;

path = '~/workspaces/matlab/vfx/Data/skinRender/microgeometry/';

part = 'cheek_spec_normal';

create_pngs = false;

if create_pngs
    normal_exr = exrread([path 'albedo_normals/Subject1/' part '.exr']);
    
    [normal_png_pos, normal_png_neg] = exr2png(normal_exr);
    
    imwrite(normal_png_pos, [path 'albedo_normals/test/' part '_pos.png']);
    imwrite(normal_png_neg, [path 'albedo_normals/test/' part '_neg.png']);
    
    part = 'cheek_spec_albedo';
    albedo_exr = exrread([path 'albedo_normals/Subject1/' part '.exr']);
    [albedo_png_pos, ~] = exr2png(albedo_exr);
    % Multiply by two as the image is almost in grayscale
    albedo_png_pos = albedo_png_pos * 2;
    imwrite(albedo_png_pos, [path 'albedo_normals/test/' part '.png']);
end

% Run image analogies code
normal_png_pos = imread([path 'albedo_normals/test/' part '_pos_res.png']);
normal_png_neg = imread([path 'albedo_normals/test/' part '_neg_res.png']);
normal_exr = png2exr(normal_png_pos, normal_png_neg);
exrwrite(normal_exr, [path 'albedo_normals/test/' part '_res.exr']);
