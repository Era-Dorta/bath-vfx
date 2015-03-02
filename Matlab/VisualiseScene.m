function [ handle ] = VisualiseScene(P1, P2, X)
% Visualise_Scene
handle = figure; hold on; axis equal;

% Decompose cameras
[K1,R1,t1] = Decompose_P(P1);
[K2,R2,t2] = Decompose_P(P2);

% Camera centres
c1 = null(P1); c2 = null(P2);
c1 = c1(1:3)./c1(end); c2 = c2(1:3)./c2(end);

% Plot world points
if P1(:,4) == [0;0;0]; % If P1 = [I|0]
    plot3(X(1,:),X(3,:),-X(2,:),'r*');
    plot3(c1(1),c1(3),-c1(2),'bO');
    plot3(c2(1),c2(3),-c2(2),'bO');
else
    plot3(X(1,:),X(2,:),X(3,:),'r*');
    plot3(c1(1),c1(2),c1(3),'bO');
    plot3(c2(1),c2(2),c2(3),'bO');
end

% Draw cameras
if P1(:,4) == [0;0;0]; % If P1 = [I|0]
    
    s = max(X(:))/20; % Scale
    
    % Camera 1
    sq1 = s*[-1,-1,2]';
    sq2 = s*[-1,1,2]';
    sq3 = s*[1,1,2]';
    sq4 = s*[1,-1,2]';
    sq = [sq1,sq2,sq3,sq4];
    plot3([sq(1,:),sq(1,1)],[sq(3,:),sq(3,1)],-[sq(2,:),sq(2,1)],'r');
    plot3([c1(1),sq1(1)],[c1(3),sq1(3)],-[c1(2),sq1(2)],'r');
    plot3([c1(1),sq2(1)],[c1(3),sq2(3)],-[c1(2),sq2(2)],'r');
    plot3([c1(1),sq3(1)],[c1(3),sq3(3)],-[c1(2),sq3(2)],'r');
    plot3([c1(1),sq4(1)],[c1(3),sq4(3)],-[c1(2),sq4(2)],'r');

    % Camera 2
    R2 = R2';
    sq1 = R2*sq1 - t2;
    sq2 = R2*sq2 - t2;
    sq3 = R2*sq3 - t2;
    sq4 = R2*sq4 - t2;
    sq = [sq1,sq2,sq3,sq4];
    plot3([sq(1,:),sq(1,1)],[sq(3,:),sq(3,1)],-[sq(2,:),sq(2,1)],'r');
    plot3([c2(1),sq1(1)],[c2(3),sq1(3)],-[c2(2),sq1(2)],'r');
    plot3([c2(1),sq2(1)],[c2(3),sq2(3)],-[c2(2),sq2(2)],'r');
    plot3([c2(1),sq3(1)],[c2(3),sq3(3)],-[c2(2),sq3(2)],'r');
    plot3([c2(1),sq4(1)],[c2(3),sq4(3)],-[c2(2),sq4(2)],'r');

else

    s = max(X(:))/5; % Scale
    % Camera 1
    sq1 = s*[-1,-1,2]';
    sq2 = s*[-1,1,2]';
    sq3 = s*[1,1,2]';
    sq4 = s*[1,-1,2]';
    R1 = R1';
    sq1 = R1*sq1 + c1;
    sq2 = R1*sq2 + c1;
    sq3 = R1*sq3 + c1;
    sq4 = R1*sq4 + c1;
    sq = [sq1,sq2,sq3,sq4];
    plot3([sq(1,:),sq(1,1)],[sq(2,:),sq(2,1)],[sq(3,:),sq(3,1)],'r');
    plot3([c1(1),sq1(1)],[c1(2),sq1(2)],[c1(3),sq1(3)],'r');
    plot3([c1(1),sq2(1)],[c1(2),sq2(2)],[c1(3),sq2(3)],'r');
    plot3([c1(1),sq3(1)],[c1(2),sq3(2)],[c1(3),sq3(3)],'r');
    plot3([c1(1),sq4(1)],[c1(2),sq4(2)],[c1(3),sq4(3)],'r');

    % Camera 2
    sq1 = s*[-1,-1,2]';
    sq2 = s*[-1,1,2]';
    sq3 = s*[1,1,2]';
    sq4 = s*[1,-1,2]';
    R2 = R2';
    sq1 = R2*sq1 + c2;
    sq2 = R2*sq2 + c2;
    sq3 = R2*sq3 + c2;
    sq4 = R2*sq4 + c2;
    sq = [sq1,sq2,sq3,sq4];
    plot3([sq(1,:),sq(1,1)],[sq(2,:),sq(2,1)],[sq(3,:),sq(3,1)],'r');
    plot3([c2(1),sq1(1)],[c2(2),sq1(2)],[c2(3),sq1(3)],'r');
    plot3([c2(1),sq2(1)],[c2(2),sq2(2)],[c2(3),sq2(3)],'r');
    plot3([c2(1),sq3(1)],[c2(2),sq3(2)],[c2(3),sq3(3)],'r');
    plot3([c2(1),sq4(1)],[c2(2),sq4(2)],[c2(3),sq4(3)],'r');

end

end

