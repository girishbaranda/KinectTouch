%Function from Bouguet's camera calibration toolbox.
% Modified by Daniel Herrera C. - Adapted to the Kinect Calibration Toolbox
function [x,X,n_sq_x,n_sq_y,ind_orig,ind_x,ind_y] = extract_grid(I,wintx,winty,fc,cc,kc,dX,dY,xr,yr,click_mode);

if nargin < 11,
    click_mode = 0;
end;


if nargin < 10,
    xr = [];
    yr = [];
end;


map = gray(256);

minI = min(I(:));
maxI = max(I(:));

Id = 255*(I - minI)/(maxI - minI);

figure(1);
imshow(Id,[]);
colormap(map);

if ~isempty(xr);
    figure(1);
    hold on;
    plot(xr,yr,'go');
    hold off;
end;


if nargin < 2,
    
    disp('Window size for corner finder (wintx and winty):');
    wintx = input('wintx ([] = 5) = ');
    if isempty(wintx), wintx = 5; end;
    wintx = round(wintx);
    winty = input('winty ([] = 5) = ');
    if isempty(winty), winty = 5; end;
    winty = round(winty);
    
    fprintf(1,'Window size = %dx%d\n',2*wintx+1,2*winty+1);
    
end;


need_to_click = 1;

color_line = 'g';

while need_to_click,
    
    
    title('Click on the four extreme corners of the rectangular pattern (first corner = origin)...');
    
    disp('Click on the four extreme corners of the rectangular complete pattern (the first clicked corner is the origin)...');
    
    x= [];y = [];
    figure(1); hold on;
    for count = 1:4,
        [xi,yi] = ginput(1);
        [xxi] = cornerfinder([xi;yi],I,winty,wintx);
        xi = xxi(1);
        yi = xxi(2);
        figure(1);
        plot(xi,yi,'+','color',[ 1.000 0.314 0.510 ],'linewidth',2);
        plot(xi + [wintx+.5 -(wintx+.5) -(wintx+.5) wintx+.5 wintx+.5],yi + [winty+.5 winty+.5 -(winty+.5) -(winty+.5)  winty+.5],'-','color',[ 1.000 0.314 0.510 ],'linewidth',2);
        x = [x;xi];
        y = [y;yi];
        plot(x,y,'-','color',[ 1.000 0.314 0.510 ],'linewidth',2);
        drawnow;
    end;
    plot([x;x(1)],[y;y(1)],'-','color',[ 1.000 0.314 0.510 ],'linewidth',2);
    drawnow;
    hold off;
    
    [Xc,good,bad,type] = cornerfinder([x';y'],I,winty,wintx); % the four corners
    
    x = Xc(1,:)';
    y = Xc(2,:)';
    
    
    % Sort the corners:
    x_mean = mean(x);
    y_mean = mean(y);
    x_v = x - x_mean;
    y_v = y - y_mean;
    
    theta = atan2(-y_v,x_v);
    [junk,ind] = sort(theta);
    
    [junk,ind] = sort(mod(theta-theta(1),2*pi));
    
    %ind = ind([2 3 4 1]);
    
    ind = ind([4 3 2 1]); %-> New: the Z axis is pointing uppward
    
    
    x = x(ind);
    y = y(ind);
    x1= x(1); x2 = x(2); x3 = x(3); x4 = x(4);
    y1= y(1); y2 = y(2); y3 = y(3); y4 = y(4);
    
    
    % Find center:
    p_center = cross(cross([x1;y1;1],[x3;y3;1]),cross([x2;y2;1],[x4;y4;1]));
    x5 = p_center(1)/p_center(3);
    y5 = p_center(2)/p_center(3);
    
    % center on the X axis:
    x6 = (x3 + x4)/2;
    y6 = (y3 + y4)/2;
    
    % center on the Y axis:
    x7 = (x1 + x4)/2;
    y7 = (y1 + y4)/2;
    
    % Direction of displacement for the X axis:
    vX = [x6-x5;y6-y5];
    vX = vX / norm(vX);
    
    % Direction of displacement for the X axis:
    vY = [x7-x5;y7-y5];
    vY = vY / norm(vY);
    
    % Direction of diagonal:
    vO = [x4 - x5; y4 - y5];
    vO = vO / norm(vO);
    
    delta = 30;
    
    
    figure(1); imshow(Id,[]);
    colormap(map);
    hold on;
    plot([x;x(1)],[y;y(1)],'g-');
    plot(x,y,'og');
    hx=text(x6 + delta * vX(1) ,y6 + delta*vX(2),'X');
    set(hx,'color','g','Fontsize',14);
    hy=text(x7 + delta*vY(1), y7 + delta*vY(2),'Y');
    set(hy,'color','g','Fontsize',14);
    hO=text(x4 + delta * vO(1) ,y4 + delta*vO(2),'O','color','g','Fontsize',14);
    hold off;
    
    
    % Try to automatically count the number of squares in the grid
    
    n_sq_x1 = count_squares(I,x1,y1,x2,y2,wintx);
    n_sq_x2 = count_squares(I,x3,y3,x4,y4,wintx);
    n_sq_y1 = count_squares(I,x2,y2,x3,y3,wintx);
    n_sq_y2 = count_squares(I,x4,y4,x1,y1,wintx);
    
    
    
    % If could not count the number of squares, enter manually
    
    if (n_sq_x1~=n_sq_x2)|(n_sq_y1~=n_sq_y2),
        
        if ~click_mode,
            
            % This way, the user manually enters the number of squares and no more clicks.
            % Otherwise, he user is asked to click again.
            
            disp('Could not count the number of squares in the grid. Enter manually.');
            n_sq_x = input('Number of squares along the X direction ([]=10) = '); %6
            if isempty(n_sq_x), n_sq_x = 10; end;
            n_sq_y = input('Number of squares along the Y direction ([]=10) = '); %6
            if isempty(n_sq_y), n_sq_y = 10; end; 
            need_to_click = 0;
            
        end;
        
        
    else
        
        n_sq_x = n_sq_x1;
        n_sq_y = n_sq_y1;
        
        need_to_click = 0;
        
    end;
    
    color_line = 'r';
    
