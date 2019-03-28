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
fromtan=surface(1:6:end,:);
totan=surface(2:6:end,:);
n1 = surface(3:6:end,:);
fromnorm=surface(4:6:end,:);
tonorm=surface(5:6:end,:);
n2 = surface(6:6:end,:);
% reconstruct as tans and norms
block = [fromtan';totan';n1'];
[r c] = size(block);
block = reshape(block, [3, c*3]);
plot3(block(1, :), block(2, :), block(3, :), '-');    %no marker
%%
% Render normal and tangent data
surface = surface';
plot3(surface(1, :), surface(2, :), surface(3, :), '-');    %no marker


%%
plot3(surf1(1, :), surf1(2, :), surf1(3, :), '-r');    %no marker
plot3(surf2(1, :), surf2(2, :), surf2(3, :), '-g');    %no marker
