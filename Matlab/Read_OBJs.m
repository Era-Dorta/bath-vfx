%% Read_OBJs
clear; close all; clc;

folder = 'C:\Users\Richard\Desktop\CDE\Semester2\Visual_Effects\Data\Deformation_Transfer\Richard_Blendshapes\';
R_OBJ = cell(1,69);
R_OBJ{1} = read_wobj([folder, 'R_simpleE_edit.', sprintf('%04d',68) ,'.obj']);

for i = 1:68
    R_OBJ{i+1} = read_wobj([folder, 'R_simpleE_edit.', sprintf('%04d',i) ,'.obj']);
end