end;


if ~exist('dX')|~exist('dY'),
    
    % Enter the size of each square
    
    dX = input(['Size dX of each square along the X direction ([]=30mm) = ']);
    dY = input(['Size dY of each square along the Y direction ([]=30mm) = ']);
    if isempty(dX), dX = 30; end;
    if isempty(dY), dY = 30; end;
    
end;


% Compute the inside points through computation of the planar homography (collineation)

a00 = [x(1);y(1);1];
a10 = [x(2);y(2);1];
a11 = [x(3);y(3);1];
a01 = [x(4);y(4);1];


% Compute the planar collineation: (return the normalization matrice as well)

[Homo,Hnorm,inv_Hnorm] = compute_homography ([a00 a10 a11 a01],[0 1 1 0;0 0 1 1;1 1 1 1]);


% Build the grid using the planar collineation:

x_l = ((0:n_sq_x)'*ones(1,n_sq_y+1))/n_sq_x;
y_l = (ones(n_sq_x+1,1)*(0:n_sq_y))/n_sq_y;
pts = [x_l(:) y_l(:) ones((n_sq_x+1)*(n_sq_y+1),1)]';

XX = Homo*pts;
XX = XX(1:2,:) ./ (ones(2,1)*XX(3,:));


% Complete size of the rectangle

W = n_sq_x*dX;
L = n_sq_y*dY;



if nargin < 6,
    
    %%%%%%%%%%%%%%%%%%%%%%%% ADDITIONAL STUFF IN THE CASE OF HIGHLY DISTORTED IMAGES %%%%%%%%%%%%%
    figure(1);
    hold on;
    plot(XX(1,:),XX(2,:),'r+');
    title('The red crosses should be close to the image corners');
    hold off;
    
    disp('If the guessed grid corners (red crosses on the image) are not close to the actual corners,');
    disp('it is necessary to enter an initial guess for the radial distortion factor kc (useful for subpixel detection)');
    quest_distort = input('Need of an initial guess for distortion? ([]=no, other=yes) ');
    
    quest_distort = ~isempty(quest_distort);
    
    if quest_distort,
        % Estimation of focal length:
        c_g = [size(I,2);size(I,1)]/2 + .5;
        f_g = Distor2Calib(0,[[x(1) x(2) x(4) x(3)] - c_g(1);[y(1) y(2) y(4) y(3)] - c_g(2)],1,1,4,W,L,[-W/2 W/2 W/2 -W/2;L/2 L/2 -L/2 -L/2; 0 0 0 0],100,1,1);
        f_g = mean(f_g);
        script_fit_distortion;
    end;
    %%%%%%%%%%%%%%%%%%%%% END ADDITIONAL STUFF IN THE CASE OF HIGHLY DISTORTED IMAGES %%%%%%%%%%%%%
    
else
    
    %xy_corners_undist = comp_distortion_oulu([(x' - cc(1))/fc(1);(y'-cc(2))/fc(2)],kc);
    xy_corners_undist = undistort([(x' - cc(1))/fc(1);(y'-cc(2))/fc(2)],kc);
    
    xu = xy_corners_undist(1,:)';
    yu = xy_corners_undist(2,:)';
    
    [XXu] = projectedGrid ( [xu(1);yu(1)], [xu(2);yu(2)],[xu(3);yu(3)], [xu(4);yu(4)],n_sq_x+1,n_sq_y+1); % The full grid
    
    r2 = sum(XXu.^2);       
    XX = (ones(2,1)*(1 + kc(1) * r2 + kc(2) * (r2.^2))) .* XXu;
    XX(1,:) = fc(1)*XX(1,:)+cc(1);
    XX(2,:) = fc(2)*XX(2,:)+cc(2);
    
end;


Np = (n_sq_x+1)*(n_sq_y+1);

disp('Corner extraction...');

grid_pts = cornerfinder(XX,I,winty,wintx); %%% Finds the exact corners at every points!

grid_pts = grid_pts - 1; % subtract 1 to bring the origin to (0,0) instead of (1,1) in matlab (not necessary in C)

ind_corners = [1 n_sq_x+1 (n_sq_x+1)*n_sq_y+1 (n_sq_x+1)*(n_sq_y+1)]; % index of the 4 corners
ind_orig = (n_sq_x+1)*n_sq_y + 1;
xorig = grid_pts(1,ind_orig);
yorig = grid_pts(2,ind_orig);
dxpos = mean([grid_pts(:,ind_orig) grid_pts(:,ind_orig+1)]');
dypos = mean([grid_pts(:,ind_orig) grid_pts(:,ind_orig-n_sq_x-1)]');


ind_x = (n_sq_x+1)*(n_sq_y + 1);
ind_y = 1;

x_box_kk = [grid_pts(1,:)-(wintx+.5);grid_pts(1,:)+(wintx+.5);grid_pts(1,:)+(wintx+.5);grid_pts(1,:)-(wintx+.5);grid_pts(1,:)-(wintx+.5)];
y_box_kk = [grid_pts(2,:)-(winty+.5);grid_pts(2,:)-(winty+.5);grid_pts(2,:)+(winty+.5);grid_pts(2,:)+(winty+.5);grid_pts(2,:)-(winty+.5)];




figure(3);
imshow(Id,[]); colormap(map); hold on;
plot(grid_pts(1,:)+1,grid_pts(2,:)+1,'r+');
plot(x_box_kk+1,y_box_kk+1,'-b');
plot(grid_pts(1,ind_corners)+1,grid_pts(2,ind_corners)+1,'mo');
plot(xorig+1,yorig+1,'*m');
h = text(xorig+delta*vO(1),yorig+delta*vO(2),'O');
set(h,'Color','m','FontSize',14);
h2 = text(dxpos(1)+delta*vX(1),dxpos(2)+delta*vX(2),'dX');
set(h2,'Color','g','FontSize',14);
h3 = text(dypos(1)+delta*vY(1),dypos(2)+delta*vY(2),'dY');
set(h3,'Color','g','FontSize',14);
xlabel('Xc (in camera frame)');
ylabel('Yc (in camera frame)');
title('Extracted corners');
zoom on;
drawnow;
hold off;

min_x = min(grid_pts(1,:))-6;
max_x = max(grid_pts(1,:))+6;
min_y = min(grid_pts(2,:))-6;
max_y = max(grid_pts(2,:))+6;
axis([min_x max_x min_y max_y]);




Xi = reshape(([0:n_sq_x]*dX)'*ones(1,n_sq_y+1),Np,1)';
Yi = reshape(ones(n_sq_x+1,1)*[n_sq_y:-1:0]*dY,Np,1)';
Zi = zeros(1,Np);

Xgrid = [Xi;Yi;Zi];


% All the point coordinates (on the image, and in 3D) - for global optimization:

x = grid_pts;
X = Xgrid;

