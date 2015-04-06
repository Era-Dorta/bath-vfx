function B1 = createImageAnalogy( a0, a1, b0, k, L)

%% Initialise

% Transpose the data, since matlab ind2sub works in a transposed manner
a0 = a0';
a1 = a1';
b0 = b0';

[nRowsA, nColsA] = size(a0);
nPixelsA = nRowsA * nColsA;
[nRowsB, nColsB] = size(b0);
sizeB = [nRowsB, nColsB];
nPixelsB = nRowsB * nColsB;

% Transpose data for matlab sake, normal transpose will not work

%% Compute Gaussian pyramids for a0, a1 and b0, take RGB values as features
A0 = cell(1,L);
A1 = cell(1,L);
B0 = cell(1,L);
B1 = cell(1,L);
s = cell(1,L);

A0{L} = a0;
A1{L} = a1;
B0{L} = b0;
B1{L} = zeros(nRowsB, nColsB);
s{L} = zeros(nRowsB, nColsB);

for l = L-1:-1:1
    A0{l} = impyramid(A0{l+1}, 'reduce');
    A1{l} = impyramid(A1{l+1}, 'reduce');
    B0{l} = impyramid(B0{l+1}, 'reduce');
    B1{l} = zeros(nRowsB, nColsB);
    s{l} = zeros(nRowsB, nColsB);
end

%% Compute features for A, A , and B


%% Initialize the search structures (e.g., for ANN)


%% Compute main loop

for l=1:L
    for q=1:nPixelsB
        p = bestMatch(A0, A1, B0, B1, s, l, q, k);
        B1{l}(q) = A1{l}(p);
        s{l}(q) = p;
       % B1 =
    end
end
end

