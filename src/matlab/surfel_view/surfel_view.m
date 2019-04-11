surface = [0 1 1 NaN 1.5 2.5 2.5 ; ...
           0 1 0 NaN 0.5 0.5 1.5 ; ...
           0 1 1 NaN 0.5 0.5 0.5 ];
save('surf.txt', 'surface', '-ascii')

%%
% Load the data
clc;
clear;
surface = load('/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/build/mat.txt', 'surface');
%%
scale = 0.005;

% Start and end of normal
fromnorm=surface(4:6:end,:);
tonorm=surface(5:6:end,:);
delta = tonorm - fromnorm;
delta = delta * scale;
tonorm = fromnorm + delta;

% + and - tans
fromtan=surface(1:6:end,:);
totan=surface(2:6:end,:);
deltatan = (fromtan - totan) / 2.0;
deltatan = deltatan * scale;

fromtan = fromnorm - deltatan;
totan = fromnorm + deltatan;

n1 = surface(3:6:end,:);
n2 = surface(6:6:end,:);

% reconstruct as tans and norms
% block = [fromtan';totan';n1'];
% [r c] = size(block);
% block = reshape(block, [3, c*3]);
% plot3(block(1, :), block(2, :), block(3, :), '-b');    %no marker

block2 = [fromnorm';tonorm';n2'];
[r c] = size(block2);
block2 = reshape(block2, [3, c*3]);
%hold
plot3(block2(1, :), block2(2, :), block2(3, :), '-r');    %no marker


%%
% Render normal and tangent data
m = 10;
plot3(block(1, 1:m), block(2, 1:m), block(3, 1:m), '-');    %no marker
hold, plot3(block2(1, 1:m), block2(2, 1:m), block2(3, 1:m), '-r');    %no marker


%%
plot3(surf1(1, :), surf1(2, :), surf1(3, :), '-r');    %no marker
plot3(surf2(1, :), surf2(2, :), surf2(3, :), '-g');    %no marker
%%

norm([0.29933, 0, 0.95415])
