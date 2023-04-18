% https://jp.mathworks.com/help/matlab/ref/input.html


for i=1:5
    prompt = "num:" + num2str(i) + ", detect key press:\n";
    x = input(prompt);

    for j=1:3
        disp(['start loop: ', num2str(x)])
    end

    prompt = "Do you want more? y/n [y]: ";
    txt = input(prompt,"s");
    if isempty(txt); txt = 'y'; end
    if txt == "n"; break; end
end
