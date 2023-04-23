% phase coherence will be occurred in MIMO system with b210
% So, we have to adjust phase with this code

% Reference, PPS
% 10 MHz output, Max 3.5Vpp Vpp, set 3V
% 1 PPS output, 25% duty cycl, max 5V, set 2V and 1V offset

%% setup usrp
% first find usrp: findsdru
clear
findsdru

loop = 50;
freq = 5e9;
prefixLength = 500;
samplesPerFrameTx = 0.3e5+prefixLength; % for tx data, +prefixLength for prefix
samplesPerFrameRx = 0.3e5+prefixLength; % for rx data
InterpolationFactor = 4;
DecimationFactor = 4;
repeatTxNum = 10; % samplesPerFrameRx/samplesPerFrameTx
clockRate = 5e6;


rx1 = comm.SDRuReceiver(...
      'Platform','B210', ...
      'SerialNum','3275259', ...
      'CenterFrequency',freq, ...
      'MasterClockRate',clockRate);
rx1.SamplesPerFrame = samplesPerFrameRx;
rx1.DecimationFactor = DecimationFactor;
rx1.ChannelMapping = 1;
rx1.PPSSource = "External";
rx1.ClockSource = "External";
rx1.ChannelMapping = [1 2];
rx1.Gain = [30 30];
disp('this is rx1 info')
info(rx1)

tx1 = comm.SDRuTransmitter(...
    'Platform','B210', ...
    'SerialNum','3275259', ...
    'CenterFrequency',freq, ...
    'MasterClockRate',clockRate);
tx1.ChannelMapping = [1 2];
tx1.Gain = [40 40];
tx1.PPSSource = "External";
tx1.ClockSource = "External";
tx1.InterpolationFactor = InterpolationFactor;
disp('this is tx1 info')
info(tx1)

rx2 = comm.SDRuReceiver(...
      'Platform','B210', ...
      'SerialNum','3275312', ...
      'CenterFrequency',freq, ...
      'MasterClockRate',clockRate);
rx2.SamplesPerFrame = samplesPerFrameRx;
rx2.DecimationFactor = DecimationFactor;
rx2.ChannelMapping = 1;
rx2.PPSSource = "External";
rx2.ClockSource = "External";
rx2.ChannelMapping = [1 2];
rx2.Gain = [30 30];
% rx.Gain = 30;
disp('this is rx2 info')
info(rx2)

tx2 = comm.SDRuTransmitter(...
    'Platform','B210', ...
    'SerialNum','3275312', ...
    'CenterFrequency',freq, ...
    'MasterClockRate',clockRate);
tx2.ChannelMapping = [1 2];
tx2.Gain = [40 40];
% tx.ChannelMapping = 1;
% tx.Gain = 40;
tx2.PPSSource = "External";
tx2.ClockSource = "External";
tx2.InterpolationFactor = InterpolationFactor;
disp('this is tx2 info')
info(tx2)


%% make tx data
% make prefix QPSK flag 50bits
% make sure length(prefix) and prefixLength are same
marginA = zeros(1,7);
marginB = zeros(1,25);
dataI = [marginA, 1,1,-1,-1,-1,1, 1,1,-1,-1,-1,1, 1,1,-1,-1,-1,1, marginB];
dataQ = [marginA, 1,-1,-1,1,-1,-1, 1,-1,-1,1,-1,-1, 1,-1,-1,1,-1,-1, marginB];
data_prefix = zeros(1,length(dataI)*repeatTxNum);
for i=1:length(dataI)
    for j=1:repeatTxNum
        data_prefix(1,j+(i-1)*repeatTxNum) = dataI(i) + 1j*dataQ(i);
    end
end

% make row data
M = 16;
dataInputRow1 = randi([0 M-1],(samplesPerFrameTx-prefixLength)/repeatTxNum,1);
dataInputRow2 = randi([0 M-1],(samplesPerFrameTx-prefixLength)/repeatTxNum,1);
modSignal1 = qammod(dataInputRow1,M,'UnitAveragePower',true);
modSignal2 = qammod(dataInputRow2,M,'UnitAveragePower',true);
repeatedModSignal1 = zeros(1,repeatTxNum*length(modSignal1));
repeatedModSignal2 = zeros(1,repeatTxNum*length(modSignal2));
for i=1:length(modSignal1)
    for j=1:repeatTxNum
        repeatedModSignal1(1,j+(i-1)*repeatTxNum) = modSignal1(i,1);
        repeatedModSignal2(1,j+(i-1)*repeatTxNum) = modSignal2(i,1);
    end
end

% join prefix and row data
data_tx_1board_Achannel = [data_prefix repeatedModSignal1];
data_tx_1board_Bchannel = [data_prefix repeatedModSignal2];
data_tx_2board_Achannel = [data_prefix repeatedModSignal1];
data_tx_2board_Bchannel = [data_prefix repeatedModSignal2];

% data_tx1 = [data_tx_1board_Achannel data_tx_1board_Bchannel];
% data_tx2 = [data_tx_2board_Achannel data_tx_2board_Bchannel];
data_tx1 = [transpose(data_tx_1board_Achannel) transpose(data_tx_1board_Achannel)];
data_tx2 = data_tx1;

data_rx1 = zeros(samplesPerFrameRx*loop,2);
data_rx2 = zeros(samplesPerFrameRx*loop,2);


%% resolve phase coherence
disp('adjust phase');
adjust_phase
disp('finished adjusting');


disp('start loop');

for i=1:loop
    underrun1 = step(tx1,data_tx1);
    underrun2 = step(tx2,data_tx2);
    data_frame1 = step(rx1);
    data_frame2 = step(rx2);
%     [data_frame1,data_rx_length1,overrun1] = step(rx1);
%     [data_frame2,data_rx_length2,overrun2] = step(rx2);
    
    data_rx1(samplesPerFrameRx*(i-1)+1:samplesPerFrameRx*i,1:2) = data_frame1;
    data_rx2(samplesPerFrameRx*(i-1)+1:samplesPerFrameRx*i,1:2) = data_frame2;

%     if overrun1 > 1, disp('Overrun rx1: output does not represent contiguous data'), end
%     if overrun2 > 1, disp('Overrun rx2: output does not represent contiguous data'), end
    if underrun1==1, disp(['Underrun detected tx1: ',int2str(i)]), end
    if underrun1==2, disp(['Underrun detected tx2: ',int2str(i)]), end
end

disp('finish loop')


release(tx1);
release(rx1);
release(tx2);
release(rx2);


% data_rx = zeros(1,samplesPerFrame*loop);
% for i=1:loop
%     data_rx(1,samplesPerFrame*(i-1)+1:samplesPerFrame*i) = data_rx_cell{i};
% end

% figure(1)
% plot(data_rx1,"o")
figure(2)
plot(abs(data_rx1))
figure(3)
plot(abs(data_rx2))

save('output/data_rx1.mat','data_rx1')
save('output/data_rx2.mat','data_rx2')
save('output/samplesPerFrameTx.mat', 'samplesPerFrameTx')
save('output/repeatTxNum.mat', 'repeatTxNum')

