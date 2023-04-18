% https://jp.mathworks.com/help/matlab/ref/input.html

prompt = "detect key press: \n";
x = input(prompt)
y = x*10

prompt = "Do you want more? Y/N [Y]: ";
txt = input(prompt,"s");
if isempty(txt)
    txt = 'Y';
end