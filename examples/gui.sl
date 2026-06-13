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

def on_ui_event(source is button)
{
    sync_output();
    clicks = clicks + 1;
}

def on_ui_event(source is edit)
{
    sync_output();
}

on_click(btn, &on_ui_event);
on_change(input, &on_ui_event);

## This blocks until the window exits.
show(main);

println("copy clicks:");
println(clicks);
