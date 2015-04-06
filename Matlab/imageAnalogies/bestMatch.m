function p = bestMatch(A0, A1, B0, B1, s, l, q, k)
pApp = bestApproxMatch(A0, A1, B0, B1, l, q);
pCoh = bestCoherentMatch(A0, A1, B0, B1, s, l, q);
dApp = dot(F(pApp) - F(q));
dCoh = dot(F(pCoh) - F(q));
if dCoh <= dApp(1+k*2^(l-L))
    p = pCoh;
else
    p = pApp;
end
end

