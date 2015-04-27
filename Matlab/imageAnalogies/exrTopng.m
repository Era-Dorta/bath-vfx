function outimg = exrTopng(input)
outimg = input * 128 + 128;
outimg = uint8(outimg);
end