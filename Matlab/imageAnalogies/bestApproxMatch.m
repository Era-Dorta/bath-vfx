function pApp = bestApproxMatch(A0, A1, B0, B1, l, q)
L = size(A0,2);
sizeBl = size(B0{l});

[i,j] = ind2sub(sizeBl, q);

% Neighbour square in l
minIl = i - 2;
maxIl = i + 2;

if minIl <= 0
    minIl = 1;
end
if maxIl > sizeBl(1)
    maxIl = sizeBl(1);
end

minJl = j - 2;
maxJl = j + 2;

if minJl <= 0
    minJl = 1;
end
if maxJl > sizeBl(2)
    maxJl = sizeBl(2);
end

if(l == 1)
    
else
    sizeBl1 = size(B0{l - 1});
    % Neighbour square in l - 1
    minIl1 = i / 2 - 1;
    maxIl1 = i / 2 + 1;
    
    if minIl1 <= 0
        minIl1 = 1;
    end
    if maxIl1 > sizeBl1(1)
        maxIl1 = sizeBl1(1);
    end
    
    minJl1 = j / 2 - 1;
    maxJl1 = j / 2 + 1;
    
    if minJl <= 0
        minJl = 1;
    end
    if maxJl > sizeB(2)
        maxJl = sizeB(2);
    end
end
end

