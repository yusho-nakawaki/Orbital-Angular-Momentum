clear

M = 8;
dataNum = 100000;
dataInputRow = randi([0 M-1],dataNum,1);
isPSK = true;
if isPSK
    x = pskmod(dataInputRow,M);
else
    x = qammod(dataInputRow,M);
end



N = 8; % number of antenna elements
L_mode = [0 1 2 3 4 5 6 7];
L = length(L_mode); % number of OAM mode
f = 5.5e9; % wave frequency
lambda = 299792458/f; % wave length

D = 3.5; % distance between anntenas
Rt = sqrt((lambda/2)^2 + lambda*D)/2+0.14; % radius of Tx antenna
Rr = Rt; % radius of Rx antenna
n = linspace(0,N-1,N);
PHItn = 2*pi*n/N; % φ:an antenna element angular of Tx
PHIrn = 2*pi*n/N; % φ:an antenna element angular of Rx
clear n;

snr = 30;

% distance between child antenna
% Each distance is different depending on number antenna
% dn(m,n): m is each Tx antenna, n is each Rx antenna
dn = zeros(N,N);
% mm is for Tx, nn is for Rx
for mm = 1:N
    for nn = 1:N
        dn(mm,nn) = sqrt(Rt^2 + Rr^2 + D^2 - 2*Rt*Rr*cos(PHItn(mm)-PHIrn(nn)));
    end
end

~~~~~~~~~


~~~~~~~~~

Y = zeros(1,dataNum);
for len=0:size(Yl,2)-1
    for row=0: size(Yl,1)-1
        Y(len*(size(Yl,1))+row+1) = Yl(row+1,len+1);
    end
end


figure
plot(Y, 'o')

if isPSK
    dataReceivedRow = transpose(pskdemod(Y,M));
else
    dataReceivedRow = transpose(qamdemod(Y,M));
end


% [errorNumber,errorRation] = biterr(dataInputRow,dataReceivedRow);
% errorRation = errorRation;
% disp(['errorRation: ' num2str(errorRation)])

% clearvars -except yDFT Y x r s Fl FTl
