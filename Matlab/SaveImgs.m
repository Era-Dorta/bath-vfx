clear ; close all;

folder = 'C:\Users\Richard\Desktop\CDE\Semester 2\Visual Effects\Data\Richard1\images';
imgType = '.png';
sDir =  dir( fullfile(folder ,['*' imgType]) );
numImgs = size(sDir, 1);

for i = 1:numImgs
    I = rgb2gray(imread([folder '/' sDir(i).name]));
    imwrite(I,['Richard1_BW_' num2str(i,'% 05.f') '.png'])
end