function [outimg1, outimg2] = exr2png(input)
outimg1 = zeros(size(input));
outimg1(input > 0) = input(input > 0);
outimg1 = outimg1 * 256;
outimg1 = uint8(outimg1);

outimg2 = zeros(size(input));
outimg2(input < 0) = input(input < 0);
outimg2 = - outimg2 * 256;
outimg2 = uint8(outimg2);
end