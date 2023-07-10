
clear
% delete(findall(0, 'Type', 'figure'));

N = 4; % number of antenna elements
L_mode = [1];
L = length(L_mode); % number of OAM mode
f = 5.5e9; % wave frequency
lambda = 299792458/f; % wave length
k = 2*pi/lambda;

D = 3.5; % distance between anntenas
Rt = sqrt((lambda/2)^2 + lambda*D)/2; % radius of Tx antenna
Rr = Rt; % radius of Rx antenna
n = linspace(0,N-1,N);
PHItn = 2*pi*n/N; % φ:an antenna element angular of Tx
clear n;

edgeLength = Rt*6;
resolution = 2e-3; 

errorTx_tilt_x = (0/360)*2*pi;
errorTx_tilt_y = (0/360)*2*pi;
errorTx_tilt_z = (0/360)*2*pi;
errorTx_move_x = 0;
errorTx_move_y = 0;
errorTx_move_z = 0;
errorTx_rotate_z = exp(-1j*2*pi*0/360);

snr = 30;
~~~~~~~~~


~~~~~~~~~


r_twoDemention = zeros(length(squreSpaceRx), length(squreSpaceRx));
for i=1: length(squreSpaceRx)
    for j=1: length(squreSpaceRx)
        r_twoDemention(i, j) = r((i-1)*length(squreSpaceRx)+j,1);
    end
end


%% plot phase&amplitude
phase_distribution = angle(r_twoDemention); % normalize phase to be between -pi and pi
cmap = hsv(256*4); % scale phase to be between 1 and 64 (or 1 and the length of your chosen colormap). or use any other colormap you prefer
% display phase with imagesc
figure
imagesc([-edgeLength/2 edgeLength/2], [-edgeLength/2 edgeLength/2], phase_distribution);
title('OAM Beam Phase Distribution')
colormap(cmap); % apply colormap
colorbar;
ax = gca;  % 現在の座標軸を取得
ax.YTickLabel = flip(ax.YTickLabel);  % YTickLabelを反転

PHIrn = 2*pi*linspace(0,N-1,N)/N; % φ:an antenna element angular of Rx
highlight_x = Rr*cos(PHIrn(1:N));
highlight_y = Rr*sin(PHIrn(1:N));
hold on;
plot(highlight_x, highlight_y, 'o', 'MarkerSize', 10, 'LineWidth', 3);
hold off;


amplitude_distribution = abs(r_twoDemention);
figure
imagesc([-edgeLength/2 edgeLength/2], [-edgeLength/2 edgeLength/2], amplitude_distribution);
title('OAM Beam Amplitude Distribution')
colorbar;
ax = gca;  % 現在の座標軸を取得
ax.YTickLabel = flip(ax.YTickLabel);  % YTickLabelを反転

hold on;
plot(highlight_x, highlight_y, 'o', 'MarkerSize', 10, 'LineWidth', 3);
hold off;

%% detect phase difference
n = linspace(0,N-1,N);
PHIrn = 2*pi*n/N; % φ:an antenna element angular of Rx
Rx_antenna_x = Rr*cos(PHIrn(1:N));
Rx_antenna_y = Rr*sin(PHIrn(1:N));
% rounding & add error
Rx_antenna_x = round(Rx_antenna_x/resolution)*resolution + errorTx_move_x;
Rx_antenna_y = round(Rx_antenna_y/resolution)*resolution + errorTx_move_y;

Rx_antenna_x_coordinates = (1/resolution)/2 * (Rx_antenna_x/(edgeLength/2) +1);
Rx_antenna_x_coordinates = round(Rx_antenna_x_coordinates+1); % add +1 because of center "(1/resolution)/2" is added +1
Rx_antenna_y_coordinates = (1/resolution)/2 * (Rx_antenna_y/(edgeLength/2) +1);
Rx_antenna_y_coordinates = round(Rx_antenna_y_coordinates+1);

phase_rx = zeros(1,N);
phase_difference_rx = zeros(1,N);
for i=1:N
    phase_rx(1,i)        = phase_distribution(Rx_antenna_y_coordinates(i),Rx_antenna_x_coordinates(i));
    phase_difference_rx(1,i) = (phase_rx(1,i) - phase_rx(1,1))*360/(2*pi);
end

disp(['phase_different = ',num2str(phase_difference_rx)])
