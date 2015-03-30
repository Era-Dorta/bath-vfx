function Imgs = ReadImgsBetween(folder, imgType, first, last, imSize)

sDir =  dir( fullfile(folder ,['*' imgType]) );
numImgs = last-first+1;
Imgs = zeros(imSize(1), imSize(2), numImgs);
count = first;

for i = 1:numImgs
    Imgs(:,:,i) = imread([folder '/' sDir(count).name]);
    count  = count+1;
end