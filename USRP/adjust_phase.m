%% change data degree from inputing keyboard

loop_adjust = 10;

dataI = [1,1,-1,-1,-1,1, 1,1,-1,-1,-1,1, 1,1,-1,-1,-1,1];
dataQ = [1,-1,-1,1,-1,-1, 1,-1,-1,1,-1,-1, 1,-1,-1,1,-1,-1];
repeatTxAdjustNum = 100;
data_adjust = complex(zeros(1,samplesPerFrameTx),0);

for m=1:fix(samplesPerFrameTx/(length(dataI)*repeatTxAdjustNum))
    for i=1:length(dataI)
        for j=1:repeatTxAdjustNum
            data_adjust(1,j+(i-1)*repeatTxAdjustNum) = dataI(i) + 1j*dataQ(i);
        end
    end
end

data_tx1_adjust = [transpose(data_adjust) transpose(data_adjust)];
data_tx2_adjust = data_tx1_adjust;

data_rx1_adjust = zeros(samplesPerFrameRx*loop_adjust,2);
data_rx2_adjust = zeros(samplesPerFrameRx*loop_adjust,2);

rotate = 0;

for askNum=1:10
    prompt = "num:" + num2str(askNum) + ", input rotate degree: ";
    rotate = input(prompt);
    data_rotated_adjust = exp(1j*2*pi*rotate/360)*data_adjust*0.7;
    data_tx1_adjust_rotate = [transpose(data_rotated_adjust) transpose(data_rotated_adjust)];
    data_tx2_adjust_rotate = data_tx1_adjust_rotate;

    %% transmit & receive
   for i=1:loop_adjust
        underrun1 = step(tx1,data_tx1_adjust_rotate);
        underrun2 = step(tx2,data_tx2_adjust_rotate);
        data_frame1 = step(rx1);
        data_frame2 = step(rx2);
        data_rx1_adjust(samplesPerFrameRx*(i-1)+1:samplesPerFrameRx*i,1:2) = data_frame1;
        data_rx2_adjust(samplesPerFrameRx*(i-1)+1:samplesPerFrameRx*i,1:2) = data_frame2;
        if underrun1==1, disp(['Underrun detected tx1: ',int2str(i)]), end
        if underrun1==2, disp(['Underrun detected tx2: ',int2str(i)]), end
    end 

    figure(1)
    plot(data_tx1_adjust_rotate)
    figure(2)
    plot(data_rx1_adjust)

    prompt = "Do you want more? y/n [y]: ";
    txt = input(prompt,"s");
    if isempty(txt); txt = 'y'; end
    if txt == "n"; break; end

end

% 
% release(tx1);
% release(rx1);
% release(tx2);
% release(rx2);
error % stop running other file calling this file

