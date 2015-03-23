% imBp = ia_process(imA, imAp, imB, options)
%
% Use Image Analogies (Herzmann et. al.) to generate an image.
%
%
% Original Author: Nathan Jacobs
%
function imBp = ia_process(imA, imAp, imB, imName, options)

if ~exist('options', 'var')
    options = struct;
end

if ~isfield(options, 'scaleFeatures')
    options.scaleFeatures = false;
end

if ~isfield(options, 'sourceWeight')
    options.sourceWeight = .5;
end

% a filter to make the middle more important
h = fspecial('gaussian', [5 5], 2);

% since we are going in column order, this is a mask of where already
% filled in pixels will be in our output image
%      1     1     1     1     1
%      1     1     1     1     1
%      1     1     0     0     0
%      0     0     0     0     0
%      0     0     0     0     0
goodPixels = true(5,5); goodPixels(3,3:5) = false; goodPixels(4:5,:) = false;

szA = size(imA);
szB = size(imB);

% just in case we want to manipulate the features and keep the raw images
% around
fA = imA;
fB = imB;

% scale the features to be in the correct range
if options.scaleFeatures
    fA = fA - mean(fA(:));
    fA = fA ./ norm(fA(:));
    
    fB = fB - mean(fB(:));
    fB = fB ./ norm(fB(:));
end

%
% build features from A (using a 5x5 neighborhood)
%
% Pad array adds 2 rows and colums at the beggining and the end of the
% image, copying the values of the first/last column/row.
% im2col rearrangles blocks into colums, so 5x5 blocks around each
% pixel are a colum in fAneigh
% x - 2, y - 2 ... x + 2, y - 2
%   .                   .
%   .                   .
% x - 2, y + 2 ... x + 2, y + 2
% The first column in the block are the first n elements in the feature
% column, then then next column, until the last
% So it creates a 25 column vector of features for each pixel
fAneigh = im2col(padarray(fA, [2 2], 'replicate'), [5 5], 'sliding');

% apply gaussian weights to empahsize the middle pixel
fAneigh = bsxfun(@times, h(:), fAneigh);

%
% build features for A'
% padarray, adding the two columns and rows, but of zeros
fApneigh = im2col(padarray(imAp, [2 2], 0), [5 5], 'sliding');

% apply gaussian weights to empahsize the middle pixel (also ignore empty
% pixels in B'), this is the same as fApneigh(goodPixels(:),:), rembember
% that (:) goes column wise, the same as im2col
% So it creates a 12 column vector of features for each pixel
fApneigh = fApneigh(goodPixels,:);
fApneigh = bsxfun(@times, h(goodPixels), fApneigh);

% weight source and target image equally
scaA = mean(sqrt(sum(fAneigh.^2,1))) \ (options.sourceWeight);
scaAp = mean(sqrt(sum(fApneigh.^2,1))) \ (1-options.sourceWeight);
fAneigh = fAneigh .* scaA;
fApneigh = fApneigh .* scaAp;

%
% build search structure
%

dataset = [fAneigh;fApneigh];

% flann_par.algorithm = 'autotuned';
% flann_par.target_precision = 0.9;
% flann_par.build_weight = 0.01;
% flann_par.memory_weight = 0;
% flann_par.sample_fraction = 0.1;

[index, params] = flann_build_index(dataset, struct);
%params.cores = 4;

imBp = nan(szB);

fBpad = padarray(fB, [2 2], 'replicate');
s = zeros([size(imBp,1), size(imBp,2), 2]);

% start adding pixels
for ixB = 1:szB(1)
    if mod(ixB, 10) == 0
        fprintf('Processing row %d of %d in image %s\n', ixB, szB(1), imName);
    end
    for jxB = 1:szB(2)
        
        %
        % build a feature that describes my local neighborhood
        %
        
        % find nearest neighbor in upper image
        feat = fBpad(ixB + (0:4), jxB + (0:4),:);
        fBppad = padarray(imBp, [2 2], 0); % lazy padding to avoid edge cases
        featp = fBppad(ixB + (0:4), jxB + (0:4),:);
        featp = featp(goodPixels);
        
        % apply gaussian weights to empahsize the middle pixel
        feat = bsxfun(@times, feat, h);
        featp = bsxfun(@times, featp, h(goodPixels));
        feat = [feat(:).*scaA; featp.*scaAp];
        
        %
        % search for nearest neighbor of (ixB, jxB) in A, A' using this feature
        %
        
        % find nearest neighbor
        [result, ~] = flann_search(index,feat,1,params);
        
        % find location in A'
        [ixAopt jxAopt] = ind2sub(szA, result);
        
        % add the pixel value to the output
        imBp(ixB, jxB, :) = imAp(ixAopt, jxAopt,:);
        s(ixB, jxB,:) = [ixAopt jxAopt];
        
    end
    %
    %   if mod(ixB, 10) == 0
    %
    %     figure(1); clf;
    %     subplot(321); imshow(imA); title('A')
    %     subplot(322); imshow(imAp); title('A''')
    %     subplot(323); imshow(imB); title('B')
    %     subplot(324); imshow(imBp); title('B''');
    %     subplot(325); imagesc(s(:,:,1), [1 size(imBp,1)]); title('i source')
    %     subplot(326); imagesc(s(:,:,2), [1 size(imBp,2)]); title('j source')
    %
    %     drawnow; pause(eps);
    %   end
    
end

flann_free_index(index);