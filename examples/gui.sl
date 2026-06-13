# An example of a simple gui.
@import io
@import gui

let main = create_wnd("Hello Windows", 100, 100, 600, 480);
let btn  = create_btn(main, "Copy", 10, 10, 75, 30);
let text = create_text(main, "Output...", 85, 10, 250, 30);
let input = create_edit(main, "Type here", 10, 50, 250, 30);

let clicks = 0;

def sync_output() 
{
    let content = get_text(input);
    set_text(text, content);
}

def on_copy()
{
    sync_output();
    clicks = clicks + 1;
}

def on_input_change()
{
    sync_output();
}

on_click(btn, &on_copy);
on_change(input, &on_input_change);

## This blocks until the window exits.
show(main);

println("copy clicks:");
println(clicks);
