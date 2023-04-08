% https://jp.mathworks.com/help/supportpkg/usrpradio/ug/comm.sdrutransmitter-system-object.html
% https://jp.mathworks.com/help/supportpkg/usrpradio/ug/comm.sdrureceiver-system-object.html

%% setup usrp
% first find usrp: findsdru
clear
findsdru

loop = 50;
freq = 6e9;
samplesPerFrameTx = 5e4; % for tx data
samplesPerFrameRx = 5e4; % for rx data
InterpolationFactor = 1;
DecimationFactor = 1;
repeatTxNum = 10; % samplesPerFrameRx/samplesPerFrameTx

rx = comm.SDRuReceiver(...
      'Platform','B210', ...
      'SerialNum','xxxx', ...
      'CenterFrequency',freq, ...
      'MasterClockRate',10e6);
rx.SamplesPerFrame = samplesPerFrameRx;
rx.DecimationFactor = DecimationFactor;
rx.ChannelMapping = 1;
rx.Gain = 30;
disp('this is rx info')
info(rx)

tx = comm.SDRuTransmitter(...
    'Platform','B210', ...
    'SerialNum','xxxx', ...
    'CenterFrequency',freq, ...
    'MasterClockRate',10e6);
tx.ChannelMapping = 1;
tx.Gain = 40;
tx.InterpolationFactor = InterpolationFactor;
disp('this is tx info')
info(tx)

%% make tx data
M = 16;
dataInputRow1 = randi([0 M-1],samplesPerFrameTx/repeatTxNum,1);
dataInputRow2 = randi([0 M-1],samplesPerFrameTx/repeatTxNum,1);
modSignal1 = qammod(dataInputRow1,M,'UnitAveragePower',true);
modSignal2 = qammod(dataInputRow2,M,'UnitAveragePower',true);
repeatModSignal1 = zeros(repeatTxNum*length(modSignal1),1);
repeatModSignal2 = zeros(repeatTxNum*length(modSignal2),1);
for i=1:length(modSignal1)
    for j=1:repeatTxNum
        repeatModSignal1(j+(i-1)*repeatTxNum,1) = modSignal1(i,1);
        repeatModSignal2(j+(i-1)*repeatTxNum,1) = modSignal2(i,1);
    end
end
% data_tx = [repeatModSignal1 repeatModSignal2];
data_tx = repeatModSignal1;


data_rx_per_frame = [];
data_rx = zeros(1,samplesPerFrameRx*loop);


disp('start loop');
%% seemed serially executed, but parallely excuted (https://jp.mathworks.com/matlabcentral/answers/227160-simulink-ettus-usrp-sdru-transmistter-receiver-blocks)
for i=1:loop
    underrun = step(tx,data_tx);
    [data_rx(1,samplesPerFrameRx*(i-1)+1:samplesPerFrameRx*i),data_rx_length,overrun] = step(rx);

    if overrun > 1
        disp('Error: output does not represent contiguous data')
    end
    
    if underrun==1, disp(['Underrun detected',int2str(i)]), end
end
disp('finish loop')

release(tx);
release(rx);

figure(1)
plot(transpose(data_rx),"o")