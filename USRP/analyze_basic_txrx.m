
startDotNum = 1000000+0;


% find where startDotNum
figure(1)
plot(transpose(data_rx(1,startDotNum:startDotNum+1000)),"o")
figure(2)
plot(abs(data_rx(1,startDotNum:startDotNum+1000)))

samplesPerFrameTx = samplesPerFrameTx;
repeatTxNum = repeatTxNum;

%% extract repeated data
data_analyzed = zeros(samplesPerFrameTx/repeatTxNum,1);
removeData = 2;

for i=1:samplesPerFrameTx/repeatTxNum
    data_range = data_rx(1,startDotNum+i*repeatTxNum+removeData:startDotNum+(i+1)*repeatTxNum-removeData-1);
    data_analyzed(i,1) = mean(data_range);
end

figure(3)
plot(transpose(data_analyzed),"o")