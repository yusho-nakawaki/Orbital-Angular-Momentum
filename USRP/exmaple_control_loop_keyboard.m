% https://jp.mathworks.com/help/matlab/ref/input.html

%% basic fuction of inputing keyboard
% for i=1:5
%     prompt = "num:" + num2str(i) + ", detect key press:\n";
%     x = input(prompt);
% 
%     for j=1:3
%         disp(['start loop: ', num2str(x)])
%     end
% 
%     prompt = "Do you want more? y/n [y]: ";
%     txt = input(prompt,"s");
%     if isempty(txt); txt = 'y'; end
%     if txt == "n"; break; end
% end

%% change data degree from inputing keyboard
dataI = [1,1,-1,-1,-1,1, 1,1,-1,-1,-1,1, 1,1,-1,-1,-1,1];
dataQ = [1,-1,-1,1,-1,-1, 1,-1,-1,1,-1,-1, 1,-1,-1,1,-1,-1];
repeatTxNum = 10;
data_prefix = zeros(1,length(dataI)*repeatTxNum);

for i=1:length(dataI)
    for j=1:repeatTxNum
        data_prefix(1,j+(i-1)*repeatTxNum) = dataI(i) + 1j*dataQ(i);
    end
end

rotate = 0;

for askNum=1:5
    prompt = "num:" + num2str(askNum) + ", input rotate degree: ";
    rotate = input(prompt);
    data_rotated_prefix = exp(-1j*rotate*2*pi/360)*data_prefix;

    figure(1)
    plot(transpose(0.9*data_prefix))
    figure(2)
    plot(transpose(0.9*data_rotated_prefix))

    prompt = "Do you want more? y/n [y]: ";
    txt = input(prompt,"s");
    if isempty(txt); txt = 'y'; end
    if txt == "n"; break; end
end

