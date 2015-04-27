function outimg = png2exr(input1, input2)
outimg = zeros(size(input1));
outimg = outimg + double(input1) * (1/256);
outimg = outimg - double(input2) * (1/256);
end