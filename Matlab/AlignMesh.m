function [ outMesh ] = AlignMesh( firstMesh, inMesh, fixedVertices )

% Transform from inMesh to firstMesh
[R, t] = rigid_transform_3D(inMesh(:, fixedVertices)', firstMesh(:, fixedVertices)');

outMesh = R * inMesh;
outMesh = bsxfun(@plus, outMesh, t);
end

