function imArray = ReadImgs(folder, imgType, numImgs)

sDir =  dir( fullfile(folder ,['*' imgType]) );

% If numImgs not specified, use all images in directory
if ~exist('numImgs','var') || isempty(numImgs)
  numImgs = size(sDir, 1);
end

imArray = cell(1, numImgs);

for i = 1:numImgs
    imArray{i} = imread([folder '/' sDir(i).name]);
end