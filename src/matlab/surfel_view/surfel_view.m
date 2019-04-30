%%
% Load the data
clc;
clear;
surface = load('/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/reports/test_results/5A_001/st_250K_mat.txt', 'surface');
normscale = .01;
tanscale = .015;

% Start and end of normal
fromnorm=surface(4:6:end,:);
tonorm=surface(5:6:end,:);
delta = tonorm - fromnorm;
deltascaled = delta * normscale;
tonorm = fromnorm + deltascaled;

nn = repmat(sqrt(sum(delta .* delta,2)),1,3);
normnorm = delta ./ nn;

% + and - tans
fromtan=surface(1:6:end,:);
totan=surface(2:6:end,:);
deltatan = (fromtan - totan) / 2.0;
deltatan = deltatan * tanscale;
fromtan = fromnorm - deltatan;
totan = fromnorm + deltatan;

% Orthogonal tans, rotate by 90
% deltatan rotated around normnorm by 90
% vrot = vcos(theta) + (k x v)sin(theta) + k(k.v)(1-cos(theta))
% cos(theta) = 0, sin(theta) = 1
k_cross_v = cross(normnorm, deltatan, 2);
k_dot_v = repmat(dot(normnorm, deltatan, 2), 1, 3);
k_k_dot_v = normnorm .* k_dot_v;
deltatan90 = k_cross_v + k_k_dot_v;

fromtan90 = fromnorm - deltatan90;
totan90 = fromnorm + deltatan90;

n1 = surface(3:6:end,:);
n2 = surface(6:6:end,:);

% reconstruct as tans and norms
block = [fromtan';totan';n1'];
[r c] = size(block);
block = reshape(block, [3, c*3]);

block2 = [fromnorm';tonorm';n2'];
[r c] = size(block2);
block2 = reshape(block2, [3, c*3]);

block3 = [fromtan90';totan90';n2'];
[r c] = size(block3);
block3 = reshape(block3, [3, c*3]);


plot3(block(1, :), block(2, :), block(3, :), '-b');    %no marker
hold
plot3(block2(1, :), block2(2, :), block2(3, :), '-r');    %no marker
plot3(block3(1, :), block3(2, :), block3(3, :), '-b');    %no marker

axis equal
view([3 0 0]);
camroll(90);
