### Array Example ###


# You can see that this function is using a variable "names"
# that doesn't exist. Because it doesn't need to yet.
# It will find the first in scope "names" when invoked.
def get_item() {

    print("Enter a item: ");
    let name = getln();

    push(names,name);

    print("Another (y|yes)?: ");
    return getln();
}


let names = new [];
let line = "y";
while(line=="yes" || line=="y")
{
    line = get_item();
}

print(names);