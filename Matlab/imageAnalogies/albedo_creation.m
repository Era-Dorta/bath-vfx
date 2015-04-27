clear all;
close all;

path = 'vfx/Data/skinRender/microgeometry/';

part = 'cheek_spec_normal';
normal_exr = exrread([path 'albedo_normals/Subject1/' part '.exr']);

[normal_png_pos, normal_png_neg] = exr2png(normal_exr);

imwrite(normal_png_pos, [path 'albedo_normals/test/' part '_pos.png']);
imwrite(normal_png_neg, [path 'albedo_normals/test/' part '_neg.png']);

part = 'cheek_spec_albedo';
albedo_exr = exrread([path 'albedo_normals/Subject1/' part '.exr']);
[albedo_png_pos, ~] = exr2png(albedo_exr);
imwrite(albedo_png_pos, [path 'albedo_normals/test/' part '.png']);