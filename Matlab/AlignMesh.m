function [ outMesh ] = AlignMesh( firstMesh, inMesh, fixedVertices )

% Compute rigid-body transform from inMesh to firstMesh
[R, t] = rigid_transform_3D(inMesh(:, fixedVertices)', firstMesh(:, fixedVertices)');

% Appy transform
outMesh = R * inMesh;
outMesh = bsxfun(@plus, outMesh, t);
end

