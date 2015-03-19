function B1 = createImageAnalogy( A0, A1, B0 )
%Compute Gaussian pyramids for A, A , and B
%Compute features for A, A , and B
%Initialize the search structures (e.g., for ANN)
n = 10;
numPixels = numRows * numCols;
for i=1:numGaussian
    for q=1:numPixels
        p = bestMatch(A0, A1, B0, B1, s, l, q);
        B1(q) = A1(p);
        s(q) = p;
        B1 =
    end
end
end

