
SIMPL - Simple Intuitive Micro Programming Language
===

A simple language, with not much functionality. More of a tool for learning, than anything else.
simpl has support for comments, variables, assignment, functions, and loops.

Usage
---

You can use SIMPL in your C++ application (requires C++17 or higher) and create bindings.
1) Add SIMPL to your include path
3) `#include <simpl/simpl.h>`
12) Do some other stuff...

Examples
---

 ```
     @import io
    
    ### Comments ###
    # this is a comment
    
    ### Features ###
    
    ## Equations
    print(1 + 3);

    ## Variables
    
    # Declare a variable
    let hw = "hello world!";
    println(hw);
    
    # I have loose typing so you can assign a number to a string, and vice versa
    hw = 4+4;
    println(hw);
    
    # quack-quack
    hw = "goodbye " + 4;
    println(hw);
    
    ## Loops

    # while loops
    let i=10;
    println(i);
    while(i>0)
    {
        println("in a while crocodile");
        i = i-1;
    }
    
    # for loops
    for(let i=0; i<5; ++i)
    {
        println(i);
    }
    
    ## Functions
    
    # I can define a function
    def foo(a,b)  {
        println(a);
        println(b);
        return "foo";
    }
    # I can call a function
    println(foo("test",5));
    
    # I even supports recursion
    def recursive(i)  {
        println(i);
        if(i>0) {
            println("i am recursive");
            recursive(i-1);
        }
    }
    
    recursive(5);

    ## Arrays
    let arr = new ["foo", 1, new {}, 4*4];
    println(arr);

    ## Structures

    let kh = new { name="king", suit="hearts" };
    println(kh);
    kh.name = "queen";
    println(kh.name);

    ## Multi-methods

    def multi_method(a is string, b is string) {
        println("m(string,string)");
    }

    def multi_method(a is string, b is number) {
        println("m(string,number)");
    }

    def multi_method(a is number, b is number) 
    {
        println("m(number,number)");
    }

    let foo = new [ "hello", "world" ];
    let bar = new ["hello", 10];
    let caz = new [13, 42];

    # prints "m(string,string)"
    multi_method(foo...);
    # prints "m(number,string)"
    multi_method(bar...);
    # prints "m(number,number)"
    multi_method(caz...);

 ```

 ### References

 - https://ruslanspivak.com/lsbasi-part18/
 - https://craftinginterpreters.com/
 - https://github.com/ChaiScript
