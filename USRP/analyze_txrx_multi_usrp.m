clear
load('output/data_rx1.mat')
load('output/data_rx2.mat')
load('output/samplesPerFrameTx.mat')
load('output/repeatTxNum.mat')

startDotNum = 0.09e6+1522+4325-5000+672+80*samplesPerFrameTx;


% find where startDotNum
% figure(1)
% plot(data_rx1(startDotNum:startDotNum+1000,1:2),"o")
figure(2)
plot(data_rx2(startDotNum:startDotNum+500,1:2),"o")


% figure(3)
% plot(abs(data_rx1(startDotNum:startDotNum+1000,1:2)))
% figure(4)
% plot(abs(data_rx2(startDotNum:startDotNum+10000,1:2)))

samplesPerFrameTx = samplesPerFrameTx;

repeatTxNum = repeatTxNum;
data_analyzed1 = zeros(samplesPerFrameTx/repeatTxNum,2);
data_analyzed2 = zeros(samplesPerFrameTx/repeatTxNum,2);
removeData = 2;
for i=1:samplesPerFrameTx/repeatTxNum
    data_range1 = data_rx1(startDotNum+i*repeatTxNum+removeData:startDotNum+(i+1)*repeatTxNum-removeData-1,1:2);
    data_range2 = data_rx2(startDotNum+i*repeatTxNum+removeData:startDotNum+(i+1)*repeatTxNum-removeData-1,1:2);
    data_analyzed1(i,1:2) = mean(data_range1);
    data_analyzed2(i,1:2) = mean(data_range2);
end


% figure(5)
% plot(abs(data_analyzed1))
% figure(6)
% plot(abs(data_analyzed1))

% 
% figure(7)
% plot(data_analyzed1,"o")
figure(8)
plot(data_analyzed2,"o")