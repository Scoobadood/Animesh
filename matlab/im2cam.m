
function xyz =  im2cam(im, fov)
% u = fx * x' + cx  
% v = fy * y' + cy
% y' = Y/Z
% x' = X/Z
%
% X = ((u-cx)/fx)
% Y = ((v-cy)/fy)
% 
% f_x = (image_width_in_pixels * 0.5) / tan(FOV_x * 0.5 * PI/180)
    [h, w] = size(im);

    f_x = (w * 0.5) / tan(deg2rad(fov(2)) * 0.5);
    f_y = (h * 0.5) / tan(deg2rad(fov(1)) * 0.5);
    
    u = repmat( 1:w, [h,1]);
    v = repmat( [1:h]', [1,w]);
    cx = ones(h,w) * w * 0.5;
    cy = ones(h,w) * h * 0.5;
    
    X = ((u - cx) / f_x) .* im;
    Y = ((v - cy) / f_y) .* im;
    
    xyz = [X(:), Y(:), im(:)];

    % strip entries which are all zero.
    xyz = xyz(logical(xyz(:,3)),:);
end