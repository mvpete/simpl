# @import gui;

let main = create_wnd("Hello Windows", 100, 100, 600, 480);
let btn = create_btn(main, "Click me", 10, 10, 50, 30);
let btn2 = create_btn(main, "Blink", 70,10, 100, 30);
let text = create_text(main, "", );


let clicks = 0;

def do_foo() 
{
    set_text(btn,clicks);
    clicks=clicks+1;
}

on_click(btn, "do_foo");

show(main);


