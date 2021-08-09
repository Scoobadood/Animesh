function out_image = pixel_filter( in_image, threshold, cam_pos)
    [height, width] = size(in_image);
    padded_in_image = [
        in_image(1,1)  in_image(1,:) in_image(1,end);
        in_image(:,1)  in_image(:,:) in_image(:,end);
        in_image(end,1)  in_image(end,:) in_image(end,end)
        ];
    
    % Compute depth discontinuities in each direction.
    ul = abs(in_image - padded_in_image( 1:end-2, 1:end-2));
    up = abs(in_image - padded_in_image( 2:end-1, 1:end-2));
    ur = abs(in_image - padded_in_image( 3:end,   1:end-2));
     l = abs(in_image - padded_in_image( 1:end-2, 2:end-1));
     r = abs(in_image - padded_in_image( 3:end,   2:end-1));
    dl = abs(in_image - padded_in_image( 1:end-2, 3:end));
    dn = abs(in_image - padded_in_image( 2:end-1, 3:end));
    dr = abs(in_image - padded_in_image( 3:end,   3:end));
    max_discon = max(max( max(ul, up), max(ur, l)), max(max(r, dl), max(dn, dr)));
    keepers = find(max_discon < threshold);
    
    
    % Compute normals
    
    
    ph1_image = zeros(height, width);
    ph1_image(keepers) = in_image(keepers);
    
    
    
end