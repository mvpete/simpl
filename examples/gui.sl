# An example of a simple gui.
@import io
@import gui

let main = create_wnd("Hello Windows", 100, 100, 600, 480);
let btn  = create_btn(main, "Click me", 10, 10, 75, 30);
let text = create_text(main, "Blink", 85,10, 100, 30);

let clicks = 0;

def do_foo() 
{
    let p = get_pos(text);
    set_pos(text, p.x+1, p.y, p.width, p.height);
    set_text(text, clicks);
    clicks=clicks+1;
}

on_click(btn, &do_foo);

## This blocks until the window exits.
show(main);

println("fin");

